CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ecc:$(OBJS)
	$(CC) -o ecc $(OBJS) $(LDFLAGS)

$(OBJS): ecc.h

test: ecc
		./test.sh

clean:
	rm -f ecc *.o tmp*
	rm -f ../test/*.o

.PHONY: test clean