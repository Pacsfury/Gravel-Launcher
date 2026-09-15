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

#include <unistd.h>

typedef struct {
    char module_name[64];
    char alias[64];
    char file_path[256];
    int is_loading;
    int is_loaded;
} ImportRecord;

#define MAX_IMPORTS 128
static ImportRecord import_registry[MAX_IMPORTS];
static int import_count = 0;

void _launcherInit() {
    vec_free(&packages);
    vec_init(&packages);
    vec_free(&names);
    vec_init(&names);
    import_count = 0;
    memset(import_registry, 0, sizeof(import_registry));
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
    import_count = 0;
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

int has_circular_import(const char* name) {
    char norm[64];
    normalize_name(norm, sizeof(norm), name);
    for (int i = 0; i < import_count; i++) {
        if (strcmp(import_registry[i].module_name, norm) == 0 && import_registry[i].is_loading) {
            return 1;
        }
    }
    return 0;
}

int import_begin(const char* name, const char* alias, const char* file_path) {
    char norm[64];
    normalize_name(norm, sizeof(norm), name);
    if (has_circular_import(norm)) {
        return -1;
    }
    for (int i = 0; i < import_count; i++) {
        if (strcmp(import_registry[i].module_name, norm) == 0) {
            import_registry[i].is_loading = 1;
            if (alias) {
                normalize_name(import_registry[i].alias, sizeof(import_registry[i].alias), alias);
            }
            return i;
        }
    }
    if (import_count < MAX_IMPORTS) {
        strncpy(import_registry[import_count].module_name, norm, sizeof(import_registry[0].module_name) - 1);
        if (alias) {
            normalize_name(import_registry[import_count].alias, sizeof(import_registry[0].alias), alias);
        } else {
            import_registry[import_count].alias[0] = '\0';
        }
        if (file_path) {
            strncpy(import_registry[import_count].file_path, file_path, sizeof(import_registry[0].file_path) - 1);
        }
        import_registry[import_count].is_loading = 1;
        import_registry[import_count].is_loaded = 0;
        return import_count++;
    }
    return 0;
}

void import_end(const char* name) {
    char norm[64];
    normalize_name(norm, sizeof(norm), name);
    for (int i = 0; i < import_count; i++) {
        if (strcmp(import_registry[i].module_name, norm) == 0) {
            import_registry[i].is_loading = 0;
            import_registry[i].is_loaded = 1;
            break;
        }
    }
}

char* resolve_module_path(const char* name, char* out_path, size_t out_size) {
    char norm[64];
    normalize_name(norm, sizeof(norm), name);
    if (norm[0] == '\0')
        return NULL;

    // 1. Check existing registered package path
    char* reg_path = getPackagePath(norm);
    if (reg_path && access(reg_path, F_OK) == 0) {
        strncpy(out_path, reg_path, out_size - 1);
        out_path[out_size - 1] = '\0';
        return out_path;
    }

    char candidate[512];

    // 2. libs/<name>.grv
    snprintf(candidate, sizeof(candidate), "libs/%s.grv", norm);
    if (access(candidate, F_OK) == 0) {
        strncpy(out_path, candidate, out_size - 1);
        out_path[out_size - 1] = '\0';
        addPackage(norm, out_path);
        return out_path;
    }

    // 3. libs/<name>/main.grv
    snprintf(candidate, sizeof(candidate), "libs/%s/main.grv", norm);
    if (access(candidate, F_OK) == 0) {
        strncpy(out_path, candidate, out_size - 1);
        out_path[out_size - 1] = '\0';
        addPackage(norm, out_path);
        return out_path;
    }

    // 4. <name>.grv
    snprintf(candidate, sizeof(candidate), "%s.grv", norm);
    if (access(candidate, F_OK) == 0) {
        strncpy(out_path, candidate, out_size - 1);
        out_path[out_size - 1] = '\0';
        addPackage(norm, out_path);
        return out_path;
    }

    // 5. <name> directly
    if (access(norm, F_OK) == 0) {
        strncpy(out_path, norm, out_size - 1);
        out_path[out_size - 1] = '\0';
        addPackage(norm, out_path);
        return out_path;
    }

    return NULL;
}