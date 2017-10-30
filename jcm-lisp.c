/*
 * JCM-LISP
 *
 * Based on http://web.sonoma.edu/users/l/luvisi/sl3.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <assert.h>

#define MAX_BUFFER_SIZE 100

typedef enum
{
  NIL = 1,
  FIXNUM,
  STRING,
  SYMBOL,
  CELL,
} obj_type;

typedef struct object object;

struct Fixnum
{
  int value;
};

struct String
{
  char *text;
};

struct Symbol
{
  char *name;
  object *value;
};

struct Cell
{
  struct object *car;
  struct object *cdr;
};

struct object
{
  obj_type type;

  union
  {
    struct Fixnum num;
    struct String str;
    struct Symbol symbol;
    struct Cell cell;
  };
};

object *globals;
object *quote;
object *define;
object *setq;
object *nil;
object *tee;

int is_fixnum(object *obj)
{
  return (obj && obj->type == FIXNUM);
}

int is_string(object *obj)
{
  return (obj && obj->type == STRING);
}

int is_symbol(object *obj)
{
  return (obj && obj->type == SYMBOL);
}

int is_cell(object *obj)
{
  return (obj && obj->type == CELL);
}

object *car(object *obj)
{
  if (is_cell(obj))
    return obj->cell.car;
  else
    return NULL;
}

object *cdr(object *obj)
{
  if (is_cell(obj))
    return obj->cell.cdr;
  else
    return NULL;
}

void setcar(object *obj, object *val)
{
  obj->cell.car = val;
}

void setcdr(object *obj, object *val)
{
  obj->cell.cdr = val;
}

#define caar(obj)    car(car(obj))
#define cadr(obj)    car(cdr(obj))

object *new_object()
{
  object *obj = malloc(sizeof(object));
  return obj;
}

object *make_cell()
{
  object *obj = new_object();
  obj->type = CELL;
  obj->cell.car = NULL;
  obj->cell.cdr = NULL;
  return obj;
}

object *cons(object *car, object *cdr)
{
  object *obj = make_cell();
  obj->cell.car = car;
  obj->cell.cdr = cdr;
  return obj;
}

object *make_string(char *str)
{
  object *obj = new_object();
  obj->type = STRING;
  obj->str.text = strdup(str);
  return obj;
}

object *make_fixnum(int n)
{
  object *obj = new_object();
  obj->type = FIXNUM;
  obj->num.value = n;
  return obj;
}

object *make_symbol(char *name)
{
  object *obj = new_object();
  obj->type = SYMBOL;
  obj->symbol.name = strdup(name);
  obj->symbol.value = NULL;
  return obj;
}

object *intern_symbol(char *name, object *env)
{
  object *sym = make_symbol(name);
  object *cell = cons(sym, env->cell.cdr);
  setcdr(env, cell);
  return sym;
}

object *assoc(char *name, object *list)
{
  if (list != NULL)
  {
    object *sym = car(list);

    if (is_symbol(sym) &&
        strcmp(sym->symbol.name, name) == 0)
    {
      return sym;
    }

    return assoc(name, cdr(list));
  }

  //printf("%s undefined.\n", name);
  return NULL;
}

object *lookup_symbol(char *name, object *env)
{
  //return assoc(name, env);

  if (env != NULL)
  {
    object *frame = car(env);
    object *sym = assoc(name, frame);

    if (sym)
    {
      return sym;
    }

    return lookup_symbol(name, cdr(env));
  }

  return NULL;

}

int is_whitespace(char c)
{
  if (isspace(c))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void skip_whitespace(FILE *in)
{
  char c;
  int done = 0;

  while (!done)
  {
    c = getc(in);
    if (c == '\n' && c == '\r')
    {
      done = 1;
    }

    if (!is_whitespace(c))
    {
      ungetc(c, in);
      done = 1;
    }
  }
}

object *read_string(FILE *in)
{
  char buffer[MAX_BUFFER_SIZE];
  int i = 0;
  char c;

  while ((c = getc(in)) != '"' &&
         i < MAX_BUFFER_SIZE - 1)
  {
    buffer[i++] = c;
  }

  buffer[i] = '\0';
  return make_string(buffer);
}

object *read_symbol(FILE *in, object *env)
{
  char buffer[MAX_BUFFER_SIZE];
  int i = 0;
  char c;

  while (!is_whitespace(c = getc(in)) &&
         isalpha(c) &&
         i < MAX_BUFFER_SIZE - 1)
  {
    buffer[i++] = c;
  }

  buffer[i] = '\0';
  ungetc(c, in);

  object *obj = lookup_symbol(buffer, env);

  if (obj == NULL)
  {
    obj = intern_symbol(buffer, car(env));
  }

  return obj;
}

object *read_number(FILE *in)
{
  int number = 0;
  char c;

  while (isdigit(c = getc(in)))
  {
    number *= 10;
    number += (int)c - 48;
  }

  ungetc(c, in);

  return make_fixnum(number);
}

object *read_lisp(FILE *, object *);
void print(object *);

object *read_list(FILE *in, object *env)
{
  char c;
  object *car, *cdr;

  car = cdr = make_cell();
  car->cell.car = read_lisp(in, env);

  while ((c = getc(in)) != ')')
  {
    if (c == '.')
    {
      getc(in);
      cdr->cell.cdr = read_lisp(in, env);
    }
    else if (!is_whitespace(c))
    {
      ungetc(c, in);

      cdr->cell.cdr = make_cell();
      cdr = cdr->cell.cdr;
      cdr->cell.car = read_lisp(in, env);
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
object *read_lisp(FILE *in, object *env)
{
  object *obj = NULL;
  char c;

  skip_whitespace(in);
  c = getc(in);

  if (c == '\'')
  {
    obj = cons(quote, read_lisp(in, env));
  }
  else if (c == '(')
  {
    obj = read_list(in, env);
  }
  else if (c == '"')
  {
    obj = read_string(in);
  }
  else if (isdigit(c))
  {
    ungetc(c, in);
    obj = read_number(in);
  }
  else if (isalpha(c))
  {
    ungetc(c, in);
    obj = read_symbol(in, env);
  }
  else if (c == ')')
  {
    ungetc(c, in);
    //obj = nil;
  }

  return obj;
}

object *eval_symbol(object *obj, object *env)
{
  if (obj == NULL)
    return obj;

  if (obj->symbol.value)
  {
    obj = obj->symbol.value;
  }
  else
  {
    printf("Undefined symbol ");
  }

  return obj;
}

object *eval(object *obj, object *env);

object *eval_list(object *obj, object *env)
{
  if (obj == NULL)
    return obj;

  if (car(obj) == define)
  {
    object *cell = obj;
    object *cell_define = car(cell); /* should be symbol named define */
    assert(cell_define == define);

    cell = cdr(cell);
    object *cell_symbol = car(cell);

    char *symbol_name = cell_symbol->symbol.name;

    return lookup_symbol(symbol_name, env);
  }
  else if (car(obj) == setq)
  {
    object *cell = obj;
    object *cell_setq = car(cell); /* should be symbol named setq */
    assert(cell_setq == setq);

    cell = cdr(cell);
    object *cell_symbol = car(cell);

    char *symbol_name = cell_symbol->symbol.name;
    obj = lookup_symbol(symbol_name, env);

    cell = cdr(cell);
    object *cell_value = car(cell);
    obj->symbol.value = eval(cell_value, env);

    return obj;
  }
  else if (car(obj) == quote)
  {
    return cdr(obj);
  }
  else
  {
    printf("Unknown function ");
    switch (car(obj)->type)
    {
      case SYMBOL:
        printf("%s\n", car(obj)->symbol.name);
        break;
      default:
        print(car(obj));
        printf("\n");
        break;
    }
  }

  // NOT REACHED (eventually, once we implement apply)
  return NULL;
}

