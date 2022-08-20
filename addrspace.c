/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
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
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>
#include <spinlock.h>

#define DUMBVM_STACKPAGES    18  

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */
       /*lo stack ha un numero di pagine fisse*/
	   
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
static struct spinlock freemem_lock = SPINLOCK_INITIALIZER;
static int nRamFrames = 0;                 
static unsigned char *freeRamFrames = NULL;





struct addrspace *as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}

	/*
	 * Initialize as needed.

	as->as_vbase1 = 0;
	as->as_pbase1 = 0;
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_pbase2 = 0;
	as->as_npages2 = 0;
	as->as_stackpbase = 0;

	 */

	return as;
}

int as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this
	 */
 	//Copio i campi della struttura newas in old
	 newas->as_vbase1=old->as_vbase1;
	 newas->as_npages1=old->as_npages1;
	 newas->as_vbase2=old->as_vbase2;
	 newas->as_npages2=old->as_npages2;

	KASSERT(newas->as_pbase1 != 0);
	KASSERT(newas->as_pbase2 != 0);
	KASSERT(newas->as_stackpbase != 0);

	(void)old;

	*ret = newas;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */

	kfree(as);
}

void as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	/*
	 * Write this.
	 */
}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
}
/*Funzione che controlla se c'è spazio libero in memoria attraverso il freeframe*/
static int getfreeppages(vaddr_t vaddr) 
{
        paddr_t addr;
        long i, found;
        int flag;
        //if (!isTableActive()) return 0;
        spinlock_acquire(&freemem_lock);
        
        // Ricerco il primo frame libero
        for (i=0; i<nRamFrames; i++) 
        {
                flag = 0;
                if (freeRamFrames[i] == 0) /*se il freeframe è 0 allora è libero*/
				{      
                    found = i; 
                    flag = 1;
                    freeRamFrames[i] == (unsigned char)1;   /*aggiorno il vettore*/
                    break; //esce dal for
                }
        }

        if (flag==1) addr = (paddr_t) found*PAGE_SIZE + (vaddr - MIPS_KSEG0);    /*Calcolo la posizione della prima page 
																				libera in memoria fisica partendo 
																				dal frame virtuale*/
        
        else  addr = 0;/*se il freeRamFrames è tutto occupato cado in questa sezione e setto l'indirizzo a zero*/
         
        spinlock_release(&freemem_lock);

        return addr;
}

/*Funzione per allocare delle pagine fisiche in memoria:
Anzicchè passargli npages non passo niente in quanto voglio che mi allochi 
una pagina per volta ogni volta che tale funzione viene richiamata*/

static paddr_t getppage(vaddr_t vaddr) 
{
	paddr_t addr;
    /*controllo se ho pagine libere attraverso il vettore freeRamFrames,gestito all'interno della funzione getfreeppages()*/
	addr = getfreeppages(vaddr); //ritorna la posizione della prima pagina fisica libera in memoria 
    if (addr != 0)
    {       
        /* se ho spazio in memoria (un frame libero) allora alloco la pagina in RAM*/
        spinlock_acquire(&stealmem_lock);
        addr = ram_stealmem(addr);  /*questa funzione alloca nella ram un singolo frame partendo dall'indirizzo desiderato
									quindi non gli passo npages*/
        spinlock_release(&stealmem_lock);
    }

    if (addr == 0)
    { 
        /*se non ho spazio in memoria allora utilizzo un algoritmo di replacement*/
        spinlock_acquire(&freemem_lock);
                



        /*IMPLEMENTARE UN ALGORITMO DI REPLACEMENT O RICHIAMRE ALGORITMO DI REPLACEMENT LASCIANDO 
        IL FREEFRAME OCCUPATO*/




    spinlock_release(&freemem_lock);
    }

    return addr;   /*mi ritorna l'indirizzo fisico dove ho allocato la pagina*/
}    

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,int readable, int writeable, int executable)
{
	//----------------------- Write this.
	size_t npages;
	paddr_t paddr;
    //dumbvm_can_sleep();

    /* Align the region. First, the base... */
    memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
    
	//paddr = vaddr & PAGE_FRAME; //<----modified

    /* ...and now the length. */
    memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;

    npages = memsize / PAGE_SIZE;
		
	//-----------------------------------------------

	/* We don't use these - all pages are read-write */

	(void)readable;
	(void)writeable;
	(void)executable;

	//-----------------------------------------------

	if (as->as_vbase1 == 0) {
                as->as_vbase1 = vaddr;
                as->as_npages1 = npages;                                
                return 0;
        }
        if (as->as_vbase2 == 0) {
                as->as_vbase2 = vaddr;
                as->as_npages2 = npages;
                return 0;
        }
	

	return ENOSYS;
}

/*   Alloca data segment e code segment*/ 
int as_prepare_load(struct addrspace *as)
{
	//--------------------------------------Write this.
	KASSERT(as->as_pbase1 != 0); // è corretto controllare se siano zero? Perchè dovrebbero esserlo per forza
								 //visto che non sono stati toccati dopo as_create dentro runprogram che è
								 //la prima funzione che dichiara una struct addrspace as
    KASSERT(as->as_pbase2 != 0);
    KASSERT(as->as_stackpbase != 0);
	KASSERT(as->as_vbase1 == 0);
	KASSERT(as->as_vbase1 == 0);
    //dumbvm_can_sleep();

    as->as_pbase1 = getppage(as->as_vbase1);
    if (as->as_pbase1 == 0) {
    return ENOMEM;  
    }

    as->as_pbase2 = getppage(as->as_vbase2);
    if (as->as_pbase2 == 0) {
        return ENOMEM;
    }

    as->as_stackpbase = getppage(/*firstaddr+DUMBVM_STACKPAGES*/);/*Gli passo l'indirizzo del primo frame nello stack*/
    if (as->as_stackpbase == 0) {
        return ENOMEM;
    }

    /*
	Non capisco perchè azzerare le porzioni di memoria se le ho appena allocate
	as_zero_region(as->as_pbase1, as->as_npages1);
    as_zero_region(as->as_pbase2, as->as_npages2);
    as_zero_region(as->as_stackpbase, DUMBVM_STACKPAGES);
	*/

    return 0;
	
}

int as_complete_load(struct addrspace *as) //Controllo se dopo il load dei segmenti l'address space sia corretto
{
	/*
	 * Write this.
	 */
	KASSERT(as->as_pbase1 == 0);
	KASSERT(as->as_npages1 == 0);
    KASSERT(as->as_pbase2 == 0);
	KASSERT(as->as_npages2 == 0);
    KASSERT(as->as_stackpbase == 0);
	KASSERT(as->as_vbase1 == 0);
	KASSERT(as->as_vbase1 == 0);
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;

	return 0;
}

