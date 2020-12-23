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

void error(char *msg) {
  printf("\nError %s\n", msg);
  exit(0);
}

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

void setcar(Object *obj, Object *val) {
  obj->cell.car = val;
}

void setcdr(Object *obj, Object *val) {
  if (obj == NULL)
    error("Cannot set NULL cdr");

  if (obj->cell.cdr != NULL && obj->cell.cdr != s_nil) {
    //printf("Changing %p -> cdr %p to %p\n", obj, obj->cell.cdr, val);
  }

  obj->cell.cdr = val;
}

Object *new_Object() {
#ifdef GC_ENABLED
  Object *obj = alloc_Object();
#else
  Object *obj = calloc(1, sizeof(Object));
#endif // GC_ENABLED

  obj->type = UNKNOWN;
  obj->mark = 0;

#ifdef GC_PIN_DEBUG
  printf("Allocated object %p\n", obj);
#endif
  return obj;
}

Object *make_cell() {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = new_Object();

  obj->type = CELL;
  obj->cell.car = s_nil;
  obj->cell.cdr = s_nil;
  unpin_variable((void **)&obj);
  return obj;
}

Object *cons(Object *car, Object *cdr) {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = make_cell();
  obj->cell.car = car;
  obj->cell.cdr = cdr;
  unpin_variable((void **)&obj);
  return obj;
}

Object *make_string(char *str) {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = new_Object();
  obj->type = STRING;
  obj->str.text = strdup(str);
  unpin_variable((void **)&obj);
  return obj;
}

Object *make_fixnum(int n) {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = new_Object();
  obj->type = FIXNUM;
  obj->num.value = n;
  unpin_variable((void **)&obj);
  return obj;
}

Object *make_symbol(char *name) {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = new_Object();
  obj->type = SYMBOL;
  obj->symbol.name = strdup(name);
  unpin_variable((void **)&obj);
  return obj;
}

Object *make_primitive(primitive_fn *fn) {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = new_Object();
  obj->type = PRIMITIVE;
  obj->primitive.fn = fn;
  unpin_variable((void **)&obj);
  return obj;
}

Object *make_proc(Object *vars, Object *body, Object *env) {
  Object *obj = NULL;

  pin_variable((void **)&obj);
  obj = new_Object();
  obj->type = PROC;
  obj->proc.vars = vars;
  obj->proc.body = body;
  obj->proc.env = env;
  unpin_variable((void **)&obj);
  printf("Made proc\n");
  return obj;
}

/* Walk linked list of symbols
 * for one matching name.
 */
Object *lookup_symbol(char *name) {
  Object *cell = symbols;
  Object *sym;

#ifdef GC_DEBUG_XX
  printf("Symbol for lookup %s\n", name);
#endif

  while (cell != s_nil) {
    sym = car(cell);

#ifdef GC_DEBUG_XX
    printf("Symbol for lookup comparison? %d %s\n", is_symbol(sym), sym->symbol.name);
#endif

    if (is_symbol(sym) &&
        strcmp(sym->symbol.name, name) == 0) {
#ifdef GC_DEBUG_XX
      printf("Symbol lookup succeeded\n");
      printf("Symbol address %p\n", sym);
#endif
      return sym;
    }

    //printf("Still looking....\n");
    cell = cdr(cell);
  }

  return NULL;
}

/* Lookup a symbol, and return it if found.
 * If not found, create a new one and
 * add it to linked list of symbols,
 * and return the new symbol.
 */
Object *intern_symbol(char *name) {
  Object *sym = lookup_symbol(name);

  if (sym == NULL) {
    //printf("Make symbol %s\n", name);
    sym = make_symbol(name);
    //printf("Made symbol %p\n", sym);
    symbols = cons(sym, symbols);
    //printf("Interned symbol %p\n", sym);
  }

  return sym;
}

// BUG: Does not walk list of environments! (?)
Object *assoc(Object *key, Object *list) {
  if (list != s_nil) {
    /* printf("Assoc:\n"); */
    /* printf("key:\n"); */
    /* print(key); */
    Object *pair = car(list);
    /* printf("pair:\n"); */
    /* print(pair); */
    /* printf("Assoc check '%s' vs '%s'\n", car(pair)->symbol.name, key->symbol.name); */
    printf("Assoc check '%s' vs '%s'\n", key->symbol.name, car(pair)->symbol.name);
    /* printf("\nAssoc check %p vs %p\n", car(pair), key); */
    if (car(pair) == key)
      return pair;

    //printf("Still looking....\n");
    return assoc(key, cdr(list));
  }

  printf("Not found:  '%s'\n", key->symbol.name);
  return NULL;
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
  if (isspace(c))
    return 1;
  else
    return 0;
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
    if (c == '\n' || c == '\r')
      done = 1;

    if (!is_whitespace(c)) {
      ungetc(c, in);
      done = 1;
    }
  }
}

