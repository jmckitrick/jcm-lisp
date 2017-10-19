#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 100

typedef enum {
    NUMBER,
    STRING
} obj_type;

struct number {
    long value;
};

struct string {
    char *text;
};

struct object {
    obj_type type;

    union {
        struct number num;
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

object *read(FILE *in) {
    char buffer[MAX_BUFFER_SIZE];
    char c;
    int i = 0;

    skip_whitespace(in);

    while ((c = getc(in)) != '\n' && i < MAX_BUFFER_SIZE - 1) {
        buffer[i++] = c;
    }

    printf("got here 1 %d\n", i);
    buffer[i] = '\0';
    return make_string(buffer);
}

object *eval(object *obj, object *env) {
    printf("got here 3xx\n");
    return obj;
}

void print(object *obj, object *env) {
    printf("got here 3\n");
    char *str = obj->data.str.text;
    int len = strlen(str);
    int i = 0;
    printf("got here 4\n");

    switch (obj->type) {
        case STRING:
            putchar('"');
            while (i < len) {
                putc(*str++, stdout);
                i++;
            }
            putchar('"');
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
