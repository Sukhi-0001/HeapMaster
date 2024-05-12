#include <stdio.h>
#include <memory.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mm.h"

size_t SYSTEM_PAGE_SIZE=0;
struct vm_page_for_faimlies_t* first_vm_page_for_faimlies=NULL;

void vm_priority_queue_insert(struct vm_page_faimly_t* pg_faimly,struct vm_priority_queue_node_t* node_to_be_inserted){
    //struct vm_priority_queue_node_t** head=&(pg_faimly->free_block_list_head);
    if(pg_faimly->free_block_list_head==NULL){
        pg_faimly->free_block_list_head=node_to_be_inserted;
        return;
    }
        
    struct vm_priority_queue_node_t* prev;
    struct vm_priority_queue_node_t* current=pg_faimly->free_block_list_head;
   
   if(VM_QUEUE_NODE_BLOCK_SIZE(current)<=VM_QUEUE_NODE_BLOCK_SIZE(node_to_be_inserted)){
        node_to_be_inserted->next_node=pg_faimly->free_block_list_head;
        pg_faimly->free_block_list_head=node_to_be_inserted;
        return;
   }
   while (current!=NULL)
   {    if(VM_QUEUE_NODE_BLOCK_SIZE(current)<VM_QUEUE_NODE_BLOCK_SIZE(node_to_be_inserted))
            break;
        prev=current;
        current=current->next_node;
   }
   node_to_be_inserted->next_node=prev->next_node;
   prev->next_node=node_to_be_inserted;
}

void vm_priority_queue_delete(struct vm_page_faimly_t* pg_faimly,struct vm_priority_queue_node_t* node_to_be_deleted){
    struct vm_priority_queue_node_t** head=&(pg_faimly->free_block_list_head);
    if(*head==node_to_be_deleted){
        *head=node_to_be_deleted->next_node;
        node_to_be_deleted->next_node=NULL;
        return;
    }
    struct vm_priority_queue_node_t* prev;
    struct vm_priority_queue_node_t* current=*head;

    while (current!=NULL)
   {    if(current==node_to_be_deleted)
            break;
        prev=current;
        current=current->next_node;
   }
   prev->next_node=node_to_be_deleted->next_node;
   node_to_be_deleted->next_node=NULL;
}


void*  mm_get_vm_page_form_kernel(uint32_t uints){
    void* vm_page=mmap(0,
    uints*SYSTEM_PAGE_SIZE,
    PROT_EXEC|PROT_READ|PROT_WRITE,
    MAP_PRIVATE|MAP_ANON,
    0,
    0);

    if(vm_page==MAP_FAILED){
        return NULL;
    }
    memset(vm_page,0,uints*SYSTEM_PAGE_SIZE);
    return vm_page;
}

void mm_return_vm_to_kernel(void* vm_page,uint32_t uints){
   int res=munmap(vm_page,uints*SYSTEM_PAGE_SIZE);
   if(res!=0)
        PRINT_S("ERROR: Unable to Free munmap to Kernel");
}

void mm_init(){
    SYSTEM_PAGE_SIZE=getpagesize();
    first_vm_page_for_faimlies=mm_get_vm_page_form_kernel(1);
}

