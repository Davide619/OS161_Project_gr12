#include "segments.h"


int which_segment(struct addrspace *as, vaddr_t faultaddress){
    /*In questi if controllo se l'informazione che ricevo dall'indirizzo virtuale fa parte del code, data o stack*/
    if ((faultaddress >= (as->code_seg_start)) && (faultaddress < ((as->code_seg_size -1)+(as->code_seg_start))))
    {       
        /*code*/
		return CODE_SEG;
    }
    else if ((faultaddress >= (as->data_seg_start)) && (faultaddress < ((as->data_seg_size -1)+(as->data_seg_start))))
    {
        /*data*/
	    return DATA_SEG;
    }
        			
    else  return ERR_SEG; /*Ritorno un errore qualora tale indirizzo non fa parte di nessuno di questi casi*/
}



/*OffsetFromELF function*/
off_t
offset_fromELF(vaddr_t segment_start, vaddr_t faultaddress, size_t segment_size, off_t segment_offset){

        off_t ret_offset;
        /* Compute the segment relative page */
        unsigned int npage_from_start = ((faultaddress & PAGE_FRAME) - (segment_start & PAGE_FRAME))/PAGE_SIZE;
        
        
        if(npage_from_start == 0){
                /* in case we are in the first page of the segment*/
                ret_offset = segment_offset;
                
        }else if(((faultaddress - segment_start)&PAGE_FRAME) > segment_size){
                /* in case we are beyond the size of segment. This means that i will have zeros in that part of elf-File*/
                ret_offset = segment_offset + segment_size;
                
        }else{
                /* intermidiate page */
                ret_offset = segment_offset + (PAGE_SIZE - segment_offset % PAGE_SIZE) + (npage_from_start-1)*PAGE_SIZE;
                
        }
        
        return ret_offset;

}
