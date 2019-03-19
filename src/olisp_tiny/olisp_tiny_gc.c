#include "ebm.h"
#include "olisp_tiny_gc.h"

#define EBM_POOL_SIZE 128
#define EBM_NUMBER_OF_POOL 16

#define OLISP_TINY_GC_FREE_PTRS 2
#define OLISP_TINY_GC_PTR_START 3
#define OLISP_TINY_GC_PTR_CURRENT 4
#define OLISP_TINY_GC_PTR_END 5
#define OLISP_TINY_GC_NEXT_POOL 6
#define OLISP_TINY_GC_POOL_TARGET_SIZE 7

#define OLISP_TINY_GC_ARENA_POOL_START 5

#define OLISP_TINY_GC_INITIAL_FREE_PTRS_SIZE 128

static uintptr_t EBM_olisp_gc_count_valid_bit(uintptr_t original_ptr){
    return ((original_ptr-1)>>2)+1;
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
        olisp_tiny_gc_env *parent_env = env->parent_env;
        uintptr_t res = parent_env->allocator(size,parent_env->arena);
        uintptr_t current_free_ptr_size =  EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_vector_ref_CA(env->arena,2));

        uintptr_t pointer_box = EBM_vector_ref_CA(EBM_vector_ref_CA(env->arena,1),current_free_ptr_size);
        
        
        char* free_size_mark = (char*)EBM_pointer_box_ref_CR(EBM_vector_ref_CA(env->arena,3));
        free_size_mark[current_free_ptr_size] = 0b00;

        //update free ptr size
        EBM_vector_primitive_set_CA(env->arena,2,EBM_allocate_FX_NUMBER_CA(current_free_ptr_size + 1));

        EBM_pointer_box_set(pointer_box,res);
        return res;
   }
}



/**
 * Allocate a gc pool.
 * gc pool = #(heap mark free-ptr heap-start heap-end current-heap-position target_size)
 */
static uintptr_t EBM_allocate_olisp_tiny_gc_pool(uintptr_t size,EBM_ALLOCATOR parent_allocator,uintptr_t parent_allocator_env){
    uintptr_t res = EBM_allocate_vector_CA(8,parent_allocator,parent_allocator_env);
    
    uintptr_t allocated_ptr = parent_allocator(sizeof(uintptr_t) * size * EBM_POOL_SIZE,parent_allocator_env);

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
    EBM_vector_primitive_set_CA(res,OLISP_TINY_GC_POOL_TARGET_SIZE,EBM_allocate_FX_NUMBER_CA(size));
    return res;
}


/**
 * Allocate an arena object.
 * arena object = #( write-barrier free-size-pool free-size-pool-size size1-pool size2-pool ... sizeN-pool)
 *
 */
static uintptr_t EBM_allocate_olisp_tiny_gc_arena(EBM_ALLOCATOR parent_allocator,uintptr_t parent_allocator_env){
    uintptr_t res = EBM_allocate_vector_CA(OLISP_TINY_GC_ARENA_POOL_START  + EBM_NUMBER_OF_POOL,parent_allocator,parent_allocator_env);
    
    {//allocate write barrier space
        EBM_vector_primitive_set_CA(res,0,EBM_NULL);
    }

    {//allocate free size pool
        uintptr_t free_size_pool = EBM_allocate_vector_CA(OLISP_TINY_GC_INITIAL_FREE_PTRS_SIZE,parent_allocator,parent_allocator_env);
        EBM_vector_primitive_set_CA(res,1,free_size_pool);
        int i;
        for (i=0;i<OLISP_TINY_GC_INITIAL_FREE_PTRS_SIZE;i++){
            EBM_vector_primitive_set_CA(free_size_pool,i,EBM_allocate_pointer_box_CA(0,parent_allocator,parent_allocator_env));
        }
    }

    {//free size pool length
        EBM_vector_primitive_set_CA(res,2,EBM_allocate_FX_NUMBER_CA(0));
    }

    {//marks for free size pool.
        char* free_size_mark = (char*)parent_allocator(OLISP_TINY_GC_INITIAL_FREE_PTRS_SIZE,parent_allocator_env);
        EBM_vector_primitive_set_CA(res,3,EBM_allocate_pointer_box_CA(free_size_mark,parent_allocator,parent_allocator_env));
    }

    {//set old generation.
        EBM_vector_primitive_set_CA(res,4,EBM_NULL);
    }



    {//allocate static size pool
        int i;
        for (i=0;i<EBM_NUMBER_OF_POOL;i++){
            uintptr_t pool = EBM_allocate_olisp_tiny_gc_pool(i+1,parent_allocator,parent_allocator_env);
            EBM_vector_primitive_set_CA(res,i+OLISP_TINY_GC_ARENA_POOL_START ,pool);
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


static uintptr_t EBM_olisp_tiny_gc_search_free_size_index(uintptr_t ptr,uintptr_t arena){
    int i;
    for (i=0;i<EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_vector_ref_CA(arena,2));i++){
       if (EBM_pointer_box_ref_CR(EBM_vector_ref_CA(EBM_vector_ref_CA(arena,1),i)) == ptr){
           return i;
       }
    }
    return -1;
}

static uintptr_t EBM_olisp_gc_search_all_static_size_pools(uintptr_t ptr,uintptr_t arena){
    int i;
    for (i= OLISP_TINY_GC_ARENA_POOL_START ;i<EBM_vector_length_CR(arena);i++){
        uintptr_t pool = EBM_vector_ref_CA(arena,i);
        while (pool != EBM_NULL){
           if ( EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_START )) <= ptr && ptr < EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_END))){
               return pool;
           }
           pool = EBM_vector_ref_CA(pool,OLISP_TINY_GC_NEXT_POOL);
        }
    }
    return EBM_NULL;
}