//NOTE- Condition to be added if user try to allocate same struct again
void mm_instantiate_new_page_faimly(char* struct_name,uint32_t struct_size){
    //if size of memory requested bigger than page size
    if(struct_size>SYSTEM_PAGE_SIZE){
        printf("ERROR CANNOT ALLOCATE PAGE AS STRUCT SIZE (%d) IS BIGGER THAN PAGE(%d) \n",struct_size,SYSTEM_PAGE_SIZE);
        return;
    }

    //if space is not there in current new_page_faimly
    // allocate page
    if(first_vm_page_for_faimlies->count_of_faimlies_present==MAX_FAIMLIES_PER_PAGE){
        struct vm_page_for_faimlies_t* temp_addr=mm_get_vm_page_form_kernel(1);
        temp_addr->count_of_faimlies_present=0;
        temp_addr->next=first_vm_page_for_faimlies;
        first_vm_page_for_faimlies=temp_addr;
    }

    // till here there should be space in first page for page_faimly
    struct vm_page_faimly_t *current;
    ITERATE_PAGE_FAIMLIES_BEGIN(first_vm_page_for_faimlies,current)
    ITERATE_PAGE_FAIMLIES_END(first_vm_page_for_faimlies,current);
    strcpy(current->struct_name,struct_name);
    current->struct_size=struct_size;
    current->free_block_list_head=NULL;
    current->first_page=NULL;
    first_vm_page_for_faimlies->count_of_faimlies_present++;
    
}

VM_BOOL_T mm_split_free_data_block_for_allocation(struct vm_page_faimly_t* pg_faimly,struct block_meta_data_t* meta_block_to_split,uint32_t mm_requierd){
    struct block_meta_data_t* next_meta_block=NULL;
    assert(meta_block_to_split->is_free==MM_TRUE);
    if(meta_block_to_split->block_size < mm_requierd)
        return MM_FLASE;
    // remaing size of memory
    uint32_t remaining_size=meta_block_to_split->block_size - mm_requierd;

    meta_block_to_split->block_size=mm_requierd;
    meta_block_to_split->is_free=MM_FLASE;
    vm_priority_queue_delete(pg_faimly,&meta_block_to_split->vm_priority_queue_node);

    //case 1 no split or hard internal fragmentation
    if(remaining_size==0 || remaining_size < sizeof(struct block_meta_data_t))
        return MM_TRUE;
    
    //case 2
    //partial split soft internal fragmentation or full split
    else if(sizeof(struct block_meta_data_t) < remaining_size && 
    remaining_size < (sizeof(struct block_meta_data_t)+pg_faimly->struct_size)){
        next_meta_block=MM_NEXT_META_BLOCK_BY_SIZE(meta_block_to_split);
        next_meta_block->is_free=MM_FLASE;
        next_meta_block->block_size=remaining_size-sizeof(struct block_meta_data_t);
        next_meta_block->offset=meta_block_to_split->offset+meta_block_to_split->block_size
        +sizeof(struct block_meta_data_t);
        vm_priority_queue_insert(pg_faimly,&next_meta_block->vm_priority_queue_node);
        //adjust pointer
        next_meta_block->next_block=meta_block_to_split->next_block;
       if(meta_block_to_split->next_block!=NULL)
            meta_block_to_split->next_block->prev_block=next_meta_block;
        next_meta_block->prev_block=meta_block_to_split;
        meta_block_to_split->next_block=next_meta_block;
    }

}



void print_vm_priority_queue(struct vm_priority_queue_node_t* head){
     while (head!=NULL)
    {   
        PRINT_I(head->meta_block_ptr->block_size);
        head=head->next_node;
    }
}



// it assume block which is passed has enough free space for data
struct block_meta_data_t* vm_split_block_for_use(struct block_meta_data_t* meta_block_to_split,uint32_t mm_requierd){
    // first check if there is space for new meta_block + 4 bytes + mm_req
    //then only create next meta block and insert it between
    uint32_t temp_block_size=meta_block_to_split->block_size;
    struct block_meta_data_t* next_meta_block;
     struct vm_page_t* vm_page=(struct vm_page_t*)((char*)meta_block_to_split-meta_block_to_split->offset);
    if((meta_block_to_split->block_size)<mm_requierd)
        return 0;

