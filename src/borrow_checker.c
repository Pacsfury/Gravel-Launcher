#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VARIABLES 10000

typedef struct {
    char name[256];
    bool is_mutable;
    int borrow_count;
    bool is_moved;
    int declaration_line;
} VariableInfo;

static VariableInfo variables[MAX_VARIABLES];
static int variable_count = 0;

void borrow_checker_init() {
    variable_count = 0;
    printf("[BORROW] Initialized\n");
}

void borrow_declare_variable(const char* name, bool is_mutable, int line) {
    if (variable_count >= MAX_VARIABLES) return;
    strncpy(variables[variable_count].name, name, 255);
    variables[variable_count].is_mutable = is_mutable;
    variables[variable_count].borrow_count = 0;
    variables[variable_count].is_moved = false;
    variables[variable_count].declaration_line = line;
    variable_count++;
    printf("[BORROW] Declared '%s' (%s) at line %d\n", name, is_mutable ? "mut" : "immut", line);
}

bool borrow_check_immutable(const char* name, int line) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].is_moved) {
                fprintf(stderr, "ERROR [E_BORROW_002]: Cannot use '%s' (moved)\n", name);
                return false;
            }
            variables[i].borrow_count++;
            printf("[BORROW] Immutable borrow of '%s' at line %d\n", name, line);
            return true;
        }
    }
    return false;
}

bool borrow_check_mutable(const char* name, int line) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (!variables[i].is_mutable) {
                fprintf(stderr, "ERROR [E_BORROW_001]: Cannot mutably borrow immutable '%s'\n", name);
                return false;
            }
            if (variables[i].borrow_count > 0) {
                fprintf(stderr, "ERROR [E_BORROW_001]: '%s' already borrowed\n", name);
                return false;
            }
            variables[i].borrow_count++;
            printf("[BORROW] Mutable borrow of '%s' at line %d\n", name, line);
            return true;
        }
    }
    return false;
}

void borrow_release(const char* name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0 && variables[i].borrow_count > 0) {
            variables[i].borrow_count--;
            return;
        }
    }
}

bool borrow_move(const char* name, int line) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].borrow_count > 0) {
                fprintf(stderr, "ERROR [E_BORROW_003]: Cannot move '%s' (still borrowed)\n", name);
                return false;
            }
            variables[i].is_moved = true;
            printf("[BORROW] Moved '%s' at line %d\n", name, line);
            return true;
        }
    }
    return false;
}

bool borrow_use(const char* name, int line) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            if (variables[i].is_moved) {
                fprintf(stderr, "ERROR [E_BORROW_002]: Cannot use '%s' (moved)\n", name);
                return false;
            }
            printf("[BORROW] Using '%s' at line %d\n", name, line);
            return true;
        }
    }
    return false;
}

void borrow_print_state() {
    printf("\n[BORROW_STATE]\n");
    for (int i = 0; i < variable_count; i++) {
        printf("  '%s': %s, borrows=%d, moved=%s\n", 
               variables[i].name,
               variables[i].is_mutable ? "mut" : "immut",
               variables[i].borrow_count,
               variables[i].is_moved ? "yes" : "no");
    }
    printf("[END_STATE]\n\n");
}

void borrow_reset() {
    variable_count = 0;
}
