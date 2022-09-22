#ifndef _PT_H_
#define _PT_H_

#include <types.h>
#include <lib.h>
#include <vm.h>

/* functions to alloc/dealloc a page table */
paddr_t * pt_create(size_t nentries);
void pt_destroy(paddr_t * pt_dest);

#endif /* _PT_H_ */