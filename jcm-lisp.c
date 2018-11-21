/*
 * JCM-LISP
 *
 * Based on http://web.sonoma.edu/users/l/luvisi/sl3.c
 * and http://peter.michaux.ca/archives
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

#define MAX_BUFFER_SIZE 100

//#define MAX_ALLOC_SIZE  24
//#define MAX_ALLOC_SIZE  56
//#define MAX_ALLOC_SIZE  64
//#define MAX_ALLOC_SIZE  128
#define MAX_ALLOC_SIZE  1000
//#define MAX_ALLOC_SIZE  10000

#define GC_ENABLED
#define GC_MARK
#define GC_SWEEP
//#define GC_DEBUG
//#define GC_DEBUG_X

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
typedef struct Object *(primitive_fn)(struct Object *);

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
  obj_type type;
  char mark;
  struct Object *next;

  union {
    struct Fixnum num;
    struct String str;
    struct Symbol symbol;
    struct Cell cell;
    struct Primitive primitive;
    struct Proc proc;
  };
};

void print(Object *);

Object *symbols;    /* linked list of symbols */
Object *s_quote;
Object *s_define;
Object *s_setq;
Object *s_nil;
Object *s_if;
Object *s_t;

//Object *prim_add, *prim_sub, *prim_mul, *prim_div;

Object *lambda_s;

Object *free_list;
Object *active_list;
Object *env;

#ifdef GC_ENABLED
struct PinnedVariable *pinned_variables;
int current_mark;
#endif

struct PinnedVariable {
  Object **variable;
  struct PinnedVariable *next;
};

#ifdef GC_ENABLED
void pin_variable(Object **obj) {

#ifdef GC_DEBUG_X
  printf("> variable %p\n", obj);
#endif

  struct PinnedVariable *pinned_var = calloc(1, sizeof(struct PinnedVariable));
  assert(pinned_var != NULL);
  pinned_var->variable = obj;

  pinned_var->next = pinned_variables;

#ifdef GC_DEBUG_X
  printf("Pinned variables before %p\n", pinned_variables);
#endif

  pinned_variables = pinned_var;

#ifdef GC_DEBUG_X
  printf("Pinned variables after %p\n", pinned_variables);
#endif
}
#else
void pin_variable(Object **obj) { }
#endif

#ifdef GC_ENABLED
void unpin_variable(Object **variable) {

#ifdef GC_DEBUG_X
  printf("< variable %p\n", variable);
#endif

  struct PinnedVariable **v;
  for (v = &pinned_variables; *v != NULL; v = &(*v)->next) {
    if ((*v)->variable == variable) {

#ifdef GC_DEBUG_X
      printf("Pinned variable  found %p\n", *v);
#endif
      struct PinnedVariable *next = (*v)->next;
      free(*v);
      *v = next;
#ifdef GC_DEBUG_X
      printf("Pinned variable head %p\n", *v);
#endif
      return;
    }
  }
  assert(0);
}
#else
void unpin_variable(Object **variable) { }
#endif

char *get_type(Object *obj) {
  if (obj == NULL)
    return "NULL";
  else if (obj->type == FIXNUM)
    return "FIXNUM";
  else if (obj->type == STRING)
    return "STRING";
  else if (obj->type == SYMBOL)
    return "SYMBOL";
  else if (obj->type == CELL)
    return "CELL";
  else if (obj->type == PRIMITIVE)
    return "PRIM";
  else if (obj->type == PROC)
    return "PROC";
  else
    return "UNKNOWN";
}

int is_fixnum(Object *obj) {
  return (obj && obj->type == FIXNUM);
}

int is_string(Object *obj) {
  return (obj && obj->type == STRING);
}

int is_symbol(Object *obj) {
  return (obj && obj->type == SYMBOL);
}

int is_cell(Object *obj) {
  return (obj && obj->type == CELL);
}

int is_primitive(Object *obj) {
  return (obj && obj->type == PRIMITIVE);
}

int is_proc(Object *obj) {
  return (obj && obj->type == PROC);
}

Object *car(Object *obj) {
  if (is_cell(obj))
    return obj->cell.car;
  else
    return s_nil;
}

Object *cdr(Object *obj) {
  if (is_cell(obj))
    return obj->cell.cdr;
  else
    return s_nil;
}

