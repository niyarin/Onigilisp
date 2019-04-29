#include "olisp_cinterface.h"
#include "ebm.h"
#include "ebm_frontend.h"

uintptr_t OLISP_car(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 car)\n");
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required car)\n");
        }
    }
    uintptr_t pair = state->args1[0];
    return EBM_CAR(pair);
}

uintptr_t OLISP_cdr(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 cdr)\n");
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required cdr)\n");
        }
    }
    uintptr_t pair = state->args1[0];
    return EBM_CDR(pair);
}

uintptr_t OLISP_cons(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 cons)\n");
        }
    }

    uintptr_t pair = EBM_allocate_pair(state->args1[0],state->args1[1],state->allocator,state->allocator_env);
    return pair;
}

uintptr_t OLISP_eq(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        //TODO
    }

    return EBM_eq(state->args1[0],state->args1[1]);
}

uintptr_t OLISP_vector(OLISP_state *state){
    uintptr_t vector = EBM_allocate_vector_CA(state->arg_size,state->allocator, state->allocator_env);
    int i;
    for (i=0;i<state->arg_size;i++){
        EBM_vector_primitive_set_CA(
                vector,
                i,
                OLISP_get_arg(state,i));
    }
    return vector;
}

uintptr_t OLISP_write_simple(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size == 0||state->arg_size > 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 write-simple)\n");
        }
    }

    uintptr_t port = state->default_output_port;
    if (state->arg_size == 2){
        port = state->args1[0];
    }
    EBM_write_simple(state->args1[0],port);
    return EBM_UNDEF;
}
