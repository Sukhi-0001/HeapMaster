#include <stdio.h>
#include "./src/uapi_mm.h"

typedef struct test_{
    int a;
    int b;
    int arr[10];
    char letter;
}test;


int main(){
    //init memory managment system
    mm_init();

    //before allcating register struct with mm system
    MM_REG_STRUCT(test);

    //now allcate memory for struct
    //1 arg struct name to allcate for memory
    //2 arg how many units should be allocates in sequence like array of structs
    test *ptr;
    ptr=XMALLOC(test,1);
    XMALLOC(test,2);
    XMALLOC(test,10);


    //it displays the blocks of memory for this struct that was were allocated even we dont keep pointer 
    // but our memory managment system remembers ans store information  
    PRINT_BLOCK_MEMORY_LAYOUT(test);
    
    //it shows total blocks that were allocated to this struct and how many used and are free
    PRINT_BLOCK_USAGE(test);

   
}
