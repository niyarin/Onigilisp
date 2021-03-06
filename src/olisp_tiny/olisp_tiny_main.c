#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

    uintptr_t error_object;
    while (1){
       printf("\nOLISP>");
       uintptr_t read_obj = OLISP_cfun_call(olisp_state,OLISP_read,1,input_port);
       if (read_obj == EBM_EOF){
            break;
       }
    
       OLISP_TRY(olisp_state){
           uintptr_t expanded_expression = EBM_olisp_tiny_expand_simple(read_obj, eval_env,gc_interface,olisp_state);
           uintptr_t res = EBM_olisp_eval_simple(expanded_expression, eval_env,gc_interface,olisp_state);

           EBM_write_simple(res,output_port);
       }OLISP_CATCH(error_object,olisp_state){
           if (olisp_state->arg_size>= 1){//TODO:std-out => std-err
               printf("ERROR::");

               EBM_write_simple(
                        OLISP_get_arg(olisp_state,0),
                        output_port);
               printf("\n\n");
               
               printf("CODE:");

               EBM_write_simple(
                        EBM_vector_ref_CA(olisp_state->runtime_var1,0),
                        output_port);


               printf("STACK:");

               int i;
               uintptr_t stack_cell = 
                   EBM_vector_ref_CA(
                           olisp_state->runtime_var1,
                           2);

               while (stack_cell != EBM_NULL){
                   
                   printf("[1]\n");
                   EBM_write_simple(
                            EBM_CAR(stack_cell),
                            output_port);
                   stack_cell = EBM_CDR(stack_cell);
               }


               printf("\n\n");
               printf("\n");

           }
       }
    }
}

static void load_script(char* filename,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state,uintptr_t eval_env){
    FILE *fp;
    if  ((fp = fopen(filename,"r")) == NULL){
        //TODO:SUPPORT ERROR
        printf("ERR%d\n",__LINE__);
        exit(1);
    }

    uintptr_t input_port = EBM_frontend_allocate_input_file_port(fp,gc_interface->allocator,gc_interface->env);
    while (1){
       uintptr_t read_obj = OLISP_cfun_call(olisp_state,OLISP_read,1,input_port);
       if (read_obj == EBM_EOF){
            break;
       }

       uintptr_t expanded_expression = EBM_olisp_tiny_expand_simple(read_obj, eval_env,gc_interface,olisp_state);
       uintptr_t res = EBM_olisp_eval_simple(expanded_expression, eval_env,gc_interface,olisp_state);
    }
}

static void bootstrap(char* libdir,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state,uintptr_t eval_env){
    char* lib_filename = (char*)malloc(strlen(libdir) + 512);
    char libs[][512] = {"base_library1.scm","base_library2.scm","base_library3.scm","base_library4.scm","scheme_base.scm"};
    int i;
    for (i=0;i<5;i++){
        sprintf(lib_filename,"%s/%s",libdir,libs[i]);
        load_script(lib_filename,gc_interface,olisp_state,eval_env);
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

static uintptr_t olisp_tiny_allocate_lib0_cell(
        uintptr_t lib,
        EBM_ALLOCATOR allocator,
        uintptr_t allocator_env,
        OLISP_state *state){
 
    uintptr_t library_name = 
        EBM_allocate_rev_list(
                2,
                allocator,
                allocator_env,
                OLISP_symbol_intern(
                   EBM_allocate_symbol_from_cstring_CA(
                       "base-library0",
                       allocator,
                       allocator_env),
                   state),
                OLISP_symbol_intern(
                    EBM_allocate_symbol_from_cstring_CA(
                       "olisp-tiny",
                       allocator,
                       allocator_env),
                    state));

    uintptr_t res = 
        EBM_allocate_pair(
            library_name,
            EBM_allocate_pair(
                lib,
                EBM_NULL,
                allocator,
                allocator_env),
            allocator,
            allocator_env);
    return res;
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

    uintptr_t eval_env;
    uintptr_t base_library0;
    {//create eval_env
        uintptr_t offset_box = 
            EBM_allocate_pair(
                    EBM_allocate_FX_NUMBER_CA(0),
                    EBM_NULL,
                    EBM_olisp_tiny_allocate,
                    gc_allocator_env);

        uintptr_t global_box = 
            EBM_allocate_pair(
                    EBM_allocate_vector_CA(
                        128,
                        EBM_olisp_tiny_allocate,
                        gc_allocator_env),
                    EBM_NULL,
                    EBM_olisp_tiny_allocate,
                    gc_allocator_env);

        eval_env =  EBM_allocate_olisp_tiny_eval_simple_environment(&gc_interface,offset_box,global_box,&olisp_state);

        base_library0 =  EBM_allocate_olisp_tiny_eval_simple_environment(&gc_interface,offset_box,global_box,&olisp_state);
        
        EBM_olisp_tiny_set_library0( base_library0,&gc_interface,&olisp_state);

        uintptr_t library0_cell =
            olisp_tiny_allocate_lib0_cell(
                    base_library0,
                    EBM_olisp_tiny_allocate,
                    gc_allocator_env,
                    &olisp_state);


        EBM_vector_set_CA(
                eval_env,
                4,
                EBM_allocate_pair(
                    library0_cell,
                    EBM_vector_ref_CA(
                        eval_env,
                        4),
                    gc_interface.allocator,
                    gc_interface.env),
                gc_interface.write_barrier,
                gc_interface.env);
    }



    char olisp_tiny_lib_dir[] = "./lib/olisp_tiny/";
    {//TODO:Add support for getting libpath from command line arguments.

    }
    
    bootstrap(olisp_tiny_lib_dir,&gc_interface,&olisp_state,eval_env);


    if (argc == 1){
        repl(&gc_interface,&olisp_state,eval_env);
    }else if (argc == 2){
        char *filename = argv[1];
        load_script(filename,&gc_interface,&olisp_state,eval_env); 
    }
}