    if((meta_block_to_split->block_size)>TOTAL_MEMORY_TO_ALLOCATE(mm_requierd)){
       //get next meta block
       meta_block_to_split->block_size=mm_requierd;
       // initializi all files of next meta block
       next_meta_block=MM_NEXT_META_BLOCK_BY_SIZE(meta_block_to_split);
       next_meta_block->is_free=MM_TRUE;
       next_meta_block->vm_priority_queue_node.meta_block_ptr=next_meta_block;
       next_meta_block->vm_priority_queue_node.next_node=NULL;
       next_meta_block->offset=meta_block_to_split->offset+mm_requierd+sizeof(struct block_meta_data_t);
       next_meta_block->block_size=temp_block_size-mm_requierd-sizeof(struct block_meta_data_t);
       //insert free block in meta block list
       next_meta_block->next_block=meta_block_to_split->next_block;
       if(meta_block_to_split->next_block!=NULL)
            meta_block_to_split->next_block->prev_block=next_meta_block;
        next_meta_block->prev_block=meta_block_to_split;
        meta_block_to_split->next_block=next_meta_block;
        //insert new meta block in free queue
       
        vm_priority_queue_insert(vm_page->pg_faimly,&next_meta_block->vm_priority_queue_node);
    }
    meta_block_to_split->is_free=MM_FLASE;
    // delete from priority queue
    vm_priority_queue_delete(vm_page->pg_faimly,&meta_block_to_split->vm_priority_queue_node);
    return meta_block_to_split;
}

void vm_merge_block(struct block_meta_data_t* first,struct block_meta_data_t* second){
    assert(first->is_free==MM_TRUE && second->is_free==MM_TRUE);
    first->block_size+=sizeof(struct block_meta_data_t)+second->block_size;
    first->next_block=second->next_block;

    if(second->next_block!=NULL)
        second->next_block->prev_block=first;
}


VM_BOOL_T mm_is_vm_page_empty(struct vm_page_t* vm_page){
    if(vm_page->block_meta_data.next_block==NULL &&
        vm_page->block_meta_data.prev_block==NULL &&
        vm_page->block_meta_data.is_free==MM_TRUE)
        return MM_TRUE;
    return MM_FLASE;
}

uint32_t mm_max_page_allocatable_memory(int uints){
    return (uint32_t)((SYSTEM_PAGE_SIZE*uints)-sizeof(struct vm_page_t));
}


struct vm_page_t* allocate_vm_page(struct vm_page_faimly_t* vm_page_faimly){
    struct vm_page_t* new_vm_page=mm_get_vm_page_form_kernel(1);
    // make new page empty
    MARK_VM_PAGE_EMPTY(new_vm_page);
    //set offset

    new_vm_page->next_page=NULL;
    new_vm_page->prev_page=NULL;

    new_vm_page->block_meta_data.vm_priority_queue_node.meta_block_ptr=&(new_vm_page->block_meta_data);
    new_vm_page->block_meta_data.vm_priority_queue_node.next_node=NULL;

    new_vm_page->block_meta_data.block_size=mm_max_page_allocatable_memory(1);

    new_vm_page->block_meta_data.offset=OFFSET_OF(new_vm_page,&new_vm_page->block_meta_data);
    //insert this new block is priority queue
    vm_priority_queue_insert(vm_page_faimly,&new_vm_page->block_meta_data.vm_priority_queue_node);
  //  vm_priority_queue_insert(vm_page_faimly,&new_vm_page->block_meta_data.vm_priority_queue_node);
    //set back pointer
    new_vm_page->pg_faimly=vm_page_faimly;

    //if first page is empty
    if(vm_page_faimly->first_page==NULL){
        vm_page_faimly->first_page=new_vm_page;
        return new_vm_page;
    }
    //insert new vm page in head
    new_vm_page->next_page=vm_page_faimly->first_page;
    vm_page_faimly->first_page=new_vm_page;
    new_vm_page->next_page->prev_page=new_vm_page;
    return new_vm_page;
}

