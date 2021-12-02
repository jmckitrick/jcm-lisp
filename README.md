
Use `lldb` for simple debugging, `gui` mode for curses interface.

BUGS/TODO:

- reader adds 'nil' cell for blank/commented lines
- top_env/env not handled correctly

To build debug:

cc -S -Og jcm-lisp.c

Better:
compile/assemble only, with debug info, optimize for size:
cc -c -g -Os jcm-lisp.c
-c only compile
-g add debug info
-Os optimize for size

disassemble with source inlined:
objdump -d -S
-d disassemble
-S with source
