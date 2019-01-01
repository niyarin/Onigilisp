#include "ebm.h"

uintptr_t EBM_malloc_wrapper(size_t size,uintptr_t env){
    void* res =  malloc(size);
    if (EBM_COPT_UNLIKELY(res == NULL)){
        //TODO: OUTPUT ERROR MESSAGE.
        exit(1);
    }
    return (uintptr_t)res;
}


uintptr_t EBM_allocate_pair(uintptr_t car,uintptr_t cdr,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t* pair = (uintptr_t*)allocator(sizeof(uintptr_t) * 2,allocator_env);
    pair[0] = car;
    pair[1] = cdr;
    return EBM_ADD_TYPE(pair,EBM_TYPE_PAIR);
}

void EBM_free_wrapper(uintptr_t obj,uintptr_t env){
    free((void*)EBM_REMOVE_TYPE(obj));
}


uintptr_t EBM_allocate_record_CA(uint32_t size,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t *res = (uintptr_t*)allocator(sizeof(uintptr_t) * (size+1),env);
    res[0] = size;
    return EBM_ADD_TYPE(res,EBM_TYPE_RECORD);
}

uintptr_t EBM_vector_re_allocate_CA(uintptr_t vector,uintptr_t new_size,uintptr_t else_fill,EBM_ALLOCATOR allocator,uintptr_t env){
    //一度re_allocateしたオブジェクトは再度re_allocateされる可能性は高そう
    //allocatorを特殊化したりできるかも

    uintptr_t res = EBM_allocate_vector_CA(new_size,allocator,env);
    int i;
    if (new_size >= EBM_vector_length_CR(vector)){
        for (i=0;i<EBM_vector_length_CR(vector);i++){
            EBM_vector_primitive_set_CA(res,i,EBM_vector_ref_CA(vector,i));
        }
        
        for (i=EBM_vector_length_CR(vector);i<new_size;i++){
            EBM_vector_primitive_set_CA(res,i,else_fill);
        }
    }else{
         for (i=0;i<new_size;i++){
            EBM_vector_primitive_set_CA(res,i,EBM_vector_ref_CA(vector,i));
        }
   
    }
    return res;
}

uintptr_t EBM_record_primitive_set_CA(uintptr_t record,size_t index,uintptr_t p){
    uintptr_t* record_v = (uintptr_t*)EBM_REMOVE_TYPE(record);
    record_v[index+1] = p;
    return EBM_UNDEF;
}

uintptr_t EBM_record_ref_CA(uintptr_t record,size_t index){
    uintptr_t* record_v = (uintptr_t*)EBM_REMOVE_TYPE(record);
    return (uintptr_t)record_v[index+1];
}



uintptr_t EBM_allocate_vector_CA(size_t size,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(size + 2,allocator,env);
    EBM_record_primitive_set_CA(res,
                  0,
                  EBM_BUILT_IN_RECORD_TYPE_VECTOR);

     EBM_record_primitive_set_CA(res,
                  1,
                  size);
     return res;
}


uintptr_t EBM_vector_primitive_set_CA(uintptr_t vector,size_t index,uintptr_t object){
    EBM_record_primitive_set_CA(vector,index+2,object);
}



uintptr_t EBM_allocate_pointer_box_CA(uintptr_t val,EBM_ALLOCATOR allocator,uintptr_t env){
    uintptr_t res = EBM_allocate_record_CA(2,allocator,env);
    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_POINTER_BOX);
    EBM_record_primitive_set_CA(res,1,val);
    return res;
}

uintptr_t EBM_pointer_box_set(uintptr_t pointer_box,uintptr_t ptr){
    EBM_record_primitive_set_CA(pointer_box,1,ptr);
    return EBM_UNDEF;
}