#define caar(obj)    car(car(obj))
#define cadr(obj)    car(cdr(obj))

void setcar(Object *obj, Object *val) {
  obj->cell.car = val;
}

void setcdr(Object *obj, Object *val) {
  obj->cell.cdr = val;
}

#ifdef GC_ENABLED
void mark(Object *obj) {
  if (obj == NULL
      //|| obj->mark == 1
    )
  {
#ifdef GC_DEBUG
    printf("\nNothing to mark");
#endif
    return;
  }

  //sleep(1);

#ifdef GC_DEBUG_X
  printf("\nBefore mark: %d", obj->mark);
#endif

  /* if (obj->mark > current_mark + 10) { */
  /*   printf("\nToo many marks:\n"); */
  /*   print(obj); */
  /*   exit(-1); */
  /* } */

  //obj->mark++;
  //obj->mark = current_mark;
  obj->mark = 1;

#ifdef GC_DEBUG_X
  printf("\nAfter mark: %d", obj->mark);
#endif

  switch (obj->type) {
    case FIXNUM:
    case STRING:
    case SYMBOL:
    case PRIMITIVE:
#ifdef GC_DEBUG
      printf("\nMark %s ", get_type(obj));
      print(obj);
#endif
      break;
    case CELL:
#ifdef GC_DEBUG
      printf("\nMark cell car ->");
#endif
      mark(obj->cell.car);
#ifdef GC_DEBUG
      printf("\nMark cell car <-");
#endif
#ifdef GC_DEBUG
      printf("\nMark cell cdr ->");
#endif
      mark(obj->cell.cdr);
#ifdef GC_DEBUG
      printf("\nMark cell cdr <-");
#endif
      break;
    case PROC:
#ifdef GC_DEBUG
      printf("\nMark proc ->");
      print(obj);
#endif
#ifdef GC_DEBUG
      printf("\nMark proc vars");
#endif
      mark(obj->proc.vars);
#ifdef GC_DEBUG
      printf("\nMark proc body");
#endif
      mark(obj->proc.body);
#ifdef GC_DEBUG
      printf("\nMark proc env");
#endif
      mark(obj->proc.env);
#ifdef GC_DEBUG
      printf("\nMark proc <-");
#endif
      break;
    default:
      printf("\nMark unknown object: %d\n", obj->type);
      break;
  }
}

void sweep() {
  Object *obj = active_list;
  int counted = 0, kept = 0, swept = 0;

  printf("Active list            = %p\n", active_list);
  printf("Free list              = %p\n", free_list);

  while (obj != NULL) {
    Object *next = obj->next;
#ifdef GC_DEBUG_X
    printf("\nObj at                 = %p\n", obj);
#endif

    if (obj->mark == 0) {
      //printf("obj->mark: %d\n", obj->mark);
      printf("SWEEP: ");
      print(obj);

      // Free any additional allocated memory.
      switch (obj->type) {
        case STRING:
          memset(obj->str.text, 0, strlen(obj->str.text));
          free(obj->str.text);
          break;
        case SYMBOL:
          memset(obj->symbol.name, 0, strlen(obj->symbol.name));
          free(obj->symbol.name);
          break;
        default:
          printf("\n");
          break;
      }

      active_list = obj->next;
      printf("Active list now        = %p\n", active_list);
      obj->next = free_list;
      free_list = obj;
      printf("Free list now          = %p\n", free_list);
      swept++;
    } else {
#ifdef GC_DEBUG_X
      printf("Do NOT sweep: ");
      print(obj);
#endif
      obj->mark = 0;

#ifdef GC_DEBUG
      // Pretty-print
      switch (obj->type) {
        case STRING:
          break;
        case SYMBOL:
          break;
        default:
          //printf("\n");
          break;
      }
#endif
      kept++;
    }
    obj = next;
    counted++;
  }
#ifdef GC_DEBUG
  printf("\nDone sweep: %d kept %d swept %d counted\n", kept, swept, counted);
#endif
}

void gc() {

  printf("\nGC v----------------------------------------v\n");

#ifdef GC_MARK
  /* current_mark++; */
  /* printf("Current mark: %d\n", current_mark); */

  printf("\n-------- Mark symbols:");
  mark(symbols);

  printf("\n-------- Mark env:");
  mark(env);
#endif

#ifdef GC_SWEEP
  printf("\n-------- Sweep\n");
  sweep();
#endif

  printf("\nGC ^----------------------------------------^\n");
}

