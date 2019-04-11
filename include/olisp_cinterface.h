#ifndef OLISP_CINTERFACE_H
#define OLISP_CINTERFACE_H

#include "ebm.h"
#include<stdarg.h>

#define OLISP_CINTERFACE_INSERT_TYPE_CHECK 1

#define OLISP_CINTERFACE_TYPE_CHECK_BLOCK if (OLISP_CINTERFACE_INSERT_TYPE_CHECK)

#define OLISP_CINTERFACE_TYPE_ERROR(message) OLISP_simple_error(message)

#define OLISP_ARG1_SIZE 7
typedef struct {
    EBM_ALLOCATOR allocator;
    uintptr_t allocator_env;
    uintptr_t arg_size;
    EBM_GC_INTERFACE *gc_interface;
    uintptr_t args1[OLISP_ARG1_SIZE];
    uintptr_t args2;
    uintptr_t returns;
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


void OLISP_simple_error(char *message);
#endif
