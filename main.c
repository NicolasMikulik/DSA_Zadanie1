#include<stdio.h>
#include <string.h>
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
    struct freeBlock *firstFree;
}arrayHead;

#define BLOCKSIZE sizeof(struct block)
char *allpointer;
void memory_init(void *ptr, unsigned int size){ //Attempt without struct
    struct arrayHead *firstHead = ptr;
    allpointer = firstHead;
    firstHead->size = size-sizeof(struct arrayHead)-sizeof(struct footer);
    struct footer *arrayFooter = (firstHead+size-1-sizeof(struct footer));
    arrayFooter->size = size-sizeof(struct arrayHead)-sizeof(struct footer);
    struct freeBlock *freeOne = (firstHead+sizeof(struct arrayHead));
    firstHead->firstFree = freeOne;
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
    char *pointer = *(char**)(allpointer+ sizeof(int));
    while(*(int*)(pointer - (2*sizeof(char*) + sizeof(int))) < size){
        pointer = *(char**)(pointer-2*sizeof(char*));
    }
    if(*(int*)(pointer - (2*sizeof(char*) + sizeof(int))) == size){
        printf("Exact size block found\n");
        *(char**)(pointer-sizeof(int*))=NULL;
        *(char**)(pointer-2*sizeof(int*))=NULL;
        *(int*)(pointer - (2*sizeof(char*) + sizeof(int))) += 1;
        pointer = pointer - 2*sizeof(int*);
        return (void*)pointer;
    }
    if(*(int*)(pointer - (2*sizeof(char*) + sizeof(int))) > size){
        if(*(int*)(pointer - (2*sizeof(char*) + sizeof(int))) - size >= (2*sizeof(int)+2*sizeof(char*))){
            split(pointer,size);
        }
        else{
            printf("Larger block found, but not large enough for a split\n");
            *(char**)(pointer-sizeof(int*))=NULL;
            *(char**)(pointer-2*sizeof(int*))=NULL;
            *(int*)(pointer - (2*sizeof(char*) + sizeof(int))) += 1;
            pointer = pointer - 2*sizeof(int*);
            return (void*)pointer;
        }
    }
    printf("Free block size %d\n",*(int*)(pointer - (2*sizeof(char*) + sizeof(int))));
    return NULL;
}
int memory_free(void *valid_ptr){
    struct block *curr, *prior;
    prior = head;
    curr=valid_ptr;
    while(prior->next!=NULL && (prior->next < curr))
        prior = prior->next;
    curr->next=prior->next;
    prior->next=curr;
}
int main(){
    //Pamat = *(aka*)ptr;
    char region[BYTECOUNT];

    for (int i = 0; i < BYTECOUNT; i++) {
        region[i] = 'a';
    }
    region[20] = 'X';
    *(char**)(region + 5) = &region[20];
    //region[5] = 999;
    //printf("Cislo: %d == %d\n\n\n\n\n", &region[20],*(*(char**)(region + 5)));
    memory_init(region,BYTECOUNT*sizeof(char));
    printf("Region[0] %d\nRegion[996] %d\n", *(int*)(region),*(int*)(region+BYTECOUNT-sizeof(int)));
    printf("Size of first free block %d\n",*(int*)(allpointer+12));
    memory_alloc(10);
    return 0;
}