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
#include <spl.h>
#include <cpu.h>
#include <spinlock.h>
#include <proc.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <unistd.h>

#define DUMBVM_STACKPAGES    18         /*lo stack ha un numero di pagine fisse*/

static int nRamFrames = 0;                    //<---added     /*GLOBAL*/
static unsigned char *freeRamFrames = NULL;   //<---added


/*
 * Wrap ram_stealmem in a spinlock.
 */
static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER; //<---added
static struct spinlock freemem_lock = SPINLOCK_INITIALIZER; //<--- added


void vm_tlbshootdown(const struct tlbshootdown *ts)
{
        (void)ts;
        panic("dumbvm tried to do tlb shootdown?!\n");
}

void vm_bootstrap(void)         /*ADDED*/
{
        /*Computation of available free frames number in RAM*/
        int nRamFrames = ((int)ram_getsize())/PAGE_SIZE;
        *stack = kmalloc(sizeof(int)*nRamFrames);
        /*alloco il vettore freeRamFrame inizializzato a 0 (vuoto)*/
        freeRamFrames = kmalloc(sizeof(unsigned char)*nRamFrames);

        /*if (freeRamFrames==NULL || allocSize==NULL)
        {
                //reset to disable this vm management 
                freeRamFrames = allocSize = NULL; 
                return;
        }*/
        for (int i=0; i<nRamFrames; i++) 
                freeRamFrames[i] = (unsigned char)0;
        
        /*spinlock_acquire(&freemem_lock);
        allocTableActive = 1;
        spinlock_release(&freemem_lock);*/
}

static void dumbvm_can_sleep(void)
{
        if (CURCPU_EXISTS()) {
                /* must not hold spinlocks */
                KASSERT(curcpu->c_spinlocks == 0); 
                /*KASSERT se l'espressione nelle parentesi è falsa allora chiama panic e termina il programma*/

                /* must not be in an interrupt handler */
                KASSERT(curthread->t_in_interrupt == 0);
        }
}

/*Funzione per allocare delle pagine fisiche in memoria*/
static paddr_t getppages(unsigned long npages) 
{
        paddr_t addr;

        spinlock_acquire(&stealmem_lock);
        addr = ram_stealmem(npages);         
        spinlock_release(&stealmem_lock);
        

        return addr;     //mi ritorna l'indirizzo fisico dove ho allocato la prima pagina
}


/*Funzione che controlla se c'è spazio libero */
static paddr_t getfreeppages(void) 
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
                if (freeRamFrames[i] == 0)   //se il freeframe è 0 allora è libero
                {       found = i; 
                        flag = 1;
                        freeRamFrames[i] == (unsigned char)1;   //aggiorno il vettore
                        break; //esce dal for
                }
        }

        if (flag==1) addr = (paddr_t) found*PAGE_SIZE;    //Setto la variabile addr alla dimensione di una page se trovo frame liberi
        
        else  addr = 0;//se il freeRamFrames è tutto occupato cado in questa sezione e setto l'indirizzo a zero
         
        spinlock_release(&freemem_lock);

        return addr;
}






/*Funzione che dealloca spazio in memoria: è solo un'interfaccia per freeRamFrames*/
static int freeppages(paddr_t addr)
{
        long i, first;

        //if (!isTableActive()) return 0;

        first = addr/PAGE_SIZE;
        //KASSERT(allocSize!=NULL);
        KASSERT(nRamFrames>first);

        spinlock_acquire(&freemem_lock);
 
        freeRamFrames[first] = (unsigned char)0;      //aggiorno il vettore
  
        spinlock_release(&freemem_lock);
        
        return 1; 
}



/* Allocate/free some kernel-space virtual pages*/
vaddr_t alloc_kpages(unsigned npages)                           /*MODIFIED*/
{
        paddr_t pa;

        dumbvm_can_sleep();
        pa = getppages(npages);
        if (pa==0) {
                return 0;
        }
        return PADDR_TO_KVADDR(pa);    /*ritorna l'indirizzo virtuale (della pagina) del kernel a cui 
                                        corrisponde l'indirizzo fisico allocato in cui è allocata 
                                        la pagina. Ovviamente il caso in cui l'indirizzo sia 0 questo non
                                        esiste all'interno del kernel e quindi deve essere isolato*/                 
}


void free_kpages(vaddr_t addr)                       /*ADDED*/                                           
{
        //if (isTableActive()) {
        paddr_t paddr = addr - MIPS_KSEG0;     /*converto l'indirizzo logico in indirizzo fisico utilizzando la maschera*/
        long first = paddr/PAGE_SIZE;           /*Converto l’indirizzo fisico in indice d’inizio del frame*/
        KASSERT(nRamFrames>first);
        //freeppages(paddr);    
        //}

        (void)addr;
}