Object *alloc_Object() {
  if (free_list == NULL) {
    gc();
  }

  Object *obj = free_list;

  if (obj == NULL) {
    printf("Out of memory\n");
    exit(-1);
  }

  //printf("\nFree list before alloc = %p\n", free_list);

  // free_list will point to the object after this one.
  free_list = obj->next;
  //printf("Free list after alloc  = %p\n", free_list);

  // this object will point to active_list
  obj->next = active_list;

  // active_list will start with this object
  active_list = obj;
  //printf("Active list            = %p\n", active_list);

  //printf("Allocate an object at  = %p\n", obj);

  return obj;
}
#endif

Object *new_Object() {
#ifdef GC_ENABLED
  Object *obj = alloc_Object();
#else
  Object *obj = calloc(1, sizeof(Object));
#endif

  obj->type = UNKNOWN;
  obj->mark = 0;
  return obj;
}

Object *make_cell() {
  Object *obj = new_Object();
  //Object *obj;

  pin_variable(&obj);
  //obj = new_Object();
  obj->type = CELL;
  obj->cell.car = s_nil;
  obj->cell.cdr = s_nil;
  unpin_variable(&obj);
  return obj;
}

Object *cons(Object *car, Object *cdr) {
  Object *obj;

  pin_variable(&obj);
  obj = make_cell();
  obj->cell.car = car;
  obj->cell.cdr = cdr;
  unpin_variable(&obj);
  return obj;
}

Object *make_string(char *str) {
  Object *obj;

  pin_variable(&obj);
  obj = new_Object();
  obj->type = STRING;
  obj->str.text = strdup(str);
  unpin_variable(&obj);
  return obj;
}

Object *make_fixnum(int n) {
  Object *obj;

  pin_variable(&obj);
  obj = new_Object();
  obj->type = FIXNUM;
  obj->num.value = n;
  unpin_variable(&obj);
  return obj;
}

Object *make_symbol(char *name) {
  Object *obj;

  pin_variable(&obj);
  obj = new_Object();
  obj->type = SYMBOL;
  obj->symbol.name = strdup(name);
  unpin_variable(&obj);
  return obj;
}

Object *make_primitive(primitive_fn *fn) {
  Object *obj;

  pin_variable(&obj);
  obj = new_Object();
  obj->type = PRIMITIVE;
  obj->primitive.fn = fn;
  unpin_variable(&obj);
  return obj;
}

Object *make_proc(Object *vars, Object *body, Object *env) {
  Object *obj;

  pin_variable(&obj);
  obj = new_Object();
  obj->type = PROC;
  obj->proc.vars = vars;
  obj->proc.body = body;
  obj->proc.env = env;
  unpin_variable(&obj);
  return obj;
}

/* Walk simple linked list of symbols
 * for one matching name.
 */
Object *lookup_symbol(char *name) {
  Object *cell = symbols;
  Object *sym;

  //printf("Symbol for lookup %s\n", name);

  while (cell != s_nil) {
    sym = car(cell);

    //printf("Symbol for lookup comparison? %d %s\n", is_symbol(sym), sym->symbol.name);

    if (is_symbol(sym) &&
        strcmp(sym->symbol.name, name) == 0) {
      //printf("Symbol lookup succeeded\n");
      return sym;
    }

    cell = cdr(cell);
  }

  //return NULL;
  return s_nil;
}

/* Lookup a symbol, and return it if found.
 * If not found, create a new one and
 * add it to linked list of symbols,
 * and return the new symbol.
 */
Object *intern_symbol(char *name) {
  Object *sym = lookup_symbol(name);

  if (sym == s_nil) {
    //printf("Make symbol %s\n", name);
    sym = make_symbol(name);
    symbols = cons(sym, symbols);
  }

  return sym;
}

Object *assoc(Object *key, Object *list) {
  if (list != s_nil) {
    Object *pair = car(list);

    if (car(pair) == key)
      return pair;

    return assoc(key, cdr(list));
  }

  return s_nil;
}

Object *primitive_add(Object *args) {
  int total = 0;

  while (args != s_nil) {
    total += car(args)->num.value;
    args = cdr(args);
  }

  return make_fixnum(total);
}

