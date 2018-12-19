#include "ebm.h"

uintptr_t EBM_malloc_wrapper(size_t size,uintptr_t env){
    void* res =  malloc(size);
    if (res == NULL){
        //TODO: OUTPUT ERROR MESSAGE.
        exit(1);
    }
    return (uintptr_t)res;
}

void EBM_free_wrapper(uintptr_t obj,uintptr_t env){
    free((void*)EBM_REMOVE_TYPE(obj));
}


uintptr_t EBM_allocate_record(uint32_t size,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t *res = (uintptr_t*)allocator(sizeof(uintptr_t) * (size+1),env);
    res[0] = size;
    return EBM_ADD_TYPE(res,EBM_TYPE_RECORD);
}

uintptr_t EBM_record_primitive_set(uintptr_t record,size_t index,uintptr_t p){
    uintptr_t* record_v = (uintptr_t*)EBM_REMOVE_TYPE(record);
    record_v[index+1] = p;
    return EBM_UNDEF;
}
