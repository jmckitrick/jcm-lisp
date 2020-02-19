/* -*- c-basic-offset: 2 ; -*- */
/*
 * JCM-LISP
 *
 * Based on http://web.sonoma.edu/users/l/luvisi/sl3.c
 * and http://peter.michaux.ca/archives
 *
 */

#define _GNU_SOURCE

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <sys/errno.h>

//#define CODE_TEST
#define FILE_TEST
//#define REPL

typedef enum {
  UNKNOWN = 0,
  NIL = 1,
  FIXNUM = 2,
  STRING = 3,
  SYMBOL = 4,
  CELL = 5,
  PRIMITIVE = 6,
  PROC = 7
} obj_type;

typedef struct Object Object;
typedef struct Object *primitive_fn(struct Object *);

struct Fixnum {
  int value;
};

struct String {
  char *text;
};

struct Symbol {
  char *name;
};

struct Cell {
  struct Object *car;
  struct Object *cdr;
};

struct Primitive {
  primitive_fn *fn;
};

struct Proc {
  struct Object *vars;
  struct Object *body;
  struct Object *env;
};

struct Object {
  union {
    struct Cell cell;
    struct Symbol symbol;
    struct Fixnum num;
    struct String str;
    struct Proc proc;
    struct Primitive primitive;
  };

  obj_type type;
  int mark;
  int id;
};

void print(Object *);

Object *symbols;    /* linked list */
Object *s_quote;
Object *s_define;
Object *s_setq;
Object *s_nil;
Object *s_if;
Object *s_t;
Object *s_lambda;

Object *env;

#define caar(obj)    car(car(obj))
#define cadr(obj)    car(cdr(obj))

Object *read_list(FILE *);
