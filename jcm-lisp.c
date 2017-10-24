#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 100

typedef enum {
    FIXNUM = 1,
    STRING,
    SYMBOL,
    CELL,
    NIL
} obj_type;

struct object;

typedef struct object object;

struct Fixnum {
    int value;
};

struct String {
    char *text;
};

struct Symbol {
    char *name;
    object *value;
};

struct cell {
    struct object *head;
    struct object *tail;
};

struct object {
    obj_type type;

    union {
        struct Fixnum num;
        struct String str;
        struct cell cell;
        struct Symbol symbol;
    } data;
};

object *globals;
object *quote;

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
    obj->data.str.text = malloc(strlen(str));
    strncpy(obj->data.str.text, str, strlen(str));
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

object *intern_symbol(char *name) {
    object *obj = car(globals);

    while (obj != NULL) {
        if (strcmp(obj->data.symbol.name, name) == 0) {
            return obj;
        }

        printf("%s undefined.\n", name);
        obj = cdr(obj);
    }

    obj = new_object();
    obj->type = SYMBOL;
    obj->data.symbol.name = malloc(strlen(name));
    strncpy(obj->data.symbol.name, name, strlen(name));
    globals = cons(obj, globals);
    //printf("make symbol\n");
    return obj;
}

object *make_nil() {
    object *obj = new_object();
    obj->type = NIL;
    //printf("make NIL\n");
    return obj;
}

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
            done = 1;
            ungetc(c, in);
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

object *read_symbol(FILE *in) {
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
    return intern_symbol(buffer);
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

object *read_all(FILE *in);
void print(object *, object *);

object *read_list(FILE *in) {
    char c = getc(in);          /* Should be '(' */

    //printf("read list\n");
    object *head, *tail;//, *obj;
    head = tail = make_cell();
    //printf("Head\n");
    //print(head, NULL);

    head->data.cell.head = read_all(in);

    while ((c = getc(in)) != ')') {
        //printf("Read tail\n");
        ungetc(c, in);
        tail->data.cell.tail = make_cell();
        tail = tail->data.cell.tail;
        tail->data.cell.head = read_all(in);
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

object *read_all(FILE *in) {
    char c;
    object *obj = NULL;

    skip_whitespace(in);

    c = getc(in);
    ungetc(c, in);

    //printf("Read found %c\n", c);
    
    //printf("check char %c\n", c);
    if (isdigit((int)c)) {
        obj = read_number(in);
    } else if ((int)c == '"') {
        obj = read_string(in);
    } else if (isalpha((int)c)) {
        obj = read_symbol(in);
    } else if (c == '(') {
        obj = read_list(in);
    } else if (c == ')') {
        obj = make_nil();
    } else if (c == '\'') {
        getc(in);
        obj = cons(quote, read_all(in));
    } else {
        getc(in);               /* Not used, so discard. */
        obj = make_nil();
    }

    //sleep(1);
    return obj;
}

object *eval(object *obj, object *env) {
    //printf("Eval\n");
    return obj;
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
}

void print_fixnum(object *obj) {
    //printf("Fixnum\n");
    printf("%d", obj->data.num.value);
}

void print(object *obj, object *env);

void print_cell(object *head, object *env) {
    //printf("Cell:\n");
    //printf("Head %p\n", obj->data.cell.head);
    //printf("Tail %p\n", obj->data.cell.tail);
    printf("(");
    object *obj = head;

    for (;;) {
        if (obj->data.cell.head != NULL &&
            obj->data.cell.head->type != NIL) {
            //printf("Head type %d\n", obj->data.cell.head->type);
            //printf("Head value:\n");
            print(obj->data.cell.head, env);
        } else {
            //printf("Found head\n");
            //break;
        }

        if (obj->data.cell.tail != NULL &&
            obj->data.cell.tail->type != NIL) {
            //printf("\nTail type %d\n", obj->data.cell.tail->type);
            //printf("Tail value:\n");
            printf(" ");
            //print(obj->data.cell.tail, env);
        } else {
            //printf("Found tail\n");
            break;
        }
        obj = obj->data.cell.tail;
        //sleep(1);
    }
    printf(") ");
}

void print(object *obj, object *env) {
    //printf("Print\n");
    //printf("Obj %p\n", obj);
    //printf("Type %d\n", obj->type);

    if (obj != NULL) {
        switch (obj->type) {
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
                printf("%s", intern_symbol(obj->data.symbol.name)->data.symbol.name);
                break;
            case CELL:
                print_cell(obj, env);
                break;
            case NIL:
                printf("NIL");
                break;
            default:
                printf("Unknown object: %d\n", obj->type);
                break;
        }
    } else {
        printf("NULL");
    }
}

int main (int argc, char* argv[])
{
    globals = NULL;
    quote = intern_symbol("quote");
    
    printf("Welcome to JCM-LISP. "
           "Use ctrl-c to exit.\n");

    //int i = 0;
    while (1) {
        printf("> ");
        print(eval(read_all(stdin), NULL), NULL);
        printf("\n");
    }

    return 0;
}
