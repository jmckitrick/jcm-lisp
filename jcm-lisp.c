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
    //NIL
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

object *car(object *obj) {
    if (obj != NULL &&
        obj->type == CELL)
        return obj->data.cell.head;
    else
        return NULL;
}

object *cdr(object *obj) {
    if (obj->type == CELL)
        return obj->data.cell.tail;
    else
        return NULL;
}

#define caar(obj)    car(car(obj))
#define cadr(obj)    car(cdr(obj))

object *new_object() {
    object *obj = malloc(sizeof(object));
    obj->data.cell.head = NULL;
    obj->data.cell.tail = NULL;
    return obj;
}

object *make_cell() {
    object *obj = new_object();
    obj->type = CELL;
    //printf("make cell\n");
    return obj;
}

object *cons(object *head, object *tail) {
    object *obj = make_cell();
    obj->data.cell.head = head;
    obj->data.cell.tail = tail;
    return obj;
}

object *make_string(char *str) {
    object *obj = new_object();
    obj->type = STRING;
    obj->data.str.text = strdup(str);//malloc(strlen(str));
    //strncpy(obj->data.str.text, str, strlen(str));
    //printf("make string\n");
    return obj;
}

object *make_fixnum(int n) {
    object *obj = new_object();
    obj->type = FIXNUM;
    obj->data.num.value = n;//malloc(sizeof(int));
    //printf("make fixnum %d\n", n);
    return obj;
}

object *make_symbol(char *name) {
    object *obj = new_object();
    obj = new_object();
    obj->type = SYMBOL;
    obj->data.symbol.name = malloc(strlen(name));
    strncpy(obj->data.symbol.name, name, strlen(name));
    obj->data.symbol.value = make_string("baadf00d");
    return obj;
}

object *intern_symbol(char *name, object *env) {
    object *obj = make_symbol(name);
    object *tmp = cons(obj, env->data.cell.tail);
    env->data.cell.tail = tmp;
    //printf("make symbol %s\n", name);
    return obj;
}

object *lookup_symbol(char *name, object *env) {
    object *obj = cdr(env);

    while (obj != NULL) {
        if (strcmp(car(obj)->data.symbol.name, name) == 0) {
            return car(obj);
        }

        //printf("%s undefined.\n", name);
        obj = cdr(obj);
    }
    return intern_symbol(name, env);
}
/*
object *make_nil() {
    object *obj = new_object();
    obj->type = NIL;
    //printf("make NIL\n");
    return obj;
}
*/
int is_whitespace(char c) {
    if (isspace(c)) {
        return 1;
    } else {
        return 0;
    }
}

void skip_whitespace(FILE *in) {
    char c;
    int done = 0;

    while (!done) {
        c = getc(in);
        if (c == '\n' && c == '\r') {
            done = 1;
        }

        if (!is_whitespace(c)) {
            ungetc(c, in);
            done = 1;
        }
    }
    //printf("Got %c %d\n", c, c);
    //printf("No whitespace\n");
}

object *read_string(FILE *in) {
    char buffer[MAX_BUFFER_SIZE];
    int i = 0;
    char c = getc(in);          /* Should be '"' */

    //printf("read string\n");
    while ((c = getc(in)) != '"' && i < MAX_BUFFER_SIZE - 1) {
        buffer[i++] = c;
    }

    //printf("got here 1 %d\n", i);
    buffer[i] = '\0';
    return make_string(buffer);
}

object *read_symbol(FILE *in, object *env) {
    char buffer[MAX_BUFFER_SIZE];
    int i = 0;
    char c;

    //printf("read symbol\n");
    while (!is_whitespace(c = getc(in)) &&
           i < MAX_BUFFER_SIZE - 1) {
        buffer[i++] = c;
    }

    //printf("Bui 1 %d\n", i);
    buffer[i] = '\0';
    return lookup_symbol(buffer, env);
}

object *read_number(FILE *in) {
    int number = 0;
    char c;

    //printf("read number\n");
    while (isdigit(c = getc(in))) {
        number *= 10;
        number += (int)c - 48;
    }

    ungetc(c, in);

    return make_fixnum(number);
}

object *read_all(FILE *, object *);
void print(object *, object *);

object *read_list(FILE *in, object *env) {
    char c = getc(in);          /* Should be '(' */

