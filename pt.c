#include <types.h>
#include <pt.h>
#include <stdio.h>
#include <stdlib.h>
#include <kern/errno.h>
#include <lib.h>



struct pgtblstruct **pgtblfunct(paddr_t lastpaddr,paddr_t firstpaddr,unsigned long npages)
{
		firstpaddrpgtbl = lastpaddr - NUM_TLB; //Decido che il primo indirizzo di memoria dedicato
		//alla page table sia 64 indirizzi prima(NUM_TLB) dell'ultimo indirizzo disponibile massimo per i frame fisici.
		
		lastpaddrpgtbl = lastpaddr; //Mentre l'ultimo indirizzo corrisponde esattamente all'ultimo indirizzo possibile allocato per i frame.

		//if (firstpaddrpagetable + size > lastpaddr)  return 0; //Se il primo indirizzo della page table allocata
		//nello spazio dei frame è maggiore dell'ultimo indirizzo possibile della memoria allocata per i frame
		//esce dalla funzione(DA VEDERE SE SI DEVE INVECE CHIAMARE KASSERT O GENERARE QUALCHE ERRORE)
        
		//Adesso associo ad ogni indirizzo di memoria della page table una struttura:
		//Così ho delle strutture per ogni indirizzo per cui so che ogni indirizzo della memoria fisica
		//per la page table a quale indice appartiene e di conseguenza posso risalire facilmente alle entry
		//perchè so tutti gli indirizzi.

		struct pgtblstruct pgtblinfo[NUM_TLB];

		for(int i=0;i<NUM_TLB;i++)
		{
			pgtblinfo[i].pgtbl_entry_address = firstpaddrpgtbl+i;
			pgtblinfo[i].valid_invalid_indx = 0; //Decido di mettere a 0 inizialmente tutti i valid-invalid
			//bit per stabilire se il frame sia in memoria o no
			//Valid address -->valid_invalid bit = 1
			//Invalid address -->valid_invalid bit = 0
		}

		for(int i=0;i<npages;i++)
		{
			if(i <= NUM_TLB) //Perchè il numero di pagine potrebbe essere maggiore del numero di entry della TLB 
			pgtblinfo[i].pgtbl_entry_content = firstpaddr +(i*PAGE_SIZE); //Riempio la page table con tutti 
			//i primi indirizzi di ogni frame in memoria
		}

        return pgtblinfo;

}
