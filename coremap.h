#ifndef _COREMAP_H_
#define _COREMAP_H_

#include <types.h>

/* data structure representing a (f)ree(f)rame(l)ist
 * implemented as a stack data structure 
 */
struct ffl 
{
    paddr_t free_frame;
    struct ffl * next, * prev;
};

/* ffl_create():  function to create a ffl struct, it calls and internal allocator to
 *                reserve the proper space in memory, then realizes a stack with "nframes"
 *                entries and sets for each of them the corresponding relations between
 *                prev/next pointers, finally, it returns a pointer to the stack base
 *                in order to allow init() to fill it with the frame addresses.
 *                for each entry, prev is set to "ffl-sizeof(ffl)" (except the first),
 *                basically the previous entry with respect to the current one, while
 *                next is set to "ffl+sizeof(ffl)" (except the last).
 * 
 * init():        function to initialize a ffl struct, it takes as input a ffl struct
 *                pointer and calls push() "nframes" times in order to fill it with 
 *                the frame addresses.
 * 
 * push():        function to push a new token inside the stack, first it assignes to 
 *                ffl_push the content of next so that the ffl pointer points to the 
 *                next entry, then it updates free_frame with the content of fr_push.
 *
 * pop():         function to pop an unused token from the stack, first it saves the
 *                frame of ffl_pop in fr_pop, then it deletes the content of ffl_pop
 *                and finally it updates ffl_pop with prev so that ffl_pop points to
 *                the previous entry.
 *
 * ffl_destroy():  
 */

struct ffl * ffl_create(const uint8_t nframes);
void init(struct ffl ** ffl_init, uint8_t nframes);
void push(struct ffl ** ffl_push, const paddr_t fr_push);
paddr_t pop(struct ffl ** ffl_pop);
void ffl_destroy(struct ffl * ffl_dest);

#endif /* _COREMAP_H_ */