#include<stdio.h>
#include <string.h>
#define BYTECOUNT 1000

typedef struct block{
    unsigned int size;
    struct block *next;
}block;
struct block *head;
#define BLOCKSIZE sizeof(struct block)

void memory_init(void *ptr, unsigned int size){ //Attempt without struct
    *(int*)(ptr) = size-sizeof(int) /*head of array*/ - sizeof(int) /*footer of array*/; //the first four elements contain size of input array
    *(int**)(ptr+4) = (ptr+12);                                                         //pointer to the first free
    *(int*)(ptr + *(int*)(ptr) + sizeof(int)) = *(int*)(ptr);                            //writing the size of input array into array footer



    //head->size = size;
    //head->next = NULL;
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
void *split(struct block *current, struct block *prior, unsigned int size){
    struct block *new = (current+size+BLOCKSIZE);
    printf("Test print\n");
    new->size=current->size - size - BLOCKSIZE;
    printf("Test print two\n");
    new->next=current->next;
    prior->next=new;
    current->size=size;
    current->next=NULL;
    return (void*)current;
}
void *memory_alloc(unsigned int size){
    struct block *curr, *prior;
    void *result;
    int flag = 0;
    curr = head;

    while(curr->next!=NULL && (curr->size)<size){
        prior = curr;
        curr = curr->next;
    }
    if(curr->size == size){
        flag = 1;
        printf("Block of exact size found\n");
        prior->next = curr->next;
        curr->next=NULL;
        result = (void *)curr;
        return result;
    }
    if((curr->size)>size){
        flag = 1;
        printf("Larger block found, performing split...\n");
        result = split(curr,prior,size);
        return result;
    }
    if(!flag){
        printf("No sufficient memory found.\n");
        return NULL;
    }
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
    region[500]=(char)2048;

    for (int i = 0; i < BYTECOUNT; i++) {
        region[i] = 'a';
    }

    region[20] = 'X';

    *(char**)(region + 5) = &region[20];
//    region[5] = 999;

    //printf("Cislo: %d == %d\n\n\n\n\n", &region[20],*(*(char**)(region + 5)));

    memory_init(region,BYTECOUNT*sizeof(char));
    region[12] = 'B';
    printf("Region[0] %d\nRegion[996] %d\nRegion[4] points to the first free block %c", *(int*)(region),*(int*)(region+BYTECOUNT-sizeof(int)),*(*(char**)(region+4)));
    // char *pointer1=(char*)memory_alloc(988);
    return 0;
}