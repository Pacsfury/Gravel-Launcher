#pragma once
#include <stdio.h>
#include <stdlib.h>

void _launcherInit();

void _launcherFree();

void addPackage(char* name, char* path);

char* getPackagePath(char* name);

char* resolve_module_path(const char* name, char* out_path, size_t out_size);

int has_circular_import(const char* name);

int import_begin(const char* name, const char* alias, const char* file_path);

void import_end(const char* name);