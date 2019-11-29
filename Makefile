.PHONY:	clean

all:	jcm-lisp
	cc -Wall -g -O0 -o jcm-lisp jcm-lisp.c

clean:
	rm -rf jcm-lisp
	rm -rf jcm-lisp.dSYM
