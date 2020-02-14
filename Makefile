CC     = cc
CFLAGS = -Wall -g -O0
DEPS   = jcm-lisp.h gc.h
OBJ    = jcm-lisp.o gc.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

jcm-lisp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:	clean
clean:
	rm -f *.o
	rm -rf *.dSYM
