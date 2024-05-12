#ifndef UAPI_MM_H
#define UAPI_MM_H


#define MM_REG_STRUCT(struct_name)  \
        (mm_instantiate_new_page_faimly(#struct_name,sizeof(struct_name)))   

#define XMALLOC(name,uints)      \
        xmalloc(#name,uints)

#define XFREE(addr)      \
        xfree(addr)

#define PRINT_BLOCK_USAGE(struct_name)  \
        mm_print_block_usage(#struct_name)

#define PRINT_BLOCK_MEMORY_LAYOUT(struct_name)     \
        mm_print_memory_usage(#struct_name)

#endif
