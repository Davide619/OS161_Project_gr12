#ifndef _PT_H_
#define _PT_H_

#include <types.h>
#include <lib.h>
#include <vm.h>

/*Define*/

#define FRAME_NUMBER 0xfffff000

/* functions to alloc/dealloc a page table */
paddr_t * pt_create(size_t nentries);
void pt_destroy(paddr_t * pt_dest);

/* functions to Manage page table and replacement*/ 
paddr_t victim_sel(paddr_t *pt,uint8_t *entry_valid,/*,uint8_t nvalidentries,*/uint8_t index); //<--added
void pt_update(paddr_t *pt,uint8_t *entry_valid,paddr_t new_frame,uint8_t index,uint8_t pt_index,uint8_t dirty_bit,uint8_t valid_bit);



#endif /* _PT_H_ */