object *eval(object *obj, object *env)
{
  object *result = NULL;

  if (obj == NULL)
    return obj;

  switch (obj->type)
  {
    case STRING:
    case FIXNUM:
      result = obj;
      break;
    case SYMBOL:
      result = eval_symbol(obj, env);
      break;
    case CELL:
      result = eval_list(obj, env);
      break;
    default:
      printf("\nUnknown object: %d\n", obj->type);
      break;
  }

  return result;
}

void print_string(object *obj)
{
  //printf("print string \n");
  char *str = obj->str.text;
  int len = strlen(str);
  int i = 0;
  //printf("got here 6\n");

  putchar('"');
  while (i < len)
  {
    putc(*str++, stdout);
    i++;
  }

  putchar('"');
  //putchar(' ');
}

void print_fixnum(object *obj)
{
  //printf("Fixnum\n");
  printf("%d", obj->num.value);
}

void print_cell(object *car)
{
  //printf("Cell:\n");
  //printf("Car %p\n", obj->cell.car);
  //printf("Cdr %p\n", obj->cell.cdr);
  printf("(");
  object *obj = car;

  while (obj)
  {
    if (obj->type == CELL)
    {
      print(obj->cell.car);
    }
    else
    {
      printf(". ");
      print(obj);
      break;
    }

    obj = obj->cell.cdr;

    if (obj)
    {
      printf(" ");
    }
  }
  printf(")");
}

void print(object *obj)
{
  //printf("Print\n");
  //printf("Obj %p\n", obj);
  //printf("Type %d\n", obj->type);

  if (obj != NULL)
  {
    switch (obj->type)
    {
      case CELL:
        print_cell(obj);
        break;
      case STRING:
        print_string(obj);
        break;
      case FIXNUM:
        print_fixnum(obj);
        break;
      case SYMBOL:
        //print(car(obj), env);
        //print(cdr(obj), env);
        //print(cadr(obj), env);
        //printf("%s", obj->symbol.name);
        printf("%s", obj->symbol.name);
        break;
        /* case NIL: */
        /*     printf("NIL"); */
        /*     break; */
      default:
        printf("\nUnknown object: %d\n", obj->type);
        break;
    }
  }
  else
  {
    //printf("NULL ");
  }
}

int main(int argc, char* argv[])
{
  /*
  globals = make_cell();
  nil = intern_symbol("nil", globals);
  */
  /*
  nil = make_symbol("nil");
  globals = cons(nil, NULL);
  */
  /*
  object *tmp = make_symbol("");
  globals = cons(tmp, NULL);
  nil = intern_symbol("nil", globals);
  */
  globals = cons(NULL, NULL);
  nil = intern_symbol("nil", globals);
  quote = intern_symbol("quote", globals);
  setq = intern_symbol("setq", globals);
  define = intern_symbol("define", globals);

  printf("Welcome to JCM-LISP. "
         "Use ctrl-c to exit.\n");

  //object *env = cons(globals, NULL);
  object *env = globals;
  object *result = NULL;

  while (1)
  {
    printf("> ");

    result = read_lisp(stdin, env);
    result = eval(result, env);
    print(result);
/*
    result = read_lisp(stdin, globals);
    result = eval(result, globals);
    print(result, globals);
*/
    printf("\n");
  }

  return 0;
}
