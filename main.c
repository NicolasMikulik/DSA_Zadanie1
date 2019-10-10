#include<stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) Block{
    int size;
    struct Block *next;
    struct Block *prior;
}Block;

typedef struct __attribute__((__packed__)) arrayHead{
    unsigned int size;
    struct Block *next;
}arrayHead;

#define BLOCKSIZE sizeof(struct Block)
char *allpointer;

void checkFree(){
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    /*while(curr != NULL && curr->next != NULL){
        if(curr->size < 0) curr->size *= -1;
        printf("Curr size %d\n",curr->size);
        curr=curr->next;
    }
    if(curr != NULL && curr->next == NULL)
        printf("LAST free size %d\n",curr->size);*/
    return;
}

void memory_init(void *ptr, unsigned int size){
    struct arrayHead *firstHead = (struct arrayHead*) ptr;
    allpointer = (char*)firstHead;
    firstHead->size = size-sizeof(struct arrayHead)-sizeof(int);
    struct Block *freeOne = (struct Block *)(allpointer + sizeof(struct arrayHead));
    freeOne->next=NULL;
    freeOne->prior=NULL;
    freeOne->size=firstHead->size;
    firstHead->next = freeOne;
}
int memory_check(void *ptr){
    //bool valid=true;
    char* reference = (char*)ptr;
    int sizeOfMemory = ((struct arrayHead*)allpointer)->size+sizeof(struct arrayHead)-1+sizeof(int);
    //printf("given %p %p %p\n",allpointer, (allpointer + ((struct arrayHead *)allpointer)->size+ sizeof(struct arrayHead) + sizeof(int)-1), allpointer+sizeOfMemory);
    if(ptr == NULL || reference<allpointer || reference>(allpointer+sizeOfMemory)){
        //valid=false;
        //printf("---Invalid pointer. Pointing out of bounds of memory.---\n");
        return 0;
    }

    if(((struct arrayHead*)allpointer)->next != NULL) {
        struct Block *curr=((struct arrayHead*)allpointer)->next;
        struct Block *check;
        while(curr->next != NULL){
            check = (struct Block*)reference;
            if(check==curr){
                //valid=false;
                //printf("---Invalid pointer. Provided pointer pointing free (not allocated) memory.---\n");
                return 0;
            }
            curr = curr->next;
        }
    }
    //printf("Provided pointer is valid.\n");
    return 1;
}
void split(struct Block *fitting, unsigned int size){
    if(size < sizeof(struct Block)) size = BLOCKSIZE;
    //printf("Performing split from %d to %d\n",fitting->size,size);
    char *reference = (char *)fitting;
    reference = reference+sizeof(int)+size;
    struct Block *new= (struct Block *)(reference);
    struct arrayHead *startHead = (struct arrayHead *) allpointer;
    new->size=fitting->size-size-sizeof(int);
    if(fitting->prior!=NULL){           //the fitting block was not the first free
        fitting->prior->next=new;
        new->prior=fitting->prior;
    }
    else {                              //the fitting block was the first free
        new->prior=NULL;
        startHead->next=new;
    }
    if(fitting->next!=NULL){
        fitting->next->prior=new;
        new->next=fitting->next;
    }
    else {new->next=NULL;}
    fitting->prior=NULL;
    fitting->next=NULL;
    fitting->size=size * (-1);
    //printf("Request for %d B, returning %d B\n", size, fitting->size);
    checkFree();
    return;
}

