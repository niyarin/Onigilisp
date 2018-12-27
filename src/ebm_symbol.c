#include "ebm.h"

uintptr_t EBM_allocate_symbol_CA(uint32_t *symbol,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t symbol_record  = EBM_allocate_record_CA(3,allocator,allocator_env);
    EBM_record_primitive_set_CA(symbol_record,
                   0,
                   EBM_BUILT_IN_RECORD_TYPE_SYMBOL); 

    size_t length = 0;
    while (symbol[length]){
        length++;
    }

    uint32_t *symbol_data = (uint32_t*)allocator(sizeof(uint32_t) * (length+1),allocator_env);
    int i=0;
    while (symbol[i]){
        symbol_data[i] = symbol[i];
    }
    symbol_data[i] = 0;
    
    EBM_record_primitive_set_CA(symbol_record,1,0);
    EBM_record_primitive_set_CA(symbol_record,2,(uintptr_t)symbol_data);
    return symbol_record;
}

uintptr_t EBM_allocate_symbol_from_cstring_CA(char *symbol,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t symbol_record  = EBM_allocate_record_CA(3,allocator,allocator_env);
    EBM_record_primitive_set_CA(symbol_record,
                   0,
                   EBM_BUILT_IN_RECORD_TYPE_SYMBOL); 

    size_t length = 0;
    while (symbol[length]){
        length++;
    }

    uint32_t *symbol_data = (uint32_t*)allocator(sizeof(uint32_t) * (length+1),allocator_env);
    int i=0;
    while (symbol[i]){
        symbol_data[i] = symbol[i];
    }
    symbol_data[i] = 0;
    
    EBM_record_primitive_set_CA(symbol_record,1,0);
    EBM_record_primitive_set_CA(symbol_record,2,(uintptr_t)symbol_data);
    return symbol_record;
}
