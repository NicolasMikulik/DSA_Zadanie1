#include<stdio.h>
#include <string.h>
#define BYTECOUNT 1000

typedef struct block{
    unsigned int size;
    struct block *next;
}block;
struct block *head;
#define BLOCKSIZE sizeof(struct block)
char *allpointer;
void memory_init(void *ptr, unsigned int size){ //Attempt without struct
    //char allpointer = ptr;
    allpointer = ptr;
    *(int*)(allpointer) = size-sizeof(int) /*head of array*/ - sizeof(int) /*footer of array*/-sizeof(int*); //the first four elements contain size of input array
    *(char**)(allpointer+ sizeof(int)) = (allpointer + 2*sizeof(int) + 3*sizeof(int*));                                                         //pointer to the first free
    *(int*)(allpointer + *(int*)(allpointer) + sizeof(int) +sizeof(int*)) = *(int*)(allpointer);                            //writing the size of input array into array footer
    printf("%d\n",*(int*)(allpointer+996));

    *(int*)(allpointer+sizeof(int)+sizeof(int*)) = *(int*)(allpointer) - 2*sizeof(int);             //first free block size
    char *reference = allpointer+sizeof(int)+sizeof(int*);
    *(char**)(reference+sizeof(int)) = NULL;                                                         //pointer to the next free block
    *(char**)(reference+sizeof(int)+sizeof(int*)) = allpointer;
}
int memory_check(void *ptr){
    struct block *curr = head, *prior;
    struct block *check = ptr;
    if(check<head || check>(head+head->size)){
        printf("Pointer invalid, out of bounds.\n");
        return 0;
    }
    while(curr->next!=NULL && (curr < head+head->size)){
        if(curr == check){
            printf("-----ERROR-Allocated pointer found between free memory blocks!----\n");
        }
        prior = curr;
        curr = curr->next;
    }
}
void *split(char *fitting, unsigned int size){
    int oldsize = *(int*)(fitting - 2*sizeof(char*) - sizeof(int));
    char *linkOriginalRest = fitting - 2*sizeof(char*);
    char *reference = fitting + oldsize;                    //move reference to the footer of fitting block
    *(int*)(reference) = oldsize - size - 2*sizeof(int) - 2*sizeof(char*);  //write size of rest block in footer

    reference = fitting + size;                             //move reference to the beginning of size of allocated block
    *(int*)(reference) = size;                              //write size of allocated block

    reference += sizeof(int);                                      //move reference to size of rest block
    *(int*)(reference) = oldsize - size - 2*sizeof(int) - 2*sizeof(char*);  //write size of rest block

    reference = reference + sizeof(int) + 2*sizeof(char*);
    linkOriginalRest = *(char**)(linkOriginalRest);                    //point the next pointer of original free block to the rest block
    *(char**)(linkOriginalRest) = reference;                                  //move reference to prior pointer
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