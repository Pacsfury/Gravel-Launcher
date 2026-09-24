#ifndef BORROW_CHECKER_H
#define BORROW_CHECKER_H

#include <stdbool.h>

void borrow_checker_init(void);
void borrow_declare_variable(const char* name, bool is_mutable, int line);
bool borrow_check_immutable(const char* name, int line);
bool borrow_check_mutable(const char* name, int line);
void borrow_release(const char* name);
bool borrow_move(const char* name, int line);
bool borrow_use(const char* name, int line);
void borrow_print_state(void);
void borrow_reset(void);

#endif
