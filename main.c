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
    printf("Performing split from %d to %d\n",fitting->size,size);
    struct Block *new= (struct Block *)(((char *)fitting)+BLOCKSIZE+size);
    new->size=fitting->size-size-sizeof(int);
    if(fitting->prior!=NULL){
        fitting->prior->next=new;
        new->prior=fitting->prior;
    }
    else {new->prior=(struct Block*)allpointer;}
    if(fitting->next!=NULL){
        fitting->next->prior=new;
        new->next=fitting->next;
    }
    else {new->next=NULL;}
    fitting->prior=NULL;
    fitting->next=NULL;
    fitting->size=size+1;
}

void *memory_alloc(unsigned int size) {
    if (size % 2 == 1) size++;
    bool flag = false;
    char *reference;
    struct arrayHead *startHead = (struct arrayHead *) allpointer;
    struct Block *curr = startHead->next;
    if (curr == NULL) {
        printf("1)No fitting block found\n");
        return NULL;
    }
    while((char*)curr < allpointer+BYTECOUNT-1){
        while (curr->next != NULL && curr->size < size) {
            printf("Curr->size %d\n", curr->size);
            curr = curr->next;
        }
        if (curr->size == size) {
            printf("Exact size %d B found\n", curr->size);
            if (curr->next != NULL)
                curr->next->prior = curr->prior;      //connect next free block with the previous one
            if (curr->prior != NULL){
                if(curr->prior == (struct Block*)startHead)
                    {startHead->next=curr->next;}
                else
                    {curr->prior->next = curr->next;}
            }
            curr->next = NULL;
            curr->prior = NULL;
            curr->size = size + 1;
            flag = true;
            reference = (char *) curr;
            reference += sizeof(int);
            return (void *) reference;
        }
        if (curr->size >= size + BLOCKSIZE) {
            printf("Bigger block found, performing split.\n");
            split(curr, size);
            flag = true;
            reference = (char *) curr;
            reference += sizeof(int);
            return (void *) reference;
        }else{
            printf("Bigger block found, insufficient for a split, returning full block.\n");
            if (curr->next != NULL)
                curr->next->prior = curr->prior;      //connect next free block with the previous one
            if (curr->prior != NULL){
                if(curr->prior == (struct Block*)startHead)
                {startHead->next=curr->next;}
                else
                {curr->prior->next=curr->next;}
            }
            curr->next = NULL;
            curr->prior = NULL;
            curr->size+=1;
            flag = true;
            reference = (char *) curr;
            reference += sizeof(int);
            return (void *) reference;
        }
    }
    if(!flag){
        printf("2)No fitting block found\n");
        return NULL;
    }
}
int memory_free(void *valid_ptr){
    bool successfullyFreed=false;
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    char *reference=(char *)valid_ptr;
    reference-= sizeof(int);
    struct Block *freed= (struct Block *)reference;
    if((char*)curr > reference){
        freed->size--; //set size even
        printf("Freed size %d\n",freed->size);
        curr->prior=freed;
        startHead->next=freed;
        freed->next=curr;
        freed->prior= (struct Block *) startHead;
        printf("Freed, occupied became first free.");
        return 0;
    }
    else{
        while(curr->next != NULL && (char*)curr->next < reference){
            curr=curr->next;
        }
        if(curr->next == NULL && ((char*)curr < reference)){
            freed->size--; //set size even
            printf("Freed size %d\n",freed->size);
            freed->next=NULL;
            freed->prior=curr;
            curr->next=freed;
            printf("Freed, occupied became LAST free.");
            return 0;
        }
    }
}
int main(){
    //Pamat = *(aka*)ptr;
    char region[BYTECOUNT];
    printf("Sizeof arrayHead %ld Sizeof Block %ld\n",sizeof(struct arrayHead),sizeof(struct Block));
    printf("given %p\n",region);
    memory_init(region,BYTECOUNT);
    printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    int *pointer =(int*)memory_alloc(970);
    char *temp = (char *)pointer;
    temp = temp - sizeof(int);
    struct Block *read = (struct Block*) temp;
    if(pointer != NULL) printf("Read occupied %d\n",((struct Block*) temp)->size);
    if(((struct Block*) allpointer)->next != NULL)printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    //memory_free((void*)pointer);
    return 0;
}