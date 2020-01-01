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

#define MAX_BUFFER_SIZE 100

#define MAX_ALLOC_SIZE  1024

#define GC_ENABLED
#define GC_MARK
#define GC_SWEEP
#define GC_PIN

#define GC_DEBUG
//#define GC_DEBUG_X
//#define GC_DEBUG_XX
//#define GC_PIN_DEBUG
//#define GC_PIN_DEBUG_X

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

Object *free_list[MAX_ALLOC_SIZE];
Object *active_list[MAX_ALLOC_SIZE];
Object *env;

int current_mark;

#ifdef GC_PIN
struct PinnedVariable {
  Object **variable;
  struct PinnedVariable *next;
  int inUse;
};

struct PinnedVariable *pinned_variables;

#endif //GC_PIN

void pin_variable(Object **obj);
void unpin_variable(Object **obj);

#define caar(obj)    car(car(obj))
#define cadr(obj)    car(cdr(obj))

Object *read_list(FILE *);

#ifdef GC_ENABLED
Object *alloc_Object();
void gc();
void error(char *msg);
#endif // GC_ENABLED
