#ifndef OLISP_CINTERFACE_H
#define OLISP_CINTERFACE_H

#include "ebm.h"
#include<stdarg.h>

/*
#define OLISP_ARG1_SIZE 7
typedef struct {
    EBM_ALLOCATOR allocator;
    uintptr_t allocate_env;
    uintptr_t arg_size;
    uintptr_t args1[OLISP_ARG1_SIZE];
    uintptr_t args2;
    uintptr_t returns;
    uintptr_t error_stack;
    uintptr_t symbol_intern;
}OLISP_state;
*/

#define OLISP_ARG1_SIZE 7
typedef struct {
    EBM_ALLOCATOR allocator;
    uintptr_t allocator_env;
    uintptr_t arg_size;
    uintptr_t args1[OLISP_ARG1_SIZE];
    uintptr_t args2;
    uintptr_t returns;
    uintptr_t error_stack;
    uintptr_t symbol_intern;
}OLISP_state;

typedef uintptr_t (*OLISP_cfun)(OLISP_state*);

uintptr_t OLISP_cfun_call(OLISP_state *state,OLISP_cfun cfun,uintptr_t arg_size,...);

void OLISP_set_arg(OLISP_state *state,uintptr_t pos,uintptr_t arg);
void OLISP_set_arg_size(OLISP_state *state,uintptr_t size);



/*
OLISP_state* OLISP_allocate_state(EBM_ALLOCATOR allocator,uintptr_t env);

uintptr_t OLISP_fun_call(OLISP_state *state);
*/
#endif
