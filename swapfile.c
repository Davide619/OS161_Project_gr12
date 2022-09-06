/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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


static const char swapfilename[] = "lhd0raw:";


static struct bitmap *swapmap;	// swap allocation map
static struct lock *swaplock;	// synchronizes swapmap and counters


static unsigned long swap_total_pages;				/*total pages number*/
static unsigned long swap_free_pages;				/*its contents give us information about free pages into the swapfile*/


static struct vnode *swapstore;	// swap file


static unsigned long index_paddr =0;  /*variable to keep trace of phisical address vector of swappage_trace*/

//struct lock *global_paging_lock;


/*
 * swap_bootstrap
 */

typedef struct st_t{
	p_addr *addr[SWAP_SIZE / PAGE_SIZE];
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
	rv = vfs_open(path, O_RDWR, 0, &swapstore); 				//it creates a file named <path>
	if (rv) {
		kprintf("swap: Error %d opening swapfile %s\n", rv, 
			swapfilename);
		kprintf("swap: Please create swapfile/swapdisk.\n");
		panic("swap: Unable to continue.\n");
	}


	VOP_STAT(swapstore, &st);		/*info about the created file*/
	

	if (st.st_size < SWAP_SIZE) {						/*st.st_size is the size of the created swapfile*/									
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

	swap_total_pages = SWAP_SIZE / PAGE_SIZE;				/*total pages number available into swapfile*/
	swap_free_pages = swap_total_pages;

	swapmap = bitmap_create(swap_total_pages);					/*it creates a map of the file useful for searching pages*/
	DEBUG(DB_VM, "creating swap map with %lld entries\n",
			swap_total_pages);
	if (swapmap == NULL) {
		panic("swap: No memory for swap bitmap\n");
	}

	swaplock = lock_create("swaplock");
	if (swaplock == NULL) {
		panic("swap: No memory for swap lock\n");
	}

	/* mark the first page of swap used so we can check for errors */
	//bitmap_mark(swapmap, 0);						/*Marks the page 0. i don't know if it is necessary*/
	//swap_free_pages--;
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

/*this function is called every time that a new page into the swapfile must be added*/
off_t
swap_alloc(paddr_t pa_mem)		/*it allocates a page on the map vector*/
{
	uint32_t rv, index;
	swappage_trace *p;
	
	lock_acquire(swaplock);

	/*check*/
	KASSERT(swap_free_pages <= swap_total_pages);
	KASSERT(swap_reserved_pages <= swap_free_pages);

	KASSERT(swap_reserved_pages>0);
	KASSERT(swap_free_pages>0);

	rv = bitmap_alloc(swapmap, &index);					/*it finds the free bit and set it.*/
	KASSERT(rv == 0);

	/*initialization of the structure that keeps data information trace of swapfile*/
	p->addr[index_paddr] = pa_mem;
	p->offset_swapfile[index_paddr] = index*PAGE_SIZE;

	swap_free_pages--;

	lock_release(swaplock);

	return index*PAGE_SIZE;		/*page allocation address*/
}


/*
 * swap_free: marks a page in the swapfile as unused.
 *
 * Synchronization: uses swaplock.
 */
void							/*i need it every time POP operation from the swapfile must be done*/
swap_free(off_t swapaddr)		/*swapaddr is the address of the page into the swapfile*/
{
	uint32_t index;

	KASSERT(swapaddr != INVALID_SWAPADDR);
	KASSERT(swapaddr % PAGE_SIZE == 0);

	index = swapaddr / PAGE_SIZE;				/*da capire se è un numero intero o con virgola*/ 

	lock_acquire(swaplock);

	KASSERT(swap_free_pages < swap_total_pages);
	KASSERT(swap_reserved_pages <= swap_free_pages);

	KASSERT(bitmap_isset(swapmap, index));
	bitmap_unmark(swapmap, index);					/*I'm going to meet 0 in correspondence of index for		
							mark a free page. the swap file does not free it
							but I overwrite it when necessary*/
	swap_free_pages++;

	lock_release(swaplock);
}

/*
 * swap_io: Does one swap I/O. Panics on failure.
 *
 * Synchronization: none specifically. The physical page should be
 * marked "pinned" (locked) so it won't be touched by other people.
 */
static
void																				/*pa_mem is the starting address of the frame in memory*/
swap_io(struct addrspace *as, paddr_t pa_mem, off_t swapaddr, enum uio_rw rw)		/*function responsible for both swapIN and swapOUT depending on the flag passed as third parameter (UIO_READ or UIO_WRITE)*/
{
	struct iovec iov;
	struct uio myuio;
	vaddr_t va;
	int result;

	iov.iov_ubase = (userptr_t)pa_mem;//vaddr;	/*starting address in memory of the frame extending from vaddr up to vaddr + memsize*/
        iov.iov_len = PAGE_SIZE;           // length of the memory space
        myuio.uio_iov = &iov;
        myuio.uio_iovcnt = 1;
        myuio.uio_resid = PAGE_SIZE;          // amount to read from the file
        myuio.uio_offset = swapaddr;		//where my page is on the file (starting address) and has resid length (PAGE_SIZE)
        myuio.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE;
        myuio.uio_rw = rw;
        myuio.uio_space = as;						/*DA CAPIRE MEGLIO*/


	KASSERT(pa_mem != INVALID_PADDR);
	KASSERT(swapaddr % PAGE_SIZE == 0);
	KASSERT(bitmap_isset(swapmap, swapaddr / PAGE_SIZE));		/*check if actually the bit of the vector that corresponds to the index of the page I am looking for in the swap is actually full and therefore the page present in the swap file*/


	uio_kinit(&iov, &myuio, (char *)va, PAGE_SIZE, swapaddr, rw);
	if (rw==UIO_READ) {
		result = VOP_READ(swapstore, &myuio); //I read from the swap file and put in memory
	
	}
	else {

		result = VOP_WRITE(swapstore, &myuio);
	}


	if (result==EIO) {										/*reading error */
		panic("swap: EIO on swapfile (offset %ld)\n",
		      (long)swapaddr);
	}
	else if (result==EINVAL) {								/*attempting to read from an incorrect, illegal offset*/
		panic("swap: EINVAL from swapfile (offset %ld)\n",
		      (long)swapaddr);
	}
	else if (result) {										/*generic read / write error*/
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
