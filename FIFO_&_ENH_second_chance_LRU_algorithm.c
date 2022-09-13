#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define N 17 //12 //20
int PR_FIFO_Algorithm(int stack[],int num_pages,int page_number);
int PR_ENH_LRU_SC_Algorithm(int stack[],int num_pages,int page_number,int modify_bit,int state_bits[],int ref_bit[]);

int cont = 0,internal_cont=0,state=0,page_faults=0,enable_control = 0,num_pages=3;
bool present;

int main()
{
    //int ref_sequence[N]={7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    //int ref_sequence[N]={1,2,3,4,1,2,5,1,2,3,4,5};
    int ref_sequence[N]={0,1,3,6,2,4,5,2,5,0,2,3,1,2,5,4,2};
    int dirty_bit[N]={0,1,0,1,0,0,1,0,0,1,0,1,0,1,0,0,0};
    int Tot_page_faults;
    int *stack = (int *)malloc(sizeof(int)*num_pages);
    int *state_bits = (int *)malloc(sizeof(int)*num_pages);
    int *ref_bit = (int *)malloc(sizeof(int)*num_pages);

    size_t n;

    for (int m = 0; m < num_pages; m++) {state_bits[m] = 0b00; ref_bit[m]= 0b0;}//Inizializzo vettore stati coppie bit e reference bit

    for(int i=0;i<N;i++)
    {
        //Tot_page_faults = PR_FIFO_Algorithm(stack,num_pages,ref_sequence[i]);
        Tot_page_faults = PR_ENH_LRU_SC_Algorithm(stack,num_pages,ref_sequence[i],dirty_bit[i],state_bits,ref_bit);
        //n=sizeof(stack)/sizeof(stack[0]);
        printf("ref page: %d ,modify bit: %d --> ",ref_sequence[i],dirty_bit[i]);
        for(int j=0;j<num_pages;j++)
        {
            printf("stack[%d]:%d, ",j,stack[j]);
        }
        printf("\n");
    }

    printf("Tot delle page faults: %d",Tot_page_faults);
    return 0;
}



int PR_FIFO_Algorithm(int stack[],int num_pages,int page_number)
{

    //printf("Page number: %d \n",page_number);
    if(cont==num_pages)
    {
        enable_control = 1;
        //printf("Risultato attuale: ");
        //for(int i=0;i<num_pages;i++) printf("%d, ",stack[i]);
        cont = 0;
        //printf("\nArray riempito,adesso si procedere al replacement \n");

    }
    else if((cont<num_pages && cont >=1) && !enable_control)
    {
        for(int i=0;i<cont;i++) {
            if (stack[i] != page_number) present = 0;
            else {
                present = 1;
                break;
            }
        }
    }
    if(enable_control)
    {
        for(int i=0;i<num_pages;i++) {
            if (stack[i] != page_number) present = 0;
            else {
                present = 1;
                break;
            }
        }
        //printf("Present : %d \n",present);
    }

    if(!present){
        stack[cont]=page_number;
        //printf("stack[%d]:%d \n",cont,stack[cont]);
        page_faults++;
        cont++;
    }

    return page_faults;


}

int PR_ENH_LRU_SC_Algorithm(int stack[],int num_pages,int page_number,int modify_bit,int state_bits[],int ref_bit[])
{

    switch(state)
    {
        case 0://STATO RIEMPIMENTO INIZIALE

            stack[cont] = page_number;
            ref_bit[cont] = 0b1;
            state_bits[cont] = (ref_bit[cont] << 1) | modify_bit;
            page_faults++;
            cont++;
            if(cont==num_pages) {state=1;cont = 0;}
            break;
        case 1: //STATO PRIMA RICERCA,CERCO coppia 00
            for(int i=0;i<num_pages;i++) //Cerco se la pagina è già in memoria prima di rimpiazzarla--->NO PAGE FAULT
            {
                if (stack[i] == page_number) {
                    state = 1;
                    ref_bit[i] = 0b1;
                    state_bits[i] = (ref_bit[i] << 1) | modify_bit;
                    break;
                }
            }
            for(int i=0;i<num_pages;i++)
            {
                if(state_bits[i]==0b00) //Cerco la pagina in memoria con sequenza 00
                {
                    stack[i] = page_number;
                    ref_bit[i] = 0b1;
                    state_bits[i] = (ref_bit[i] << 1) | modify_bit;
                    page_faults++;
                    state = 1;
                    cont ++;
                    break;
                }
                else internal_cont++;
            }
            if(internal_cont == num_pages) {state=2;internal_cont = 0;break;}//Se non ho trovato nessuna pagina con sequenza 00 cerco se ci sono sequenze 01

        default: //STATO SECONDA RICERCA,CERCO coppia 01,azzero tutti i ref bit delle page che analizzo nel frattempo

            if(cont==num_pages) //Se ho riempito un numero di frame in memoria pari al numero totale di frame inseribili nello stack azzero tutti i ref bit
                for(int i=0;i<num_pages;i++) {
                    ref_bit[i] = 0b0;
                    state_bits[i] = (ref_bit[i] << 1) & 0b11; //modifico solo ref bit dei frame
                    cont=0;
                }

            for(int i=0;i<num_pages;i++)
            {
                if(state_bits[i]==0b00)//Questo caso è quello in dubbio: Devo rimpiazzare in questo stato la pagina con sequenza 00 che trovo (se la trovo)?
                {
                    stack[i] = page_number;
                    ref_bit[i] = 0b1;
                    state_bits[i] = (ref_bit[i] << 1) | modify_bit;
                    state = 1;
                    page_faults++;
                    cont ++;
                    break;
                }
            }
            for(int i=0;i<num_pages;i++)
            {
                if(state_bits[i]==0b01)//Cerco la pagina in memoria con sequenza 01
                {
                    stack[i] = page_number;
                    ref_bit[i] = 0b1;
                    state_bits[i] = (ref_bit[i] << 1) | modify_bit;
                    page_faults++;
                    cont ++;
                    state = 1;
                    break;
                }

            }


    }


    return page_faults;
}
