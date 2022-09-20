# DAVIDE
## FILE MODIFICATI/GENERATI PER LO SWAP FILE ED EXIT:
- swap.c (file generato)
- swap.h (file generato)
- src/kern/syscall/file_syscall.c
- src/kern/include/syscall.h
- src/kern/arch/mips/syscall/syscall.c
- src/kern/include/proc.h
- src/kern/proc/proc.c
- src/kern/include/thread.h
- src/kern/thread/thread.c
- root/sys161.conf
- src/kern/arch/mips/conf/conf.arch
- src/kern/conf/conf.kern
- src/kern/conf/(nome del kernel generato)
- src/kern/arch/mips/vm/dumbvm.c

<b>RUNPROGRAM.C:</b>
1. tale codice richiama LOADELF.C

<b>LOADELF.C:</b>
1. la funzione load_elf() chiama due cicli for. l'indice dei cicli va da i=0 ad e_phnum. Quest'ultimo si riferisce al program header number che tipicamente per os161 è 3 che significa che l'elf file ha potenzialmente tre header quindi 3 sezioni ma solo due di loro sono usate per prendere informazioni. Infatti nell' iterazione 0 non viene preso niente quindi la sezione 0 non viene presa ma viene presa la sezione 1 e 2 che corrisponde a prendere all'interno dell elf file solo le informazioni relative al segmento dato e al segmento codice. Questo perchè la sezione 0 riguarda il segmento di reginfo che non ha a che fare con i segmenti importanti per la memorizzazione dei dati e del codice in memoria. Inoltre non viene considerato il segmento di stack perchè per come è implementato il tutto assumiamo che lo stack abbia un numero di pagine predefinito e fisso(18) e che non cambi con il cambiare del programma e/o processo che si esegue,quindi esso non viene specificato nel file elf per questo.

2. il primo ciclo for prepara lo spazio di memoria ad ospitare i segmenti attraverso la funzione as_define_region(). Questo ciclo infatti ha il compito di leggere dal elf file gli header dato e codice ed associare ad essi i rispettivi indirizzi dei segmenti codice e dato. Nella pratica setta as_vbase1, as_vbase2, as_npages1, as_npages2. Riserva dello spazio CONTIGUO per il segmento dato e codice. 
Finito questo ciclo viene chiamata la funzione as_prepare_load(as) che alloca tutta la memoria fisica necessaria per quello spazio logico degli indirizzi.

3. il secondo ciclo for rilegge nuovamente l'header dato e codice dall'elf file e avendo già definito la regione di memoria al ciclo for precedente, carica ciasun segmento nella locazione opportuna.

# ALESSANDRO

<b> Struttura del debug partendo dal loadelf.c:</b>
Funzione int load_elf():questa funzione carica un user program elf file nell'address space del processo attuale e ritorna l'entry point, cioè l'indirizzo iniziale da cui parte il Program Counter:
	
1. Con as=proc_getas(); acquisisco la struttura dati dell'addresspace del process attuale.
Con uio_kinit(...); leggo l'header eseguibile partendo da un offset per il file elf pari a 0.

2. Eseguo un primo ciclo for(di 3 iterazioni,la prima non fa niente,la seconda e la terza riguardano il code segment e il data segment) per acuqisire dall'header eseguibile le informazioni sui segmenti di code e data, in particolare il virtual address space(p_vaddr) da cui partono e le loro dimensioni(p_memsz).
	
3. Una volta acquisite le info richiamo la funzione as_define_region(as,....,...,..); :
Funzione as_define_region(as,....,...,..):
Questa funzione,dati i due virtual address dei segmenti e le memsize,definisce le zone della RAM in cui posizionarli e quanti frame conterranno in totale,allocabili.
	
	3.1. Dopo aver verificato che gli indirizzi fisici siano tutti nulli(da controllare questa cosa),vengono aggiornati con gli indirizzi della page in memoria corrispondenti che vogliamo allocare chiamando la funzione getppages(vaddr_t vaddr);
		
<b>Funzione getppage(unsigned long npages):</b>
Questa funzione si occupa di fornire l'indirizzo della pagina fisica in memoria richiamando al suo interno la funzione ram_stealmem()(*ATTENZIONE*:se siamo in fase di inizializzazione della virtual memory come adesso,ha senso usare la ram_stealmem.Dopo quindi la fase di inizializzazione dovremo trovare un altro meccanismo per allocare memoria).
		
La getppage a sua volta deve controllare se vi sono frame liberi,per fare cio si serve della getfreepages();(essa ritorna l'indirizzo del primo frame libero dopo l'indirizzo fisico restituito da ram_stealmem()). Se la getppage non trova frame liberi dovra adottare una politica di page replacement per trovarne uno da utilizzare.
		
4. Adesso,con il secondo for carico effettivamente i segmenti in memoria. Dopo aver letto nuovamente i dati dell'elf file dall'header eseguibile richiamo la funzione load_segment();
	
5. Infine,dopo il secondo for,richiamo la funzione as_complete_load(as) per controllare se la struttura dati dell'address space è stata correttamente riempita.
	
6. Ritorno l'entry point 


# WILLIAM
## FILE MODIFICATI
- src/kern/arch/mips/vm/ram.c
- src/kern/include/coremap.h
- src/kern/vm/coremap.c
- src/kern/include/pt.h
- src/kern/vm/pt.c

## APPUNTI

<b>parametro entrypoint:</b> <br>
è necessario vedere in runprogram e loadelf qual è il comportamento di "entrypoint", in teoria dovrebbe essere un indirizzo specificato da noi che viene passato al program counter e che rappresenta l'indirizzo della prima istruzione da caricare in memoria.

<b>gestione allocatore/deallocatore:</b> <br>
è necessario valutare il comportamento delle funzioni che si occuperanno di allocare effettivamente della memoria fisica riservandola per qualunque struttura/frame sia necessaria ad un certo processo.
L'allocatore può essere strutturato in maniera molto simile a ram_stealmem, tuttavia va considerato che se alloco memoria per proc.a e di seguito un'altra porzione per proc.b, se proc.a finisce l'esecuzione prima di proc.b, mi ritrovo a dover deallocare la porzione di memoria di proc.a, mentre proc.b è ancora in esecuzione: questo crea un buco di locazioni che sono potenzialmente libere e occupabili, e ho bisogno di una opportuna struttura dati che gestisca e tenga traccia delle 
porzioni di memoria che si liberano.

<b>vaddr relativo alle istruzioni:</b> <br>
è necessario cercare la porzione di codice che fornisce al sistema VM l'indirizzo virtuale relativo alle istruzioni a cui il programma in esecuzione cerca di fare accesso.
