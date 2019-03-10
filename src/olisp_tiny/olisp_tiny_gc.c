#include "ebm.h"
#include "olisp_tiny_gc.h"

#define EBM_POOL_SIZE 128
#define EBM_NUMBER_OF_POOL 16

#define OLISP_TINY_GC_FREE_PTRS 2
#define OLISP_TINY_GC_PTR_START 3
#define OLISP_TINY_GC_PTR_CURRENT 4
#define OLISP_TINY_GC_PTR_END 5
#define OLISP_TINY_GC_NEXT_POOL 6

#define OLISP_TINY_GC_ARENA_POOL_START 3

#define OLISP_TINY_GC_INITIAL_FREE_PTRS_SIZE 128

//TODO:ぴったりサイズのとき1サイズ大きくなるのを修正する
static uintptr_t EBM_olisp_gc_count_valid_bit(uintptr_t original_ptr){
    return (original_ptr+(1<<EBM_CONFIG_POINTER_INFORMATION_SIZE))>>EBM_CONFIG_POINTER_INFORMATION_SIZE;
} 

uintptr_t EBM_olisp_tiny_allocate(size_t size,uintptr_t env_ptr){
   olisp_tiny_gc_env *env = (olisp_tiny_gc_env*)EBM_pointer_box_ref_CR(env_ptr);


   if (size < sizeof(uintptr_t) * EBM_NUMBER_OF_POOL){
       uintptr_t sz = EBM_olisp_gc_count_valid_bit(size);
       uintptr_t pool = EBM_vector_ref_CA(env->arena,(sz + OLISP_TINY_GC_ARENA_POOL_START ) - 1);


       if (EBM_vector_ref_CA(pool,OLISP_TINY_GC_FREE_PTRS) != EBM_NULL){
           //TODO:Return a freed ptr if pool has.
           exit(1);
       }
    
       if ( EBM_pointer_box_ref_CR( EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_END)) == EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_CURRENT))){
            //TODO:Allocate a next pool.
            exit(1);
       }

        uintptr_t res = EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_CURRENT));

        EBM_pointer_box_set(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_CURRENT),res + sizeof(uintptr_t) * sz);
        return res;

  }else{
        //TODO:
        
      olisp_tiny_gc_env *parent_env = env->parent_env;
        uintptr_t res = parent_env->allocator(size,parent_env->arena);
        uintptr_t current_free_ptr_size =  EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_vector_ref_CA(env->arena,2));

        printf("FREE SIZE[%ld]\n",size);

        return res;
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

    env->arena = EBM_allocate_olisp_tiny_gc_arena(parent_allocator,parent_allocator_env);

    env->root = EBM_NULL;

    return EBM_allocate_pointer_box_CA((uintptr_t)env,parent_allocator,parent_allocator_env);
}

uintptr_t EBM_olisp_gc_mark_and_check(uintptr_t ptr,uintptr_t size,uintptr_t env_ptr){
    //For only current generation gc???
    olisp_tiny_gc_env *env = (olisp_tiny_gc_env*)EBM_pointer_box_ref_CR(env_ptr);

   uintptr_t arena = env->arena;

   if (size == 0){
        //TODO:
   }else{
       uintptr_t sz = EBM_olisp_gc_count_valid_bit(size);
       uintptr_t pool = EBM_vector_ref_CA(env->arena,(sz + OLISP_TINY_GC_ARENA_POOL_START ) - 1);
        while (pool != EBM_NULL){
            uintptr_t heap_start_ptr = EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_START ));

           if (heap_start_ptr <= ptr && ptr < EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_END))){
               uintptr_t size_on_pool = (( 1<<EBM_CONFIG_POINTER_INFORMATION_SIZE) *sz);
               uint32_t *mark_ptr = (uint32_t*)EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,1));
                int mark_position1 = (ptr - heap_start_ptr)/(16 * size_on_pool);
                int mark_position2 = (ptr - (heap_start_ptr + mark_position1 * size_on_pool * 16)) / size_on_pool;

                if (mark_ptr[mark_position1]>>(mark_position2*2)&0b01){
                    //marked
                    return 0;
                }


                mark_ptr[mark_position1] = 
                    mark_ptr[mark_position1]| 0b01<<(mark_position2*2);
                return 1;
                break;
           }
           pool = EBM_vector_ref_CA(pool,OLISP_TINY_GC_NEXT_POOL);
       }
   }
}