    //printf("read list\n");
    object *head, *tail;//, *obj;
    head = tail = make_cell();
    //printf("Head\n");
    //print(head, NULL);

    head->data.cell.head = read_all(in, env);

    while ((c = getc(in)) != ')') {
        //printf("Read tail\n");
        ungetc(c, in);
        tail->data.cell.tail = make_cell();
        tail = tail->data.cell.tail;
        tail->data.cell.head = read_all(in, env);
        skip_whitespace(in);
    };

    //printf("Tail\n");
    //print(tail, NULL);
    //printf("populating cell\n");

    //tail->data.cell.head = ;
    //printf("Created list\n");
    //print(head, NULL);
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
    char c;
    object *obj = NULL;

    skip_whitespace(in);

    c = getc(in);
    ungetc(c, in);

    //printf("Read found %c\n", c);
    
    //printf("check char %c\n", c);
    if (c == '\'')
    {
        getc(in);
        obj = cons(quote, read_all(in, env));
    }
    else if (c == '(')
    {
        obj = read_list(in, env);
    }
    else if ((int)c == '"')
    {
        obj = read_string(in);
    }
    else if (isdigit((int)c))
    {
        obj = read_number(in);
    }
    else if (isalpha((int)c))
    {
        obj = read_symbol(in, env);
    }
    else
    {
        printf("Error at %c\n", c);
    }

    //sleep(1);
    return obj;
}

object *eval_symbol(object *obj, object *env)
{
    object *result = NULL;

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

    if (!is_symbol(car(obj)))
    {
        printf("CAR is a %d, not a function\n", car(obj)->type);
        return result;
    }
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
    else if (strcmp(car(obj)->data.symbol.name, "quote") == 0)
    {
        result = cdr(obj);
    }
    else
    {
        printf("Unknown function %s\n", car(obj)->data.symbol.name);
    }

    return result;
}

object *eval(object *obj, object *env)
{
    object *result = NULL;

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

void print_string(object *obj) {
    //printf("print string \n");
    char *str = obj->data.str.text;
    int len = strlen(str);
    int i = 0;
    //printf("got here 6\n");

    putchar('"');
    while (i < len) {
        putc(*str++, stdout);
        i++;
    }
    putchar('"');
    putchar(' ');
}

void print_fixnum(object *obj) {
    //printf("Fixnum\n");
    printf("%d ", obj->data.num.value);
}

void print(object *obj, object *env);

void print_cell(object *head, object *env) {
    //printf("Cell:\n");
    //printf("Head %p\n", obj->data.cell.head);
    //printf("Tail %p\n", obj->data.cell.tail);
    printf("(");
    object *obj = head;

    for (;;) {
        if (obj->data.cell.head != NULL //&&
            //obj->data.cell.head->type != NIL &&
            //car(obj) != quote
            )
        {
            //printf("Head type %d\n", obj->data.cell.head->type);
            //printf("Head value:\n");
            print(car(obj), env);
            //printf(" ");
        } else {
            //printf("Found head\n");
            //break;
        }

        if (obj->data.cell.tail != NULL)
        {
            //printf("\nTail type %d\n", obj->data.cell.tail->type);
            //printf("Tail value:\n");
            //printf(" ");
            //print(obj->data.cell.tail, env);
        } else {
            //printf("Found tail\n");
            break;
        }
        obj = obj->data.cell.tail;
        //sleep(1);
    }
    putchar('\b');
    printf(") ");
}

void print(object *obj, object *env) {
    //printf("Print\n");
    //printf("Obj %p\n", obj);
    //printf("Type %d\n", obj->type);

    if (obj != NULL) {
        switch (obj->type) {
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
    } else {
        printf("NULL ");
    }
}

int main (int argc, char* argv[])
{
    globals = make_cell();
    globals->data.cell.head = make_symbol("");
    quote = intern_symbol("quote", globals);
    nil = intern_symbol("nil", globals);
    setq = intern_symbol("setq", globals);
    define = intern_symbol("define", globals);
    
    printf("Welcome to JCM-LISP. "
           "Use ctrl-c to exit.\n");

    //int i = 0;
    while (1) {
        printf("> ");
        print(eval(read_all(stdin, globals), globals), globals);
        printf("\n");
    }

    return 0;
}
