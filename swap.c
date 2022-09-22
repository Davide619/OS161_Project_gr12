#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/stat.h>
#include <lib.h>
#include <uio.h>
#include <bitmap.h>
#include <synch.h>
#include <thread.h>
#include <current.h>
#include <mainbus.h>
#include <addrspace.h>
#include <vm.h>
#include <swapfile.h>
#include <machine/coremap.h>
#include <vfs.h>
#include <vnode.h>


static const char swapfilename[] = "SWAPFILE";


static struct bitmap *swapmap;	// swap allocation map
static struct lock *swaplock;	// synchronizes swapmap and counters


static unsigned long swap_total_pages;				/*total pages number*/
static unsigned long swap_free_pages;				/*free pages in swap file*/

static unsigned long index_paddr =0;  


static struct vnode *swapstore;	// swap file


//struct lock *global_paging_lock;


/*
 * swap_bootstrap
 */

typedef struct st_t{
	paddr_t *addr[SWAP_SIZE / PAGE_SIZE];
	off_t offset_swapfile[SWAP_SIZE / PAGE_SIZE];
}swappage_trace;


void
swap_bootstrap(void)
{
	int rv;
	struct stat st;
	char path[sizeof(swapfilename)];
	off_t filesize; 							
	
	kmalloc(sizeof(swappage_trace));   /*non so se devo farlo*/

	strcpy(path, swapfilename);
	rv = vfs_open(path, O_RDWR, 0, &swapstore); 				//file name <path>
	if (rv) {
		kprintf("swap: Error %d opening swapfile %s\n", rv, 
			swapfilename);
		kprintf("swap: Please create swapfile/swapdisk.\n");
		panic("swap: Unable to continue.\n");
	}


	VOP_STAT(swapstore, &st);	/*info of the created file*/
	

	if (st.st_size < SWAP_SIZE) {							/*st.st_size is the size of created file*/									
		kprintf("swap: swapfile %s is only %lu bytes.\n", swapfilename,
			(unsigned long) st.st_size);
		kprintf("swap: Please extend it.\n");
		kprintf("swap: Extended!\n");
	}else{
		kprintf("swap: swapfile %s is %lu bytes.\n", swapfilename,
			(unsigned long) SWAP_SIZE);
		kprintf("swap: i am adjusting it...\n");
		kprintf("swap: Adjusted!\n");
	}

	kprintf("swap: Total number of possible loaded pages into the file: %lu\n",
		(unsigned long) SWAP_SIZE / PAGE_SIZE);

	swap_total_pages = SWAP_SIZE / PAGE_SIZE;				
	swap_free_pages = swap_total_pages;

	swapmap = bitmap_create(swap_total_pages);					/*map of pages in swapfile*/
	DEBUG(DB_VM, "creating swap map with %lld entries\n",
			swap_total_pages);
	if (swapmap == NULL) {
		panic("swap: No memory for swap bitmap\n");
	}

	swaplock = lock_create("swaplock");
	if (swaplock == NULL) {
		panic("swap: No memory for swap lock\n");
	}

}

/*
 * swap_shutdown
 *
 * Destroys data structures and closes the swap vnode.
 */
void
swap_shutdown(void)
{
	lock_destroy(swaplock);
	bitmap_destroy(swapmap);
	vfs_close(swapstore);
}

/*
 * swap_alloc: allocates a page in the swapfile.
 *
 * Synchronization: uses swaplock.
 */


off_t
swap_alloc(paddr_t pa_mem)	
{
	uint32_t rv, index;
	swappage_trace *p;
	
	lock_acquire(swaplock);

	KASSERT(swap_free_pages <= swap_total_pages);

	
	KASSERT(swap_free_pages>0);

	rv = bitmap_alloc(swapmap, &index);					/*it finds the free bit, sets it and return its index*/
	KASSERT(rv == 0);

	p->addr[index_paddr] = pa_mem;
	p->offset_swapfile[index_paddr] = index*PAGE_SIZE;

	index_paddr++;

	swap_free_pages--;

	lock_release(swaplock);

	return index*PAGE_SIZE;		/*address allocation of the page*/
}


/*
 * swap_io: Does one swap I/O. Panics on failure.
 *
 * Synchronization: none specifically. The physical page should be
 * marked "pinned" (locked) so it won't be touched by other people.
 */
static
void											/*pa_mem is the phisical address of the frame in memory*/
swap_io(struct addrspace *as, paddr_t pa_mem, off_t swapaddr, enum uio_rw rw)		/*swap in and swap out function*/
{
	struct iovec iov;
	struct uio myuio;
	vaddr_t va;
	int result;

	iov.iov_ubase = (userptr_t)pa_mem;//vaddr;	
        iov.iov_len = PAGE_SIZE;           // length of the memory space
        myuio.uio_iov = &iov;
        myuio.uio_iovcnt = 1;
        myuio.uio_resid = PAGE_SIZE;          // amount to read from the file
        myuio.uio_offset = swapaddr;		
        myuio.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE;
        myuio.uio_rw = rw;
        myuio.uio_space = as;						/*DA CAPIRE MEGLIO*/


	KASSERT(pa_mem != INVALID_PADDR);
	KASSERT(swapaddr % PAGE_SIZE == 0);
	KASSERT(bitmap_isset(swapmap, swapaddr / PAGE_SIZE));		/*check if the bit is setted*/


	uio_kinit(&iov, &myuio, (char *)va, PAGE_SIZE, swapaddr, rw);
	if (rw==UIO_READ) {
		result = VOP_READ(swapstore, &myuio); //read from swap file
	
	}
	else {

		result = VOP_WRITE(swapstore, &myuio);
	}


	if (result==EIO) {								/*read error */
		panic("swap: EIO on swapfile (offset %ld)\n",
		      (long)swapaddr);
	}
	else if (result==EINVAL) {					/*illegal attemption reading from a wrong offset*/
		panic("swap: EINVAL from swapfile (offset %ld)\n",
		      (long)swapaddr);
	}
	else if (result) {						/*generic error of write/read*/
		panic("swap: Error %d from swapfile (offset %ld)\n",
		      result, (long)swapaddr);
	}
}

/*
 * swap_pagein: load one page from swap into physical memory.
 * Synchronization: none here. See swap_io().
 */
void
swap_pagein(struct addrspace *as, paddr_t pa_mem, off_t swapaddr)
{
	swap_io(as,pa_mem, swapaddr, UIO_READ);
}


/* 
 * swap_pageout: write one page from physical memory into swap.
 * Synchronization: none here. See swap_io().
 */
void
swap_pageout(struct addrspace *as, paddr_t pa_mem, off_t swapaddr)
{
	swap_io(as,pa_mem, swapaddr, UIO_WRITE);
}
