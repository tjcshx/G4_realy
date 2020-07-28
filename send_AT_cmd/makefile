C = gcc
OBJ= send_msg.o comport.o log.o 
OUTPUT = comport

ALL: ${OBJ}
	${CC} ${CFLAGS}${OBJ}  -o ${OUTPUT} -lpthread ${LDFLAGS}
.PHONY: clean 

clean:
	-rm -rf *.o ${OUTPUT}
disclean:
	-rm -rf *.o ${OUTPUT}   comport_log* cs* tags

