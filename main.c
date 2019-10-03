#include<stdio.h>
#include <string.h>
#include <stdbool.h>
#define BYTECOUNT 1000

typedef struct freeBlock{
    unsigned int size;
    struct freeBlock *next;
    struct freeBlock *prior;
};

typedef struct occupiedBlock{
    unsigned int size;
};

typedef struct footer{
    unsigned int size;
}footer;

typedef struct arrayHead{
    unsigned int size;
    struct freeBlock *next;
}arrayHead;

#define BLOCKSIZE sizeof(struct block)
char *allpointer;
void memory_init(void *ptr, unsigned int size){ //Attempt without struct
    struct arrayHead *firstHead = ptr;
    allpointer = (char*)firstHead;

    firstHead->size = size-sizeof(struct arrayHead)-sizeof(struct footer);
    struct footer *arrayFooter = (char *)(firstHead)+size-1-sizeof(struct footer);
    arrayFooter->size = size-sizeof(struct arrayHead)-sizeof(struct footer);
    struct freeBlock *freeOne = (char *)(firstHead)+sizeof(struct arrayHead);
    freeOne->next=NULL;
    freeOne->prior=(struct freeBlock *)firstHead;
    firstHead->next = freeOne;
}
int memory_check(void *ptr);
void *split(char *fitting, unsigned int size){
    int oldsize = *(int*)(fitting - 2*sizeof(char*) - sizeof(int));
    char *linkOriginalRest = fitting - 2*sizeof(char*);
    char *reference = fitting + oldsize;                    //move reference to the footer of fitting block
    *(int*)(reference) = oldsize - size - 2*sizeof(int) - 2*sizeof(char*);  //write size of rest block in footer

    reference = fitting + size;                             //move reference to the beginning of size of allocated block
    *(int*)(reference) = size;                              //write size of allocated block

    reference += sizeof(int);                                      //move reference to size of rest block
    *(int*)(reference) = oldsize - size - 2*sizeof(int) - 2*sizeof(char*);  //write size of rest block

    reference = reference + sizeof(int) + 2*sizeof(char*);         //move reference to beginning of rest block
    linkOriginalRest = *(char**)(linkOriginalRest);                //move link pointer to next of original
    if(linkOriginalRest != NULL){
        *(char**)(linkOriginalRest) = reference;                                  //move reference to prior pointer
    }
    //*(char**)(reference) = *(char**)(linkOriginalRest+sizeof(char*));              //point rest prior to previous
}

void *memory_alloc(unsigned int size){
    if(size % 2 == 1) size++;
    bool flag = false;
    struct arrayHead *startHead = (struct arrayHead*)allpointer;
    struct freeBlock *curr = startHead->next, *prior;
    while(curr->next != NULL && curr->size < size){
        prior=curr;
        curr = curr->next;
    }
    if(curr->size >= size){
        printf("Block of exact size found\n");
        if(curr->prior != NULL)
            curr->prior->next = curr->next;
        if(curr->next != NULL)
            curr->next->prior = curr->prior;
        if(curr->size%2 == 0) curr->size++;
        flag = 1;
        char *temp = (char *)curr;
        temp -= sizeof(struct freeBlock);
        temp += sizeof(struct occupiedBlock);
        curr = (struct occupiedBlock*) temp;
        return (void*)(curr);
    }
    /*if(curr->size > size){
        printf("Larger block found, performing split...\n");
        flag = 1;
    }*/
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
    printf("Sizeof occupiedBlock %d\nSizeof arrayHead %d\nSizeof freeBlock %d\n",sizeof(struct occupiedBlock),sizeof(struct arrayHead),sizeof(struct freeBlock));
    printf("given %p\n",region);
    memory_init(region,BYTECOUNT);
    int *pointer =(int*)memory_alloc(100);
    printf("received %p\n",pointer);
    struct occupiedBlock *curr = (void*)pointer;
    char *temp = (char*)curr;
    temp -= sizeof(struct occupiedBlock);
    printf("moved %p\n",temp);
    curr = (struct occupiedBlock*)temp;
    printf("%d\n",curr->size);
    return 0;
}