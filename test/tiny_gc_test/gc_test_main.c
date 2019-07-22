#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "ebm.h"
#include "ebm_frontend.h"
#include "olisp_cinterface.h"
#include "olisp_tiny_gc.h"
#include "olisp_tiny_functions.h"

static uintptr_t olisp_tiny_gc_setup(olisp_tiny_gc_env *parent_gc_env){
    uintptr_t parent_gc_env_box = EBM_allocate_pointer_box_CA(parent_gc_env,EBM_malloc_wrapper,0);
    
    {
        parent_gc_env->allocator = EBM_malloc_wrapper;
        parent_gc_env->parent_env = 0;
        parent_gc_env->allocator_env = parent_gc_env_box;
        parent_gc_env->gc_interface = NULL;
    }


    uintptr_t allocator_env =  EBM_allocate_olisp_tiny_gc_env(EBM_malloc_wrapper,parent_gc_env_box);

    return allocator_env;
}

static void olisp_state_setup(OLISP_state *olisp_state,uintptr_t gc_allocator_env,EBM_GC_INTERFACE *gc_interface){
   olisp_state->allocator = EBM_olisp_tiny_allocate;
   olisp_state->allocator_env = gc_allocator_env;
   olisp_state->gc_interface = gc_interface;
    //TODO:EBM_USE_IO使う
   olisp_state->default_output_port = 
       EBM_frontend_allocate_output_file_port_CA(
               stdout,
               EBM_malloc_wrapper,
               0);
   //malloc
   olisp_state->symbol_intern = 
       EBM_allocate_symbol_trie(
               EBM_malloc_wrapper,
               0);
}


olisp_tiny_gc_env parent_gc_env;
OLISP_state olisp_state;
uintptr_t gc_allocator_env;
EBM_GC_INTERFACE gc_interface;

int test1(){
    uintptr_t p1 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);
    uintptr_t p2 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);


    EBM_olisp_tiny_gc_full_free(gc_allocator_env);

    uintptr_t p3 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

    uintptr_t p4 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);


    //初回だけはこの順
    if (p1 == p4 && p2 == p3){
        printf("TEST1:OK\n");
        return 1;
    }else{
        printf("TEST1:NG\n");
        return 0;
    }
}

void test2(){
     EBM_olisp_tiny_gc_full_free(gc_allocator_env);
     uintptr_t p1 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);
     uintptr_t p2 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

     EBM_olisp_tiny_gc_full_free(gc_allocator_env);

    uintptr_t p3 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

    uintptr_t p4 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);
    
    if (p1 == p3 && p2 == p4){
        printf("TEST2:OK\n");
        return 1;
    }else{
        printf("TEST2:NG\n");
        return 0;
    }
}

void test3(){
     EBM_olisp_tiny_gc_full_free(gc_allocator_env);
     uintptr_t p1 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);
     uintptr_t p2 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);


    EBM_olisp_tiny_gc_full_mark(p1,gc_allocator_env);
    EBM_olisp_tiny_gc_full_mark(p2,gc_allocator_env);

    EBM_olisp_tiny_gc_full_free(gc_allocator_env);

    uintptr_t p3 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

    uintptr_t p4 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

    if (p1 != p3 && p1 != p4 && p2 != p3 && p2 != p4){
        printf("TEST3:OK\n");
        return 1;
    }else{
        printf("TEST3:NG\n");
        return 0;
    }
}

void test4(){
     EBM_olisp_tiny_gc_full_free(gc_allocator_env);


     uintptr_t p1 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);
     uintptr_t p2 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

     EBM_olisp_tiny_gc_full_mark(p2,gc_allocator_env);
     EBM_olisp_tiny_gc_full_free(gc_allocator_env);

    uintptr_t p3 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

    uintptr_t p4 = EBM_allocate_pair(EBM_TRUE,EBM_FALSE,EBM_olisp_tiny_allocate,gc_allocator_env);

    if (p1 != p3 && p1 != p4 && p2 != p3 && p2 != p4){
        printf("TEST4:OK\n");
        return 1;
    }else{
        printf("TEST4:NG\n");
        return 0;
    }

}

void test5(){

     EBM_olisp_tiny_gc_full_free(gc_allocator_env);
    uintptr_t v1 = EBM_allocate_vector_CA(3,EBM_olisp_tiny_allocate,gc_allocator_env);
     EBM_olisp_tiny_gc_full_mark(v1,gc_allocator_env);
     EBM_olisp_tiny_gc_full_free(gc_allocator_env);

    uintptr_t v2 = EBM_allocate_vector_CA(3,EBM_olisp_tiny_allocate,gc_allocator_env);

    if (v1 != v2){
        printf("TEST5:OK\n");
        return 1;
    }else{
        printf("TEST5:NG\n");
        return 1;
    }
    
}

int main(void){

    gc_allocator_env =  olisp_tiny_gc_setup(&parent_gc_env);
    {
        gc_interface.allocator = EBM_olisp_tiny_allocate;
        gc_interface.env = gc_allocator_env;
        gc_interface.write_barrier =  EBM_olisp_tiny_gc_write_barrier;
    }

    olisp_state_setup(&olisp_state,gc_allocator_env,&gc_interface);

    printf("INTY GC TEST\n");
    {
        test1();
        test2();
        test3();
        test4();
        test5();
    }
    return 0;
}