Object *primitive_sub(Object *args) {
  long result = car(args)->num.value;

  args = cdr(args);
  while (args != s_nil) {
    result -= car(args)->num.value;
    args = cdr(args);
  }

  return make_fixnum(result);
}

Object *primitive_mul(Object *args) {
  long total = 1;

  while (args != s_nil) {
    total *= car(args)->num.value;
    args = cdr(args);
  }

  return make_fixnum(total);
}

Object *primitive_div(Object *args) {
  long dividend = car(args)->num.value;
  long divisor = cadr(args)->num.value;
  long quotient = 0;

  if (divisor != 0)
    quotient = dividend / divisor;

  return make_fixnum(quotient);
}

int is_whitespace(char c) {
  if (isspace(c)) {
    return 1;
  } else {
    return 0;
  }
}

int is_symbol_char(char c) {
  return (isalnum(c) ||
          strchr("+-*/", c));
}

void skip_whitespace(FILE *in) {
  char c;
  int done = 0;

  while (!done) {
    c = getc(in);
    if (c == '\n' && c == '\r')
      done = 1;

    if (!is_whitespace(c)) {
      ungetc(c, in);
      done = 1;
    }
  }
}

Object *read_string(FILE *in) {
  char buffer[MAX_BUFFER_SIZE];
  int i = 0;
  char c;

  while ((c = getc(in)) != '"' &&
         i < MAX_BUFFER_SIZE - 1) {
    buffer[i++] = c;
  }

  buffer[i] = '\0';
  return make_string(buffer);
}

Object *read_symbol(FILE *in) {
  char buffer[MAX_BUFFER_SIZE];
  int i = 0;
  char c;

  while (!is_whitespace(c = getc(in)) &&
         is_symbol_char(c) &&
         i < MAX_BUFFER_SIZE - 1) {
    buffer[i++] = c;
  }

  buffer[i] = '\0';
  ungetc(c, in);

  Object *obj = intern_symbol(buffer);

  return obj;
}

Object *read_number(FILE *in) {
  int number = 0;
  char c;

  while (isdigit(c = getc(in))) {
    number *= 10;
    number += (int)c - 48;
  }

  ungetc(c, in);

  return make_fixnum(number);
}

Object *read_lisp(FILE *);

Object *read_list(FILE *in) {
  char c;
  Object *car, *cdr;

  car = cdr = make_cell();
  car->cell.car = read_lisp(in);

  while ((c = getc(in)) != ')') {
    if (c == '.') {
      // Discard the char after '.'
      // but we should check for whitespace.
      getc(in);
      // The rest goes into the cdr.
      cdr->cell.cdr = read_lisp(in);
    } else if (!is_whitespace(c)) {
      ungetc(c, in);

      cdr->cell.cdr = make_cell();
      cdr = cdr->cell.cdr;
      cdr->cell.car = read_lisp(in);
    }
  };

  return car;
}

// XXX Review these:
// strchr
// strdup
// strcmp
// strspn
// atoi
Object *read_lisp(FILE *in) {
  Object *obj = s_nil;
  char c;

  skip_whitespace(in);
  c = getc(in);

  if (c == '\'') {
    obj = cons(s_quote, cons(read_lisp(in), s_nil));
  } else if (c == '(') {
    obj = read_list(in);
  } else if (c == '"') {
    obj = read_string(in);
  } else if (isdigit(c)) {
    ungetc(c, in);
    obj = read_number(in);
  } else if (isalpha(c)) {
    ungetc(c, in);
    obj = read_symbol(in);
  } else if (strchr("+-/*", c)) {
    ungetc(c, in);
    obj = read_symbol(in);
  } else if (c == ')') {
    ungetc(c, in);
  } else if (c == EOF) {
    exit(0);
  }

  return obj;
}

void error(char *msg) {
  printf("\nError %s\n", msg);
  exit(0);
}

Object *eval_symbol(Object *obj, Object *env) {
  //printf("s_nil %p\n", s_nil);
  //printf("Symbol? %p\n", obj);
  //printf("Symbol name? '%s'\n", obj->symbol.name);
  if (obj == s_nil) {
    //printf("\ns_nil??\n");
    return obj;
  }

  Object *tmp = assoc(obj, env);

  if (tmp == s_nil) {
    char *buff = NULL;
    asprintf(&buff, "Undefined symbol '%s'", obj->symbol.name);
    error(buff);
  }

  return cdr(tmp);
}

