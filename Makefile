CFLAGS += -I./include

default:all

localdir:
	mkdir -p bin

bin/olisp_tiny:src/olisp_tiny/main.c src/ebm.c src/ebm_frontend.c src/olisp_cinterface.c src/ebm_character_table.c src/ebm_symbol.c
	$(CC) src/olisp_tiny/main.c src/ebm_frontend.c src/ebm.c src/olisp_cinterface.c src/ebm_character_table.c src/ebm_symbol.c -o bin/olisp_tiny $(CFLAGS)

all:localdir bin/olisp_tiny

.PHONY:clean
clean:
	rm bin/olisp_tiny
