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
    FIXNUM = 1,
    STRING,
    SYMBOL,
    CELL,
} obj_type;

struct object;

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
    struct object *head;
    struct object *tail;
};

struct object
{
    obj_type type;

    union
    {
        struct Fixnum num;
        struct String str;
        struct Cell cell;
        struct Symbol symbol;
    } data;
};

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

object *globals;
object *quote;
object *define;
object *setq;
object *nil;
object *tee;

object *car(object *obj)
{
    if (obj != NULL &&
        obj->type == CELL)
        return obj->data.cell.head;
    else
        return NULL;
}

object *cdr(object *obj)
{
    if (obj->type == CELL)
        return obj->data.cell.tail;
    else
        return NULL;
}

#define caar(obj)    car(car(obj))
#define cadr(obj)    car(cdr(obj))

object *new_object()
{
    object *obj = malloc(sizeof(object));
    obj->data.cell.head = NULL;
    obj->data.cell.tail = NULL;
    return obj;
}

object *make_cell()
{
    object *obj = new_object();
    obj->type = CELL;
    return obj;
}

object *cons(object *head, object *tail)
{
    object *obj = make_cell();
    obj->data.cell.head = head;
    obj->data.cell.tail = tail;
    return obj;
}

object *make_string(char *str)
{
    object *obj = new_object();
    obj->type = STRING;
    obj->data.str.text = strdup(str);
    return obj;
}

object *make_fixnum(int n)
{
    object *obj = new_object();
    obj->type = FIXNUM;
    obj->data.num.value = n;
    return obj;
}

object *make_symbol(char *name)
{
    object *obj = new_object();
    obj->type = SYMBOL;
    obj->data.symbol.name = strdup(name);
    return obj;
}

object *intern_symbol(char *name, object *env)
{
    object *obj = make_symbol(name);
    env->data.cell.tail = cons(obj, env->data.cell.tail);
    return obj;
}

object *lookup_symbol(char *name, object *env)
{
    object *obj = env;
    //object *obj = cdr(env);

    while (obj != NULL)
    {
        //object *cell = car(obj);
        //object *sym = car(cell);
        object *sym = car(obj);

        if (strcmp(sym->data.symbol.name, name) == 0)
        {
            return sym;
        }

        obj = cdr(obj);
    }
    //printf("%s undefined.\n", name);
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

    while ((c = getc(in)) != '"' && i < MAX_BUFFER_SIZE - 1)
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
           i < MAX_BUFFER_SIZE - 1)
    {
        buffer[i++] = c;
    }

    buffer[i] = '\0';

    object *obj = lookup_symbol(buffer, env);

    if (obj == NULL)
    {
        obj = intern_symbol(buffer, env);
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

object *read_all(FILE *, object *);
void print(object *, object *);

object *read_list(FILE *in, object *env)
{
    char c;
    object *head, *tail;

    head = tail = make_cell();
    head->data.cell.head = read_all(in, env);

    while ((c = getc(in)) != ')')
    {
        if (c == '.')
        {
            getc(in);
            tail->data.cell.tail = read_all(in, env);
        }
        else if (!is_whitespace(c))
        {
            ungetc(c, in);

            tail->data.cell.tail = make_cell();
            tail = tail->data.cell.tail;
            tail->data.cell.head = read_all(in, env);
        }

        //skip_whitespace(in);
    };

    return head;
}

// XXX Review these:
// strchr
// strdup
// strcmp
// strspn
// atoi

object *read_all(FILE *in, object *env)
{
    object *obj = NULL;
    char c;

    skip_whitespace(in);
    c = getc(in);

    if (c == '\'')
    {
        obj = cons(quote, read_all(in, env));
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
        obj = nil;
        //printf("Error at %c\n", c);
    }

    return obj;
}

object *eval_symbol(object *obj, object *env)
{
    object *result = NULL;

    if (obj == NULL)
        return obj;

    if (strcmp(obj->data.symbol.name, "define") == 0)
    {
    }
    else if (strcmp(obj->data.symbol.name, "setq") == 0)
    {
    }
    else
    {
        result = lookup_symbol(obj->data.symbol.name, env);
        result = result->data.symbol.value;
    }

    return result;
}

object *eval_list(object *obj, object *env)
{
    object *result = NULL;

    if (obj == NULL)
        return obj;
/*    
    if (!is_symbol(car(obj)))
    {
        printf("CAR is a %d, not a function\n", car(obj)->type);
        return result;
    }
*/
/*
    if (car(obj) == define)
    {

    }
    else */
    if (car(obj) == setq)
    {
        object *cell = obj;
        object *cell_setq = car(cell); /* should be symbol named setq */
        assert(cell_setq == setq);
        cell = cdr(cell);
        object *cell_symbol = car(cell);
        cell = cdr(cell);
        object *cell_value = car(cell);

        char *symbol_name = cell_symbol->data.symbol.name;
        result = lookup_symbol(symbol_name, env);
        result->data.symbol.value = cell_value;
    }
    else if (car(obj) == quote)
    {
        result = cdr(obj);
    }
    else
    {
      result = obj;
      //printf("Unknown function %s\n", car(obj)->data.symbol.name);
    }

    return result;
}

object *eval(object *obj, object *env)
{
    object *result = NULL;

    if (obj == NULL)
        return obj;

    // ???
    // if symbol, eval symbol
    // if not cell, return object
    // if lambda, get body, eval args, eval body
    // ???
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
    char *str = obj->data.str.text;
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
    printf("%d", obj->data.num.value);
}

void print(object *obj, object *env);

void print_cell(object *head, object *env)
{
    //printf("Cell:\n");
    //printf("Head %p\n", obj->data.cell.head);
    //printf("Tail %p\n", obj->data.cell.tail);
    printf("(");
    object *obj = head;

    for (;;)
    {
        if (obj == NULL ||
            obj->data.cell.head == nil)
        {
            //putchar('\b');
            printf(")");
            break;
        }

        if (obj->type == CELL)
        {
            print(obj->data.cell.head, env);
        }
        else
        {
            printf(". ");
            print(obj, env);
        }

        obj = obj->data.cell.tail;
        if (obj != NULL)
            printf(" ");
    }
    // XXX Fix this by reordering the for loop
    // to write the car,
    // then output a ) if cdr is nil
    // and bump the pointer if not
    // and if the new obj is not a cons output a dot
    // ... etc
//    putchar('\b');
//    printf(") ");
}

void print(object *obj, object *env)
{
    //printf("Print\n");
    //printf("Obj %p\n", obj);
    //printf("Type %d\n", obj->type);

    if (obj != NULL)
    {
        switch (obj->type)
        {
            case CELL:
                print_cell(obj, env);
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
                //printf("%s", obj->data.symbol.name);
                printf("%s ", lookup_symbol(obj->data.symbol.name, env)->data.symbol.name);
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

int main (int argc, char* argv[])
{
    //tmp = make_symbol("");
    nil = make_symbol("nil");
    //globals = cons(cons(nil, NULL), nil);
    //globals = cons(nil, cons(NULL, NULL));
    //globals = cons(nil, cons(nil, NULL));
    globals = cons(nil, NULL);

    quote = intern_symbol("quote", globals);
    setq = intern_symbol("setq", globals);
    //define = intern_symbol("define", globals);

    printf("Welcome to JCM-LISP. "
           "Use ctrl-c to exit.\n");

    while (1)
    {
        printf("> ");
        print(eval(read_all(stdin, globals), globals), globals);
        printf("\n");
    }

    return 0;
}
