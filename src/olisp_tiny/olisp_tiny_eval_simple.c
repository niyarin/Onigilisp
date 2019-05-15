#include "ebm.h"
#include "ebm_aux.h"

#include "olisp_tiny_eval_simple.h"
#include "olisp_tiny_functions.h"
#include "olisp_cinterface.h"

#include<stdio.h>

#define OLISP_TINY_SIMPLE_LENGTH_OF_FUCTION_NAME 13

#define SYNTAX_FX_NUMBER_DEFINE EBM_allocate_FX_NUMBER_CA(0)
#define SYNTAX_FX_NUMBER_LAMBDA EBM_allocate_FX_NUMBER_CA(1)
#define SYNTAX_FX_NUMBER_IF EBM_allocate_FX_NUMBER_CA(2)
#define SYNTAX_FX_NUMBER_SET EBM_allocate_FX_NUMBER_CA(3)
#define SYNTAX_FX_NUMBER_QUOTE EBM_allocate_FX_NUMBER_CA(4)
#define SYNTAX_FX_NUMBER_DEFINE_RECORD_TYPE EBM_allocate_FX_NUMBER_CA(5)
#define SYNTAX_FX_NUMBER_BEGIN EBM_allocate_FX_NUMBER_CA(6)
#define SYNTAX_FX_NUMBER_DEFINE_LIBRARY EBM_allocate_FX_NUMBER_CA(7)
#define SYNTAX_FX_NUMBER_IMPORT EBM_allocate_FX_NUMBER_CA(8)
#define SYNTAX_FX_NUMBER_EXPORT EBM_allocate_FX_NUMBER_CA(9)
#define SYNTAX_FX_NUMBER_IR_MACRO_TRANSFORMER  EBM_allocate_FX_NUMBER_CA(10)
#define SYNTAX_FX_NUMBER_DEFINE_SYNTAX  EBM_allocate_FX_NUMBER_CA(11)


#define SYNTAX_FX_NUMBER_SYNTACTIC_LSET EBM_allocate_FX_NUMBER_CA(97)

#define SYNTAX_FX_NUMBER_SYNTACTIC_QUOTE EBM_allocate_FX_NUMBER_CA(98)
#define SYNTAX_FX_NUMBER_COMPARE_SYMBOL EBM_allocate_FX_NUMBER_CA(99)
#define SYNTAX_FX_NUMBER_FRUN EBM_allocate_FX_NUMBER_CA(100)

#define OLISP_TINY_CLOSUR_FX_NUMBER EBM_allocate_FX_NUMBER_CA(2)

#define OLISP_TINY_SIMPLE_CAN_EVAL(x) (EBM_IS_PAIR_CR(x) || EBM_IS_SYMBOL_CR(x))

//DEBUG #####################################################

static uintptr_t EBM_olisp_tiny_reverse_simple_function(uintptr_t list,EBM_ALLOCATOR allocator,uintptr_t allocator_env);

static uintptr_t write(uintptr_t obj,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t output_port =  EBM_frontend_allocate_output_file_port_CA(stdout,allocator,allocator_env);
    
    EBM_write_simple(obj,output_port);
    EBM_newline(output_port);
}

///////////////////////////////////////////////////////////////

static uintptr_t _OLISP_allocate_namespace_ref(uintptr_t symbol,uintptr_t environment,OLISP_state *state);
static uintptr_t _OLISP_allocate_global_ref(uintptr_t addr_fx,uintptr_t original_symbol,OLISP_state *state);

static uintptr_t _EBM_olisp_tiny_expand_simple_internal(uintptr_t expression,uintptr_t environment,uintptr_t local_cell,EBM_GC_INTERFACE *gc_interface,OLISP_state *state);


static uintptr_t EBM_olisp_tiny_lookup(uintptr_t simple_hash_table,uintptr_t symbol){
    return EBM_simple_hash_table_ref(simple_hash_table,symbol);
}

//TODO:未完成
static uintptr_t EBM_olisp_tiny_lookup_rec(uintptr_t trie,uintptr_t symbol){

    if (EBM_IS_RECORD_CR(symbol)&&
        EBM_IS_SYMBOL_CR(EBM_record_first(symbol))){

        if (EBM_symbol_compare_cstring_CACR(EBM_record_first(symbol),"<namespace-ref>")){
            uintptr_t res_tmp = 
                EBM_olisp_tiny_lookup_rec(
                        EBM_vector_ref_CA(EBM_record_ref_CA(symbol,2),2),
                        EBM_record_ref_CA(symbol,1));
            if (res_tmp == EBM_UNDEF){
                return symbol;
            }
            return res_tmp;
        }

        return symbol;
    }
    if (!EBM_IS_SYMBOL_CR(symbol)){
        return symbol;
    }
    uintptr_t res = EBM_olisp_tiny_lookup(trie,symbol);
    return EBM_olisp_tiny_lookup_rec(trie,res);
}


static uintptr_t EBM_olisp_tiny_reverse_simple_function(uintptr_t list,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t cell = list;
    uintptr_t res = EBM_NULL;
    while (cell != EBM_NULL){
        res = EBM_allocate_pair(
                EBM_CAR(cell),
                res,
                allocator,
                allocator_env);
        cell = EBM_CDR(cell);
    }
    return res;
}

uintptr_t OLISP_tiny_eval_closure(uintptr_t code,uintptr_t lexical_information,uintptr_t args,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t res = EBM_allocate_record_CA(6,allocator,allocator_env);
    EBM_record_primitive_set_CA(res,0,EBM_BUILT_IN_RECORD_TYPE_OLISP_FUNCTION);
    EBM_record_primitive_set_CA(res,1,EBM_allocate_FX_NUMBER_CA(1));
    EBM_record_primitive_set_CA(res,2,EBM_allocate_FX_NUMBER_CA(2));
    EBM_record_primitive_set_CA(res,3,code);
    EBM_record_primitive_set_CA(res,4,lexical_information);
    EBM_record_primitive_set_CA(res,5,args);
    return res;
}



