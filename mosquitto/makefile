C = gcc
OBJ= mosquitto_sub.o
OUTPUT = mosquitto_sub

ALL: ${OBJ}
	${CC} ${CFLAGS}${OBJ}  -o ${OUTPUT} -lmosquitto ${LDFLAGS}
.PHONY: clean 

clean:
	-rm -rf *.o ${OUTPUT}

