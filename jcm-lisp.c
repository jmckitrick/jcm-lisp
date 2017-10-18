#include <stdio.h>
#include <stdlib.h>

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

object *read(FILE *in) {
}

object *eval(object *exp, object *env) {
}

void print(object *exp, object *env) {
}

int main (int argc, char* argv[])
{
    printf("Welcome to JCM-LISP. "
           "Use ctrl-c to exit.\n");

    while (1) {
        printf("> ");
        print(eval(read(stdin)));
        printf("\n");
    }

    return 0;
}
