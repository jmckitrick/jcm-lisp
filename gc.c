/* -*- c-basic-offset: 2 ; -*- */
/*
 * JCM-LISP
 *
 * Based on http://web.sonoma.edu/users/l/luvisi/sl3.c
 * and http://peter.michaux.ca/archives
 *
 */

#include "jcm-lisp.h"
#include "gc.h"

#ifdef GC_PIN
int pv_count = 0;

void print_pins() {
  printf("\nPinned variables:\n");
  struct PinnedVariable *pv;
  for (pv = pv_head; pv != NULL; pv = pv->next) {
    printf("Pinned variable: %p %p\n", pv, pv->var);
  }
  printf("Done.\n");
}

void pin_variable(void **var) {
#ifdef GC_PIN_DEBUG
  printf("Pin\n");
  printf("> var %p\n", var);
#endif //GC_PIN_DEBUG

  struct PinnedVariable *pv = NULL;
  pv = calloc(1, sizeof(struct PinnedVariable));
  assert(pv != NULL);

  pv->var = var;
  pv->in_use = 1;
  pv->next = pv_head;

#ifdef GC_PIN_DEBUG_X
  printf("Pinned variable         = %p\n", pv);
  //printf("Pinned variable         = ");
  //print((struct Object *)pv->var);
  printf("Pinned variables before = %p\n", pv_head);
#endif //GC_PIN_DEBUG_X

  pv_head = pv;
  pv_count++;

#ifdef GC_PIN_DEBUG_X
  printf("Pinned variables after  = %p\n", pv_head);
  printf("Pinned variable count %d\n", pv_count);
#endif //GC_PIN_DEBUG_X
}
#else
void pin_variable(void **var) { // void
  //printf("PIN\n");
}
#endif // GC_PIN

#ifdef GC_PIN
void unpin_variable(void **var) {
#ifdef GC_PIN_DEBUG
  printf("< var %p\n", var);
#endif //GC_PIN_DEBUG

  struct PinnedVariable *pv = NULL;
  for (pv = pv_head; pv != NULL; pv = pv->next) {

    if (pv->var == var) {

#ifdef GC_PIN_DEBUG
      printf("Containing: ");
      print(pv->var);
      //print(*var);

      if (pv->in_use != 1) {
        printf("\nIn use? %d\n", pv->in_use);
      }
#endif
      struct PinnedVariable *next_pv = pv->next;
      pv->in_use = 0;
      free(pv);

      pv_head = next_pv;
      pv_count--;
#ifdef GC_PIN_DEBUG
      printf("\nUnpin\n");
      printf("Pinned variable count %d\n", pv_count);
#endif
      return;
    }
  }
  assert(0);
}
#else
void unpin_variable(void **var) { // void
  //printf("UNPIN %p\n", var);
  //print(var);
  //printf("\n");
}
#endif // GC_PIN

#ifdef GC_ENABLED
void mark(Object *obj) {
  if (obj == NULL) {
#ifdef GC_DEBUG
    printf("\nNothing to mark: NULL");
#endif // GC_DEBUG
    return;
  }
#ifdef GC_DEBUG_XX
  Object *temp = obj; printf("\nMarking %p\n", temp);
#endif // GC_DEBUG_XX
  if (obj->mark > 0) {
#ifdef GC_DEBUG_XX
    printf("\nNothing to mark: already marked\n");
#endif // GC_DEBUG_XX
    return;
  }

#ifdef GC_DEBUG
  //printf("\nBefore mark: %p %d", obj, obj->mark);
#endif // GC_DEBUG

  obj->mark = current_mark;

#ifdef GC_DEBUG
  //printf("\nAfter mark: %p %d", obj, obj->mark);
#endif // GC_DEBUG

  switch (obj->type) {
    case FIXNUM:
    case STRING:
    case SYMBOL:
    case PRIMITIVE:
#ifdef GC_DEBUG_XX
      printf("\nMark %d %s ", obj->id, get_type(obj));
      print(obj);
#endif // GC_DEBUG_XX
      break;
    case CELL:
#ifdef GC_DEBUG_XX
      printf("\nMark cell car %p -> %p", obj, obj->cell.car);
#endif // GC_DEBUG_XX
      mark(obj->cell.car);
#ifdef GC_DEBUG_XX
      printf("\nMark cell car %p <-", obj);
#endif // GC_DEBUG
#ifdef GC_DEBUG_XX
      printf("\nMark cell cdr %p -> %p", obj, obj->cell.cdr);
#endif // GC_DEBUG_XX
      mark(obj->cell.cdr);
#ifdef GC_DEBUG_XX
      printf("\nMark cell cdr %p <-", obj);
#endif // GC_DEBUG_XX
      break;
    case PROC:
#ifdef GC_DEBUG_XX
      printf("\nMark proc ->");
#endif // GC_DEBUG_XX
#ifdef GC_DEBUG_XX
      printf("\nMark proc vars");
#endif // GC_DEBUG_XX
      mark(obj->proc.vars);
#ifdef GC_DEBUG_XX
      printf("\nMark proc body");
#endif // GC_DEBUG_XX
      mark(obj->proc.body);
#ifdef GC_DEBUG_XX
      printf("\nMark proc env");
#endif // GC_DEBUG_XX
      mark(obj->proc.env);
#ifdef GC_DEBUG
      printf("\nMark proc <-");
#endif // GC_DEBUG_XX
      break;
    default:
      printf("\nMark unknown object: %d\n", obj->type);
      break;
  }
}

