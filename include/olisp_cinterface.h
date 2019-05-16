#ifndef OLISP_CINTERFACE_H
#define OLISP_CINTERFACE_H

#include "ebm.h"
#include<stdarg.h>

#include<setjmp.h>

#define OLISP_CINTERFACE_INSERT_TYPE_CHECK 1

#define OLISP_CINTERFACE_TYPE_CHECK_BLOCK if (OLISP_CINTERFACE_INSERT_TYPE_CHECK)

#define OLISP_CINTERFACE_TYPE_ERROR(message,state) OLISP_simple_error(message,state)

#define OLISP_ARG1_SIZE 7
typedef struct {
    EBM_ALLOCATOR allocator;
    uintptr_t allocator_env;
    uintptr_t arg_size;
    EBM_GC_INTERFACE *gc_interface;
    uintptr_t args1[OLISP_ARG1_SIZE];
    uintptr_t args2;
    uintptr_t returns;
    uintptr_t runtime_var1;
    uintptr_t error_stack;
    uintptr_t symbol_intern;
    uintptr_t default_output_port;
}OLISP_state;

typedef uintptr_t (*OLISP_cfun)(OLISP_state*);

uintptr_t OLISP_cfun_call(OLISP_state *state,OLISP_cfun cfun,uintptr_t arg_size,...);
uintptr_t OLISP_fun_call(OLISP_state *state);

void OLISP_set_arg(OLISP_state *state,uintptr_t pos,uintptr_t arg);
void OLISP_set_arg_size(OLISP_state *state,uintptr_t size);
uintptr_t OLISP_create_function_for_ebm(OLISP_cfun cfun,EBM_ALLOCATOR  allocator,uintptr_t allocator_env);
uintptr_t OLISP_create_function_for_ebm_with_name(OLISP_cfun cfun,uintptr_t name_symbol,EBM_ALLOCATOR allocator,uintptr_t allocator_env);

void EBM_C_exception_handler_set(jmp_buf *jbuf,uintptr_t env,EBM_ALLOCATOR allocator,uintptr_t allocator_env);

uintptr_t OLISP_error(OLISP_state *state);

uintptr_t OLISP_AUX_set_exception_handler_CA(jmp_buf *jbuf,OLISP_state *state);
uintptr_t OLISP_AUX_pop_exception_handler(OLISP_state *state);

#define OLISP_TRY(state) \
{\
    jmp_buf jbuf;\
    OLISP_AUX_set_exception_handler_CA(&jbuf,state);\
    if (setjmp(jbuf) == 0){

#define OLISP_CATCH(error_object,state) _OLISP_CATCH(_TMPLABEL##__LINE__,error_object,state)
#define _OLISP_CATCH(unique_label,error_object,state) \
    }else{\
        OLISP_AUX_pop_exception_handler(state);\
        goto unique_label;\
    }\
}\
if (0)\
    unique_label:

uintptr_t OLISP_simple_error(char *message,OLISP_state *state);

uintptr_t OLISP_symbol_intern(uintptr_t symbol,OLISP_state *state);
#endif
