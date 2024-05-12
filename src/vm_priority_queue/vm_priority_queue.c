/*
#include <stdio.h>
#include <stdint.h>
#include "vm_priority_queue.h"


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

void print_vm_priority_queue(struct vm_priority_queue_node_t* head){
     while (head!=NULL)
    {   
        PRINT_I(head->meta_block_ptr->block_size);
        head=head->next_node;
    }
}
*/