void skip_comment(FILE *in) {
  char c;
  int done = 0;

  while (!done) {
    c = getc(in);
    if (c == '\n' || c == '\r')
      done = 1;
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

// XXX Review these: strchr, strdup, strcmp, strspn, atoi
Object *read_lisp(FILE *in) {
  Object *obj = s_nil;
  pin_variable((void **)&obj);

  skip_whitespace(in);
  char c = getc(in);

  if (c == '\'') {
    obj = cons(s_quote, cons(read_lisp(in), s_nil));
  } else if (c == '(') {
    obj = read_list(in);
  } else if (c == '"') {
    obj = read_string(in);
  } else if (isdigit(c)) {
    ungetc(c, in);
    obj = read_number(in);
  } else if (isalpha(c) ||
             strchr("+-/*", c)) {
    ungetc(c, in);
    obj = read_symbol(in);
  } else if (c == ')') {
    ungetc(c, in);
  } else if (c == ';') {
    skip_comment(in);
  } else if (c == EOF) {
#ifdef REPL
    exit(0);
#else
    printf("EOL\n");
    return NULL;
#endif
  }

  unpin_variable((void **)&obj);

  return obj;
}

Object *read_list(FILE *in) {
  Object *car = NULL;
  Object *cdr = NULL;
  pin_variable((void **)&car);

  car = cdr = make_cell();
  car->cell.car = read_lisp(in);

  char c;

  while ((c = getc(in)) != ')') {
    if (c == '.') {
      // Discard the char after '.'
      // but we should check for whitespace.
      getc(in);

      // The rest goes into the cdr.
      car->cell.cdr = read_lisp(in);
    } else if (!is_whitespace(c)) {
      ungetc(c, in);

      cdr->cell.cdr = make_cell();
      cdr = cdr->cell.cdr;
      cdr->cell.car = read_lisp(in);
    }
  }

  unpin_variable((void **)&car);

  return car;
}

/*
 * Returns a list with a new cons cell
 * containing var and val at the head
 * and the existing env in the tail.
 */
Object *extend(Object *env, Object *var, Object *val) {
  Object *pair = NULL;
  pin_variable((void **)&pair);

  pair = cons(var, val);

  Object *result = NULL;
  pin_variable((void **)&result);

  result = cons(pair, env);

  unpin_variable((void **)&result);
  unpin_variable((void **)&pair);

  return result;
}

void print_env(Object *env) {
  printf("\nEnv entry: ");

  Object *head = car(env);
  print(head);
  printf(" at %p:\n", head);

  Object *tail = cdr(env);

  if (tail != s_nil) {
    print_env(tail);
  }

  //printf("Done.\n");
}


/*
 * Set the tail of this env to a new list
 * with var and val at the head.
 */
Object *extend_env(Object* env, Object *var, Object *val) {
  Object *current_env = cdr(env);
  Object *updated_env = extend(current_env, var, val);

  setcdr(env, updated_env);

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
  printf("progn\n");
  //print_env(env);

  if (forms == s_nil)
    return s_nil;

  for (;;) {
    if (cdr(forms) == s_nil)
    {
      //printf("Eval 1 in progn: ");
      print(car(forms));
      printf("\n");
      Object *temp = eval(car(forms), env);
      //printf("\n------------------> End of progn\n");
      //print_env(env);
      return temp;

      //return eval(car(forms), env);
    }

    //printf("Eval 2 in progn: ");
    eval(car(forms), env);
    print(car(forms));
    printf("\nRecurse in progn: ");
    print(cdr(forms));
    printf("\n");
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

Object *apply(Object *obj, Object *args, Object *env) {
  printf("Apply\n");
  //print_env(env);

  if (is_primitive(obj))
    return (*obj->primitive.fn)(args);

  if (is_proc(obj))
    return progn(obj->proc.body, multiple_extend_env(env, obj->proc.vars, args));

  // If this is neither a primitive function nor a proc,
  // we are in a bad state.
  printf("Dumping %s:\n", get_type(obj));
  print(obj);
  printf("\n");

  printf("Proc vars:\n");
  printf("Proc vars pointer: %p\n", obj->proc.vars);
  printf("Proc vars type: %d\n", obj->proc.vars->type);
  print(obj->proc.vars);
  printf("Proc body:\n");
  print(obj->proc.body);
  printf("Proc env:\n");
  print(obj->proc.env);

  error("Bad apply");

  return s_nil;
}

Object *eval_symbol(Object *obj, Object *env) {
  Object *pair = assoc(obj, env);

  if (pair == NULL) {
    char *buff = NULL;
    asprintf(&buff, "Undefined symbol '%s'", obj->symbol.name);
    error(buff);
  }

  return cdr(pair);
}

Object *eval_list(Object *obj, Object *env) {
  if (obj == s_nil)
    return obj;

  if (car(obj) == s_define) {
    Object *cell = obj; // car(cell) should be symbol named define

    cell = cdr(cell);
    Object *cell_symbol = car(cell);

    cell = cdr(cell);
    Object *cell_value = car(cell);

    Object *val = eval(cell_value, env);

    // Check for existing binding?
    Object *pair = assoc(cell_symbol, env);

    if (pair == NULL) {
      printf("Creating new binding: ");
      print(cell_symbol);
      printf("\n");
      Object *var = cell_symbol;
      return extend_env(env, var, val);
    } else {
      setcdr(pair, val);

      return val;
    }
  } else if (car(obj) == s_setq) {
    //printf("SETQ\n");
    Object *cell = obj; // car(cell) should be symbol named setq
    //print(cell);

    cell = cdr(cell);
    Object *cell_symbol = car(cell);
    //print(cell_symbol);

    cell = cdr(cell);
    Object *cell_value = car(cell);
    //print(cell_value);

    Object *pair = assoc(cell_symbol, env);

    if (pair == NULL)
      error("SETQ failed to find symbol in env.");

    Object *newval = eval(cell_value, env);

    setcdr(pair, newval);

    return newval;
  } else if (car(obj) == s_if) {
    Object *cell = obj;

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
  } else if (car(obj) == s_lambda) {
    Object *vars = car(cdr(obj));
    Object *body = cdr(cdr(obj));

    printf("Lambda with env:\n");
    print_env(env);
    printf("\n");
    return make_proc(vars, body, env);
  }

  /* This list is not a builtin, so treat it as a function call. */
  Object *proc = eval(car(obj), env);
  Object *args = eval_args(cdr(obj), env);

  printf("Assuming proc\n");
  print_env(env);
  printf("\n");
  return apply(proc, args, env);
}

Object *eval(Object *obj, Object *env) {
  if (obj == NULL)
    return obj;

  printf("Eval\n");
  print(obj);
  printf(" with env \n");
  print_env(env);

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
      printf("\nEval Unknown Object: %d\n", obj->type);
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
  printf("%d", obj->num.value);
}

void print_cell(Object *car) {
  Object *obj = car;

  printf("(");

  while (obj != s_nil && obj != NULL) {
    if (obj->type == CELL) {
      print(obj->cell.car);
    } else {
      printf(". ");
      print(obj);
      break;
    }

    obj = obj->cell.cdr;

    if (obj != s_nil && obj != NULL)
      printf(" ");
  }

  printf(")");
}

void print(Object *obj) {
  if (obj == NULL) {
    printf("NULL OBJECT\n");
    return;
  }

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
      printf("\nPrint Unknown Object - type? %d\n", obj->type);
      //sleep(1);
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
  int a_val = a->num.value;
  int b_val = b->num.value;

  if (a_val == b_val) {
    return s_t;
  } else {
    return s_nil;
  }
}

Object *primitive_eq(Object *args) {
  if (is_fixnum(car(args)) &&
      is_fixnum(cadr(args)))
    return primitive_eq_num(car(args), cadr(args));
  else
    return s_nil;
}

void init() {

#ifdef GC_ENABLED
  current_mark = 1;
#endif

  free_list[0] = NULL;

  for (int i = 1; i <= MAX_ALLOC_SIZE; i++) {
    Object *obj = calloc(1, sizeof(struct Object));
    obj->id = i;
    obj->type = UNKNOWN;
    free_list[i] = obj;
  }

#ifdef GC_PIN_DEBUG
  printf("Done init.\n");
#endif
}

void run_code_tests() {
  printf("\n\nBEGIN CODE TESTS\n");

  Object *obj1 = NULL;
  pin_variable((void **)&obj1);
  obj1 = new_Object();
  obj1->type = NIL;

  Object *obj2 = NULL;
  pin_variable((void **)&obj2);
  obj2 = new_Object();
  obj2->type = NIL;

  Object *obj3 = NULL;
  pin_variable((void **)&obj3);
  obj3 = new_Object();
  obj3->type = NIL;

  gc();

  unpin_variable((void **)&obj3);
  unpin_variable((void **)&obj2);
  unpin_variable((void **)&obj1);

  printf("END CODE TESTS\n");
}

void run_file_tests(char *fname) {
  printf("\n\n--------------------BEGIN FILE TESTS: %s\n", fname);

  FILE *fp = fopen(fname, "r");

  if (fp == NULL) {
    printf("File open failed: %d", errno);
    return;
  }

  Object *result = s_nil;
  pin_variable((void **)&result);

  while (result != NULL) {
    printf("\n----\nREAD from file\n");
    result = read_lisp(fp);
    printf("After read:\n");
    print(result);
    printf("\n----\nEVALUATE from file\n");
    printf("Before eval:\n");
    print(result);
    printf("\n");
    result = eval(result, env);
    printf("After eval:\n");
    print(result);
    printf("\n");
  }

  unpin_variable((void **)&result);

  fclose(fp);
  printf("END FILE TESTS\n");
}

int main(int argc, char* argv[]) {
  init();

  /* Make symbol nil (end of list). */
  s_nil = make_symbol("nil");

  /* Create symbol table.   */
  /*           'nil'   EOL  */
  /*             |      |   */
  /*             v      v   */
  //symbols = cons(NULL, NULL);
  symbols = cons(s_nil, s_nil);

  s_t = intern_symbol("t");
  s_lambda = intern_symbol("lambda");
  s_define = intern_symbol("define");
  s_quote = intern_symbol("quote");
  s_setq = intern_symbol("setq");
  s_if = intern_symbol("if");

  /* Create top level environment (list of lists).
   * Head is empty list and should never change,
   * so global references to env are stable
   * and changing the env does not require
   * returning a new head.
   */
  env = cons(cons(s_nil, s_nil), s_nil);
  extend_env(env, s_t, s_t);

  extend_env(env, intern_symbol("cons"), make_primitive(prim_cons));
  extend_env(env, intern_symbol("car"), make_primitive(prim_car));
  extend_env(env, intern_symbol("cdr"), make_primitive(prim_cdr));

  extend_env(env, intern_symbol("eq"), make_primitive(primitive_eq));

  extend_env(env, intern_symbol("+"), make_primitive(primitive_add));
  extend_env(env, intern_symbol("-"), make_primitive(primitive_sub));
  extend_env(env, intern_symbol("*"), make_primitive(primitive_mul));
  extend_env(env, intern_symbol("/"), make_primitive(primitive_div));

#ifdef CODE_TEST
  run_code_tests();
#endif

#ifdef FILE_TEST
  /* run_file_tests("./test1.lsp"); */
  /* run_file_tests("./test2.lsp"); */
  /* run_file_tests("./test3.lsp"); */
  /* run_file_tests("./test4.lsp"); */
  /* run_file_tests("./test5.lsp"); */
  /* run_file_tests("./test6.lsp"); */
//  run_file_tests("./testP.lsp");
  run_file_tests("./testQ.lsp");
  /* run_file_tests("./testR.lsp"); */
  /* run_file_tests("./testS.lsp"); */
  /* run_file_tests("./testT.lsp"); */
  /* run_file_tests("./testU.lsp"); */
  /* run_file_tests("./testV.lsp"); */
  /* run_file_tests("./testW.lsp"); */
  /* run_file_tests("./testX.lsp"); */
  /* run_file_tests("./testY.lsp"); */
  /* run_file_tests("./testZ.lsp"); */
#endif

#ifdef REPL
  printf("\nWelcome to JCM-LISP. Use ctrl-c to exit.\n");

  while (1) {
    Object *result = s_nil;

    printf("> ");
    result = read_lisp(stdin);
    result = eval(result, env);
    print(result);
    printf("\n");
  }
#endif

  return 0;
}
