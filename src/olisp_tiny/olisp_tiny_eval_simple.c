#include "ebm.h"
#include "olisp_tiny_eval_simple.h"
#include "olisp_tiny_functions.h"
#include "olisp_cinterface.h"
#include<stdio.h>

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

#define SYNTAX_FX_NUMBER_FRUN EBM_allocate_FX_NUMBER_CA(100)

#define OLISP_TINY_CLOSUR_FX_NUMBER EBM_allocate_FX_NUMBER_CA(2)


#define _OLISP_CAN_EVAL(x) (EBM_IS_PAIR_CR(x) || EBM_IS_SYMBOL_CR(x))

//DEBUG
static uintptr_t write(uintptr_t obj,EBM_ALLOCATOR allocator,uintptr_t allocator_env){
    uintptr_t output_port =  EBM_frontend_allocate_output_file_port_CA(stdout,allocator,allocator_env);
    
    EBM_write_simple(obj,output_port);
    EBM_newline(output_port);
}



uintptr_t _OLISP_allocate_namespace_ref(uintptr_t symbol,uintptr_t environment,OLISP_state *state);

static uintptr_t EBM_olisp_tiny_lookup(uintptr_t trie,uintptr_t symbol){
    return EBM_symbol_trie_ref(trie,symbol);
}

