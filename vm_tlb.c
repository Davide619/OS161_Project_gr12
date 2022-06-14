#include <mips/tlb.h>
#include <mips/vm_tlb.h> /*file header di questo file. CONTROLLARE IL PERCORSO*/


/*prototipi dichiarati in tlb.h  DA CANCELLARE QUI*/
void tlb_random(uint32_t entryhi, uint32_t entrylo);
void tlb_write(uint32_t entryhi, uint32_t entrylo, uint32_t index);
void tlb_read(uint32_t *entryhi, uint32_t *entrylo, uint32_t index);
int tlb_probe(uint32_t entryhi, uint32_t entrylo);


/*tlb_random scrive la voce entryhi e entrylo in uno slot casuale che sceglie il processore
PER QUESTA FUNZIONE NON SCRIVERE PIù DI UNA VOCE DELLA TLB CON LO STESSO CAMPO DELLA PAGINA VIRTUALE*/


/*tlb_write scrive in uno slot specifico che scegli tu*/


/*void tlb_read legge una voce TLB in *entryhi e *entrylo index specifica quale delle due voci ottenere*/


/*tlb_probe cerca la voce corrispondente a entryhi. questa funzione torna l'indice della voce corrispondente
mentre un valore negativo se non la trova. entrylo non è usato ma deve essere settato a 0. 
La voce potrebbe essere trovata anche se il valid bit non è settato. Per invalidare completamente la TLB carica tale voce in 
un indirizzo che non è mappato in modo tale che non sarà mai matchato*/



/*SCRIVERE QUI ALGORITMO DI REPLACEMENT INOLTRE DEVO TENER CONTO CHE SE LA VOCE DA RIMPIAZZARE FA RIFERIMENTO AD UN SEGMENTO CODICE
DEVO IMPOSTARE NELLA TLB IL PERMESSO DI SOLA LETTURA*/
int tlb_get_rr_victim(void){
	
	int victim;
	static unsigned int next_victim =0;
	victim = (next_victim +1) % NUM_TLB;
	return victim;

}


/*SCRIVERE UNA FUNZIONE PER TERMINARE UN PROCESSO*/
/*DA CAPIRE COME OTTENERE IL PID DEL PROCESSO CHE è ENTRATO NELLA TLB*/






/*FUNZIONE PER IL FLUSH SPECIFICO NEL TLB*/
static 
void vm_tlbflush(vaddr_t target) 
{
    int spl;
    int index;

    spl = splhigh();
    index = tlb_probe(target & PAGE_FRAME, 0);
    if (index >=0) 
        tlb_write(TLBHI_INVALID(index), TLBLO_INVALID(), index);
    splx(spl);
}



/*SCRIVERE UNA FUNZIONE PER VEDERE SE CI SONO SPAZI VUOTI NELLA TLB
CAPIRE INIZIALMENTE COSA SIGNIFICA AVERE UNO SPAZIO VUOTO, COSA CI TROVO?*/







/*SCRIVERE UNA FUNZIONE CHE CERCA NELLA PAGE TABLE SE è PRESENTE LA VOCE RICHIESTA. SE Sì BISOGNA AGGIORNARE LA PAGE TABLE*/
/*IN BASE ALL'ESITO DI QUESTA FUNZIONE SI DOVREBBE POI RICHIAMARE L'ALGORITMO DI REPLACEMENT CHE MI AGGIORNA LA TLB*/
/*LìIMPLEMENTAZIONE DI QUESTA FUNZIONE FORSE è MEGLIO FARLA NELLA SEZIONE PT*/


