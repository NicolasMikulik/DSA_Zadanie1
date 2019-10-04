#include<stdio.h>
#include <string.h>
#include <stdbool.h>
#define BYTECOUNT 1000

typedef struct Block{
    unsigned int size;
    struct Block *next;
    struct Block *prior;
};

typedef struct arrayHead{
    unsigned int size;
    struct Block *next;
};

#define BLOCKSIZE sizeof(struct Block)
char *allpointer;
void memory_init(void *ptr, unsigned int size){ //Attempt without struct
    struct arrayHead *firstHead = ptr;
    allpointer = (char*)firstHead;

    firstHead->size = size-sizeof(struct arrayHead);
    struct Block *freeOne = (char *)(firstHead)+sizeof(struct arrayHead);
    freeOne->next=NULL;
    freeOne->prior=(struct Block *)firstHead;
    freeOne->size=firstHead->size-sizeof(int);
    firstHead->next = freeOne;
}
int memory_check(void *ptr);
void *split(struct Block *fitting, unsigned int size){
    struct Block *new= (char*)((char *)fitting+BLOCKSIZE+size);
    new->size=fitting->size-size-sizeof(int);
    if(fitting->prior!=NULL){
        fitting->prior->next=new;
        new->prior=fitting->prior;
    }
    else {new->prior=NULL;}
    if(fitting->next!=NULL){
        fitting->next->prior=new;
        new->next=fitting->next;
    }
    else {new->next=NULL;}
    fitting->prior=NULL;
    fitting->next=NULL;
    fitting->size=size+1;
}

void *memory_alloc(unsigned int size){
    if(size % 2 == 1) size++;
    bool flag = false;
    char *reference;
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next, *prior;
    while(curr->next != NULL && curr->size < size){
        prior=curr;
        curr = curr->next;
    }
    if(curr->size == size){
        printf("Size %d B found\n",curr->size);
        if(curr->next != NULL)
            curr->next->prior=curr->prior;
        if(curr->prior != NULL)
            curr->prior->next = curr->next;
        curr->next=NULL;
        curr->prior=NULL;
        curr->size=size+1;
        flag=true;
        reference = (char*)curr;
        reference += sizeof(int);
        return (void *)reference;
    }
    if(curr->size > size){
        split(curr,size);
        flag=true;
        reference = (char*)curr;
        reference += sizeof(int);
        return (void *)reference;
    }
    if(!flag){
        printf("No fitting block found\n");
        return NULL;
    }
}
int memory_free(void *valid_ptr){
}
int main(){
    //Pamat = *(aka*)ptr;
    char region[BYTECOUNT];
    printf("Sizeof arrayHead %ld Sizeof Block %ld\n",sizeof(struct arrayHead),sizeof(struct Block));
    printf("given %p\n",region);
    memory_init(region,BYTECOUNT);
    int *pointer =(int*)memory_alloc(900);
    char *temp = (char *)pointer;
    temp = temp - sizeof(int);
    struct Block *read = (struct Block*) temp;
    printf("Read %d\n",((struct Block*) temp)->size);
    printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    return 0;
}