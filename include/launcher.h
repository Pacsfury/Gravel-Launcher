#pragma once
#include <stdio.h>
#include <stdlib.h>

void _launcherInit();

void _launcherFree();

void addPackage(char* name, char* path);

char* getPackagePath(char* name);