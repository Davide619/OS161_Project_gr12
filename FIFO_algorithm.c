#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 12 //20
int cont = 0;
int page_faults=0;

bool present;
int enable_control = 0;
int PR_FIFO_Algorithm(int stack[],int num_pages,int data);
void PR_LRU_Algorithm(int stack[],int num_pages,int page_number);
//void push_page(int top,int stack[],int element);
//int pop_page(int top,int stack[]);

int main()
{
    //int ref_sequence[N]={7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1};
    int ref_sequence[N]={1,2,3,4,1,2,5,1,2,3,4,5};
    int num_pages=4,Tot_page_faults;
    int stack[num_pages];
    
    for(int i=0;i<N;i++) Tot_page_faults = PR_FIFO_Algorithm(stack,num_pages,ref_sequence[i]);

    printf("Tot delle page faults: %d",Tot_page_faults);
    return 0;
}


/*void push_page(int top,int stack[],int value)
{
    top++;
    stack[top] = value;
    printf("%d, ", stack[top]);

}


int pop_page(int top,int stack[])
{
    int value = stack[top];
    top--;
    return value;
}*/

int PR_FIFO_Algorithm(int stack[],int num_pages,int page_number)
{

    printf("Page number: %d \n",page_number);
    if(cont==num_pages)
    {
        enable_control = 1;
        printf("Risultato attuale: ");
        for(int i=0;i<num_pages;i++) printf("%d, ",stack[i]);
        cont = 0;
        printf("\nArray riempito,adesso si procedere al replacement \n");

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
        printf("Present : %d \n",present);
    }

    if(!present){
        stack[cont]=page_number;
        printf("stack[%d]:%d \n",cont,stack[cont]);
        page_faults++;
        cont++;
    }

    return page_faults;


}

void PR_LRU_Algorithm(int stack[],int num_pages,int page_number)
{


}
