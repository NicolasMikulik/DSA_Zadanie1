#include<stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) Block{
    unsigned int size;
    struct Block *next;
    struct Block *prior;
}Block;
typedef struct __attribute__((__packed__)) arrayHead{
    unsigned int size;
    struct Block *next;
}arrayHead;
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
    int sizeOfMemory = ((struct arrayHead*)allpointer)->size+sizeof(struct arrayHead)-1+sizeof(int);
    if(ptr == NULL || reference<allpointer || reference>(allpointer+sizeOfMemory)){
        valid=false;
        //printf("---Invalid pointer. Pointing out of bounds of memory.---\n");
        return 0;
    }
    //reference -= sizeof(int);
    if(((struct Block*)(reference-sizeof(int)))->size < 0 || (((struct Block*)(reference-sizeof(int)))->size > 100000)){
        valid=false;
        //printf("---Invalid pointer, invalid size written in header of block.---\n");
        return 0;
    }
    if(((struct arrayHead*)allpointer)->next != NULL) {
        struct Block *curr=((struct arrayHead*)allpointer)->next;
        struct Block *check;
        while(curr->next != NULL){
            check = (struct Block*)reference;
            if(check==curr){
                valid=false;
                //printf("---Invalid pointer. Provided pointer pointing free (not allocated) memory.---\n");
                return 0;
            }
            curr = curr->next;
        }
    }
    if(true==valid){
        //printf("Provided pointer is valid.\n");
        return 1;
    }
}
void *split(struct Block *fitting, unsigned int size){
    //printf("Performing split from %d to %d\n",fitting->size,size);
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
    bool traversed = false;
    char *reference;
    struct arrayHead *startHead = (struct arrayHead *) allpointer;
    struct Block *curr = startHead->next;
    printf("Size to be allocated %d\n",size);
    if (curr == NULL) {
        //printf("1)No fitting block found\n");
        return NULL;
    }
    while(curr->next != NULL && curr->size < size){
        if(((char*)curr)+sizeof(int)+curr->size == ((char*)curr->next)){
            if(curr->size%2==0 && curr->next->size%2==0){
                curr->size+=sizeof(int)+curr->next->size;
                if(curr->next->next!=NULL){
                    curr->next->next->prior=curr;
                    curr->next=curr->next->next;
                }
                //printf("New merged size %d\n",curr->size);
            }
        }
        if(curr->next == NULL) traversed=true;
        if(curr!=NULL && curr->next!=NULL && ((char*)curr+sizeof(int)+curr->size==(char*)curr->next))
        {continue;}
        else {curr=curr->next;}
    }
    traversed=false;
    curr=startHead->next;
    while(!traversed){

        while (curr->next != NULL && curr->size < size) {
            //printf("Curr->size %d\n", curr->size);
            curr = curr->next;
        }
        if (curr->size == size) {
            //printf("Exact size %d B found\n", curr->size);
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
            //printf("Bigger block found, performing split.\n");
            split(curr, size);
            flag = true;
            reference = (char *) curr;
            reference += sizeof(int);
            return (void *) reference;
        }else {
            if((curr->size < size + BLOCKSIZE) && (size < ((struct arrayHead*)allpointer)->size)){
                //printf("Bigger block found, insufficient for a split, returning full block of size %d.\n",curr->size);
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
        if(curr==NULL || curr->next == NULL) {
            traversed=true;
            continue;
        }
    }
    if(!flag){
        //printf("2)No fitting block found\n");
        return NULL;
    }
}
int memory_free(void *valid_ptr){
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    char *reference=(char *)valid_ptr;
    reference-= sizeof(int);
    struct Block *freed= (struct Block *)reference;
    //if(memory_check(valid_ptr)){
    if(curr == NULL){
        freed->size--; //set size even
        //printf("Freed size %d\n",freed->size);
        startHead->next=freed;
        freed->next=NULL;
        freed->prior= (struct Block *) startHead;
        //printf("Freed, occupied became first free. Memory was full.\n");
        return 0;
    }
    if((char*)curr > reference){
        freed->size--; //set size even
        //printf("Freed size %d\n",freed->size);
        curr->prior=freed;
        startHead->next=freed;
        freed->next=curr;
        freed->prior= (struct Block *) startHead;
        //printf("Freed, occupied became first free.\n");
        return 0;
    }
    if((char*)curr < reference){
        while(curr->next != NULL && (char*)curr->next < reference){
            curr=curr->next;
        }
        if(curr->next == NULL && ((char*)curr < reference)){
            freed->size--; //set size even
            //printf("Freed size %d\n",freed->size);
            freed->next=NULL;
            freed->prior=curr;
            curr->next=freed;
            //printf("Freed, occupied became LAST free.\n");
            return 0;
        }
        if(curr->next != NULL && ((char*)curr < reference)){
            freed->size--; //set size even
            //printf("4)Freed size %d\n",freed->size);
            freed->next=curr->next;
            curr->next->prior=freed;
            freed->prior=curr;
            curr->next=freed;
            //printf("Freed, occupied is not last free.\n");
            return 0;
        }
    }
    //}
}
void checkFree(){
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    while(curr != NULL && curr->next != NULL){
        //printf("Curr size %d\n",curr->size);
        curr=curr->next;
    }
    /*if(curr != NULL && curr->next == NULL)
        printf("LAST free size %d\n",curr->size);*/
}

// Vlastna funkcia main() je pre vase osobne testovanie. Dolezite: pri testovacich scenaroch sa nebude spustat!
int main()
{
    char region[50];
    memory_init(region, 50);
    char* pointer = (char*) memory_alloc(10);
    if (pointer)
        memset(pointer, 0, 10);
    if (pointer)
        memory_free(pointer);
    return 0;
}