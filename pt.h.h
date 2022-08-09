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

struct pgtblstruct **pgtblfunct(paddr_t lastpaddr,paddr_t firstpaddr,unsigned long npages); //Questa funzione crea
//e ritorna la struttura dati completa necessaria a gestire tutta la page table