void mm_vm_page_delete_and_free(struct vm_page_t* vm_page){
    struct vm_page_faimly_t* vm_page_faimly=vm_page->pg_faimly;

    //if page is first
    if(vm_page==vm_page_faimly->first_page){
        vm_page_faimly->first_page=vm_page->next_page;
        if(vm_page->next_page!=NULL)
            vm_page->next_page->prev_page=NULL;
        vm_page->next_page=NULL;
        vm_page->prev_page=NULL;
        mm_return_vm_to_kernel((void *)vm_page,1);
        return;
    }
   //if page is not the first
   if(vm_page->next_page!=NULL)
        vm_page->next_page->prev_page=vm_page->prev_page;
    vm_page->prev_page->next_page=vm_page->next_page;
    mm_return_vm_to_kernel((void *)vm_page,1);
}   

struct vm_page_faimly_t* lookup_page_faimly_by_name(char* struct_name){
    struct vm_page_faimly_t* current_faimly;
    struct vm_page_for_faimlies_t* current_vm_page_for_faimly;

    ITERATE_VM_PAGE_FOR_FAIMLIES_BEGIN(first_vm_page_for_faimlies,current_vm_page_for_faimly)
        ITERATE_PAGE_FAIMLIES_BEGIN(first_vm_page_for_faimlies,current_faimly)
            if(strcmp(struct_name,current_faimly->struct_name)==0)
                return current_faimly;
        ITERATE_PAGE_FAIMLIES_END(first_vm_page_for_faimlies,current_faimly);
    ITERATE_VM_PAGE_FOR_FAIMLIES_END(first_vm_page_for_faimlies,current_vm_page_for_faimly);
    return NULL;
}

struct block_meta_data_t* mm_get_biggest_free_block_page_faimly(struct vm_page_faimly_t* pg_faimly){
    if(pg_faimly->free_block_list_head==NULL)
        return NULL;
    return pg_faimly->free_block_list_head->meta_block_ptr;
}

struct block_meta_data_t* mm_allocate_free_memory_block(struct vm_page_faimly_t* pg_faimly,uint32_t req_size){
    struct block_meta_data_t* biggest_meta_block=mm_get_biggest_free_block_page_faimly(pg_faimly);
    
    struct vm_page_t* new_vm_page=NULL;
    // add a new page to vm_page
    if(biggest_meta_block==NULL || biggest_meta_block->block_size<req_size){
        new_vm_page=allocate_vm_page(pg_faimly);
    }
    // from here there should be enough space in new_vm_page
    biggest_meta_block=mm_get_biggest_free_block_page_faimly(pg_faimly);
   // vm_priority_queue_delete(pg_faimly,biggest_meta_block);
     struct block_meta_data_t* temp_meta=vm_split_block_for_use(biggest_meta_block,req_size);
     return temp_meta!=NULL?temp_meta:NULL;
}

void* xmalloc(char* struct_name,int uints){
    struct vm_page_faimly_t* pg_faimly=lookup_page_faimly_by_name(struct_name);
        if(pg_faimly==NULL)
            return NULL;
        if(uints*pg_faimly->struct_size>=mm_max_page_allocatable_memory(1)){
            printf("ERROR : CANNOT ALLOCATE THIS MANY UNITS IN SEQUENCE AS IT CANT FIT IN SINGLE PAGE \n");
            return NULL;
        }
        struct block_meta_data_t* free_block=mm_allocate_free_memory_block(pg_faimly,uints*pg_faimly->struct_size);
        
        if(free_block!=NULL){
            memset((char*)(free_block+1),0,uints*pg_faimly->struct_size);
            return (void*)(free_block+1);
        }
        return NULL;
}