uintptr_t EBM_olisp_eval_simple(uintptr_t expanded_expression,uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state){
    uintptr_t eval_environment = EBM_vector_ref_CA(environment,1);
    EBM_ALLOCATOR allocator = gc_interface->allocator;
    uintptr_t allocator_env = gc_interface->env;

    uintptr_t namespace_ref_symbol;
    uintptr_t global_ref_symbol;
    {
        namespace_ref_symbol =
            OLISP_symbol_intern(
               EBM_allocate_symbol_from_cstring_CA(
                   "<namespace-ref>",
                   olisp_state->allocator,
                   olisp_state->allocator_env),
                olisp_state);
        global_ref_symbol = 
            OLISP_symbol_intern(
                EBM_allocate_symbol_from_cstring_CA(
                   "<global-ref>",
                   olisp_state->allocator,
                   olisp_state->allocator_env),
                olisp_state);
    }

    uintptr_t stack = EBM_NULL;//((code,next-evals,evaled,lex) ... )
    uintptr_t res = EBM_UNDEF;

    uintptr_t code = expanded_expression;
    uintptr_t eval_info = EBM_NULL;// (次の評価 . 評価済み)
    uintptr_t lexical_information = EBM_vector_ref_CA(environment,3);
    uintptr_t global = EBM_CAR(EBM_vector_ref_CA(environment,8));
    olisp_state->runtime_var1 = 
        EBM_allocate_vector_CA(3,allocator,allocator_env);

    while (stack != EBM_NULL ||  code != EBM_NULL){
        if (EBM_IS_PAIR_CR(code)){
            uintptr_t operator = EBM_CAR(code);
            switch(operator){
                case SYNTAX_FX_NUMBER_DEFINE:
                    {
                        if (OLISP_TINY_SIMPLE_CAN_EVAL(EBM_CADDR(code))){
                            if (EBM_NULL == eval_info){
                               stack = EBM_allocate_pair(
                                           EBM_allocate_rev_list(
                                               4,
                                               allocator,
                                               allocator_env,
                                               lexical_information,
                                               EBM_NULL,
                                               EBM_CDDR(code),
                                               code
                                               ),
                                           stack,
                                           allocator,
                                           allocator_env);

                               code = EBM_CADDR(code);
                               eval_info = EBM_NULL;
                               continue;
                           }else{
                               EBM_vector_set_CA(
                                       global,
                                       EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_second(EBM_CADR(code))),
                                       EBM_CADR(eval_info),
                                       gc_interface->write_barrier,
                                       gc_interface->env);

                               res = EBM_UNDEF;
                           }
                        }else{
                            EBM_vector_set_CA(
                                       global,
                                       EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_second(EBM_CADR(code))),
                                       EBM_CADDR(code),
                                       gc_interface->write_barrier,
                                       gc_interface->env);

                            res = EBM_UNDEF;
                        }
                    }
                    break;
                case SYNTAX_FX_NUMBER_SYNTACTIC_LSET:
                    {
                        if (OLISP_TINY_SIMPLE_CAN_EVAL(EBM_CADDR(code))){
                            if (EBM_NULL == eval_info){
                               stack = EBM_allocate_pair(
                                           EBM_allocate_rev_list(
                                               4,
                                               allocator,
                                               allocator_env,
                                               lexical_information,
                                               EBM_NULL,
                                               EBM_CDDR(code),
                                               code
                                               ),
                                           stack,
                                           allocator,
                                           allocator_env);

                               code = EBM_CADDR(code);
                               eval_info = EBM_NULL;
                               continue;
                           }else{
                               uintptr_t cell = lexical_information;
                               while (cell != EBM_NULL){
                                    if (EBM_CAAR(cell) == EBM_CADR(code)){
                                        EBM_set_cdr(
                                                EBM_CAR(cell),
                                                EBM_CADR(eval_info),
                                                gc_interface);
                                        break;
                                    }
                                    cell = EBM_CDR(cell);
                               }
                               res = EBM_UNDEF;
                           }
                        }else{
                           uintptr_t cell = lexical_information;
                           while (cell != EBM_NULL){
                                if (EBM_CAAR(cell) == EBM_CADR(code)){
                                    EBM_set_cdr(
                                            EBM_CAR(cell),
                                            EBM_CADR(EBM_CDR(code)),
                                            gc_interface);
                                    break;
                                }
                                cell = EBM_CDR(cell);
                           }
                           res = EBM_UNDEF;

                        }
                    }
                    break;
                case SYNTAX_FX_NUMBER_QUOTE:
                    {
                        res = EBM_CADR(code);
                        code = EBM_NULL;
                    }
                    break;
                case SYNTAX_FX_NUMBER_LAMBDA:
                    {
                        uintptr_t closure = 
                            OLISP_tiny_eval_closure(
                                    EBM_CADDR(code),
                                    lexical_information,
                                    EBM_CADR(code),
                                    allocator,
                                    allocator_env);
                        res = closure;
                        code = EBM_NULL;
                    }
                    break;
                case SYNTAX_FX_NUMBER_BEGIN:
                    {

                        if (eval_info == EBM_NULL){
                            if (EBM_CDR(code) == EBM_NULL){
                                eval_info = 
                                    EBM_allocate_pair(
                                        EBM_NULL,
                                        EBM_allocate_pair(
                                            EBM_UNDEF,
                                            EBM_NULL,
                                            allocator,
                                            allocator_env),
                                        allocator,
                                        allocator_env);

                            }else{
                                eval_info = 
                                    EBM_allocate_pair(
                                        EBM_CDR(code),
                                        EBM_NULL,
                                        allocator,
                                        allocator_env);
                            }
                        }

                        if (EBM_CAR(eval_info) == EBM_NULL){
                            res = EBM_CADR(eval_info);
                            eval_info = EBM_NULL;
                        }else{
                           if (OLISP_TINY_SIMPLE_CAN_EVAL(EBM_CAAR(eval_info))){
                                stack = EBM_allocate_pair(
                                   EBM_allocate_rev_list(
                                       4,
                                       allocator,
                                       allocator_env,
                                       lexical_information,
                                       EBM_CDR(eval_info),
                                       EBM_CAR(eval_info),
                                       code),
                                   stack,
                                   allocator,
                                   allocator_env);
                                code = EBM_CAAR(eval_info);
                                eval_info = EBM_NULL;
                                continue;         
                           }else{
                               eval_info = 
                                   EBM_allocate_pair(
                                           EBM_CDAR(eval_info),
                                           EBM_allocate_pair(
                                               EBM_CAAR(eval_info),
                                               EBM_CDR(eval_info),
                                               allocator,
                                               allocator_env),
                                           allocator,
                                           allocator_env);
                               continue;
                           } 
                        }
                        break;
                    }
                case SYNTAX_FX_NUMBER_COMPARE_SYMBOL:
                    {//TODO:eval_info baseに書き換える
                        uintptr_t symb = EBM_CDR(EBM_CAR(lexical_information));
                        uintptr_t syma = EBM_CDR(EBM_CADR(lexical_information));
                        if (syma == symb){
                            res = EBM_TRUE;
                        }else{
                            
                            printf("TBA!\n");
                            //exit(1);
                            res = EBM_FALSE;//atode
                        }
                    }
                    break;
                case SYNTAX_FX_NUMBER_FRUN:
                    {
                        if (EBM_NULL == eval_info){
                            eval_info = EBM_allocate_pair(
                                            EBM_CDDR(code),
                                            EBM_NULL,
                                            allocator,
                                            allocator_env);
                            continue;
                        }
                        
                        if (EBM_CAR(eval_info) == EBM_NULL){
                            uintptr_t length = 
                                EBM_FX_NUMBER_TO_C_INTEGER_CR(
                                    EBM_CADR(code));
                            if (length+1 > OLISP_ARG1_SIZE){
                                //error
                                //TODO:atode
                                printf("ATODE!\n");
                                exit(1);
                            }
                            
                            uintptr_t cell = EBM_CDR(eval_info);
                            int i;
                            OLISP_set_arg_size(olisp_state,length+1);
                            for (i=0;i<length+1;i++){
                                OLISP_set_arg(
                                        olisp_state,
                                        length-i,
                                        EBM_CAR(cell));
                                cell = EBM_CDR(cell);
                            }

                            uintptr_t fun =  OLISP_get_arg(olisp_state,0);
                            if (!EBM_IS_RECORD_CR(fun)){
                                write(fun,allocator,allocator_env);
                                write(code,allocator,allocator_env);
                                printf("ERROR::%s %d\n",__FILE__,__LINE__);
                                exit(1);
                            }


                            EBM_vector_primitive_set_CA(
                                    olisp_state->runtime_var1,
                                    0,
                                    code);

                            EBM_vector_primitive_set_CA(
                                    olisp_state->runtime_var1,
                                    1,
                                    EBM_CDR(eval_info));

                            EBM_vector_primitive_set_CA(
                                    olisp_state->runtime_var1,
                                    2,
                                    stack);

                            if (EBM_record_ref_CA(fun,1) == 0){
                                uintptr_t _res = OLISP_fun_call(olisp_state);
                                res = _res;
                            }else if (EBM_record_ref_CA(fun,1) == EBM_allocate_FX_NUMBER_CA(1)){
                                //closure
                                code = EBM_record_ref_CA(fun,3);
                                lexical_information = 
                                    EBM_record_ref_CA(
                                            fun,
                                            4);

                                uintptr_t args = EBM_olisp_tiny_reverse_simple_function(EBM_CDR(eval_info),allocator,allocator_env);

                                args = EBM_CDR(args);
                                uintptr_t formals = EBM_record_ref_CA(fun,5);
                                while (formals!=EBM_NULL){
                                    if (EBM_IS_SYMBOL_CR(formals)){
                                    
                                        lexical_information = 
                                            EBM_allocate_pair(
                                                EBM_allocate_pair(
                                                    formals,
                                                    args,
                                                    allocator,
                                                    allocator_env),
                                                lexical_information,
                                                allocator,
                                                allocator_env);
                                        break;
                                    }
                                     
                                    lexical_information = 
                                        EBM_allocate_pair(
                                            EBM_allocate_pair(
                                                EBM_CAR(formals),
                                                EBM_CAR(args),
                                                allocator,
                                                allocator_env),
                                            lexical_information,
                                            allocator,
                                            allocator_env);
                                    formals = EBM_CDR(formals);
                                    args = EBM_CDR(args);
                                }
                                eval_info = EBM_NULL;
                                continue;
                            }
                        }else if OLISP_TINY_SIMPLE_CAN_EVAL(EBM_CAR(eval_info)){
                           stack = EBM_allocate_pair(
                                       EBM_allocate_rev_list(
                                           4,
                                           allocator,
                                           allocator_env,
                                           lexical_information,
                                           EBM_CDR(eval_info),
                                           EBM_CAR(eval_info),
                                           code
                                           ),
                                       stack,
                                       allocator,
                                       allocator_env);
                           code = EBM_CAAR(eval_info);
                           eval_info = EBM_NULL;
                           continue;
                        }else{
                            eval_info = EBM_allocate_pair(
                                            EBM_CDR(EBM_CAR(eval_info)),
                                            EBM_allocate_pair(
                                                EBM_CAR(eval_info),
                                                EBM_CDR(eval_info),
                                                allocator,
                                                allocator_env),
                                            allocator,
                                            allocator_env);
                            continue;
                        }
                    }
                    break;
                case SYNTAX_FX_NUMBER_DEFINE_LIBRARY:
                    {
                        EBM_olisp_eval_simple(
                                EBM_CDDR(code),
                                EBM_CADR(code),
                                gc_interface,
                                olisp_state);
                        return EBM_UNDEF;
                    }
                    break;
                case SYNTAX_FX_NUMBER_IF:
                    {
                        if (eval_info == EBM_NULL){
                            if (OLISP_TINY_SIMPLE_CAN_EVAL(EBM_CADR(code))){
                                eval_info =
                                    EBM_allocate_pair(
                                            EBM_CADR(code),
                                            EBM_NULL,
                                            allocator,
                                            allocator_env);
 
                                stack = EBM_allocate_pair(
                                           EBM_allocate_rev_list(
                                               4,
                                               allocator,
                                               allocator_env,
                                               lexical_information,
                                               EBM_CDR(eval_info),
                                               EBM_CAR(eval_info),
                                               code
                                               ),
                                           stack,
                                           allocator,
                                           allocator_env);
                                code = EBM_CADR(code);
                                eval_info = EBM_NULL;
                                continue;
                            }else if (EBM_CADR(code) == EBM_FALSE){
                                code = EBM_CADDR(EBM_CDR(code));
                                eval_info = EBM_NULL;
                                continue;
                            }else{
                                code = EBM_CADDR(code);
                                eval_info = EBM_NULL;
                                continue;
                            }
                        }
                        if (EBM_CADR(eval_info) == EBM_FALSE){
                            code = EBM_CADDR(EBM_CDR(code));
                            eval_info = EBM_NULL;
                            continue;                           
                        }else{
                            code = EBM_CADDR(code);
                            eval_info = EBM_NULL;
                            continue;                                                  
                        }
                        
                    }
                    break;
                dafault:
                    {
                        printf("ERR\n");exit(1);
                    
                    }
            }
        }else if (EBM_IS_SYMBOL_CR(code)){
            uintptr_t cell = lexical_information;
            uintptr_t _res = EBM_UNDEF;
            while (cell != EBM_NULL){
                if (EBM_CAAR(cell) == code){
                    _res = EBM_CDAR(cell);
                }
                cell = EBM_CDR(cell);
            }
            if (_res == EBM_UNDEF){
                _res =  EBM_olisp_tiny_lookup(eval_environment,code);
                if (_res == EBM_UNDEF){
                    //ERROR
                    write(code,allocator,allocator_env);
                    printf("//EXIT\n");
                    exit(1);
                }
            }
            res = _res;
        }else if (EBM_IS_RECORD_CR(code) 
                && EBM_record_first(code) == global_ref_symbol){
            res = code;
            while (EBM_IS_RECORD_CR(res) 
                && EBM_record_first(res) == global_ref_symbol){
                res = EBM_vector_ref_CA(global,EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_second(res)));
            }

