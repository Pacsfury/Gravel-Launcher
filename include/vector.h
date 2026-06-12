#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char **data;
    int size;
    int capacity;
} Vector;

void vec_init(Vector *v);

void vec_push(Vector *v, char *value);

char *vec_get(Vector *v, int index);

void vec_replace(Vector *v, int index, char *value);

void vec_free(Vector *v);

int vec_where(Vector *v, char *value);