void xfree(void* free_addr){
    struct block_meta_data_t* final_meta_block,*meta_to_free=(struct block_meta_data_t*)free_addr-1;
    meta_to_free->is_free=MM_TRUE;
    final_meta_block=meta_to_free;
    //first see next block is there
    if(MM_NEXT_META_BLOCK(meta_to_free) != NULL){
        //check if next block is free if it is free merge these blocks
        if(MM_NEXT_META_BLOCK(meta_to_free)->is_free==MM_TRUE)
        vm_merge_block(meta_to_free,meta_to_free->next_block);    
    }
    
    //check if there is prev free block
    if(meta_to_free->prev_block!=NULL){
        if(MM_PREV_META_BLOCK(meta_to_free)->is_free==MM_TRUE){
        final_meta_block=MM_PREV_META_BLOCK(meta_to_free);
        vm_merge_block(meta_to_free->prev_block,meta_to_free);
         //meta_to_free is obsolute this var should not be used
        }
    }

    //check if whole page is empty
    struct vm_page_t* vm_page=(void*)((char*)final_meta_block-final_meta_block->offset);
    
    if(mm_is_vm_page_empty(vm_page)==MM_TRUE){
        mm_vm_page_delete_and_free(vm_page);
        return;
    }
    vm_priority_queue_insert(vm_page->pg_faimly,&final_meta_block->vm_priority_queue_node);
}

void mm_print_memory_usage(char *struct_name){
    struct vm_page_faimly_t* pg_faimly=lookup_page_faimly_by_name(struct_name);
    if(pg_faimly==NULL){
        printf("ERROR\n");
        return;
    }
    printf("\n");
    printf("PAGE FAIMLY: %s, SIZE: %u \n",pg_faimly->struct_name,pg_faimly->struct_size);
    struct vm_page_t* current_vm_page;
    struct block_meta_data_t* current_meta_block;
    int block_count=0,page_count=0;
    char* temp_print;
    ITERATE_VM_PAGE_BEGIN(pg_faimly,current_vm_page)
        printf("%p   PAGE NO: %i \n",(void*)current_vm_page,page_count);
        printf("Next_PAGE: %p , PREV_PAGE: %p \n",(void*)current_vm_page->next_page,(void*)current_vm_page->prev_page);
        block_count=0;
        ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(current_vm_page,current_meta_block)
            temp_print=current_meta_block->is_free==MM_FLASE ? "ALLOCATED" : "FREE     ";
            printf("%p  BLOCK %i   %s block_size=%u  offset=%u   prev_block=%p   next_block=%p \n",
            (void*)current_meta_block,block_count,temp_print,current_meta_block->block_size,current_meta_block->offset,
            (void*)current_meta_block->prev_block,(void*)current_meta_block->next_block);
            block_count++;
        ITERATE_VM_PAGE_ALL_BLOCKS_END(current_vm_page,current_meta_block);
        printf("\n");
        page_count++;
    ITERATE_VM_PAGE_END(pg_faimly,current_vm_page);
    printf("\n");
}

void mm_print_block_usage(char *struct_name){
    struct vm_page_faimly_t* pg_faimly=lookup_page_faimly_by_name(struct_name);
    if(pg_faimly==NULL){
        printf("ERROR STRUCT NOT REGISTERED\n");
        return;
    }
    printf("\n");
    struct vm_page_t* current_vm_page;
    struct block_meta_data_t* current_meta_block;
    int page_count=0;
    int TBC=0,FBC=0,OBC=0,app_mem_usage=0;

    ITERATE_VM_PAGE_BEGIN(pg_faimly,current_vm_page)
        ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(current_vm_page,current_meta_block)
            if(current_meta_block->is_free==MM_FLASE)
                OBC++;
            else
                FBC++;
            TBC++;
        ITERATE_VM_PAGE_ALL_BLOCKS_END(current_vm_page,current_meta_block);
        page_count++;
    ITERATE_VM_PAGE_END(pg_faimly,current_vm_page);
    app_mem_usage=OBC*pg_faimly->struct_size;
    printf("TOTAL VM PAGES USED BY %s : %d\n",struct_name,page_count);
    printf("%s      TBC: %d        FBC: %d          OBC: %d         AppMemUsage: %d\n",struct_name,TBC,FBC,OBC,app_mem_usage);
    printf("\n");
}
