#include<stdio.h>
#include <string.h>
#define INTCOUNT 100
int *memory;
void memory_init(void *ptr, unsigned int size){
    memset(ptr,0,size);
    memory=(void*)ptr;
    *memory = size/4;   //size of array in integers == INTCOUNT 100
    *(memory+1)=2*sizeof(int);  //"pointer" to the first free block == 8
    *(memory+2)=(size/sizeof(int)) - 3*sizeof(int); //size of first free block in integers == 88
    *(memory+3)=0;  //"pointer" to next free block. At the start it is zero, because there is only one free block == 0
}
void *memory_alloc(unsigned int size){
    int *curr = memory;//+*(memory+1)/sizeof(int);
    int *prev, *result;
    printf("Curr at the start of search: %d\n",*curr);
    printf("curr %d memory+INTCOUNT %d\n",curr,(memory+INTCOUNT*sizeof(int)));
    while(curr<(memory+INTCOUNT*sizeof(int)) && ((*curr)<0 || (*curr)<size)){
        prev=curr;
        curr += (*curr);
        printf("One block checked\n");
    }
    printf("Curr found a suitable block of size: %d\n",*curr);
}
int main(){

    /*int *p=(int)MyMalloc(100*sizeof(int));
    char *q=(char)MyMalloc(250*sizeof(char));
    int *r=(int)MyMalloc(1000*sizeof(int));
    MyFree(p);
    char *w=(char)MyMalloc(700);
    MyFree(r);
    int *k=(int)MyMalloc(500*sizeof(int));
    printf("Allocation and deallocation is done successfully!");*/

    /*
     int main()
    {
    char region[50];
    memory_init(region, 50);
    char* pointer = (char*) memory_alloc(10);
    if (pointer)
    memset(pointer, 0, 10); //na adresu, kam ukazuje pointer, sa zapise velkost bloku
    if (pointer)
    memory_free(pointer);
    return 0;
    }
    */
    int region[INTCOUNT];
    memory_init(region,INTCOUNT*sizeof(int));
    printf("%d %d %d %d\n",region[0],region[1],region[2],region[3]);
    int *pointer1=(int*)memory_alloc(8);
    return 0;
}