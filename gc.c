/* -*- c-basic-offset: 2 ; -*- */
/*
 * JCM-LISP
 *
 * Based on http://web.sonoma.edu/users/l/luvisi/sl3.c
 * and http://peter.michaux.ca/archives
 *
 */

#include "jcm-lisp.h"

void gc() {

  printf("\nGC v----------------------------------------v\n");
  check_mem();

#ifdef GC_MARK
  printf("\n-------- Mark symbols:");
  mark(symbols);

  printf("\n-------- Mark env:");
  mark(env);

#ifdef GC_PIN
  printf("\n-------- Mark pins:\n");
  struct PinnedVariable **v = NULL;
  v = &pinned_variables;

  for (v = &pinned_variables; *v != NULL; v = &(*v)->next) {
    if ((*v)->inUse != 1) {
      printf("\nIn use? %d\n", (*v)->inUse);
      continue;
    }

    printf("Pointer to pointer to variable: %p\n", v);
    printf("Pointer to variable: %p\n", *v);
    printf("Pointer to pointer to object: %p\n", (*v)->variable);
    printf("Pointer to object: %p\n", *(*v)->variable);

    Object *tempObj = (Object *)(*(*v)->variable);
    if (tempObj != NULL) {
      print(tempObj);
      mark(tempObj);
      print(*(*v)->variable);
      printf("\nMarked pinned var\n");
    }
  }
#endif // GC_PIN
#endif // GC_MARK

#ifdef GC_SWEEP
  printf("\n-------- Sweep\n");
  sweep();
  check_mem();
#endif // GC_SWEEP

  printf("\nGC ^----------------------------------------^\n");
}
