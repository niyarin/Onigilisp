CFLAGS += -I./include

default:all

localdir:
	mkdir -p bin

bin/olisp_tiny:src/olisp_tiny/main.c
	$(CC) src/olisp_tiny/main.c -o bin/olisp_tiny $(CFLAGS)

all:localdir bin/olisp_tiny

.PHONY:clean
clean:
	rm bin/olisp_tiny
