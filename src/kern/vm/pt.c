#include <pt.h>

struct pt * pt_create(size_t nentries)
{
    /* simply recalls an allocator from a low-level file
     * (probably called "userram.c"?).
     */
}

void pt_destroy(struct pt * pt_dest)
{
    /* an interface to a deallocator from a low-level file, 
     * as pt_create. 
     */
}