#include <coremap.h>

/* in order to manage frames reserved for the user program,
 * a pointer to paddr_t is needed to allocate the requested
 * frames: this pointer is initialized inside ffl_init, where
 * we request "nframes" frames and store the address inside
 * frameblock
 */
static paddr_t frameblock;

struct ffl * ffl_create(const uint8_t nframes)
{
    struct ffl * new_ffl = kmalloc(sizeof(struct ffl) * nframes);
    if(new_ffl == NULL) return NULL;

    for(int idx = 0; idx < nframes; idx++)
    {
        new_ffl[idx].prev = (idx != 0) ? &new_ffl[idx - 1] : NULL;
        new_ffl[idx].next = (idx != nframes - 1) ? &new_ffl[idx + 1] : NULL;
    }

    return new_ffl;
}

void ffl_init(struct ffl ** ffl_init, uint8_t nframes)
{
    paddr_t cur_frame = frameblock = getppages(nframes);
    
    (*ffl_init)->free_frame = cur_frame;
    while((*ffl_init)->next != NULL)
    {
        cur_frame += PAGE_SIZE;
        ffl_push(ffl_init, cur_frame);
    } 
    
    return;
}

void ffl_push(struct ffl ** ffl_push, const paddr_t fr_push)
{
    *ffl_push = (*ffl_push)->next;
    (*ffl_push)->free_frame = fr_push;

    return;
}

paddr_t ffl_pop(struct ffl ** ffl_pop)
{
    paddr_t fr_pop = (*ffl_pop)->free_frame;
    (*ffl_pop)->free_frame = 0;
    *ffl_pop = ((*ffl_pop)->prev != NULL) ? (*ffl_pop)->prev : *ffl_pop;

    return fr_pop;
}

void ffl_destroy(struct ffl * ffl_dest)
{
    freeppages(frameblock); 
    kfree(ffl_dest);

    return;
}