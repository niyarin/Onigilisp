#include "olisp_cinterface.h"
#include "ebm.h"


void OLISP_set_arg(OLISP_state *state,uintptr_t pos,uintptr_t arg){
    if (pos < OLISP_ARG1_SIZE){
        state->args1[pos] = arg;
    }else{
        if (EBM_vector_length_CR(state->args2) > pos-OLISP_ARG1_SIZE){
            state->args2 = EBM_vector_re_allocate_CA(state->args2,pos,EBM_NULL,state->allocator,state->allocator_env);
        }
        EBM_vector_primitive_set_CA(state->args2,pos-OLISP_ARG1_SIZE,arg);
    }
}

void OLISP_set_arg_size(OLISP_state *state,uintptr_t size){
    state->arg_size = size;
}

uintptr_t OLISP_get_arg(OLISP_state *state,uintptr_t pos){
    if (pos < OLISP_ARG1_SIZE){
        return state->args1[pos];
    }else{
        return EBM_vector_ref_CA(state->args2,pos-OLISP_ARG1_SIZE);
    }   
}

uintptr_t OLISP_cfun_call(OLISP_state *state,OLISP_cfun cfun,uintptr_t arg_size,...){
    va_list argp;
    va_start(argp,arg_size);

    OLISP_set_arg_size(state,arg_size);
    int i;
    for (i=0;i<arg_size;i++){
        OLISP_set_arg(state,i,va_arg(argp,uintptr_t));
    }
    
    return cfun(state);
}
