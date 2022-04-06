CC     = cc
CFLAGS = -Wall -g -Og
DEPS   = jcm-lisp.h gc.h
OBJ    = jcm-lisp.o gc.o

# $@ - filename of the target
# $< - filename of the first prerequisite
# $^ - filenames of all the prerequisites

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

jcm-lisp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:	clean
clean:
	rm -f jcm-lisp
	rm -f *.o
	rm -rf *.dSYM
