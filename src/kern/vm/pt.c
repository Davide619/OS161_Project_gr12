#include <pt.h>

static uint8_t index;

paddr_t * pt_create(size_t nentries)
{
    paddr_t * new_pt = kmalloc(sizeof(paddr_t) * nentries);
    if(new_pt == NULL) return NULL;

    return new_pt;
}

void pt_destroy(paddr_t * pt_dest)
{
    kfree(pt_dest);

    return;
}

/*Questa funzione,tramite index seleziona una vittima tra le entry della page table da rimpiazzare 
secondo un algoritmo di replacement*/
paddr_t get_victim(paddr_t *pt,uint8_t *entry_valid)
{
        return pt[entry_valid[index]];
}

/*Questa funzione,aggiorna una entry della page table e contemporaneamente un elemento del vettore entry valid:
- entry_valid è il vettore tiene conto delle sole pagine caricate in memoria (valide) in modo da rispettare il DEMAND PAGING.*/
void pt_update(paddr_t *pt,uint8_t *entry_valid,paddr_t new_frame,uint8_t nvalidentries,uint8_t pt_index)
{
    
    entry_valid[index]==pt_index;

    pt[entry_valid[index]] = new_frame;
    
    index = ((index + 1) == nvalidentries) ? 0 : (index+1);

}