static uintptr_t EBM_olisp_tiny_set_env(uintptr_t trie,uintptr_t symbol,uintptr_t object,EBM_GC_INTERFACE *gc_interface){
    return EBM_symbol_trie_set(trie,symbol,object,gc_interface);
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
    {
        namespace_ref_symbol =
            OLISP_symbol_intern(
               EBM_allocate_symbol_from_cstring_CA(
                   "<namespace-ref>",
                   olisp_state->allocator,
                   olisp_state->allocator_env),
                olisp_state);
    }


    uintptr_t stack = EBM_NULL;//((code,next-evals,evaled,lex) ... )
    uintptr_t res = EBM_UNDEF;

    uintptr_t code = expanded_expression;
    uintptr_t eval_info = EBM_NULL;// (次の評価 . 評価済み)
    uintptr_t lexical_information = EBM_vector_ref_CA(environment,3);

    while (stack != EBM_NULL ||  code != EBM_NULL){
        if (EBM_IS_PAIR_CR(code)){
            uintptr_t operator = EBM_CAR(code);
            switch(operator){
                case SYNTAX_FX_NUMBER_DEFINE:
                    {
                        if (_OLISP_CAN_EVAL(EBM_CADDR(code))){
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
                               EBM_olisp_tiny_set_env(eval_environment,EBM_CADR(code),EBM_CAR(EBM_CDR(eval_info)),gc_interface);
                               res = EBM_UNDEF;
                           }
                        }else{
                            EBM_olisp_tiny_set_env(eval_environment,EBM_CADR(code),EBM_CADDR(code),gc_interface);
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
                           if (_OLISP_CAN_EVAL(EBM_CAAR(eval_info))){
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
                                                    EBM_CAR(formals),
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
                        }else if _OLISP_CAN_EVAL(EBM_CAR(eval_info)){
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
                            if (_OLISP_CAN_EVAL(EBM_CADR(code))){
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
                                code = EBM_CADDR(code);
                                eval_info = EBM_NULL;
                                continue;
                            }else{
                                code = EBM_CADDR(EBM_CDR(code));
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
            if (EBM_IS_RECORD_CR(_res) 
                    && (EBM_record_first(_res) == namespace_ref_symbol)){
                uintptr_t library_env = 
                    EBM_record_third(_res);
                uintptr_t target_symbol = 
                    EBM_record_second(_res);
                _res =  EBM_olisp_tiny_lookup(EBM_vector_ref_CA(library_env,1),target_symbol);
                
            }
                    
            res = _res;
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

static uintptr_t _EBM_olisp_aux_recursive_expand_simple(uintptr_t expression,uintptr_t environment,uintptr_t local_cell,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state){

    if (expression == EBM_NULL){
        return EBM_NULL;
    }   

    return EBM_allocate_pair( 
            EBM_olisp_tiny_expand_simple(EBM_CAR(expression),environment,gc_interface,olisp_state),
            _EBM_olisp_aux_recursive_expand_simple(EBM_CDR(expression),environment,local_cell,gc_interface,olisp_state),
            gc_interface->allocator,
            gc_interface->env);
}

static uintptr_t _EBM_olisp_tiny_expand_simple_internal(uintptr_t expression,uintptr_t environment,uintptr_t local_cell,EBM_GC_INTERFACE *gc_interface,OLISP_state *state){
    //Don't set! value to expression.

    uintptr_t namespace_ref_symbol;
    {
        namespace_ref_symbol =
            OLISP_symbol_intern(
               EBM_allocate_symbol_from_cstring_CA(
                   "<namespace-ref>",
                   state->allocator,
                   state->allocator_env),
                state);
    }

    //TODO:check syntax
    uintptr_t expand_environment = EBM_vector_ref_CA(environment,2);
    if (EBM_IS_PAIR_CR(expression)){
        uintptr_t operator = EBM_CAR(expression);
        uintptr_t new_operator = EBM_UNDEF;

        if (EBM_IS_FX_NUMBER_CR(operator)){
            //error
        }else if (EBM_IS_SYMBOL_CR(operator)){
            new_operator = EBM_olisp_tiny_lookup(expand_environment,operator);
        }
        
        switch (new_operator){
            case SYNTAX_FX_NUMBER_QUOTE:
                {
                    return EBM_allocate_pair(new_operator,EBM_CDR(expression),gc_interface->allocator,gc_interface->env);
                }
            case SYNTAX_FX_NUMBER_DEFINE:
                {
                    uintptr_t exp = _EBM_olisp_tiny_expand_simple_internal(EBM_CADDR(expression),environment,local_cell, gc_interface,state);
                    return EBM_allocate_rev_list(3,
                            gc_interface->allocator,
                            gc_interface->env,
                            exp,
                            EBM_CADR(expression),
                            SYNTAX_FX_NUMBER_DEFINE);
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
                    //外側の環境のimportをマージする
                    uintptr_t new_environment = EBM_allocate_olisp_tiny_eval_simple_environment(gc_interface);
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

                            uintptr_t eval_environment = EBM_vector_ref_CA(environment,1);
                            while (exports != EBM_NULL){
                                uintptr_t sym = EBM_CAR(exports);
                                if (EBM_IS_SYMBOL_CR(sym)){
                                    uintptr_t ref_object =  
                                        _OLISP_allocate_namespace_ref(
                                                sym,
                                                library_env,
                                                state);

                                    EBM_symbol_trie_set(
                                            expand_environment,
                                            sym,
                                            ref_object,
                                            gc_interface);

                                    EBM_symbol_trie_set(
                                            eval_environment,
                                            sym,
                                            ref_object,
                                            gc_interface);
                                }else{
                                    //sorry
                                    //TODO:export rename考慮
                                    printf("SORRY\n");
                                    exit(1);
                                }
                                exports = EBM_CDR(exports);
                            }
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
        //look up
        return expression;//TODO
    }else if (expression == EBM_TRUE){
        return EBM_TRUE;
    }else if (expression == EBM_FALSE){
        return EBM_FALSE;
    }else{
        printf("WHO ARE YOU??? [%ld]\n",(long int)expression);
    }
}


uintptr_t EBM_olisp_tiny_expand_simple(uintptr_t expression,uintptr_t environment,EBM_GC_INTERFACE *gc_interface,OLISP_state *olisp_state){
    return _EBM_olisp_tiny_expand_simple_internal(expression,environment,EBM_NULL ,gc_interface,olisp_state);
}

static uintptr_t EBM_olisp_tiny_set_default_expand_environtment(uintptr_t trie_expand_env,EBM_GC_INTERFACE *gc_interface){
    char syntax_names[][18] = {"define","lambda","if","set!","quote","define-record-type","begin","define-library","import","export"};
    int i;
    for (i=0;i<10;i++){
        EBM_symbol_trie_set(trie_expand_env, EBM_allocate_symbol_from_cstring_CA(syntax_names[i],gc_interface->allocator,gc_interface->env),EBM_allocate_FX_NUMBER_CA(i),gc_interface);
    }
    return EBM_NULL;
}

static uintptr_t EBM_olisp_tiny_set_default_eval_environment(uintptr_t env,EBM_GC_INTERFACE *gc_interface){

    char fnames[][13] = {"cons","car","cdr","eq?","write-simple","vector"};
    OLISP_cfun olisp_cfuns[] = {OLISP_cons,OLISP_car,OLISP_cdr,OLISP_eq,OLISP_write_simple,OLISP_vector};

    int i;
    for (i=0;i<6;i++){
        uintptr_t function_name_symbol = 
                    EBM_allocate_symbol_from_cstring_CA(fnames[i],gc_interface->allocator,gc_interface->env);

        EBM_olisp_tiny_set_env(
             env,
             function_name_symbol,
             OLISP_create_function_for_ebm_with_name(
                olisp_cfuns[i],
                function_name_symbol,
                gc_interface->allocator,
                gc_interface->env),
            gc_interface);
    }
    return EBM_UNDEF;
}

uintptr_t _OLISP_allocate_namespace_ref(uintptr_t symbol,uintptr_t environment,OLISP_state *state){
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

uintptr_t EBM_allocate_olisp_tiny_eval_simple_environment(EBM_GC_INTERFACE *gc_interface){

    EBM_ALLOCATOR allocator = gc_interface->allocator;
    uintptr_t allocator_env = gc_interface->env;

    uintptr_t res = EBM_allocate_vector_CA(6,allocator,allocator_env);
    EBM_vector_primitive_set_CA(res,0,EBM_NULL);
     
    //set eval environment
    {
        uintptr_t symbol_trie = EBM_allocate_symbol_trie(allocator,allocator_env); 
        EBM_olisp_tiny_set_default_eval_environment(symbol_trie,gc_interface);

        EBM_vector_set_CA(res,1,symbol_trie,gc_interface->write_barrier,gc_interface->env);
    }   

    //set expand environment
    {
        uintptr_t symbol_trie = EBM_allocate_symbol_trie(allocator,allocator_env);
        EBM_olisp_tiny_set_default_expand_environtment(symbol_trie,gc_interface);
        EBM_vector_set_CA(res,2,symbol_trie,gc_interface->write_barrier,gc_interface->env);
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
    return res;
}
