C = gcc
OBJ= relay.o
OUTPUT = run_relay

ALL: ${OBJ}
	${CC} ${CFLAGS}${OBJ}  -o ${OUTPUT} -lwiringPi ${LDFLAGS}
.PHONY: clean 

clean:
	-rm -rf *.o ${OUTPUT}

