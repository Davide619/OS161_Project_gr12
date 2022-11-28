#ifndef _SEGMENTS_H_
#define _SEGMENTS_H_

#include <elf.h>
#include <vnode.h>

/*Macros*/
/*return values of "which_segment" function*/
#define DATA_SEG 0 
#define CODE_SEG 1
#define ERR_SEG 2

/*function that checks from which segment faultaddress comes*/
int which_segment(struct addrspace *as, vaddr_t faultaddress);

/*Computes the offset from elf file*/
off_t offset_fromELF(vaddr_t segment_start, vaddr_t faultaddress, size_t segment_size, off_t segment_offset);

#endif /* _SEGMENTS_H_ */
