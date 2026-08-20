#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/vector.h"

Vector packages;
Vector names;

static void normalize_name(char* dest, size_t dest_size, const char* value) {
    if (dest_size == 0)
        return;

    while (*value == ' ' || *value == '\t' || *value == '\r' || *value == '\n' || *value == '"' || *value == '\'')
        value++;

    size_t length = 0;
    while (*value && *value != ' ' && *value != '\t' && *value != '\r' && *value != '\n' && *value != '"' &&
           *value != '\'' && length + 1 < dest_size) {
        dest[length++] = *value++;
    }
    dest[length] = '\0';
}

static char* duplicate_string(const char* value) {
    if (!value)
        return NULL;
    size_t len = strlen(value) + 1;
    char* copy = (char*)malloc(len);
    if (!copy)
        return NULL;
    memcpy(copy, value, len);
    return copy;
}

void _launcherInit() {
    vec_free(&packages);
    vec_init(&packages);
    vec_free(&names);
    vec_init(&names);
}

void _launcherFree() {
    for (int i = 0; i < names.size; i++) {
        free(names.data[i]);
    }
    for (int i = 0; i < packages.size; i++) {
        free(packages.data[i]);
    }
    vec_free(&packages);
    vec_free(&names);
}

void addPackage(char* name, char* path) {
    if (!name || !path)
        return;

    char normalized_name[64];
    normalize_name(normalized_name, sizeof(normalized_name), name);
    if (normalized_name[0] == '\0')
        return;

    char* saved_name = duplicate_string(normalized_name);
    char* saved_path = duplicate_string(path);
    if (!saved_name || !saved_path) {
        free(saved_name);
        free(saved_path);
        return;
    }

    vec_push(&names, saved_name);
    vec_push(&packages, saved_path);
}

char* getPackagePath(char* name) {
    char normalized_name[64];
    normalize_name(normalized_name, sizeof(normalized_name), name);
    int index = vec_where(&names, normalized_name);
    if (index < 0)
        return NULL;
    return vec_get(&packages, index);
}