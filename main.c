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
void memory_init(void *ptr, unsigned int size){
    struct arrayHead *firstHead = ptr;
    allpointer = (char*)firstHead;
    firstHead->size = size-sizeof(struct arrayHead)-sizeof(int);
    struct Block *freeOne = (struct Block *)((char *) (firstHead) + sizeof(struct arrayHead));
    freeOne->next=NULL;
    freeOne->prior=(struct Block *)firstHead;
    freeOne->size=firstHead->size;
    firstHead->next = freeOne;
}
int memory_check(void *ptr){
    bool valid=true;
    char* reference = (char*)ptr;
    int sizeOfMemory = ((struct arrayHead*)allpointer)->size+sizeof(struct arrayHead)-1;
    if(reference<allpointer || reference>(allpointer+sizeOfMemory)){
        valid=false;
        printf("---Invalid pointer. Pointing out of bounds of memory.---\n");
        return 0;
    }
    reference -= sizeof(int);
    if(((struct Block*)reference)->size < 0 || ((struct Block*)reference)->size > 100000){
        valid=false;
        printf("---Invalid pointer, invalid size written in header of block.---\n");
        return 0;
    }
    if(((struct arrayHead*)allpointer)->next != NULL) {struct Block *curr=((struct arrayHead*)allpointer)->next;
    char *check;
    while(curr->next != NULL){
        check = (char*)curr;
        if(check<=reference && reference<=(check+BLOCKSIZE+curr->size)){
            valid=false;
            printf("---Invalid pointer. Provided pointer pointing free (not allocated) memory.---\n");
            return 0;
        }
        curr = curr->next;
    }
    }
    if(true==valid){
        printf("Provided pointer is valid.\n");
        return 1;
    }
}
void *split(struct Block *fitting, unsigned int size){
    struct Block *new= (struct Block *)(((char *)fitting)+BLOCKSIZE+size);
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
    struct Block *curr = startHead->next;
    if(curr == NULL){
        printf("No fitting block found\n");
        return NULL;
    }
    while(curr->next != NULL && curr->size < size){
        printf("Curr->size %d\n",curr->size);
        curr = curr->next;
    }
    if(curr->size == size){
        printf("Size %d B found\n",curr->size);
        if(curr->next != NULL)
            curr->next->prior=curr->prior;      //connect next free block with the previous one
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
    if(curr->size >= size+BLOCKSIZE){
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
    bool successfullyFreed=false;
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    char *reference=(char *)valid_ptr;
    struct Block *freed= (struct Block *)reference;
    if(memory_check(valid_ptr)){
        reference-=sizeof(int);
        freed=(struct Block*)reference;
        if(curr == NULL){
            curr=freed;
            curr->next=NULL;
        }
        while(curr->next != NULL && (curr->next < freed))
            curr = curr->next;
        if(((char*)curr+curr->size*sizeof(char)+BLOCKSIZE)==reference){ //merge with preceding free block
            curr->size+=sizeof(int)+freed->size;
            printf("Freed, merged with preceding free block.\n");
            if((curr->next != NULL) && (reference+freed->size*sizeof(char)==(char*)curr->next)){    //try to merge also with following free block
                curr->size+=sizeof(int)+curr->next->size;
                if(curr->next->next != NULL){
                    curr->next=curr->next->next;
                } else {
                    curr->next=NULL;}
                printf("Also merged with following free block.\n");
            }
            return successfullyFreed;
        }
        else {
            if((curr->next != NULL) && (reference+freed->size*sizeof(char)==(char*)curr->next)){    //merge with following free block
                freed->size+=sizeof(int)+curr->next->size;                                          //freed block increases by the size of first following free block
                printf("Freed, merged with following free block.\n");
                if(curr->next->next != NULL){
                    freed->next=curr->next->next;
                } else freed->next=NULL;
                curr->next=freed;
                return successfullyFreed;
            }
            else {
                if(curr->next!=NULL){
                    freed->next=curr->next;
                    curr->next->prior=freed;
                }
                else freed->next=NULL;
                curr->next=freed;
                freed->prior=curr;
                printf("Freed without merge.\n");
                return successfullyFreed;
            }
        }
    }
    else {
        printf("---Invalid provided pointer.\n");
        return !successfullyFreed;
    }
}
int main(){
    //Pamat = *(aka*)ptr;
    char region[BYTECOUNT];
    printf("Sizeof arrayHead %ld Sizeof Block %ld\n",sizeof(struct arrayHead),sizeof(struct Block));
    printf("given %p\n",region);
    memory_init(region,BYTECOUNT);
    printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    int *pointer =(int*)memory_alloc(982);
    char *temp = (char *)pointer;
    temp = temp - sizeof(int);
    struct Block *read = (struct Block*) temp;
    if(pointer != NULL )printf("Read %d\n",((struct Block*) temp)->size);
    if(((struct Block*) allpointer)->next != NULL)printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    memory_check((void*)pointer);
    memory_free((void*)pointer);
    return 0;
}