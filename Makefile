CFLAGS += -I./include -g

OLISP_TINY_SRC = src/olisp_tiny/olisp_tiny_main.c src/ebm.c src/ebm_frontend.c src/olisp_cinterface.c src/ebm_character_table.c src/ebm_symbol.c src/olisp_tiny/olisp_tiny_gc.c src/olisp_tiny/olisp_tiny_eval_simple.c src/ebm_symbol_trie.c src/olisp_tiny/olisp_tiny_functions_zero.c src/ebm_aux.c src/ebm_simple_hashtable.c src/ebm_string.c src/ebm_bytevector.c 

default:all

localdir:
	mkdir -p bin

bin/olisp_tiny:$(OLISP_TINY_SRC)
	$(CC) $(OLISP_TINY_SRC) -o bin/olisp_tiny $(CFLAGS)

all:localdir bin/olisp_tiny

.PHONY:clean
clean:
	rm bin/olisp_tiny
