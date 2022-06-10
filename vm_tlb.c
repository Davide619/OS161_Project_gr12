#include <mips/tlb.h>


/*prototipi dichiarati in vm_tlb.h*/
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



/*funzione a cui passo l'indirizzo logico e ritorna TLB hit (indirizzo fisico) e TLB miss(indirizzo da cercare nella PT)*/
/*QUI DENTRO VADO A IMPLEMENTARE LE TRE FUNZIONI*/
void tlb_random(uint32_t entryhi, uint32_t entrylo){
	
}

void tlb_write(uint32_t entryhi, uint32_t entrylo, uint32_t index){
	/*scrivo all'interno della TLB quindi controllo prima se c'è spazio*/
	if()
}


void tlb_read(uint32_t *entryhi, uint32_t *entrylo, uint32_t index){
	
}


/*SCRIVERE QUI ALGORITMO DI REPLACEMENT*/
int tlb_get_rr_victim(void){
	
	int victim;
	static unsigned int next_victim =0;
	victim = (next_victim +1) % NUM_TLB;
	return victim;

}