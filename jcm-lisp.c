#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NUMBER
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
    obj->data.str.text = malloc(4);
    strncpy(obj->data.str.text, str, 4);
    return obj;
}

object *read(FILE *in) {
    char str[] = "yes";
    str[3] = '\0';
    return make_string(str);
}

object *eval(object *exp, object *env) {
    return exp;
}

void print(object *exp, object *env) {
    char *str = exp->data.str.text;
    putchar('"');
    while (*str != '\0') {
        putc(*str, stdout);
        str++;
    }
    putchar('"');
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
