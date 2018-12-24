#ifndef EBM_FRONTEND_H
#define EBM_FRONTEND_H
#include<stdint.h>
#include<stdio.h>
#include "ebm.h"
#include "olisp_cinterface.h"


#define EBM_FRONTEND_INPUT_PORT 0

#define EBM_FRONTEND_FILE_PORT 0
#define EBM_FRONTEND_STRING_PORT (1<<3)

uintptr_t EBM_frontend_allocate_input_file_port(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_frontend_allocate_input_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env);

uintptr_t OLISP_read(OLISP_state *state);

#define EBM_IS_PORT_CR(object) (EBM_IS_RECORD_CR(object)&&(EBM_record_first(object)==EBM_BUILT_IN_RECORD_TYPE_PORT))
#define EBM_IS_INPUT_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_second(object) == EBM_FRONTEND_INPUT_PORT))
#define EBM_IS_FILE_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_third(object) == EBM_FRONTEND_FILE_PORT))
#define EBM_IS_STRING_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_third(object) == EBM_FRONTEND_STRING_PORT))
#endif
