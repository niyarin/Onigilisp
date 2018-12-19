#ifndef EBM_FRONTEND_H
#define EBM_FRONTEND_H
#include<stdint.h>
#include<stdio.h>
#include "ebm.h"
uintptr_t EBM_frontend_allocate_input_file_port(FILE *fp,EBM_ALLOCATOR allocator,uintptr_t env);
uintptr_t EBM_frontend_allocate_input_string_port(uintptr_t ebm_string,EBM_ALLOCATOR allocator,uintptr_t env);

#endif
