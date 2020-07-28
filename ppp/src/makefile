OUT = PPP
CFLAGS=-Wall -Werror
CC = gcc

ALL:
	${CC} ${CFLAGS} ppp.c proc.c -o ${OUT} -lpthread

clean:
	rm -rf ${OUT} a.out ppp_log
