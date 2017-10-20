#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 100

typedef enum {
    FIXNUM = 1,
    STRING,
    CELL,
    NIL
} obj_type;

struct Fixnum {
    int value;
};

struct String {
    char *text;
};

struct object;

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
    } data;
};

typedef struct object object;

object *new_object() {
    object *obj = malloc(sizeof(object));
    obj->data.cell.head = NULL;
    obj->data.cell.tail = NULL;
    return obj;
}

object *make_string(char *str) {
    object *obj = new_object();
    obj->type = STRING;
    obj->data.str.text = malloc(strlen(str));
    strncpy(obj->data.str.text, str, strlen(str));
    printf("make string\n");
    return obj;
}

object *make_fixnum(int n) {
    object *obj = new_object();
    obj->type = FIXNUM;
    obj->data.num.value = n;//malloc(sizeof(int));
    printf("make fixnum\n");
    return obj;
}

object *make_cell() {
    object *obj = new_object();
    obj->type = CELL;
    printf("make cell\n");
    return obj;
}

object *make_nil() {
    object *obj = new_object();
    obj->type = NIL;
    printf("make NIL\n");
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

    while ((c = getc(in)) != '\n') {
        if (is_whitespace(c))
            continue;

        break;
    }

    ungetc(c, in);
}

object *read_string(FILE *in) {
    char buffer[MAX_BUFFER_SIZE];
    char c;
    int i = 0;

    printf("read string\n");
    while ((c = getc(in)) != '\n' && i < MAX_BUFFER_SIZE - 1) {
        buffer[i++] = c;
    }

    //printf("got here 1 %d\n", i);
    buffer[i] = '\0';
    return make_string(buffer);
}

object *read_number(FILE *in) {
    int number = 0;
    char c;

    printf("read number\n");
    while (isdigit(c = getc(in))) {
        number *= 10;
        number += (int)c - 48;
    }

    ungetc(c, in);

    return make_fixnum(number);
}

object *read2(FILE *in);
void print(object *, object *);

object *read_list(FILE *in) {
    char c = getc(in);          /* Should be '(' */

    printf("read list\n");
    object *list = make_cell();
    object *obj = read2(in);
    //printf("Head\n");
    //print(list, NULL);

    list->data.cell.head = obj;
    object *tail = make_cell();
    list->data.cell.tail = tail;

    do {
        printf("Read tail\n");
        //ungetc(c, in);
        object *tail = make_cell();
        tail->data.cell.head = read2(in);
        obj->data.cell.tail = tail;
        skip_whitespace(in);
    } while ((c = getc(in)) != ')');

    //printf("Tail\n");
    //print(tail, NULL);
    //printf("populating cell\n");
    
    return list;
}

object *read2(FILE *in) {
    char c;
    object *obj = NULL;

    skip_whitespace(in);

    c = getc(in);
    ungetc(c, in);

    printf("Read found %c\n", c);
    
    //printf("check char %c\n", c);
    if (isdigit((int)c)) {
        obj = read_number(in);
    } else if (isalpha((int)c)) {
        obj = read_string(in);
    } else if (c == '(') {
        obj = read_list(in);
    } else if (c == ')') {
        obj = make_nil();
    }

    //sleep(1);
    return obj;
}

object *eval(object *obj, object *env) {
    printf("eval\n");
    return obj;
}

void print_string(object *obj) {
    //printf("print stirng \n");
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
    //printf("got here 4\n");
    printf("%d", obj->data.num.value);
}

void print(object *obj, object *env);

void print_cell(object *obj, object *env) {
    //printf("Cell:\n");
    printf("(");
    if (obj->data.cell.head->type != NIL) {
        //printf("Head:\n");
        print(obj->data.cell.head, env);
    }
    if (obj->data.cell.tail->type != NIL) {
        //printf("Tail:\n");
        printf(" ");
        print(obj->data.cell.tail, env);
    }
    printf(") ");
}

void print(object *obj, object *env) {
    //printf("print\n");
    //printf("Obj %p\n", obj);
    //printf("Type %d\n", obj->type);

    switch (obj->type) {
        case STRING:
            print_string(obj);
            break;
        case FIXNUM:
            print_fixnum(obj);
            break;
        case CELL:
            print_cell(obj, env);
            break;
        case NIL:
            printf("NIL");
            break;
        default:
            printf("Unknown object\n");
            break;
    }
}

int main (int argc, char* argv[])
{
    printf("Welcome to JCM-LISP. "
           "Use ctrl-c to exit.\n");

//    while (1) {
        printf("> ");
        print(eval(read2(stdin), NULL), NULL);
        printf("\n");
//    }

    return 0;
}