OLISP_CINTERFACE_TYPE_CHECK_BLOCK{
            if (res == EBM_UNDEF){
                EBM_vector_primitive_set_CA(
                        olisp_state->runtime_var1,
                        0,
                        EBM_record_second(code));

                EBM_vector_primitive_set_CA(
                        olisp_state->runtime_var1,
                        2,
                        stack);

                OLISP_simple_error("(error variable not found.)", olisp_state);
            }
}
        }else{
            res = code;
        }

        code = EBM_NULL;
        if (stack != EBM_NULL){
            uintptr_t stack_cell = EBM_CAR(stack);
            code = EBM_CAR(stack_cell);
            eval_info = EBM_allocate_pair(
                            EBM_CDR(EBM_CADR(stack_cell)),
                            EBM_allocate_pair(
                                res,
                                EBM_CADDR(stack_cell),
                                allocator,
                                allocator_env),
                            allocator,
                            allocator_env);
            lexical_information = EBM_CAR(EBM_CDDDR(stack_cell));
            stack = EBM_CDR(stack);
        }
    }
    return res;
}

static uintptr_t _EBM_olisp_allocate_ir_macro_object(uintptr_t lambda,uintptr_t environment,uintptr_t local_cell,OLISP_state *state){
     uintptr_t res = EBM_allocate_record_CA(4,state->allocator,state->allocator_env);

    EBM_record_primitive_set_CA(res,
                                0,
                                OLISP_symbol_intern(
                                   EBM_allocate_symbol_from_cstring_CA(
                                       "<ir-macro-object>",
                                       state->allocator,
                                       state->allocator_env),
                                    state));
    EBM_record_primitive_set_CA(res,1,lambda);
    EBM_record_primitive_set_CA(res,2,environment);   
    EBM_record_primitive_set_CA(res,3,local_cell);   
    return res;

}


