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
        i++;
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
        i++;
    }
    symbol_data[i] = 0;
    
    EBM_record_primitive_set_CA(symbol_record,1,0);
    EBM_record_primitive_set_CA(symbol_record,2,(uintptr_t)symbol_data);
    return symbol_record;
}

uintptr_t EBM_symbol_compare_cstring_CACR(uintptr_t symbol,char* cascii_string){
    uint32_t *symbol_data = EBM_record_ref_CA(symbol,2);

    int i=0;
    while (symbol_data[i]&&cascii_string[i]){
        if (symbol_data[i] != cascii_string[i]){
            return 0;
        }
        i++;
    }

    if (symbol_data[i] != cascii_string[i]){
        return 0;
    }
    return 1;
}


uintptr_t EBM_allocate_symbol_no_copy(uint32_t *symbol_data,EBM_GC_INTERFACE *gc_interface){
     uintptr_t symbol_record  = EBM_allocate_record_CA(3,gc_interface->allocator,gc_interface->env);
    EBM_record_primitive_set_CA(symbol_record,
                   0,
                   EBM_BUILT_IN_RECORD_TYPE_SYMBOL); 

    EBM_record_set_CA(symbol_record,2,(uintptr_t)symbol_data,gc_interface->write_barrier,gc_interface->env);
    return symbol_record;
}
