#ifndef MM_H
#define MM_H
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#define PRINT_S(S) printf("%s\n",S)
#define PRINT_I(S) printf(#S " value is  %i\n",S)

#define MM_MAX_STRUCT_NAME 64
  

struct block_meta_data_t;
struct vm_priority_queue_node_t;


struct vm_page_faimly_t{
    char struct_name[MM_MAX_STRUCT_NAME];
    struct vm_page_t* first_page;
    struct vm_priority_queue_node_t *free_block_list_head;
    uint32_t struct_size;
};

struct vm_page_for_faimlies_t{
    struct vm_page_for_faimlies_t* next;
    uint32_t count_of_faimlies_present;
    struct vm_page_faimly_t vm_page_faimly[0];
};

#define MAX_FAIMLIES_PER_PAGE   \
        (SYSTEM_PAGE_SIZE - sizeof(struct vm_page_for_faimlies_t*)) / \
        sizeof(struct vm_page_faimly_t)




#define ITERATE_VM_PAGE_FOR_FAIMLIES_BEGIN(vm_page_for_faimlies_ptr,current)  \
        for(current=vm_page_for_faimlies_ptr;current!=NULL;current=current->next){

#define ITERATE_VM_PAGE_FOR_FAIMLIES_END        }

#define ITERATE_PAGE_FAIMLIES_BEGIN(vm_page_faimlies_ptr,current){    \
        int count=0;              \
        for(current=vm_page_faimlies_ptr->vm_page_faimly;               \
        (count<MAX_FAIMLIES_PER_PAGE) && (current->struct_size);                            \
        current++,count++){                                                         \
                                                                                    
#define ITERATE_PAGE_FAIMLIES_END }}

typedef enum{
    MM_TRUE,
    MM_FLASE
}VM_BOOL_T;


struct vm_priority_queue_node_t
{
    struct block_meta_data_t *meta_block_ptr;
    //int block_size;
    struct vm_priority_queue_node_t* next_node;
};


struct block_meta_data_t{
    VM_BOOL_T is_free;
    uint32_t block_size;
    struct block_meta_data_t *next_block;
    struct block_meta_data_t *prev_block;
    struct vm_priority_queue_node_t vm_priority_queue_node;
    uint32_t offset;

};

#define MM_NEXT_META_BLOCK(block_meta_data_ptr) \
        (block_meta_data_ptr->next_block)

#define MM_NEXT_META_BLOCK_BY_SIZE(block_meta_data_ptr) \
        (struct block_meta_data_t*)((char*)(block_meta_data_ptr+1)+block_meta_data_ptr->block_size)

#define MM_PREV_META_BLOCK(block_meta_data_ptr) \
        (block_meta_data_ptr->prev_block)

#define OFFSET_OF(vm_page,meta_block)   \
        ((char*)meta_block-(char*)vm_page)

#define TOTAL_MEMORY_TO_ALLOCATE(m) \
        (sizeof(struct block_meta_data_t)+m+4)

#define MARK_VM_PAGE_EMPTY(vm_page)     \
        vm_page->block_meta_data.next_block=NULL;       \
        vm_page->block_meta_data.next_block=NULL;       \
        vm_page->block_meta_data.is_free=MM_TRUE 

struct vm_page_t{
        struct vm_page_t* next_page;
        struct vm_page_t* prev_page;
        struct vm_page_faimly_t* pg_faimly;
        struct block_meta_data_t block_meta_data;
        char page_memory[0];
};

#define VM_QUEUE_NODE_BLOCK_SIZE(N) (N->meta_block_ptr->block_size)



#define ITERATE_VM_PAGE_BEGIN(vm_page_faimly_ptr,current) \
        for(current=vm_page_faimly_ptr->first_page;current!=NULL;current=current->next_page){

#define ITERATE_VM_PAGE_END(vm_page_faimly_ptr,current) }

#define ITERATE_VM_PAGE_ALL_BLOCKS_BEGIN(vm_page_ptr,current)   \
        for (current = &vm_page_ptr->block_meta_data; current != NULL; current=current->next_block){

#define ITERATE_VM_PAGE_ALL_BLOCKS_END(vm_page_prt,current) }


void mm_instantiate_new_page_faimly(char* struct_name,uint32_t struct_size);
void mm_init();
void* xmalloc(char* struct_name,int uints);
void mm_print_block_usage(char *struct_name);
void mm_print_memory_usage(char *struct_name);
void xfree(void* free_addr);
#endif