static uintptr_t _EBM_olisp_aux_recursive_expand_simple(uintptr_t expression,uintptr_t environment,uintptr_t local_cell,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state){

    if (expression == EBM_NULL){
        return EBM_NULL;
    }
    uintptr_t expanded = 
            _EBM_olisp_tiny_expand_simple_internal(EBM_CAR(expression),environment,local_cell,gc_interface,olisp_state);

    return EBM_allocate_pair( 
            expanded,
            _EBM_olisp_aux_recursive_expand_simple(EBM_CDR(expression),environment,local_cell,gc_interface,olisp_state),
            gc_interface->allocator,
            gc_interface->env);
}

static uintptr_t _EBM_olisp_tiny_eval_define(uintptr_t environment,uintptr_t symbol,uintptr_t object,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){

    uintptr_t address_hash = 
        EBM_vector_ref_CA(environment,7);

    uintptr_t global_ref = 
        EBM_simple_hash_table_ref(
                address_hash,
                symbol);
    if (global_ref == EBM_UNDEF){
        uintptr_t address_box =
            EBM_vector_ref_CA(environment,6);
        global_ref = _OLISP_allocate_global_ref(EBM_CAR(address_box),symbol,state);
        EBM_PRIMITIVE_SET_CAR(address_box,EBM_FX_ADD(EBM_CAR(address_box),EBM_allocate_FX_NUMBER_CA(1)));
        EBM_simple_hash_table_set(
                address_hash,
                symbol,
                global_ref,
                gc_interface);
    }
    uintptr_t global = EBM_CAR(EBM_vector_ref_CA(environment,8));
    if (EBM_vector_length_CR(global) <= EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_second(global_ref))){
    
        global = EBM_vector_re_allocate_CA(global,EBM_vector_length_CR(global) * 2,EBM_UNDEF, gc_interface->allocator,gc_interface->env);
        EBM_set_car(
                EBM_vector_ref_CA(environment,8),
                global,
                gc_interface);
    }

    EBM_vector_set_CA(
            global,
            EBM_FX_NUMBER_TO_C_INTEGER_CR(EBM_record_second(global_ref)),
            object,
            gc_interface->write_barrier,
            gc_interface->env);
    return EBM_UNDEF;
}

static uintptr_t _EBM_olisp_tiny_renaming(uintptr_t code,uintptr_t alist_wrap,char rev_flag,uintptr_t env1,uintptr_t env2,uintptr_t local1,uintptr_t local2,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    
    if (EBM_IS_PAIR_CR(code)){
        return 
            EBM_allocate_pair(
                _EBM_olisp_tiny_renaming(
                    EBM_CAR(code),
                    alist_wrap,
                    rev_flag,
                    env1,
                    env2,
                    local1,
                    local2,
                    gc_interface,
                    state),
                _EBM_olisp_tiny_renaming(
                    EBM_CDR(code),
                    alist_wrap,
                    rev_flag,
                    env1,
                    env2,
                    local1,
                    local2,
                    gc_interface,
                    state),
                gc_interface->allocator,
                gc_interface->env);

    }else if (EBM_IS_VECTOR_CR(code)){
        //TODO;
        printf("ERR\n");
        exit(1);
    }else if (EBM_IS_SYMBOL_CR(code)){
        uintptr_t expand_env1 = EBM_vector_ref_CA(env1,2);
        uintptr_t expand_env2 = EBM_vector_ref_CA(env2,2);
         
        uintptr_t lookup_res1 = EBM_olisp_tiny_lookup_rec(expand_env1,code);
        uintptr_t lookup_res2 = EBM_olisp_tiny_lookup_rec(expand_env2,code);

        //TODO:localも調べる

        if (lookup_res1 == lookup_res2
            && lookup_res1 != EBM_UNDEF){
            return code;
        }

        if (lookup_res1 == EBM_UNDEF){
            _EBM_olisp_tiny_eval_define(env1,code,EBM_UNDEF,gc_interface,state);
        }

        uintptr_t alist = EBM_CAR(alist_wrap);
        if (rev_flag){
            uintptr_t apair = EBM_rassq(code,alist);
            if (EBM_IS_PAIR_CR(apair)){
                return EBM_CAR(apair);
            }
            return code;
        }else{
            uintptr_t apair = EBM_assq(code,alist);
            if (apair == EBM_FALSE){
                uintptr_t res = 
                    EBM_allocate_symbol_no_copy(
                        EBM_record_third(code),
                        gc_interface);
                alist = EBM_allocate_pair(
                            EBM_allocate_pair(
                                code,
                                res,
                                gc_interface->allocator,
                                gc_interface->env),
                            alist,
                            gc_interface->allocator,
                            gc_interface->env);
                EBM_set_car(alist_wrap,alist,gc_interface);
                return res;
            }
            return EBM_CDR(apair);
        }
    }else{
        return code;           
    }
}


