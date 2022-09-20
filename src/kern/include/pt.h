#ifndef _PT_H_
#define _PT_H_

#include <types.h>

/* data structure representing a page table */
struct pt
{
    paddr_t frame;
    bool valid;
};

/* functions to alloc/dealloc a page table */
struct pt * pt_create(size_t nentries);
void pt_destroy(struct pt * pt_dest);

#endif /* _PT_H_ */