/*
 * Returns a list with a new cons cell
 * containing var and val at the head
 * and the existing env in the tail.
 */
Object *extend(Object *env, Object *var, Object *val) {
  return cons(cons(var, val), env);
}

/*
 * Set the tail of this env to a new list
 * with var and val at the head.
 */
Object *extend_env(Object* env, Object *var, Object *val) {
  setcdr(env, extend(cdr(env), var, val));
  return val;
}

Object *eval(Object *obj, Object *env);

/* Return list of evaluated args. */
Object *eval_args(Object *args, Object *env) {
  if (args != s_nil)
    return cons(eval(car(args), env), eval_args(cdr(args), env));

  return s_nil;
}

Object *progn(Object *forms, Object *env) {
  if (forms == s_nil)
    return s_nil;

  for (;;) {
    if (cdr(forms) == s_nil)
      return eval(car(forms), env);

    forms = cdr(forms);
  }

  return s_nil;
}

/* Iterate vars and vals, adding each pair to this env. */
Object *multiple_extend_env(Object *env, Object *vars, Object *vals) {
  if (vars == s_nil)
    return env;
  else
    return multiple_extend_env(extend(env, car(vars), car(vals)), cdr(vars), cdr(vals));
}

Object *apply(Object *proc, Object *args, Object *env) {
  if (is_primitive(proc))
    return (*proc->primitive.fn)(args);

  if (is_proc(proc))
    return progn(proc->proc.body, multiple_extend_env(env, proc->proc.vars, args));

  printf("Dumping %s\n", get_type(proc));
  /* print(proc); */
  /* print(proc->proc.vars); */
  /* print(proc->proc.body); */
  /* print(proc->proc.env); */
  error("Bad apply");

  return s_nil;
}

Object *eval_list(Object *obj, Object *env) {
  if (obj == s_nil)
    return obj;

  if (car(obj) == s_define) {
    Object *cell = obj;
    //Object *cell_define = car(cell); // should be symbol named define

    cell = cdr(cell);
    Object *cell_symbol = car(cell);

    cell = cdr(cell);
    Object *cell_value = car(cell);

    Object *var = cell_symbol;
    Object *val = eval(cell_value, env);

    return extend_env(env, var, val);
  } else if (car(obj) == s_setq) {
    Object *cell = obj;
    //Object *cell_setq = car(cell); // should be symbol named setq

    cell = cdr(cell);
    Object *cell_symbol = car(cell);

    cell = cdr(cell);
    Object *cell_value = car(cell);

    Object *pair = assoc(cell_symbol, env);
    Object *newval = eval(cell_value, env);
    setcdr(pair, newval);

    return newval;
  } else if (car(obj) == s_if) {
    Object *cell = obj;
    //Object *cell_if = car(cell);

    cell = cdr(cell);
    Object *cell_condition = car(cell);

    cell = cdr(cell);
    Object *cell_true_branch = car(cell);

    cell = cdr(cell);
    Object *cell_false_branch = car(cell);

    if (eval(cell_condition, env) != s_nil)
      return eval(cell_true_branch, env);
    else
      return eval(cell_false_branch, env);

  } else if (car(obj) == s_quote) {
    return car(cdr(obj));
  } else if (car(obj) == lambda_s) {
    Object *vars = car(cdr(obj));
    Object *body = cdr(cdr(obj));

    return make_proc(vars, body, env);
  }

  /* This list is not a builtin, so treat it as a function call. */
  Object *proc = eval(car(obj), env);
  Object *args = eval_args(cdr(obj), env);
  return apply(proc, args, env);
}

Object *eval(Object *obj, Object *env) {
  /* if (obj == s_nil) */
  /*   return obj; */

  Object *result = s_nil;

  switch (obj->type) {
    case STRING:
    case FIXNUM:
    case PRIMITIVE:
    case PROC:
      result = obj;
      break;
    case SYMBOL:
      result = eval_symbol(obj, env);
      break;
    case CELL:
      result = eval_list(obj, env);
      break;
    default:
      printf("\nUnknown Object: %d\n", obj->type);
      break;
  }

  return result;
}

void print_string(Object *obj) {
  char *str = obj->str.text;
  int len = strlen(str);
  int i = 0;

  putchar('"');
  while (i < len) {
    putc(*str++, stdout);
    i++;
  }

  putchar('"');
}

