#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define N 20 //12
int cont = 0;
int page_faults=0;
int *time;
bool present;
int enable_control = 0;
int PR_FIFO_Algorithm(int stack[],int num_pages,int data);
int PR_LRU_Algorithm(int stack[],int num_pages,int ref_string[],int string_len,int time[]);


int main()
{
    int ref_sequence[N]={7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    //int ref_sequence[N]={1,2,3,4,1,2,5,1,2,3,4,5};
    int num_pages=3,Tot_page_faults;
    //int stack[num_pages];
    int *stack = (int *)malloc(sizeof(int)*num_pages);
    size_t n;

    for(int i=0;i<N;i++)
    {
        Tot_page_faults = PR_FIFO_Algorithm(stack,num_pages,ref_sequence[i]);
        n=sizeof(stack)/sizeof(stack[0]);
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

int PR_LRU_Algorithm(int stack[],int num_pages,int ref_string[],int string_len,int time[])
{
    for(int i=0;i<string_len;i++) {
        if(cont==num_pages)
        {
            enable_control = 1;
            cont = 0;
        }
        if(enable_control)
        {
            for(int j=0;j<num_pages;j++) {
                if (stack[j] != ref_string[i]) present = 0;
                else {
                    present = 1;
                    break;
                }
            }
        }
        if (!present) {
            stack[cont] = ref_string[i];
            printf("stack[%d]:%d \n", cont, stack[cont]);
            page_faults++;
            cont++;
        }
    }


    return page_faults;
}
