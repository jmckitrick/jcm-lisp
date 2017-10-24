.PHONY:	clean

all:	jcm-lisp
	cc -Wall -g -o jcm-lisp jcm-lisp.c

clean:
	rm jcm-lisp