void print_fixnum(Object *obj) {
  //printf("Fixnum\n");
  printf("%d", obj->num.value);
}

void print_cell(Object *car) {
  printf("(");
  Object *obj = car;

  while (obj != s_nil) {
    if (obj->type == CELL) {
      print(obj->cell.car);
    } else {
      printf(". ");
      print(obj);
      break;
    }

    obj = obj->cell.cdr;

    if (obj != s_nil)
      printf(" ");
  }
  printf(")\n");
}

void print(Object *obj) {
  if (obj == NULL) {
    printf("NULL ERROR");
    return;
  }

  /* if (obj == s_nil) { */
  /*   printf("nil"); */
  /*   return; */
  /* } */

  switch (obj->type) {
    case FIXNUM:
      print_fixnum(obj);
      break;
    case STRING:
      print_string(obj);
      break;
    case SYMBOL:
      printf("%s", obj->symbol.name);
      break;
    case CELL:
      print_cell(obj);
      break;
    case PRIMITIVE:
      printf("<PRIM>");
      break;
    case PROC:
      printf("<PROC>");
      break;
    default:
      printf("\nUnknown Object: %d\n", obj->type);
      break;
  }
}

Object *prim_cons(Object *args) {
  return cons(car(args), car(cdr(args)));
}

Object *prim_car(Object *args) {
  return car(car(args));
}

Object *prim_cdr(Object *args) {
  return cdr(car(args));
}

Object *primitive_eq_num(Object *a, Object *b) {
/*
  printf("Compare 2 nums\n");
  print_fixnum(a);
  print_fixnum(b);
*/
  int a_val = a->num.value;
  int b_val = b->num.value;

  if (a_val == b_val) {
    //printf("equal!\n");
    return s_t;
  } else {
    //printf("not equal!\n");
    return s_nil;
  }
}

Object *primitive_eq(Object *args) {
  if (is_fixnum(car(args)) &&
      is_fixnum(cadr(args))) {
    return primitive_eq_num(car(args), cadr(args));
  } else {
    return s_nil;
  }
}

void init() {
  free_list = NULL;
  active_list = NULL;

#ifdef GC_ENABLED
  current_mark = 1;
#endif

  for (int i = 0; i < MAX_ALLOC_SIZE; i++) {
    Object *obj = calloc(1, sizeof(struct Object));
    obj->type = UNKNOWN;
    obj->next = free_list;
    free_list = obj;
  }
}

int main(int argc, char* argv[]) {
  init();

  /* Make symbol nil (end of list). */
  s_nil = make_symbol("nil");

  /* Create empty symbol table. */
  symbols = cons(s_nil, s_nil);

  s_t = intern_symbol("t");
  s_quote = intern_symbol("quote");
  s_setq = intern_symbol("setq");
  s_define = intern_symbol("define");
  s_if = intern_symbol("if");
  lambda_s = intern_symbol("lambda");

  /* Create top level environment (list of lists).
   * Head is empty list and should never change,
   * so global references to env are stable
   * and changing the env does not require
   * returning a new head.
   */
  //Object *env = cons(cons(s_nil, s_nil), s_nil);
  //extend_env(env, s_t, s_t);
  env = cons(cons(s_nil, s_nil), s_nil);

  extend_env(env, intern_symbol("cons"), make_primitive(prim_cons));
  extend_env(env, intern_symbol("car"), make_primitive(prim_car));
  extend_env(env, intern_symbol("cdr"), make_primitive(prim_cdr));

  extend_env(env, intern_symbol("eq"), make_primitive(primitive_eq));

  extend_env(env, intern_symbol("+"), make_primitive(primitive_add));
  extend_env(env, intern_symbol("-"), make_primitive(primitive_sub));
  extend_env(env, intern_symbol("*"), make_primitive(primitive_mul));
  extend_env(env, intern_symbol("/"), make_primitive(primitive_div));

#ifdef GC_ENABLED
  //gc();
#endif

  printf("\nWelcome to JCM-LISP. Use ctrl-c to exit.\n");

  while (1) {
    Object *result = s_nil;

    printf("> ");
    result = read_lisp(stdin);
    result = eval(result, env);
    print(result);
    printf("\n");
  }

  return 0;
}
