#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/vector.h"

void vec_init(Vector *v) {
    v->size = 0;
    v->capacity = 4;
    v->data = (char **)malloc(v->capacity * sizeof(char *));
}

void vec_push(Vector *v, char *value) {
    if (v->size == v->capacity) {
        v->capacity *= 2;
        v->data = (char **)realloc(v->data, v->capacity * sizeof(char *)); 
    }
    v->data[v->size++] = value;
}

char *vec_get(Vector *v, int index) {
    if (index >= 0 && index < v->size) {
        return v->data[index];
    }
    return NULL;
}

void vec_replace(Vector *v, int index, char *value) { 
    if (index >= 0 && index < v->size) {
        free(v->data[index]);
        v->data[index] = value;
    }
}

void vec_free(Vector *v) {
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;
}

int vec_where(Vector *v, char *value) {
    if (v == NULL || value == NULL) {
        return -1;
    }
    
    for (int i = 0; i < v->size; i++) {
        if (strcmp(v->data[i], value) == 0) {
            return i;
        }
    }
    return -1;
}