CFLAGS += -I./include -g

OLISP_TINY_SRC = src/olisp_tiny/main.c src/ebm.c src/ebm_frontend.c src/olisp_cinterface.c src/ebm_character_table.c src/ebm_symbol.c src/olisp_tiny/olisp_tiny_gc.c

default:all

localdir:
	mkdir -p bin

bin/olisp_tiny:$(OLISP_TINY_SRC)
	$(CC) $(OLISP_TINY_SRC) -o bin/olisp_tiny $(CFLAGS)

all:localdir bin/olisp_tiny

.PHONY:clean
clean:
	rm bin/olisp_tiny
