#include <stdio.h>
#include <stdlib.h>
#include "../include/vector.h"

Vector packages;

void _launcherInit() {
    vec_free(&packages);
    vec_init(&packages);
}

void _launcherFree() {
    vec_free(&packages);
}