static uintptr_t _EBM_olisp_tiny_expand_simple_internal(uintptr_t expression,uintptr_t environment,uintptr_t local_cell,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    //Don't set! value to expression.
    uintptr_t namespace_ref_symbol;
    uintptr_t ir_macro_object_symbol;
    {
        namespace_ref_symbol =
            OLISP_symbol_intern(
               EBM_allocate_symbol_from_cstring_CA(
                   "<namespace-ref>",
                   state->allocator,
                   state->allocator_env),
                state);
        ir_macro_object_symbol = 
            OLISP_symbol_intern(
                    EBM_allocate_symbol_from_cstring_CA(
                        "<ir-macro-object>",
                        state->allocator,
                        state->allocator_env),
                    state);

    }

    //TODO:check syntax
    uintptr_t expand_environment = EBM_vector_ref_CA(environment,2);
    if (EBM_IS_PAIR_CR(expression)){
        uintptr_t operator = EBM_CAR(expression);
        uintptr_t new_operator = EBM_UNDEF;

        if (operator == SYNTAX_FX_NUMBER_SYNTACTIC_QUOTE){
            new_operator = EBM_CADR(expression);
            return new_operator;
        }

        if (EBM_IS_PAIR_CR(operator)){
            new_operator = 
                _EBM_olisp_tiny_expand_simple_internal(
                         operator,
                         environment,
                         local_cell,
                         gc_interface,
                         state);
        } else if (EBM_IS_FX_NUMBER_CR(operator)){
            printf("ERRRRRRRRRR\n");
            //error
            exit(1);//TODO;
        }else if (EBM_IS_SYMBOL_CR(operator)){
            new_operator = EBM_olisp_tiny_lookup(expand_environment,operator);
        }

        uintptr_t original_expand_environment = expand_environment;
        while (EBM_IS_RECORD_CR(new_operator) &&
                  EBM_record_first(new_operator) == 
                    namespace_ref_symbol){

            operator = EBM_record_ref_CA(new_operator,1);

            expand_environment = 
                EBM_vector_ref_CA(
                    EBM_record_ref_CA(new_operator,2),
                    2);
            new_operator = EBM_olisp_tiny_lookup(expand_environment,operator);
        }
        expand_environment = original_expand_environment;

        if (EBM_IS_RECORD_CR(new_operator) &&
                EBM_record_first(new_operator) == ir_macro_object_symbol){
            //3-IMPLICIT RENAMING MACRO
            uintptr_t lmd = EBM_record_second(new_operator);
           
            uintptr_t compare_function;
            {//define compare_function
                uintptr_t sym_a = 
                    OLISP_symbol_intern(
                        EBM_allocate_symbol_from_cstring_CA(
                            "a",
                            gc_interface->allocator,
                            gc_interface->env),
                        state);
                        
                uintptr_t sym_b = 
                    OLISP_symbol_intern(
                        EBM_allocate_symbol_from_cstring_CA(
                            "b",
                            gc_interface->allocator,
                            gc_interface->env),
                        state);

                compare_function = 
                    EBM_allocate_rev_list(
                        3,
                        gc_interface->allocator,
                        gc_interface->env,
                        EBM_allocate_rev_list(
                            3,
                            gc_interface->allocator,
                            gc_interface->env,
                            sym_b,
                            sym_a,
                            EBM_allocate_rev_list(
                                2,
                                gc_interface->allocator,
                                gc_interface->env,
                                SYNTAX_FX_NUMBER_COMPARE_SYMBOL,
                                SYNTAX_FX_NUMBER_SYNTACTIC_QUOTE)),
                        EBM_allocate_rev_list(
                            2,
                            gc_interface->allocator,
                            gc_interface->env,
                            sym_b,
                            sym_a),
                        EBM_allocate_pair(
                                SYNTAX_FX_NUMBER_SYNTACTIC_QUOTE,
                                EBM_allocate_pair(
                                    SYNTAX_FX_NUMBER_LAMBDA,
                                    EBM_NULL,
                                    gc_interface->allocator,
                                    gc_interface->env),
                                gc_interface->allocator,
                                gc_interface->env));
               compare_function = _EBM_olisp_tiny_expand_simple_internal(compare_function, environment, local_cell, gc_interface,state);
            }


            uintptr_t defined_env = EBM_record_third(new_operator);
            uintptr_t alist_wrap = 
                EBM_allocate_pair(
                        EBM_NULL,
                        EBM_NULL,
                        gc_interface->allocator,
                        gc_interface->env);

            if (defined_env != environment){
                //locals
                expression = 
                    _EBM_olisp_tiny_renaming(
                            expression,
                            alist_wrap,
                            0,
                            environment,
                            defined_env,
                            local_cell,
                            EBM_record_ref_CA(new_operator,3),
                            gc_interface,
                            state);
                uintptr_t cell = EBM_CAR(alist_wrap);
                while (cell != EBM_NULL){
                    uintptr_t original_symbol = EBM_CAAR(cell);
                    uintptr_t renamed_symbol = EBM_CDAR(cell);

                    uintptr_t reference_object =  //TODO:あとで変数名調整
                        _EBM_olisp_tiny_expand_simple_internal(
                                original_symbol,
                                environment,
                                local_cell,
                                gc_interface,
                                state);

                EBM_simple_hash_table_set(
                        EBM_vector_ref_CA(defined_env,7),
                        renamed_symbol,
                        reference_object,
                        gc_interface);

                    {//TODO:あとでリファクタリング
                        //TODO:セットしたrenamed_symbolの削除
                        
                        uintptr_t namespace_reference_object =  
                            _OLISP_allocate_namespace_ref(
                                                original_symbol,
                                                environment,
                                                state);

                        EBM_simple_hash_table_set(
                                EBM_vector_ref_CA(
                                    defined_env,
                                    2),
                                renamed_symbol,
                                namespace_reference_object,
                                gc_interface);
                        
                     
                    }

                    cell = EBM_CDR(cell);
                }
            }

            //TO QUOTE
            expression = 
                EBM_allocate_pair(
                       SYNTAX_FX_NUMBER_QUOTE,
                       EBM_allocate_pair(
                           expression,
                           EBM_NULL,
                           gc_interface->allocator,
                           gc_interface->env),
                       gc_interface->allocator,
                       gc_interface->env);

            uintptr_t args = 
                EBM_allocate_rev_list(
                        3,
                        gc_interface->allocator,
                        gc_interface->env,
                        compare_function,
                        EBM_UNDEF,
                        expression);

            uintptr_t to_expand_code = 
                EBM_allocate_pair(
                        SYNTAX_FX_NUMBER_FRUN,
                        EBM_allocate_pair(
                            EBM_allocate_FX_NUMBER_CA(3),
                            EBM_allocate_pair(
                                lmd,
                                args,
                                gc_interface->allocator,
                                gc_interface->env),
                            gc_interface->allocator,
                            gc_interface->env),
                        gc_interface->allocator,
                        gc_interface->env);
            uintptr_t ecode = EBM_olisp_eval_simple(to_expand_code, environment,gc_interface,state);
            uintptr_t expanded_code = 
                _EBM_olisp_tiny_expand_simple_internal(
                    ecode,
                    defined_env,
                    local_cell,
                    gc_interface,
                    state);

            expanded_code = 
                _EBM_olisp_tiny_renaming(
                        expanded_code,
                        alist_wrap,
                        1,
                        environment,
                        defined_env,
                        local_cell,
                        EBM_record_ref_CA(new_operator,3),
                        gc_interface,
                        state);
            return expanded_code;
        }

        
        switch (new_operator){
            case SYNTAX_FX_NUMBER_QUOTE:
                {
                    return EBM_allocate_pair(new_operator,EBM_CDR(expression),gc_interface->allocator,gc_interface->env);
                }
            case SYNTAX_FX_NUMBER_DEFINE:
                {
                    uintptr_t sym_object = 
                        _EBM_olisp_tiny_expand_simple_internal(
                            EBM_CADR(expression),
                            environment,
                            local_cell,
                            gc_interface,
                            state);
                    
                    uintptr_t exp = _EBM_olisp_tiny_expand_simple_internal(EBM_CADDR(expression),environment,local_cell, gc_interface,state);
                    return EBM_allocate_rev_list(3,
                            gc_interface->allocator,
                            gc_interface->env,
                            exp,
                            sym_object,
                            SYNTAX_FX_NUMBER_DEFINE);
                }
                break;
            case SYNTAX_FX_NUMBER_SET:
                {

                    uintptr_t exp = _EBM_olisp_tiny_expand_simple_internal(EBM_CADDR(expression),environment,local_cell, gc_interface,state);

                    uintptr_t sym_object = 
                        _EBM_olisp_tiny_expand_simple_internal(
                            EBM_CADR(expression),
                            environment,
                            local_cell,
                            gc_interface,
                            state);

                    if (EBM_IS_SYMBOL_CR(sym_object)){
                        //local set!

                        return EBM_allocate_rev_list(3,
                                gc_interface->allocator,
                                gc_interface->env,
                                exp,
                                sym_object,
                                SYNTAX_FX_NUMBER_SYNTACTIC_LSET);

                    }else{
                        //global set!
                        printf("TBA GLOBAL SET %s %d\n",__FILE__,__LINE__);
                        exit(1);
                    }
                }
                break;
            case SYNTAX_FX_NUMBER_LAMBDA:
                {
                    uintptr_t current_local_cell = EBM_CADR(expression);
                
                    uintptr_t cell = current_local_cell;
                    uintptr_t new_local_cell = local_cell;
                    while (cell != EBM_NULL){
                        if (EBM_IS_SYMBOL_CR(cell)){
                            new_local_cell = 
                                EBM_allocate_pair(
                                        cell,
                                        new_local_cell,
                                        gc_interface->allocator,
                                        gc_interface->env);
                            break;
                        }
                    
                        new_local_cell = 
                            EBM_allocate_pair(
                                    EBM_CAR(cell),
                                    new_local_cell,
                                    gc_interface->allocator,
                                    gc_interface->env);
                        cell = EBM_CDR(cell);
                    }
                    
                    uintptr_t body = 
                        _EBM_olisp_tiny_expand_simple_internal(
                                EBM_CADDR(expression),
                                environment,
                                new_local_cell,
                                gc_interface,
                                state);
                    return
                        EBM_allocate_rev_list(
                           3,
                           gc_interface->allocator,
                           gc_interface->env,
                           body,
                           current_local_cell,
                           new_operator);
                }
                break;
            case SYNTAX_FX_NUMBER_DEFINE_RECORD_TYPE:
                {
                    uintptr_t record_name = EBM_CADR(expression);
                    uintptr_t constructor = EBM_CADDR(expression);
                }
                break;
            case SYNTAX_FX_NUMBER_DEFINE_LIBRARY:
                {
                    uintptr_t offset_box = 
                        EBM_vector_ref_CA(environment,6);
                    uintptr_t global_box = 
                        EBM_vector_ref_CA(environment,8);

                    //外側の環境のimportをマージする
                    uintptr_t new_environment = EBM_allocate_olisp_tiny_eval_simple_environment(gc_interface,offset_box,global_box,state);

                    EBM_vector_set_CA(
                            new_environment,
                            4,
                            EBM_vector_ref_CA(
                                environment,
                                4),
                            gc_interface->write_barrier,
                            gc_interface->env);
                    
                    uintptr_t tails = 
                        _EBM_olisp_aux_recursive_expand_simple(
                                 EBM_CDDR(expression),
                                 new_environment,
                                 local_cell,
                                 gc_interface,
                                 state);

                    uintptr_t library_cell = 
                        EBM_allocate_pair(
                                EBM_CADR(expression),
                                EBM_allocate_pair(
                                    new_environment,
                                    tails,
                                    gc_interface->allocator,
                                    gc_interface->env),
                                gc_interface->allocator,
                                gc_interface->env);
        
                    EBM_vector_set_CA(
                            environment,
                            4,
                            EBM_allocate_pair(
                                library_cell,
                                EBM_vector_ref_CA(
                                    environment,
                                    4),
                                gc_interface->allocator,
                                gc_interface->env),
                            gc_interface->write_barrier,
                            gc_interface->env);
                    
                    tails = EBM_allocate_pair(
                                SYNTAX_FX_NUMBER_BEGIN ,
                                tails,
                                gc_interface->allocator,
                                gc_interface->env);

                    return EBM_allocate_pair(
                            SYNTAX_FX_NUMBER_DEFINE_LIBRARY,
                            EBM_allocate_pair(
                                new_environment,
                                tails,
                                gc_interface->allocator,
                                gc_interface->env),
                            gc_interface->allocator,
                            gc_interface->env);
                }
                break;
            case SYNTAX_FX_NUMBER_COMPARE_SYMBOL:
                {
                    uintptr_t tails = 
                         _EBM_olisp_aux_recursive_expand_simple(
                                 EBM_CDR(expression),
                                 environment,
                                 local_cell,
                                 gc_interface,
                                 state);
                    return 
                        EBM_allocate_pair(
                                SYNTAX_FX_NUMBER_COMPARE_SYMBOL,
                                tails,
                                gc_interface->allocator,
                                gc_interface->env);
                }
                break;
            case SYNTAX_FX_NUMBER_EXPORT:
                {//TODO:EXPORTって複数書けたかな?
                                       
                    EBM_vector_set_CA(
                            environment,
                            5,
                            EBM_CDR(expression),
                            gc_interface->write_barrier,
                            gc_interface->env);
                    return EBM_UNDEF;
                }
                break;
            case SYNTAX_FX_NUMBER_IMPORT:
                {//ひとまず renameとかは考慮しない
                    uintptr_t namespace_list = 
                        EBM_vector_ref_CA(
                                environment,
                                4);

                    uintptr_t import_targets = EBM_CDR(expression);

                    while (import_targets != EBM_NULL){
                        uintptr_t import_target = EBM_CAR(import_targets);
                        uintptr_t flag = 0;
                        
                        uintptr_t namespace_list_cell = namespace_list;
                        while (namespace_list_cell != EBM_NULL){
                            uintptr_t namespace_cell = EBM_CAAR(namespace_list_cell);
                            uintptr_t import_target_cell = import_target;
                            uintptr_t same_name = 1;
                            while (namespace_cell != EBM_NULL
                                      && import_target_cell != EBM_NULL){
                                if (EBM_CAR(namespace_cell) != EBM_CAR(import_target_cell)){
                                    same_name = 0;
                                    break;
                                }
                                namespace_cell = EBM_CDR(namespace_cell);
                                import_target_cell = EBM_CDR(import_target_cell);
                            }

                            if (namespace_cell != EBM_NULL ||
                                    import_target_cell != EBM_NULL){
                                same_name = 0;
                            }
                            if (same_name){
                                flag = EBM_CAR(namespace_list_cell);
                                break;
                            }
                            namespace_list_cell = EBM_CDR(namespace_list_cell);
                        }
                        import_targets = EBM_CDR(import_targets);


                        if (flag){
                            uintptr_t library_env = EBM_CADR(flag);
                            uintptr_t exports = 
                                EBM_vector_ref_CA(
                                        library_env,
                                        5);
                            while (exports != EBM_NULL){
                                uintptr_t def_sym = EBM_CAR(exports);
                                uintptr_t export_sym = EBM_CAR(exports);

                                if (EBM_IS_SYMBOL_CR(def_sym)){
                                    //TODO:global refに変更


                                }else if (EBM_IS_PAIR_CR(def_sym) &&
                                            EBM_IS_SYMBOL_CR(EBM_CAR(def_sym))
                                            && EBM_IS_PAIR_CR(EBM_CDR(def_sym))
                                            && EBM_IS_SYMBOL_CR(EBM_CADR(def_sym))
                                            && EBM_IS_PAIR_CR(EBM_CDDR(def_sym))
                                            && EBM_IS_SYMBOL_CR(EBM_CADR(EBM_CDR(def_sym)))
                                            && EBM_CAR(def_sym) == 
                                                OLISP_symbol_intern(
                                                        EBM_allocate_symbol_from_cstring_CA(
                                                            "rename",
                                                            state->allocator,
                                                            state->allocator_env),
                                                        state)){
                                    export_sym = EBM_CADR(EBM_CDR(def_sym));
                                    def_sym = EBM_CADR(def_sym);
                                }else{
                                    printf("ERROR %s %d\n",__FILE__,__LINE__);
                                    exit(1);
                                }


                                {
                                    uintptr_t ref_object =  
                                        _OLISP_allocate_namespace_ref(
                                                def_sym,
                                                library_env,
                                                state);

                                    EBM_simple_hash_table_set(
                                            expand_environment,
                                            export_sym,
                                            ref_object,
                                            gc_interface);
                                    
                                    uintptr_t eval_global_ref =
                                        _EBM_olisp_tiny_expand_simple_internal(
                                            def_sym,
                                            library_env,
                                            local_cell,
                                            gc_interface,
                                            state);
                                            
                                    _EBM_olisp_tiny_eval_define(
                                            environment,
                                            export_sym,
                                            eval_global_ref,
                                            gc_interface,
                                            state);
                                }






                                exports = EBM_CDR(exports);
                            }
                        }else{
                            printf("ERR \n");
                            write( import_target,
                                    gc_interface->allocator,
                                    gc_interface->env);
                            printf("IS NOT FOUND\n");
                            write( namespace_list,
                                    gc_interface->allocator,
                                    gc_interface->env);
                            exit(1);
                        }
                    }
                    return EBM_UNDEF;
                }
                break;
            case SYNTAX_FX_NUMBER_BEGIN:
                {

                    uintptr_t tails = 
                         _EBM_olisp_aux_recursive_expand_simple(
                                 EBM_CDR(expression),
                                 environment,
                                 local_cell,
                                 gc_interface,
                                 state);
                    uintptr_t res = 
                        EBM_allocate_pair(
                                new_operator,
                                tails,
                                gc_interface->allocator,
                                gc_interface->env);
                }
                break;
            case SYNTAX_FX_NUMBER_IF:
                {
                    if (EBM_CDR(EBM_CDDR(expression)) ==
                            EBM_NULL){
                        return 
                            EBM_allocate_rev_list(
                                    4,
                                    gc_interface->allocator,
                                    gc_interface->env,
                                    EBM_UNDEF,
                                    _EBM_olisp_tiny_expand_simple_internal(
                                             EBM_CADDR(expression),
                                             environment,
                                             local_cell,
                                             gc_interface,
                                             state),
                                    _EBM_olisp_tiny_expand_simple_internal(
                                             EBM_CADR(expression),
                                             environment,
                                             local_cell,
                                             gc_interface,
                                             state),
                                    SYNTAX_FX_NUMBER_IF);
                    }

                    return 
                        EBM_allocate_pair(
                                SYNTAX_FX_NUMBER_IF,
                                _EBM_olisp_aux_recursive_expand_simple(
                                    EBM_CDR(expression),
                                    environment,
                                    local_cell,
                                    gc_interface,
                                    state),
                                gc_interface->allocator,
                                gc_interface->env);
                }
                break;
            case SYNTAX_FX_NUMBER_IR_MACRO_TRANSFORMER:
                {
                    uintptr_t expanded_lambda = 
                        _EBM_olisp_tiny_expand_simple_internal(
                                EBM_CADR(expression),
                                environment,
                                local_cell,
                                gc_interface,
                                state);


                    return _EBM_olisp_allocate_ir_macro_object(expanded_lambda, environment,local_cell,state);
                }
            case SYNTAX_FX_NUMBER_DEFINE_SYNTAX:
                {
                    uintptr_t syntax = 
                        _EBM_olisp_tiny_expand_simple_internal(
                                EBM_CADDR(expression),
                                environment,
                                local_cell,
                                gc_interface,
                                state);

                    EBM_simple_hash_table_set(
                            expand_environment,
                            EBM_CADR(expression),
                            syntax,
                            gc_interface);

                    return EBM_UNDEF;
                            
                }
            default:
                {
                    uintptr_t tails = 
                             _EBM_olisp_aux_recursive_expand_simple(
                                     expression,
                                     environment,
                                     local_cell,
                                     gc_interface,
                                     state);

                     uintptr_t arg_size = 0;
                     uintptr_t cell = tails;
                     {
                         while (cell != EBM_NULL){
                            arg_size++;
                             cell = EBM_CDR(cell);
                         }
                     }
                     return
                         EBM_allocate_pair(
                                 SYNTAX_FX_NUMBER_FRUN,
                                 EBM_allocate_pair(
                                     EBM_allocate_FX_NUMBER_CA(arg_size-1),
                                     tails,
                                     gc_interface->allocator,
                                     gc_interface->env),
                                 gc_interface->allocator,
                                 gc_interface->env);
                }
                break;
        }
    }else if ( EBM_IS_SYMBOL_CR(expression)){
        //local
        uintptr_t cell = local_cell;
        char hit = 0;
        while (cell != EBM_NULL){
            if (EBM_CAR(cell) == expression){
                hit = 1;
                break;
            }
            cell = EBM_CDR(cell);
        }


        if (!hit){
            //global reference
            uintptr_t address_hash = EBM_vector_ref_CA(environment,7);

            uintptr_t global_ref = 
                EBM_simple_hash_table_ref(
                        address_hash,
                        expression);
            
            if (global_ref == EBM_UNDEF){
                uintptr_t address_box =
                    EBM_vector_ref_CA(environment,6);
                global_ref = _OLISP_allocate_global_ref(EBM_CAR(address_box),expression,state);
                EBM_PRIMITIVE_SET_CAR(address_box,EBM_FX_ADD(EBM_CAR(address_box),EBM_allocate_FX_NUMBER_CA(1)));

                EBM_simple_hash_table_set(
                        address_hash,
                        expression,
                        global_ref,
                        gc_interface);
            }
            return global_ref;
        }
        
        //look up
        return expression;//TODO
    }else if (expression == EBM_TRUE){
        return EBM_TRUE;
    }else if (expression == EBM_FALSE){
        return EBM_FALSE;
    }else if (EBM_IS_FX_NUMBER_CR(expression)){
        return expression;
    }else{
        printf("WHO ARE YOU??? [%ld]\n",(long int)expression);
        exit(1);
    }
}

