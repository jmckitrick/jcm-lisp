
# jcm-lisp:
# 	cc -Wall -g -O0 -o jcm-lisp jcm-lisp.c

# #.PHONY:	clean
# clean:
# 	rm -rf jcm-lisp
# 	rm -rf jcm-lisp.dSYM

CC = cc
CFLAGS = -Wall -g -O0
DEPS = jcm-lisp.h
OBJ = jcm-lisp.o #gc.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

jcm-lisp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY:	clean
clean:
	rm -f *.o
	rm -f *.dSYM
