#include<stdio.h>
#include <string.h>
#define BYTECOUNT 1000

typedef struct block{
    int size;
    struct block *next;
};
struct block *head;
#define BLOCKSIZE sizeof(struct block)

void memory_init(void *ptr, unsigned int size){
    head=(void*)ptr;
    head->size = size - BLOCKSIZE;
    head->next = NULL;
}
void *split(struct block *current, struct block *prior, unsigned int size){
    struct block *new = (current+size+BLOCKSIZE);
    new->size=current->size - size - BLOCKSIZE;
    new->next=current->next;
    prior->next=new;
    current->size=size;
    current->next=NULL;
    return (void*)current;
}
void *memory_alloc(unsigned int size){
    struct block *curr, *prior;
    void *result;
    curr = head;

    while(curr->next!=NULL && (curr->size)<size){
        prior = curr;
        curr = curr->next;
    }
    if(curr->size == size){
        printf("Block of exact size found\n");
        prior->next = curr->next;
        curr->next=NULL;
        result = (void *)curr;
        return result;
    }
    if((curr->size)<size){
        printf("Smaller block found, performing split...\n");
        result = split(curr,prior,size);
        return result;
    }
    printf("Curr found a suitable block of size: %d\n",curr->size);
}
int memory_free(void *valid_ptr){
    struct block *curr, *prior;
    prior = head;
    curr=valid_ptr;
    while(prior->next!=NULL && (prior < curr))
        prior = prior->next;
    curr->next=prior->next;
    prior->next=curr;
}
int main(){

    char region[BYTECOUNT];
    memory_init(region,BYTECOUNT*sizeof(char));
    char *pointer1=(char*)memory_alloc(984);
    printf("%ld\n", sizeof(struct block));
    return 0;
}

/*

malloc.c
void initialize(){
 freeList->size=20000-sizeof(struct block);
 freeList->free=1;
 freeList->next=NULL;
}

void split(struct block *fitting_slot,size_t size){
 struct block *new=(void*)((void*)fitting_slot+size+sizeof(struct block));
 new->size=(fitting_slot->size)-size-sizeof(struct block);
 new->free=1;
 new->next=fitting_slot->next;
 fitting_slot->size=size;
 fitting_slot->free=0;
 fitting_slot->next=new;
}


void *MyMalloc(size_t noOfBytes){
 struct block *curr,*prev;
 void *result;
 if(!(freeList->size)){
  initialize();
  printf("Memory initialized\n");
 }
 curr=freeList;
 while((((curr->size)<noOfBytes)||((curr->free)==0))&&(curr->next!=NULL)){
  prev=curr;
  curr=curr->next;
  printf("One block checked\n");
 }
 if((curr->size)==noOfBytes){
  curr->free=0;
  result=(void*)(++curr);
  printf("Exact fitting block allocated\n");
  return result;
 }
 else if((curr->size)>(noOfBytes+sizeof(struct block))){
  split(curr,noOfBytes);
  result=(void*)(++curr);
  printf("Fitting block allocated with a split\n");
  return result;
 }
 else{
  result=NULL;
  printf("Sorry. No sufficient memory to allocate\n");
  return result;
 }
}

void merge(){
 struct block *curr,*prev;
 curr=freeList;
 while((curr->next)!=NULL){
  if((curr->free) && (curr->next->free)){
   curr->size+=(curr->next->size)+sizeof(struct block);
   curr->next=curr->next->next;
  }
  prev=curr;
  curr=curr->next;
 }
}

void MyFree(void* ptr){
 if(((void*)memory<=ptr)&&(ptr<=(void*)(memory+20000))){
  struct block* curr=ptr;
  --curr;
  curr->free=1;
  merge();
 }
 else printf("Please provide a valid pointer allocated by MyMalloc\n");
}



malloc.h


#include<stdio.h>
#include<stddef.h>

char memory[20000];

struct block{
 size_t size;
 int free;
 struct block *next;
};

struct block *freeList=(void*)memory;

void initialize();
void split(struct block *fitting_slot,size_t size);
void *MyMalloc(size_t noOfBytes);
void merge();
void MyFree(void* ptr);



Main.c


#include<stdio.h>

int main(){

 int *p=(int)MyMalloc(100*sizeof(int));
 char *q=(char)MyMalloc(250*sizeof(char));
 int *r=(int)MyMalloc(1000*sizeof(int));
 MyFree(p);
 char *w=(char)MyMalloc(700);
 MyFree(r);
 int *k=(int)MyMalloc(500*sizeof(int));
 printf("Allocation and deallocation is done successfully!");

}


 */