uintptr_t EBM_olisp_tiny_expand_simple(uintptr_t expression,uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state){
    return _EBM_olisp_tiny_expand_simple_internal(expression,environment,EBM_NULL ,gc_interface,olisp_state);
}



static uintptr_t EBM_olisp_tiny_set_import_syntax_environment(uintptr_t expand_env,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state){


    uintptr_t import_symbol = 
        OLISP_symbol_intern(
            EBM_allocate_symbol_from_cstring_CA("import",gc_interface->allocator,gc_interface->env),
            olisp_state);

    EBM_simple_hash_table_set(
            expand_env,
            import_symbol,
            SYNTAX_FX_NUMBER_IMPORT,
            gc_interface);

    return EBM_UNDEF;
}


static uintptr_t _OLISP_allocate_namespace_ref(uintptr_t symbol,uintptr_t environment,OLISP_state *state){
    uintptr_t res = EBM_allocate_record_CA(3,state->allocator,state->allocator_env);

    EBM_record_primitive_set_CA(res,
                                0,
                                OLISP_symbol_intern(
                                   EBM_allocate_symbol_from_cstring_CA(
                                       "<namespace-ref>",
                                       state->allocator,
                                       state->allocator_env),
                                    state));
    EBM_record_primitive_set_CA(res,1,symbol);
    EBM_record_primitive_set_CA(res,2,environment);

    return res;
}