int is_active(void *needle) {
  for (int i = 0; i <= MAX_ALLOC_SIZE; i++) {
    if (active_list[i] == needle)
      return 1;
  }

  return 0;
}

void sweep() {
  int counted = 0;
  int kept = 0;
  int swept = 0;
  int cells = 0;

  for (int i = 0; i < MAX_ALLOC_SIZE; i++) {
    Object *obj = active_list[i];

    if (obj == NULL)
      continue;

#ifdef GC_DEBUG
    //printf("\nObj at                 = %p\n", obj);
#endif // GC_DEBUG

    if (obj->mark == 0) {
#ifdef GC_DEBUG
      printf("\nSWEEP: %p id: %d mark: %d ", obj, obj->id, obj->mark);
      print(obj);
#endif // GC_DEBUG

      // Free any additional allocated memory.
      switch (obj->type) {
        case STRING:
          memset(obj->str.text, 0, strlen(obj->str.text));
          free(obj->str.text);
          //printf("\n");
          break;
        case SYMBOL:
          memset(obj->symbol.name, 0, strlen(obj->symbol.name));
          free(obj->symbol.name);
          //printf("\n");
          break;
        case CELL:
          cells++;
          break;
        default:
          //printf("\n");
          break;
      }

      free_list[i] = obj;
      active_list[i] = NULL;
      swept++;

    } else {
#ifdef GC_DEBUG_XX
      printf("\nDo NOT sweep: %p id: %d mark: %d ", obj, obj->id, obj->mark);

      print(obj);
      switch (obj->type) {
        case FIXNUM:
          printf("\n");
          break;
        case STRING:
          break;
        case SYMBOL:
          printf("\n");
          break;
        case PROC:
          printf("\n");
          break;
        case PRIMITIVE:
          printf("\n");
          break;
        default:
          break;
      }
      if (!is_active(obj)) {
        error("NOT found in active list!");
      }
#endif // GC_DEBUG_XX
      obj->mark = 0;
      kept++;
    }

    counted++;
  }
#ifdef GC_DEBUG
  printf("\nDone sweep.  kept: %d swept: %d counted: %d\n\n", kept, swept, counted);
  printf("%d are cells\n", cells);
#endif // GC_DEBUG
}

int check_active() {
  int counted = 0;

  for (int i = 0; i < MAX_ALLOC_SIZE; i++) {
    if (active_list[i] != NULL)
      counted++;
  }

#ifdef GC_DEBUG
  printf("Done check_active: %d counted\n", counted);
#endif // GC_DEBUG
  return counted;
}

int check_free() {
  int counted = 0;

  for (int i = 1; i <= MAX_ALLOC_SIZE; i++) {
    if (free_list[i] != NULL)
      counted++;
  }

#ifdef GC_DEBUG
  printf("Done check_free: %d counted\n", counted);
#endif // GC_DEBUG
  return counted;
}

void check_mem() {
  int active = check_active();
  int free = check_free();
  int total = active + free;
  printf("\nDone check_mem: %d total\n", total);
  if (total != MAX_ALLOC_SIZE) {
    printf("Missing %d objects", MAX_ALLOC_SIZE - total);
    error("check_mem fail!");
  }
}

void mark_pins(struct PinnedVariable *pvs) {
  struct PinnedVariable *pv = NULL;
  pv = pvs;

  for (pv = pvs; pv != NULL; pv = pv->next) {
    /* if (pv->in_use != 1) { */
    /*   printf("\nIn use? %d\n", pv->in_use); */
    /*   continue; */
    /* } */

    printf("Pointer to pointer to variable: %p\n", pv);
    //printf("Pointer to struct pinned variable: %p\n", *pv);
    //printf("Pointer to variable: %p\n", (PinnedVariable *)*pv);
    printf("Pointer to pointer to object: %p\n", pv->var);
    printf("Pointer to object: %p\n", (Object *)pv->var);

    //void *tempObj = (void *)(*(*v)->var);
    Object *obj = (Object *)pv->var;
    if (obj != NULL) {
      print(obj);
      mark(obj);
      print((Object *)pv->var);
      printf("\nMarked pinned var\n");
    }
  }
}

void gc() {

  printf("\nGC v----------------------------------------v\n");
  check_mem();

#ifdef GC_MARK
  printf("\n-------- Mark symbols:");
  mark(symbols);

  printf("\n-------- Mark top_env:");
  mark(top_env);

#ifdef GC_PIN
  printf("\n-------- Mark pins:\n");
  mark_pins(pv_head);
#endif // GC_PIN
#endif // GC_MARK

#ifdef GC_SWEEP
  printf("\n-------- Sweep\n");
  sweep();
  check_mem();
#endif // GC_SWEEP

  printf("\nGC ^----------------------------------------^\n");
}

void *find_next_free() {
  void *obj = NULL;

  for (int i = 0; i < MAX_ALLOC_SIZE; i++) {
    obj = free_list[i];

    if (obj != NULL) {
      active_list[i] = obj;
      free_list[i] = NULL;
      break;
    }
  }

  return obj;
}

void *alloc_Object() {
  void *obj = find_next_free();

  if (obj == NULL) {
    //print_pins();
    gc();

    obj = find_next_free();
  }

  if (obj == NULL) {
    printf("Out of memory\n");
    exit(-1);
  }

#ifdef GC_DEBUG_X
  printf("Allocated %d ", obj->id);
#endif

  return obj;
}
#endif // GC_ENABLED
