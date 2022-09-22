#include <mips/tlb.h>
#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <clock.h>




/*FUNZIONE CHE AGGIUNGE UNA NEW ENTRY ALLA TLB*/
int tlb_insert(paddr_t paddr, vaddr_t faultaddress){        /*paddr is the entrypoint from load_elf function*/
    
    int spl, victim;
    uint32_t elo;
    vaddr_t entrypoint;

    spl = splhigh();

// Look for an invalid entry on the TLB
    for (i=0; i<NUM_TLB; i++) {
        TLB_Read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            continue;
        }
        ehi = faultaddress;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;                //vedere in questo caso come settare il dirty bit
        DEBUG(DB_VM, "TLB Added: 0x%x -> 0x%x at location %d\n", faultaddress, paddr, i);
        TLB_Write(ehi, elo, i);
        splx(spl); // Leave that to calling function
        return 0;
    }

//if i am here, i have no TLB space, so i use replace algorithm

    ehi = faultaddress;
    elo = paddr | TLBLO_DIRTY | TLBLO_VALID; 

    victim = tlb_get_rr_victim();
    TLB_Write(ehi, elo, victim);

    return 1; //returns 1 if replace algorithm is executed 
}



/*FIFO TLB Algorithm*/
int tlb_get_rr_victim(void){
	
	int victim;
	static unsigned int next_victim =0;
	victim = (next_victim +1) % NUM_TLB;
	return victim;

}



/*invalidate entry function*/
int TLB_Invalidate(paddr_t paddr)
{
    uint32_t ehi,elo,i;

    for (i=0; i<NUM_TLB; i++) 
    {
        TLB_Read(&ehi, &elo, i);
        if ((elo & TLBLO_PPAGE) == (paddr & TLBLO_PPAGE)) 
        {
            TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);        
        }
    }

    return 0;
}
