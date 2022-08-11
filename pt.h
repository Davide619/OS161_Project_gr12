#include <types.h>
#include <vm.h>
#include <tlb.h>
#include <stdio.h>
#include <stdlib.h>

/*GLOBAL VARIABLES DEFINITION*/

struct pgtblstruct
{
	paddr_t pgtbl_entry_address;
	paddr_t pgtbl_entry_content;
	short int valid_invalid_indx;
};

paddr_t firstpaddrpgtbl; 
paddr_t lastpaddrpgtbl;


/*FUNCTIONS DEFINITION*/

struct pgtblstruct *pgtblgen(paddr_t lastpaddr,paddr_t firstpaddr,unsigned long npages); //Questa funzione genera
//e ritorna la struttura dati completa necessaria a gestire tutta la page table

struct pgtblstruct *getactualpagetable();