void *memory_alloc(unsigned int size) {
    int memsize = size;
    if(memsize < sizeof(struct Block)) memsize = sizeof(struct Block);
    int difference = 100000;
    bool traversed = false;
    char *reference, *referenceNext;
    struct arrayHead *startHead = (struct arrayHead *) allpointer;
    struct Block *curr = startHead->next, *candidate=NULL;
    //printf("\nSize to be allocated %d\n",memsize);
    if (curr == NULL) {
        //printf("1)No fitting block found\n");
        return NULL;
    }
    while(curr != NULL){
        //printf("Checking equality curr %p curr->next %p moved %p\n", curr, curr->next, ((char*)curr) + sizeof(int) + curr->size);
        reference = (char*) curr;
        referenceNext = (char*) curr->next;
        //checkFree();
        if((reference+sizeof(int)+curr->size) == referenceNext){
            if(curr->size > 0 && curr->next->size > 0){
                curr->size += (int)sizeof(int)+curr->next->size;
                //printf("curr %d curr->next %d\n",curr->size, curr->next->size);
                if(curr->next->next!=NULL){
                    curr->next->next->prior=curr;
                    curr->next=curr->next->next;
                }else{curr->next=NULL;
                      break;}
                reference = (char*) curr;
                referenceNext = (char*) curr->next;
                if((reference+sizeof(int)+curr->size) == referenceNext){
                    continue;}
                //printf("New merged size %d\n",curr->size);
            }
        }
        if(curr->next == NULL) break;
        curr=curr->next;
    }
    traversed=false;
    curr=startHead->next;
    while(curr!=NULL){
        //printf("CHecked here.\n");
        if (curr->size == memsize){
            candidate=curr;
            difference=0;
            //printf("Exact size %d B found\n", curr->size);
            break;
        }
        if ((curr->size >= memsize + BLOCKSIZE) || (curr->size < memsize + BLOCKSIZE)){
            if((curr->size - memsize <= difference) && (curr->size - memsize > 0)){
                candidate=curr;
                difference= curr->size - memsize;
                //printf("Bigger block found, enough room for a split %d .\n",curr->size);
                curr=curr->next;
                continue;
            }
        }
        curr=curr->next;
    }
    curr=candidate;
    if(curr->size == memsize){
        //printf("Exact size %d B chosen\n", curr->size);
        if(curr->prior == NULL){
            if(curr->next == NULL){startHead->next = NULL;} //in case of allocating last free block
            else {startHead->next = curr->next;}            //in case of start head should point to another free block
        }
        else {
            if(curr->next == NULL){curr->prior->next=NULL;} //in case of the block being allocated has been the last free block and has a predecessor
            else {curr->prior->next = curr->next;}          //in case of allocated block having a predecessor and pointing to another free block on a higher address
        }
        curr->next = NULL;
        curr->prior = NULL;
        curr->size = size * (-1);
        reference = (char *) curr;
        reference += sizeof(int);
        return (void *) reference;
    }
    if(curr->size >= memsize + BLOCKSIZE){
        //printf("Bigger block chosen, enough room for a split.\n");
        split(curr, size);
        reference = (char *) curr;
        reference += sizeof(int);
        return (void *) reference;
    }
    if((curr->size < memsize + BLOCKSIZE) && (size < ((struct arrayHead*)allpointer)->size)){
        //printf("Bigger block chosen, insufficient for a split, returning full block of size %d.\n",curr->size);
        if(curr->prior == NULL){
            if(curr->next == NULL){startHead->next = NULL;} //in case of allocating last free block
            else {startHead->next = curr->next;}            //in case of start head should point to another free block
        }
        else {
            if(curr->next == NULL){curr->prior->next=NULL;} //in case of the block being allocated has been the last free block and has a predecessor
            else {curr->prior->next = curr->next;}          //in case of allocated block having a predecessor and pointing to another free block on a higher address
        }
        curr->next = NULL;
        curr->prior = NULL;
        curr->size*=(-1);
        reference = (char *) curr;
        reference += sizeof(int);
        return (void *) reference;
    }
    //printf("2)No fitting block found\n");
    return NULL;
}
int memory_free(void *valid_ptr){
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    char *reference=(char *)valid_ptr;
    reference -= sizeof(int);
    struct Block *freed= (struct Block *)reference;
    valid_ptr=NULL;
    //if(memory_check(valid_ptr)){
    if(curr == NULL){
        freed->size*= (-1); //set size even
        //printf("Freed size %d\n",freed->size);
        startHead->next=freed;
        freed->next=NULL;
        freed->prior= NULL;
        //printf("Freed, occupied became first free. Memory was full.\n");
        return 0;
    }
    if((char*)curr > reference){
        freed->size*=(-1); //set size even
        //printf("Freed size %d\n",freed->size);
        curr->prior=freed;
        startHead->next=freed;
        freed->next=curr;
        freed->prior= NULL;
        //printf("Freed, occupied became first free with another free block on a higher address.\n");
        return 0;
    }
    if((char*)curr < reference){
        while(curr->next != NULL && (char*)curr->next < reference){
            curr=curr->next;
        }
        if(curr->next == NULL && ((char*)curr < reference)){
            freed->size*=(-1); //set size even
            //printf("Freed size %d\n",freed->size);
            freed->next=NULL;
            freed->prior=curr;
            curr->next=freed;
            //printf("Freed, occupied became LAST free.\n");
            return 0;
        }
        if(curr->next != NULL && ((char*)curr < reference)){
            freed->size*=(-1); //set size even
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
    return 1;
}

// Vlastna funkcia main() je pre vase osobne testovanie. Dolezite: pri testovacich scenaroch sa nebude spustat!
int main(){
    //Pamat = *(aka*)ptr;
    char region[1000];
    printf("Sizeof arrayHead %ld Sizeof Block %ld\n",sizeof(struct arrayHead),BLOCKSIZE);
    memory_init(region,1000);
    printf("given %p %p %p\n",region, &region[999],(allpointer + ((struct arrayHead *)allpointer)->size+ sizeof(struct arrayHead) + sizeof(int)-1));
    printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    int *pointer =(int*)memory_alloc(200);
    int *pointer1 =(int*)memory_alloc(200);
    int *pointer2 =(int*)memory_alloc(200);
    int *pointer3 =(int*)memory_alloc(200);
    int *pointer4 =(int*)memory_alloc(150);
    memory_free(pointer);
    memory_free(pointer1);
    memory_free(pointer2);
    memory_free(pointer3);
    memory_free(pointer4);
    checkFree();
    int *pointer5 =(int*)memory_alloc(150);
    pointer2=(int*)memory_alloc(824);
    checkFree();
    return 0;
}

/*
#include<stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) Block{
    int size;
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
    struct arrayHead *firstHead = (struct arrayHead*) ptr;
    allpointer = (char*)firstHead;
    firstHead->size = size-sizeof(struct arrayHead)-sizeof(int);
    struct Block *freeOne = (struct Block *)(allpointer + sizeof(struct arrayHead));
    freeOne->next=NULL;
    freeOne->prior=NULL;
    freeOne->size=firstHead->size;
    firstHead->next = freeOne;
}
int memory_check(void *ptr){
    //bool valid=true;
    char* reference = (char*)ptr;
    int sizeOfMemory = ((struct arrayHead*)allpointer)->size+sizeof(struct arrayHead)-1+sizeof(int);
    //printf("given %p %p %p\n",allpointer, (allpointer + ((struct arrayHead *)allpointer)->size+ sizeof(struct arrayHead) + sizeof(int)-1), allpointer+sizeOfMemory);
    if(ptr == NULL || reference<allpointer || reference>(allpointer+sizeOfMemory)){
        //valid=false;
        //printf("---Invalid pointer. Pointing out of bounds of memory.---\n");
        return 0;
    }

    if(((struct arrayHead*)allpointer)->next != NULL) {
        struct Block *curr=((struct arrayHead*)allpointer)->next;
        struct Block *check;
        while(curr->next != NULL){
            check = (struct Block*)reference;
            if(check==curr){
                //valid=false;
                //printf("---Invalid pointer. Provided pointer pointing free (not allocated) memory.---\n");
                return 0;
            }
            curr = curr->next;
        }
    }
    //printf("Provided pointer is valid.\n");
    return 1;
}
void split(struct Block *fitting, unsigned int size){
    if(size < BLOCKSIZE) size = BLOCKSIZE;
    //printf("Performing split from %d to %d\n",fitting->size,size);
    char *reference = (char *)fitting;
    reference = reference+sizeof(int)+size;
    struct Block *new= (struct Block *)(reference);
    struct arrayHead *startHead = (struct arrayHead *) allpointer;
    new->size=fitting->size-size-sizeof(int);
    if(fitting->prior!=NULL){           //the fitting block was not the first free
        fitting->prior->next=new;
        new->prior=fitting->prior;
    }
    else {                              //the fitting block was the first free
         new->prior=NULL;
         startHead->next=new;
    }
    if(fitting->next!=NULL){
        fitting->next->prior=new;
        new->next=fitting->next;
    }
    else {new->next=NULL;}
    fitting->prior=NULL;
    fitting->next=NULL;
    fitting->size=size * (-1);
    printf("Request for %d B, returning %d B\n", size, fitting->size);
    checkFree();
    return;
}

void *memory_alloc(unsigned int size) {
    int memsize = size;
    int difference = 100000;
    bool traversed = false;
    char *reference, *referenceNext;
    struct arrayHead *startHead = (struct arrayHead *) allpointer;
    struct Block *curr = startHead->next, *candidate=NULL;
    printf("\nSize to be allocated %d\n",memsize);
    if (curr == NULL) {
        //printf("1)No fitting block found\n");
        return NULL;
    }
    while(curr != NULL){
        //printf("Checking equality curr %p curr->next %p moved %p\n", curr, curr->next, ((char*)curr) + sizeof(int) + curr->size);
        reference = (char*) curr;
        referenceNext = (char*) curr->next;
        checkFree();
        if((reference+sizeof(int)+curr->size) == referenceNext){
            if(curr->size > 0 && curr->next->size > 0){
                curr->size += sizeof(int)+curr->next->size;
                //printf("curr %d curr->next %d\n",curr->size, curr->next->size);
                if(curr->next->next!=NULL){
                    curr->next->next->prior=curr;
                    curr->next=curr->next->next;
                }else{curr->next=NULL;}
                //printf("New merged size %d\n",curr->size);
            }
        }
        //printf("Test print22\n");
        if(curr->next == NULL) break;
        curr=curr->next;
    }
    //printf("Test print23\n");
    traversed=false;
    curr=startHead->next;
    while(curr!=NULL){
        //printf("CHecked here.\n");
        if (curr->size == memsize){
            candidate=curr;
            difference=0;
            //printf("Exact size %d B found\n", curr->size);
            break;
        }
        if ((curr->size >= memsize + BLOCKSIZE) || (curr->size < memsize + BLOCKSIZE)){
            if((curr->size - memsize <= difference) && (curr->size - memsize > 0)){
                candidate=curr;
                difference= curr->size - memsize;
                //printf("Bigger block found, enough room for a split %d .\n",curr->size);
                curr=curr->next;
                continue;
            }
        }
        curr=curr->next;
    }
    curr=candidate;
    if(curr->size == memsize){
        //printf("Exact size %d B chosen\n", curr->size);
        if(curr->prior == NULL){
            if(curr->next == NULL){startHead->next = NULL;} //in case of allocating last free block
            else {startHead->next = curr->next;}            //in case of start head should point to another free block
        }
        else {
            if(curr->next == NULL){curr->prior->next=NULL;} //in case of the block being allocated has been the last free block and has a predecessor
            else {curr->prior->next = curr->next;}          //in case of allocated block having a predecessor and pointing to another free block on a higher address
        }
        curr->next = NULL;
        curr->prior = NULL;
        curr->size = size * (-1);
        reference = (char *) curr;
        reference += sizeof(int);
        return (void *) reference;
    }
    if(curr->size >= memsize + BLOCKSIZE){
        //printf("Bigger block chosen, enough room for a split.\n");
        split(curr, size);
        reference = (char *) curr;
        reference += sizeof(int);
        return (void *) reference;
    }
    if((curr->size < memsize + BLOCKSIZE) && (size < ((struct arrayHead*)allpointer)->size)){
        //printf("Bigger block chosen, insufficient for a split, returning full block of size %d.\n",curr->size);
        if(curr->prior == NULL){
            if(curr->next == NULL){startHead->next = NULL;} //in case of allocating last free block
            else {startHead->next = curr->next;}            //in case of start head should point to another free block
        }
        else {
            if(curr->next == NULL){curr->prior->next=NULL;} //in case of the block being allocated has been the last free block and has a predecessor
            else {curr->prior->next = curr->next;}          //in case of allocated block having a predecessor and pointing to another free block on a higher address
        }
        curr->next = NULL;
        curr->prior = NULL;
        curr->size*=(-1);
        reference = (char *) curr;
        reference += sizeof(int);
        return (void *) reference;
    }
    //printf("2)No fitting block found\n");
    return NULL;
}
int memory_free(void *valid_ptr){
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    char *reference=(char *)valid_ptr;
    reference -= sizeof(int);
    struct Block *freed= (struct Block *)reference;
    valid_ptr=NULL;
    //if(memory_check(valid_ptr)){
        if(curr == NULL){
            freed->size*= (-1); //set size even
            //printf("Freed size %d\n",freed->size);
            startHead->next=freed;
            freed->next=NULL;
            freed->prior= NULL;
            //printf("Freed, occupied became first free. Memory was full.\n");
            return 0;
        }
        if((char*)curr > reference){
            freed->size*=(-1); //set size even
            //printf("Freed size %d\n",freed->size);
            curr->prior=freed;
            startHead->next=freed;
            freed->next=curr;
            freed->prior= NULL;
            //printf("Freed, occupied became first free with another free block on a higher address.\n");
            return 0;
        }
        if((char*)curr < reference){
            while(curr->next != NULL && (char*)curr->next < reference){
                curr=curr->next;
            }
            if(curr->next == NULL && ((char*)curr < reference)){
                freed->size*=(-1); //set size even
                //printf("Freed size %d\n",freed->size);
                freed->next=NULL;
                freed->prior=curr;
                curr->next=freed;
                //printf("Freed, occupied became LAST free.\n");
                return 0;
            }
            if(curr->next != NULL && ((char*)curr < reference)){
                freed->size*=(-1); //set size even
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
    return 1;
}
void checkFree(){
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct Block *curr = startHead->next;
    while(curr != NULL && curr->next != NULL){
        if(curr->size < 0) curr->size *= -1;
        printf("Curr size %d\n",curr->size);
        curr=curr->next;
    }
    if(curr != NULL && curr->next == NULL)
        printf("LAST free size %d\n",curr->size);
}
// Vlastna funkcia main() je pre vase osobne testovanie. Dolezite: pri testovacich scenaroch sa nebude spustat!
int main(){
    //Pamat = *(aka*)ptr;
    char region[1000];
    printf("Sizeof arrayHead %ld Sizeof Block %ld\n",sizeof(struct arrayHead),BLOCKSIZE);
    memory_init(region,1000);
    printf("given %p %p %p\n",region, &region[999],(allpointer + ((struct arrayHead *)allpointer)->size+ sizeof(struct arrayHead) + sizeof(int)-1));
    printf("First free size %d\n",((struct Block*) allpointer)->next->size);
    int *pointer =(int*)memory_alloc(200);
    int *pointer1 =(int*)memory_alloc(200);
    int *pointer2 =(int*)memory_alloc(200);
    int *pointer3 =(int*)memory_alloc(200);
    int *pointer4 =(int*)memory_alloc(150);
    memory_free(pointer);
    memory_free(pointer1);
    memory_free(pointer2);
    memory_free(pointer3);
    memory_free(pointer4);
    checkFree();
    int *pointer5 =(int*)memory_alloc(150);
    pointer2=(int*)memory_alloc(824);
    checkFree();
    return 0;
}
*/