static uintptr_t _OLISP_allocate_global_ref(uintptr_t addr_fx,uintptr_t original_symbol,OLISP_state *state){
    uintptr_t res = EBM_allocate_record_CA(3,state->allocator,state->allocator_env);

    EBM_record_primitive_set_CA(res,
                                0,
                                OLISP_symbol_intern(
                                   EBM_allocate_symbol_from_cstring_CA(
                                       "<global-ref>",
                                       state->allocator,
                                       state->allocator_env),
                                    state));
    EBM_record_primitive_set_CA(res,1,addr_fx);
    EBM_record_primitive_set_CA(res,2,original_symbol);
    return res;
}

uintptr_t EBM_allocate_olisp_tiny_eval_simple_environment(EBM_GC_INTERFACE *gc_interface,uintptr_t offset_box,uintptr_t global_box,OLISP_state *state){

    EBM_ALLOCATOR allocator = gc_interface->allocator;
    uintptr_t allocator_env = gc_interface->env;

    uintptr_t res = EBM_allocate_vector_CA(9,allocator,allocator_env);
    EBM_vector_primitive_set_CA(res,0,EBM_NULL);
    //set eval environment
    // index 1 消えた

    //set expand environment
    {
        uintptr_t hash_table = 
            EBM_allocate_simple_hash_table(
                    128,
                    gc_interface->allocator,
                    gc_interface->env);

        EBM_olisp_tiny_set_import_syntax_environment( hash_table, gc_interface,state);

        EBM_vector_set_CA(res,2,hash_table,gc_interface->write_barrier,gc_interface->env);
    }

    {//set local env for eval
        EBM_vector_set_CA(res,3,EBM_NULL,gc_interface->write_barrier,gc_interface->env);
    
    }

    {//name spaces
        EBM_vector_primitive_set_CA(res,4,EBM_NULL);
    }

    {//exports
        EBM_vector_primitive_set_CA(res,5,EBM_NULL);
    }

    {//offset
        EBM_vector_primitive_set_CA(res,6,offset_box);
    }

    {//global name to address

        EBM_vector_primitive_set_CA(
                res,
                7,
                EBM_allocate_simple_hash_table(
                    128,
                    gc_interface->allocator,
                    gc_interface->env)
                );
    }

    {//globals
        EBM_vector_primitive_set_CA(res,8,global_box);
    }
    
    return res;
}



