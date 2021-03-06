#include "olisp_cinterface.h"
#include "ebm.h"
#include "ebm_frontend.h"

uintptr_t OLISP_car(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 car)\n",state);
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required car)",state);
        }
    }
    uintptr_t pair = state->args1[0];
    return EBM_CAR(pair);
}

uintptr_t OLISP_set_car(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 set-car)\n",state);
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required set-car)",state);
        }
    }

    return EBM_set_car( state->args1[0], state->args1[1], state->gc_interface);
}

uintptr_t OLISP_cdr(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 cdr)\n",state);
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required cdr)\n",state);
        }
    }
    uintptr_t pair = state->args1[0];
    return EBM_CDR(pair);
}

uintptr_t OLISP_set_cdr(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 set-cdr)\n",state);
        }
        if (!EBM_IS_PAIR_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR pair-required set-cdr)",state);
        }
    }

    return EBM_set_cdr( state->args1[0], state->args1[1], state->gc_interface);
}

uintptr_t OLISP_cons(OLISP_state *state){

    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 cons)\n",state);
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

uintptr_t OLISP_pair_p(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 pair?)\n",state);
        }
    }
    return EBM_IS_PAIR_CR(state->args1[0])?EBM_TRUE:EBM_FALSE;
}

uintptr_t OLISP_symbol_p(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 symbol?)\n",state);
        }
    }
    return EBM_IS_SYMBOL_CR(state->args1[0])?EBM_TRUE:EBM_FALSE;
}

uintptr_t OLISP_make_byte_vector(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size > 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 or 3 make-vector)\n",state);
        }
    }

    uintptr_t res = 
        EBM_allocate_byte_vector_CA(
                EBM_FX_NUMBER_TO_C_INTEGER_CR(state->args1[0]),
                state->allocator,
                state->allocator_env);

    return res;
}

uintptr_t OLISP_byte_vector_set(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 3){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 3 byte-vector-set!)\n",state);
        }
    }
    EBM_byte_vector_set_CA(state->args1[0],EBM_FX_NUMBER_TO_C_INTEGER_CR(state->args1[1]),EBM_FX_NUMBER_TO_C_INTEGER_CR(state->args1[2]));
    return EBM_UNDEF;
}

uintptr_t OLISP_write_simple(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size == 0||state->arg_size > 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 write-simple)\n",state);
        }
    }

    uintptr_t port = state->default_output_port;
    if (state->arg_size == 2){
        port = state->args1[0];
    }
    EBM_write_simple(state->args1[0],port);
    return EBM_UNDEF;
}

uintptr_t OLISP_record_p(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 1){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 1 record?)\n",state);
        }
    }
    return EBM_IS_RECORD_CR(state->args1[0])?EBM_TRUE:EBM_FALSE;
}

uintptr_t OLISP_record_ref(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 record-ref)\n",state);
        }
        //TODO:add support big number
        if (!EBM_IS_FX_NUMBER_CR(state->args1[1])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR integer required record-ref)\n",state);
        }

        if (!EBM_IS_RECORD_CR(state->args1[0])){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR record required record-ref)\n",state);
        }
    }

    return EBM_record_ref_CA(
            state->args1[0],
            EBM_FX_NUMBER_TO_C_INTEGER_CR(state->args1[1]));
}

uintptr_t OLISP_make_record(OLISP_state *state){
    uintptr_t res = EBM_allocate_record_CA(state->args1[0],state->allocator,state->allocator_env);
    //OLISP_get_arg
    int i;
    for (i=0;i<state->arg_size;i++){
        EBM_record_primitive_set_CA(res,i,OLISP_get_arg(state,i));
    }
    return res;
}

uintptr_t OLISP_fx_add(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 fx+)\n",state);
        }

        if (!EBM_IS_FX_NUMBER_CR(state->args1[0]) ||
            !EBM_IS_FX_NUMBER_CR(state->args1[1])){
        
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR fx-number required fx+ )\n",state);
        }
    }

    return EBM_FX_ADD(state->args1[0],state->args1[1]);
}

uintptr_t OLISP_fx_shift_right(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 fxarithmetic-shift-right )\n",state);
        }

        if (!EBM_IS_FX_NUMBER_CR(state->args1[0]) ||
            !EBM_IS_FX_NUMBER_CR(state->args1[1])){
        
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR fx-number required fxarithmetic-shift-right)\n",state);
        }
    }
    return EBM_FX_SHIFT_RIGHT(state->args1[0],state->args1[1]);
}
uintptr_t OLISP_fx_shift_left(OLISP_state *state){
    OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
        if (state->arg_size != 2){
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR wrong-number-of-arguments 2 fxarithmetic-shift-right )\n",state);
        }

        if (!EBM_IS_FX_NUMBER_CR(state->args1[0]) ||
            !EBM_IS_FX_NUMBER_CR(state->args1[1])){
        
            OLISP_CINTERFACE_TYPE_ERROR("(ERROR fx-number required fxarithmetic-shift-right)\n",state);
        }
    }
    return EBM_FX_SHIFT_LEFT(state->args1[0],state->args1[1]);
}
