#include <coremap.h>

struct ffl * ffl_create(const uint8_t nframes)
{

}

void ffl_init(struct ffl ** ffl_init, uint8_t nframes)
{

}

void fflpush(struct ffl ** ffl_push, const paddr_t fr_push)
{
    /* verify if there is necessity of some sort of
     * boundary checking, to avoid push/pop outside che 
     * address space reserved to the stack.
     * 
     * if everything is done properly during allocation of
     * PAGE_SIZE frames, this check should not be necessary.
     */
    *ffl_push = (*ffl_push)->next;
    (*ffl_push)->free_frame = fr_push;
}

paddr_t ffl_pop(struct ffl ** ffl_pop)
{
    paddr_t fr_pop = (*ffl_pop)->free_frame;
    (*ffl_pop)->free_frame = 0;
    *ffl_pop = (*ffl_pop)->prev;

    return fr_pop;
}

void ffl_destroy(struct ffl * ffl_dest)
{

}