/*TLB MISS HANDLER: 
Quando si controlla se il campo p di un certo virtual(logical) address
è presente tra le entry della TLB e non c'è si ha un TLB MISS. La vm_fault gestisce questa 
situazione.Usa le informazioni dall'address space del processo corrente per costruire e caricare
la entry necessaria nella TLB.
Questa funzione non tiene conto del momento in cui la TLB viene riempita tutta,quando accade bisogna 
gestire la cosa sennò OS161 CRASHA*/  
int vm_fault(int faulttype, vaddr_t faultaddress)                        
{
        vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
        paddr_t paddr;
        int i;
        uint32_t ehi, elo;
        struct addrspace *as;
        int spl;
        

        faultaddress &= PAGE_FRAME; //Tramite la maschera con la PAGE_FRAME(=0xfffff000) estraggo il page number,che è l'address che non è stato
        //trovato nelle entry della TLB. 
        
        DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);

        switch (faulttype) {
            case VM_FAULT_READONLY: 
                panic("dumbvm: got VM_FAULT_READONLY\n");    
            case VM_FAULT_READ:                                 
            case VM_FAULT_WRITE:                                            
                break;
            default:
                return EINVAL;                                                         
        }                                        

        if (curproc == NULL) {
                /*
                 * No process. This is probably a kernel fault early
                 * in boot. Return EFAULT so as to panic instead of
                 * getting into an infinite faulting loop.
                 */
                return EFAULT;
        }

        as = proc_getas(); //Acquisisco l'address space dell'attuale processo che dopo andrò a controllare con le KASSERT
        if (as == NULL) {
                /*
                 * No address space set up. This is probably also a
                 * kernel fault early in boot.
                 */
                return EFAULT; 
        }

        /* Assert that the address space has been set up properly. */
        
        KASSERT((as->as_vbase1 != 0) & ((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1)); // <---modified(solo reso compatto)
        KASSERT((as->as_pbase1 != 0) & ((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1));
        KASSERT(as->as_npages1 != 0);
        KASSERT((as->as_vbase2 != 0) & ((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2));
        KASSERT((as->as_pbase2 != 0) & ((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2));
        KASSERT(as->as_npages2 != 0);
        KASSERT((as->as_stackpbase != 0) & ((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase));

        vbase1 = as->as_vbase1;
        vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
        vbase2 = as->as_vbase2;
        vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
        stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
        stacktop = USERSTACK;

        /*In questi if controllo se l'informazione che ricevo dall'indirizzo virtuale fa parte del code, data o stack*/

        if (faultaddress >= vbase1 && faultaddress < vtop1)
        {       
                paddr = (faultaddress - vbase1) + as->as_pbase1; /*code*/

        }
        else if (faultaddress >= vbase2 && faultaddress < vtop2) {
                paddr = (faultaddress - vbase2) + as->as_pbase2; /*data*/
        }
        else if (faultaddress >= stackbase && faultaddress < stacktop) {
                paddr = (faultaddress - stackbase) + as->as_stackpbase; /*stack*/
        }
        else  return EFAULT; /*Ritorno un errore qualora tale indirizzo non fa parte di nessuno di questi casi*/
        

        /* make sure it's page-aligned */
        KASSERT((paddr & PAGE_FRAME) == paddr);


        /* Disable interrupts on this CPU while frobbing the TLB. */
        spl = splhigh();/*disabilita l'interrupt per evitare che mentre modifico la tlb si genera un 
        interrupt e non finisco l'azione*/


        for (i=0; i<NUM_TLB; i++) 
        {
                tlb_read(&ehi, &elo, i);

                if (elo & TLBLO_VALID) //Controllo se la entry che sto leggendo della TLB è valida
                {
                      continue;  
                }
                ehi = faultaddress;
                elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
                DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
                tlb_write(ehi, elo, i);
                splx(spl);
                return 0;
                
        }
        

        kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");

        splx(spl); //Riabilito interrupt 
        
        return EFAULT;
}


struct addrspace * as_create(void)
{
        struct addrspace *as = kmalloc(sizeof(struct addrspace));
        if (as==NULL) {
                return NULL;
        }

        as->as_vbase1 = 0;
        as->as_pbase1 = 0;
        as->as_npages1 = 0;
        as->as_vbase2 = 0;
        as->as_pbase2 = 0;
        as->as_npages2 = 0;
        as->as_stackpbase = 0;

        return as;
}

void
as_destroy(struct addrspace *as)                /*<ADDED*/
{
        dumbvm_can_sleep();
        freeppages(as->as_pbase1);
        freeppages(as->as_pbase2);
        freeppages(as->as_stackpbase);
        kfree(as);
}

 /*Questa funzione va a invalidare le voci della TLB quando è necessario uno switch cioè quando si deve togliere 
un processo dalla memoria e portarlo su disco e rimpiazzare quello spazio con il nuovo processo preso da disco. 
Ogni volta che la CPU sta gestendo un process nella TLB è rimasta la traduzione di quell'indirizzo, quindi questa funzione
va a pulire quel campo e far spazio ad altri*/
void as_activate(void)
{
        int i, spl;
        struct addrspace *as;

        as = proc_getas();
        if (as == NULL) {
                return;
        }

        /* Disable interrupts on this CPU while frobbing the TLB. */
        spl = splhigh();

        for (i=0; i<NUM_TLB; i++) {
                tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i); /*<------ Invalida tutte le voci della TLB*/
        }

        splx(spl);
}

void as_deactivate(void)
{
        /* nothing */
}

int as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,int readable, int writeable, int executable)             
{
        size_t npages;

        dumbvm_can_sleep();

        /* Align the region. First, the base... */
        sz += vaddr & ~(vaddr_t)PAGE_FRAME;
        vaddr &= PAGE_FRAME;

        /* ...and now the length. */
        sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

        npages = sz / PAGE_SIZE;

        /* We don't use these - all pages are read-write */
        (void)readable;
        (void)writeable;
        (void)executable;

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

        /*
         * Support for more than two regions is not available.
         */
        //kprintf("dumbvm: Warning: too many regions\n");
        return ENOSYS;                                                                  /*capire dove vado a finire con questo errore*/
}

static void as_zero_region(paddr_t paddr, unsigned npages)
{
        bzero((void *)PADDR_TO_KVADDR(paddr), npages * PAGE_SIZE);
}




/*Funzione importante che prepara lo spazio di lavoro                
Poiche in loaelf.c definiamo le regioni rispettivamente di sola lettura per code ecc... se noi volessimo 
però caricare quella regione ad esempio all'inizio, non possiamo scriverci. Queste due funzioni 
(as_prepare_load, as_complete_load) che sono richiamate dal kernel automaticamente, risolvono il problema. 
La prima funzione resetta tutti i permessi delle regioni come Writeble, ma hai bisogno anche di fare un backup 
di quali erano i permessi delle regioni prima di questo evento. A tal proposito quando viene richiamata 
as_complete_load allora tutti i permessi vengono rimpostati come all'inizio*/
int as_prepare_load(struct addrspace *as)                                  
{
        //Kassert termina il programma solo se la condizione nella parentesi è falsa
        KASSERT(as->as_pbase1 == 0);
        KASSERT(as->as_pbase2 == 0);
        KASSERT(as->as_stackpbase == 0);

        dumbvm_can_sleep();

        as->as_pbase1 = getppages(as->as_npages1);
        if (as->as_pbase1 == 0) {
                return ENOMEM;          //capire dove vado a finire con questo errore
        }

        as->as_pbase2 = getppages(as->as_npages2);
        if (as->as_pbase2 == 0) {
                return ENOMEM;
        }

        as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
        if (as->as_stackpbase == 0) {
                return ENOMEM;
        }

        as_zero_region(as->as_pbase1, as->as_npages1);
        as_zero_region(as->as_pbase2, as->as_npages2);
        as_zero_region(as->as_stackpbase, DUMBVM_STACKPAGES);

        return 0;
}

int as_complete_load(struct addrspace *as)
{
        dumbvm_can_sleep();
        (void)as;
        return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
        KASSERT(as->as_stackpbase != 0);

        *stackptr = USERSTACK;
        return 0;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
        struct addrspace *new;

        dumbvm_can_sleep();

        new = as_create();

        if (new==NULL) {
                return ENOMEM;
        }

        new->as_vbase1 = old->as_vbase1;
        new->as_npages1 = old->as_npages1;
        new->as_vbase2 = old->as_vbase2;
        new->as_npages2 = old->as_npages2;

        /* (Mis)use as_prepare_load to allocate some physical memory. */
        if (as_prepare_load(new)) {
                as_destroy(new);
                return ENOMEM;
        }

        KASSERT(new->as_pbase1 != 0);
        KASSERT(new->as_pbase2 != 0);
        KASSERT(new->as_stackpbase != 0);

        memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
                (const void *)PADDR_TO_KVADDR(old->as_pbase1),
                old->as_npages1*PAGE_SIZE);

        memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
                (const void *)PADDR_TO_KVADDR(old->as_pbase2),
                old->as_npages2*PAGE_SIZE);

        memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
                (const void *)PADDR_TO_KVADDR(old->as_stackpbase),
                DUMBVM_STACKPAGES*PAGE_SIZE);

        *ret = new;
        return 0;
}
