.PHONY:	clean

all:	jcm-lisp
	cc -Wall -o jcm-lisp jcm-lisp.c

clean:
	rm jcm-lisp