uintptr_t EBM_olisp_gc_mark_and_check(uintptr_t ptr,uintptr_t size,uintptr_t env_ptr,int counter_for_old_arenas){
    //For only current generation gc???
    olisp_tiny_gc_env *env = (olisp_tiny_gc_env*)EBM_pointer_box_ref_CR(env_ptr);

   uintptr_t arena = env->arena;
   while (counter_for_old_arenas && arena != EBM_NULL){
       if (size == 0){
          uintptr_t free_pool_index =  EBM_olisp_tiny_gc_search_free_size_index(ptr,arena);
          if (free_pool_index != -1){
                char* mark = EBM_pointer_box_ref_CR(EBM_vector_ref_CA(arena,3));
                mark[free_pool_index] |= 0b01;
          }
       }else{
           uintptr_t sz = EBM_olisp_gc_count_valid_bit(size);
           if (sz < EBM_NUMBER_OF_POOL){
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
           }else{
              //TODO:
              //large size
           }
       }
       arena = EBM_vector_ref_CA(arena,4);
       counter_for_old_arenas--;
   }
}

static void EBM_olisp_tiny_gc_push_free_ptr(uintptr_t ptr,uintptr_t pool){
    ptr = EBM_ADD_TYPE(ptr,EBM_TYPE_PAIR);

    EBM_PRIMITIVE_SET_CAR(ptr,
        EBM_vector_ref_CA(pool,OLISP_TINY_GC_FREE_PTRS));

    //TODO:全くprimitiveではない。あとで直す
    EBM_vector_primitive_set_CA(pool,OLISP_TINY_GC_FREE_PTRS,ptr);
    
}


uintptr_t EBM_olisp_tiny_gc_free(uintptr_t object,uintptr_t env_ptr){

   olisp_tiny_gc_env *env = (olisp_tiny_gc_env*)EBM_pointer_box_ref_CR(env_ptr);

   uintptr_t ptr = EBM_REMOVE_TYPE(object);
   uintptr_t arena = env->arena;
   uintptr_t size = EBM_object_heap_size_CR(object);
   while (arena!=EBM_NULL){
        if (size == 0){

            uintptr_t free_pool_index =  EBM_olisp_tiny_gc_search_free_size_index(ptr,arena);
            if (free_pool_index != -1){
                //TODO:つめる?(別のタイミングでまとめてやる)
                uintptr_t pointer_box = EBM_vector_ref_CA(EBM_vector_ref_CA(arena,1),free_pool_index);

                void* target_ptr = (void*)EBM_pointer_box_ref_CR(pointer_box);
                free(target_ptr);
                EBM_pointer_box_set(pointer_box,0);
                return EBM_TRUE;
            }

            uintptr_t pool = EBM_olisp_gc_search_all_static_size_pools(ptr,arena);
            uintptr_t pool_target_size = EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_POOL_TARGET_SIZE)) * sizeof(uintptr_t);

            if (pool != EBM_NULL){
                uint32_t *mark_ptr = (uint32_t*)EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,1));

                uintptr_t heap_start_ptr = EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_START ));
                int mark_position1 = (ptr - heap_start_ptr)/(16 * pool_target_size);

                int mark_position2 = (ptr - (heap_start_ptr + mark_position1 * pool_target_size * 16)) / pool_target_size;
                mark_ptr[mark_position1] = 
                            mark_ptr[mark_position1]& (~(0b11<<(mark_position2*2)));
                EBM_olisp_tiny_gc_push_free_ptr(ptr,pool);
            }
        }else{
           uintptr_t sz = EBM_olisp_gc_count_valid_bit(size);
           if (sz < EBM_NUMBER_OF_POOL){
               uintptr_t pool = EBM_vector_ref_CA(env->arena,(sz + OLISP_TINY_GC_ARENA_POOL_START ) - 1);
                while (pool != EBM_NULL){
                    uintptr_t heap_start_ptr = EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_START ));

                   if (heap_start_ptr <= ptr && ptr < EBM_pointer_box_ref_CR(EBM_vector_ref_CA(pool,OLISP_TINY_GC_PTR_END))){

                   }
                   pool = EBM_vector_ref_CA(pool,OLISP_TINY_GC_NEXT_POOL);
               }
           }else{
              //TODO:
              //large size
           }
        }
       arena = EBM_vector_ref_CA(arena,4);
   }
}

