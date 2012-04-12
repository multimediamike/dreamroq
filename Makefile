all: test-dreamroq

CFLAGS += -Wall

test-dreamroq: test-dreamroq.o dreamroqlib.o

clean:
	rm -f *.o test-dreamroq
