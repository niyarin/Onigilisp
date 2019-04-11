#ifndef EBM_FRONTEND_H
#define EBM_FRONTEND_H
#include<stdint.h>
#include "ebm.h"
#include "olisp_cinterface.h"

#ifdef EBM_USE_IO
#include<stdio.h>
#endif

#define EBM_FRONTEND_INPUT_PORT  EBM_allocate_FX_NUMBER_CA(0)
#define EBM_FRONTEND_OUTPUT_PORT  EBM_allocate_FX_NUMBER_CA(1)

#define EBM_FRONTEND_FILE_PORT  EBM_allocate_FX_NUMBER_CA(0)
#define EBM_FRONTEND_STRING_PORT  EBM_allocate_FX_NUMBER_CA(1)


#ifdef EBM_USE_IO
uintptr_t EBM_frontend_allocate_input_file_port(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_frontend_allocate_output_file_port_CA(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env);
#endif

uintptr_t EBM_frontend_allocate_input_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_frontend_allocate_output_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env);

uintptr_t OLISP_read(OLISP_state *state);

uintptr_t EBM_write_char_CA(uint32_t c,uintptr_t port);
uintptr_t EBM_write_simple(uintptr_t object,uintptr_t port);
uintptr_t EBM_write_cstring_CA(char *cstring,uintptr_t port);
uintptr_t EBM_newline(uintptr_t port);

#define EBM_IS_PORT_CR(object) (EBM_IS_RECORD_CR(object)&&(EBM_record_first(object)==EBM_BUILT_IN_RECORD_TYPE_PORT))
#define EBM_IS_INPUT_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_second(object) == EBM_FRONTEND_INPUT_PORT))
#define EBM_IS_OUTPUT_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_second(object) == EBM_FRONTEND_OUTPUT_PORT))
#define EBM_IS_FILE_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_third(object) == EBM_FRONTEND_FILE_PORT))
#define EBM_IS_STRING_PORT_CR(object) (EBM_IS_PORT_CR(object) &&(EBM_record_third(object) == EBM_FRONTEND_STRING_PORT))
#endif
