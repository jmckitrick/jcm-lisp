#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 100

typedef enum {
    FIXNUM,
    STRING
} obj_type;

struct fixnum {
    int value;
};

struct string {
    char *text;
};

struct object {
    obj_type type;

    union {
        struct fixnum num;
        struct string str;
    } data;
};

typedef struct object object;

object *new_object() {
    object *obj = malloc(sizeof(object));
    return obj;
}

object *make_string(char *str) {
    object *obj = new_object();
    obj->type = STRING;
    obj->data.str.text = malloc(strlen(str));
    strncpy(obj->data.str.text, str, strlen(str));
    printf("got here 2\n");
    return obj;
}

object *make_fixnum(int n) {
    object *obj = new_object();
    obj->type = FIXNUM;
    obj->data.num.value = n;//malloc(sizeof(int));
    printf("make fixnum\n");
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

    printf("read string");
    while ((c = getc(in)) != '\n' && i < MAX_BUFFER_SIZE - 1) {
        buffer[i++] = c;
    }

    printf("got here 1 %d\n", i);
    buffer[i] = '\0';
    return make_string(buffer);
}

object *read_number(FILE *in) {
    int number = 0;
    char c;

    printf("read number");
    while (isdigit(c = getc(in))) {
        number *= 10;
        number += (int)c - 48;
    }

    ungetc(c, in);

    return make_fixnum(number);
}

object *read(FILE *in) {
    char c;
    object *obj;

    skip_whitespace(in);

    c = getc(in);
    ungetc(c, in);

    printf("check char %c\n", c);
    if (isdigit((int)c)) {
        obj = read_number(in);
    } else {
        obj = read_string(in);
    }

    return obj;
}

object *eval(object *obj, object *env) {
    printf("got here 3xx\n");
    return obj;
}

void print_string(object *obj) {
    printf("got here 5\n");
    char *str = obj->data.str.text;
    int len = strlen(str);
    int i = 0;
    printf("got here 6\n");

    putchar('"');
    while (i < len) {
        putc(*str++, stdout);
        i++;
    }
    putchar('"');
}

void print_fixnum(object *obj) {
    printf("got here 4\n");
    printf("%d", obj->data.num.value);
}

void print(object *obj, object *env) {
    printf("got here 3\n");

    switch (obj->type) {
        case STRING:
            print_string(obj);
            break;
        case FIXNUM:
            print_fixnum(obj);
            break;
        default:
            break;
    }
}

int main (int argc, char* argv[])
{
    printf("Welcome to JCM-LISP. "
           "Use ctrl-c to exit.\n");

//    while (1) {
        printf("> ");
        print(eval(read(stdin), NULL), NULL);
        printf("\n");
//    }

    return 0;
}
