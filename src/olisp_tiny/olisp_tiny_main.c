#include<stdio.h>
#include<stdlib.h>

#include "ebm.h"
#include "ebm_frontend.h"
#include "olisp_cinterface.h"
#include "olisp_tiny_gc.h"
#include "olisp_tiny_eval_simple.h"
#include "olisp_tiny_functions.h"


static void repl(EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state,uintptr_t eval_env){
    olisp_state->arg_size = 1;
    printf("Onigilisp tiny repl.\n");
    uintptr_t input_port = EBM_frontend_allocate_input_file_port(stdin,gc_interface->allocator,gc_interface->env);
    uintptr_t output_port = EBM_frontend_allocate_output_file_port_CA(stdout,gc_interface->allocator,gc_interface->env);

    while (1){
       printf("\nOLISP>");
       uintptr_t read_obj = OLISP_cfun_call(olisp_state,OLISP_read,1,input_port);
       if (read_obj == EBM_EOF){
            break;
       }

       uintptr_t expanded_expression = EBM_olisp_tiny_expand_simple(read_obj, eval_env,gc_interface);
       uintptr_t res = EBM_olisp_eval_simple(expanded_expression, eval_env,gc_interface,olisp_state);

       EBM_write_simple(res,output_port);
    }
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

static void olisp_state_close(OLISP_state *olisp_state){
    
}


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

static void olisp_tiny_gc_close(){

}

int main(int argc,char **argv){
    olisp_tiny_gc_env parent_gc_env;

    uintptr_t gc_allocator_env =  olisp_tiny_gc_setup(&parent_gc_env);
    EBM_GC_INTERFACE gc_interface;
    {
        gc_interface.allocator = EBM_olisp_tiny_allocate;
        gc_interface.env = gc_allocator_env;
        gc_interface.write_barrier =  EBM_olisp_tiny_gc_write_barrier;
    }

    OLISP_state olisp_state;
    olisp_state_setup(&olisp_state,gc_allocator_env,&gc_interface);

    uintptr_t eval_env =  EBM_allocate_olisp_tiny_eval_simple_environment(&gc_interface);
    
    if (argc == 1){
        repl(&gc_interface,&olisp_state,eval_env);
    }else if (argc == 2){
        char *filename = argv[1];
    }
}
