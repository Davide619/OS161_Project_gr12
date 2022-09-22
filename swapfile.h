////////////////////////////////////////////////////////////
//
// swap
//

/*
 * swapfile operations in swap.c:
 *
 * swap_bootstrap:   bootstraps the swapfile and completes VM-system 
 *                   bootstrap. Declared in vm.h.
 *
 * swap_shutdown:    closes the swapfile vnode. Declared in vm.h.
 * 
 * swap_alloc:       finds a free swap page and marks it as used.
 *                   A page should have been previously reserved.
 *
 * swap_free:        unmarks a swap page.
 *
 * swap_pagein:      Reads a page from the requested swap address 
 *                   into the requested physical page.
 *
 * swap_pageout:     Writes a page to the requested swap address 
 *                   from the requested physical page.
 */

off_t	 	swap_alloc(paddr_t pa_mem);
void 		swap_free(off_t swapaddr);

void 		swap_pagein(struct addrspace *as, paddr_t pa_mem, off_t swapaddr);
void 		swap_pageout(struct addrspace *as, paddr_t pa_mem, off_t swapaddr);

/*
 * Special disk address:
 *   INVALID_SWAPADDR is an invalid swap address.
 */
#define INVALID_SWAPADDR	(0)
#define SWAP_SIZE 9437184 /*Byte      9M*1024B*1024B*/


//extern struct lock *global_paging_lock;

////////////////////////////////////////////////////////////