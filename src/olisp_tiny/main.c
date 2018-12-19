#include<stdio.h>
#include<stdlib.h>

#include "ebm.h"
#include "ebm_frontend.h"

void main_load_1script(char *filename){
    FILE *fp;
    if ((fp = fopen(filename,"r")) == NULL){
        //TODO:OUTPUT ERROR MESSAGE
        exit(1);
    }
}

int main(int argc,char **argv){
    if (argc == 1){
    
    }else if (argc == 2){
        char *filename = argv[1];
        main_load_1script(filename);
    }
    return 0;
}