static uintptr_t _EBM_olisp_tiny_set_fun_to_environment(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state,char fnames[][OLISP_TINY_SIMPLE_LENGTH_OF_FUCTION_NAME],OLISP_cfun* olisp_cfuns,int number_of_function){
    int i;
    uintptr_t exports = EBM_vector_ref_CA(environment,5);
    for (i=0;i<number_of_function;i++){
        uintptr_t function_name_symbol = 
                    EBM_allocate_symbol_from_cstring_CA(fnames[i],gc_interface->allocator,gc_interface->env);
        
        function_name_symbol = 
            OLISP_symbol_intern(
                    function_name_symbol,
                    state);

        exports = 
            EBM_allocate_pair(
                    function_name_symbol,
                    exports,
                    gc_interface->allocator,
                    gc_interface->env);

        uintptr_t function = 
             OLISP_create_function_for_ebm_with_name(
                olisp_cfuns[i],
                function_name_symbol,
                gc_interface->allocator,
                gc_interface->env);

        _EBM_olisp_tiny_eval_define(environment, function_name_symbol, function ,gc_interface,state);
    }

    EBM_vector_set_CA(
            environment,
            5,
            exports,
            gc_interface->write_barrier,
            gc_interface->env);
    return EBM_UNDEF;

}

static uintptr_t _EBM_olisp_tiny_set_library0_fun(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    char fnames[][OLISP_TINY_SIMPLE_LENGTH_OF_FUCTION_NAME ] 
            = {"cons","car","cdr","eq?","write-simple","vector","pair?","symbol?"};

    OLISP_cfun olisp_cfuns[] = {OLISP_cons,OLISP_car,OLISP_cdr,OLISP_eq,OLISP_write_simple,OLISP_vector,OLISP_pair_p,OLISP_symbol_p};

    _EBM_olisp_tiny_set_fun_to_environment(environment,gc_interface,state,&(fnames[0]),&olisp_cfuns[0],8);
    return EBM_UNDEF;
}

static uintptr_t _EBM_olisp_tiny_set_fxnumber_fun(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    char fnames[][OLISP_TINY_SIMPLE_LENGTH_OF_FUCTION_NAME ] 
            = {"fx+"};

    OLISP_cfun olisp_cfuns[] = {OLISP_fx_add};

    _EBM_olisp_tiny_set_fun_to_environment(environment,gc_interface,state,&(fnames[0]),&olisp_cfuns[0],1);
    return EBM_UNDEF;
}

static uintptr_t _EBM_olisp_tiny_set_base_aux_fun(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    char fnames[][OLISP_TINY_SIMPLE_LENGTH_OF_FUCTION_NAME ] 
            = {"record-ref","record?","make-record"};

    OLISP_cfun olisp_cfuns[] = {OLISP_record_ref,OLISP_record_p,OLISP_make_record};

    _EBM_olisp_tiny_set_fun_to_environment(environment,gc_interface,state,&(fnames[0]),&olisp_cfuns[0],3);
    return EBM_UNDEF;
}

static uintptr_t  _EBM_olisp_tiny_set_library0_syntax(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    uintptr_t expand_env = 
        EBM_vector_ref_CA(
                environment,
                2);

    char syntax_names[][21] = {"define","lambda","if","set!","quote","define-record-type","begin","define-library","import","export","ir-macro-transformer","define-syntax"};
    int i;
    uintptr_t exports = EBM_vector_ref_CA(environment,5);
    for (i=0;i<12;i++){
        uintptr_t sym = 
            OLISP_symbol_intern(
                EBM_allocate_symbol_from_cstring_CA(syntax_names[i],gc_interface->allocator,gc_interface->env),
                state);

        exports = 
            EBM_allocate_pair(
                    sym,
                    exports,
                    gc_interface->allocator,
                    gc_interface->env);
        EBM_simple_hash_table_set(
                expand_env,
                sym,
                EBM_allocate_FX_NUMBER_CA(i),
                gc_interface);
    }

    EBM_vector_set_CA(
            environment,
            5,
            exports,
            gc_interface->write_barrier,
            gc_interface->env);
    return EBM_NULL;
}

uintptr_t EBM_olisp_tiny_set_library0(uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){

    _EBM_olisp_tiny_set_library0_fun(
            environment,
            gc_interface,
            state);
    _EBM_olisp_tiny_set_library0_syntax(
            environment,
            gc_interface,
            state);

    _EBM_olisp_tiny_set_fxnumber_fun(
            environment,
            gc_interface,
            state);

     _EBM_olisp_tiny_set_base_aux_fun(
            environment,
            gc_interface,
            state);

    return EBM_UNDEF;
}
