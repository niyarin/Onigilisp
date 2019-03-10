#include "ebm.h"
#include "olisp_tiny_gc.h"

#define EBM_POOL_SIZE 128
#define EBM_NUMBER_OF_POOL 16



#define OLISP_TINY_GC_FREE_PTRS 2
#define OLISP_TINY_GC_PTR_START 3
#define OLISP_TINY_GC_PTR_CURRENT 4
#define OLISP_TINY_GC_PTR_END 5
#define OLISP_TINY_GC_NEXT_POOL 6



uintptr_t EBM_olisp_tiny_allocate(size_t size,uintptr_t env_ptr){
   olisp_tiny_gc_env *env = (olisp_tiny_gc_env*)EBM_pointer_box_ref_CR(env_ptr);


   if (size < sizeof(uintptr_t) * EBM_NUMBER_OF_POOL){
       if (size & EBM_POINTER_INFORMATION_BITS){
           size += (1<<EBM_CONFIG_POINTER_INFORMATION_SIZE);
       }
       uintptr_t sz = size>>EBM_CONFIG_POINTER_INFORMATION_SIZE;

       uintptr_t pool = EBM_vector_ref_CA(env->env,(sz + 2) - 1);


       if (EBM_vector_ref_CA(pool,OLISP_TINY_GC_FREE_PTRS) != EBM_NULL){
            //TODO:            
            exit(1);
       }
    
       if ( EBM_pointer_box_ref_CR( EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_END)) == EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_CURRENT))){
            //next
            exit(1);
       }

        uintptr_t res = EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_CURRENT));

        EBM_pointer_box_set(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_CURRENT),res + sizeof(uintptr_t) * sz);
        return res;

  }else{
       //TODO:
        //free size
        printf("FREE SIZE\n");
        
        exit(1);
   }




}

/**
 * Allocate a gc pool.
 * gc pool = #(heap mark free-ptr heap-start heap-end current-heap-position)
 */
static uintptr_t EBM_allocate_olisp_tiny_gc_pool(uintptr_t size,EBM_ALLOCATOR parent_allocator,uintptr_t parent_allocator_env){
    uintptr_t res = EBM_allocate_vector_CA(7,parent_allocator,parent_allocator_env);
    
    uintptr_t allocated_ptr = parent_allocator(sizeof(uintptr_t) * size,parent_allocator_env);

    //allocate objects
    uintptr_t obj_ptr = EBM_allocate_pointer_box_CA(allocated_ptr,parent_allocator,parent_allocator_env);
    EBM_vector_primitive_set_CA(res,0,obj_ptr);
    
    //allocate mark
    //mark 2bit 
    uintptr_t mark_ptr = EBM_allocate_pointer_box_CA(parent_allocator((1 + size/16) * sizeof(uint32_t),parent_allocator_env),parent_allocator,parent_allocator_env);
    EBM_vector_primitive_set_CA(res,1,mark_ptr);

    //allocate free ptr
    EBM_vector_primitive_set_CA(res,OLISP_TINY_GC_FREE_PTRS,EBM_NULL);

    EBM_vector_primitive_set_CA(res,OLISP_TINY_GC_PTR_START,EBM_allocate_pointer_box_CA(allocated_ptr,parent_allocator,parent_allocator_env));
    
    EBM_vector_primitive_set_CA(res,OLISP_TINY_GC_PTR_END,EBM_allocate_pointer_box_CA(allocated_ptr + sizeof(uintptr_t) * EBM_POOL_SIZE * size,parent_allocator,parent_allocator_env));

    EBM_vector_primitive_set_CA(res,OLISP_TINY_GC_PTR_CURRENT,EBM_allocate_pointer_box_CA(allocated_ptr,parent_allocator,parent_allocator_env));

    EBM_vector_primitive_set_CA(res,OLISP_TINY_GC_NEXT_POOL,EBM_NULL);
    return res;
}


static uintptr_t EBM_allocate_olisp_tiny_gc_arena(EBM_ALLOCATOR parent_allocator,uintptr_t parent_allocator_env){
    uintptr_t res = EBM_allocate_vector_CA(2 + EBM_NUMBER_OF_POOL,parent_allocator,parent_allocator_env);
    
    {//allocate write barier space
        EBM_vector_primitive_set_CA(res,0,EBM_NULL);
    }

    {//allocate free size pool
        EBM_vector_primitive_set_CA(res,1,EBM_NULL);
    }

    {//allocate static size pool
        int i;
        for (i=0;i<EBM_NUMBER_OF_POOL;i++){
            uintptr_t pool = EBM_allocate_olisp_tiny_gc_pool(i+1,parent_allocator,parent_allocator_env);
            EBM_vector_primitive_set_CA(res,i+2,pool);
        }
    }
    return res;
}



 
uintptr_t EBM_allocate_olisp_tiny_gc_env(EBM_ALLOCATOR parent_allocator,uintptr_t parent_allocator_env){
    olisp_tiny_gc_env *env = (olisp_tiny_gc_env*)parent_allocator(sizeof(olisp_tiny_gc_env),parent_allocator_env);

    env->parent_env = (olisp_tiny_gc_env*)EBM_pointer_box_ref_CR(parent_allocator_env);


    env->env = EBM_allocate_olisp_tiny_gc_arena(parent_allocator,parent_allocator_env);

    return EBM_allocate_pointer_box_CA((uintptr_t)env,parent_allocator,parent_allocator_env);
}


