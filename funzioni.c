#include <lib.h>


////////////////nel .h /////////////////
#define DATA_PAGE 0
#define CODE_PAGE 1
#define INVALID_PAGE 2
///////////////////////////////////////


/*l'idea è conosce a che tipo di segmento corrisponde il faultaddress che devo cercare nell'elf file mi serve per i bit nella TLB e PT*/

int which_page (vaddr_t faultaddress){

Elf_Ehdr eh;   /* Executable header */
Elf_Phdr ph;   /* "Program header" = segment header */
int result, i;
struct iovec iov;
struct uio ku;

vaddr_t first_start_addr, second_start_addr;
off_t size_first_segment, size_second_segment;


for (i=0; i<eh.e_phnum; i++) {          

        off_t offset = eh.e_phoff + i*eh.e_phentsize;
        uio_kinit(&iov, &ku, &ph, sizeof(ph), offset, UIO_READ);

        result = VOP_READ(v, &ku);                        
        if (result) {
                return result;
        }

        if (ku.uio_resid != 0) {
                /* short read; problem with executable? */
                kprintf("ELF: short read on phdr - file truncated?\n");
                return ENOEXEC;
        }

        switch (ph.p_type) {
                case PT_NULL: /* skip */ continue;
                case PT_PHDR: /* skip */ continue;
                case PT_MIPS_REGINFO: /* skip */ continue;
                case PT_LOAD: 
                        if (i == 1){
                              first_start_addr = ph.p_vaddr; //indirizzo d'inizio del primo segmento che può essere di dato o codice
                              size_first_segment = ph.p_filesz; //dimensione del primo segmento  
                        }
                        if (i == 2){
                              second_start_addr = ph.p_vaddr; //indirizzo d'inizio del secondo segmento che può essere di dato o codice
                              size_second_segment = ph.p_filesz; //dimensione del secondo segmento  
                        }

                break;
                default:
                kprintf("loadelf: unknown page type %d\n",
                        ph.p_type);
                return ENOEXEC;
        }

        /*verifico a quale dei segmenti fauladdress appartiene all'interno dell'elf file*/
        if(faultaddress >= first_start_addr && faultaddress < second_start_addr){
                kprintf("I am searching for Data page!\n");                                           
                return DATA_PAGE;

        }else if(faultaddress >= second_start_addr &&  faultaddress < (second_start_addr + size_second_segment-1)+(PAGE_SIZE+1)){
                kprintf("I am searching for Code page!\n");                     
                return CODE_PAGE;
        }else{
                kprintf("Invalid ADDRESS!\n");                  
                return INVALID_PAGE;
        }


            
}
}


/*carico il frame in memoria (ElfFile ----> RAM). QUESTA FUNZIONE NON BISOGNA CHIAMARLA QUANDO SI VUOL SWAPPARE UN FRAME*/
static
int 
load_page_fromElf(vaddr_t page_to_load, vaddr_t faultaddress)         
{
        Elf_Ehdr eh;   /* Executable header */
        int result, which_page;
        struct iovec iov;
        struct uio u;
        struct vnode *v;
        struct addrspace *as;
        vaddr_t first_start_addr;
        unsigned int npage_from_start;
        struct elfFile_info *info;                      /*struttura da creare che contiene tutte le info dell'elf file*/

        /*Return the program name*/
        char *progname = proc_getprogname();

        /* Retain the addresspace */
        as = proc_getas();

        /* Open the elf_file. */
        result = vfs_open(progname, O_RDONLY, 0, &v);
        if (result) {
                return result;
        }

        /*verifying which page is*/
        which_page = which_page(faultaddress);

        if(which_page == DATA_PAGE){
                is_executable = 1;
                first_start_addr = /*(vbase1 o vbase2????)lo recupero dalla struttura che contiene info sull elf file (dall' addrspace)*/;

                npage_from_start = ((faultaddress & PAGE_FRAME) - (first_start_addr & PAGE_FRAME))/PAGE_SIZE; /*rivedere operazione*/

                /*qui verifico il numero di pagine ottentute dall'inizio del segmento all'indirizzo
                della pagina in questione per capire quale offset prendere*/
                if(npage_from_start ==1){
                        /*se sono qui dentro vuol dire che l'offset è 0 rispetto al segmento dato
                        ma essendo che tale segmento viene dopo il segmento code, l'offset complessivo della
                        pagina corrispondente è dato dall'ofset base del segmento dato*/

                        offset_elf = /*(la re3cupero dall'as)lo recupero dalla struttura che contiene info sull elf file (p_offset del seg dato)*/;

                }else{

                        /*se sono qui dentro vuol dire che l'offset della pagina è un valore intermedio tra l'indirizzo
                        d'inizio del segmento dato e indirizzo di fine quindi una pagina intermedia*/
                        
                        offset_elf = first_start_addr + (PAGE_SIZE - first_start_addr % PAGE_SIZE) + (npage_from_start_segment-1)*PAGE_SIZE;
                }
                

        }else if(which_page == CODE_PAGE){
                is_executable = 0;
                first_start_addr = /*lo recupero dalla struttura che contiene info sull elf file*/;

                npage_from_start = ((faultaddress & PAGE_FRAME) - (first_start_addr & PAGE_FRAME))/PAGE_SIZE;

                if(npage_from_start ==1){
                        /*se sono qui dentro vuol dire che l'offset è 0 rispetto al segmento code*/

                        offset_elf = /*lo recupero dalla struttura che contiene info sull elf file (p_offset del seg code)*/;

                }else{

                        /*se sono qui dentro vuol dire che l'offset della pagina è un valore intermedio tra l'indirizzo
                        d'inizio del segmento code e indirizzo di fine quindi una pagina intermedia*/

                        offset_elf = first_start_addr + (PAGE_SIZE - first_start_addr % PAGE_SIZE) + (npage_from_start_segment-1)*PAGE_SIZE;
                }

        }else{
                panic("ERROR page not found in elf_file! Process is stopping...\n");
        }

        

        iov.iov_ubase = (userptr_t)page_to_load;                                      //capire se deve essere fisico o virtuale, mi verrebbe da dire fisico
        iov.iov_len = PAGE_SIZE;           // length of the memory space
        u.uio_iov = &iov;
        u.uio_iovcnt = 1;
        u.uio_resid = PAGE_SIZE          // amount to read from the file
        u.uio_offset = offset_elf;
        u.uio_segflg = is_executable ? UIO_USERISPACE : UIO_USERSPACE;  //UIO_USERISPACE = appartiene a code --- UIO_USERSPACE = dato
        u.uio_rw = UIO_READ;
        u.uio_space = as;

        
        result = VOP_READ(v, &u); 

        if (result) {
                vfs_close(v);
                return result;
        }

        vfs_close(v);


        return 0;
}
