/* -*- c-basic-offset: 2 ; -*- */
/*
 * JCM-LISP
 *
 * Based on http://web.sonoma.edu/users/l/luvisi/sl3.c
 * and http://peter.michaux.ca/archives
 *
 */

//#define _GNU_SOURCE

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <sys/errno.h>

#define GC_ENABLED
#define GC_MARK
#define GC_SWEEP
#define GC_PIN

#define GC_DEBUG
//#define GC_DEBUG_X
//#define GC_DEBUG_XX
//#define GC_PIN_DEBUG
//#define GC_PIN_DEBUG_X

void *free_list[MAX_ALLOC_SIZE];
void *active_list[MAX_ALLOC_SIZE];

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

#ifdef GC_ENABLED
Object *alloc_Object();
void gc();
void error(char *msg);
#endif // GC_ENABLED
