#include <pt.h>

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