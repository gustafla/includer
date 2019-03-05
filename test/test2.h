#ifndef TEST2_H
#define TEST2_H

#ifdef DEBUG
char *jokuglobaali;
#ifndef RELEASE
char *makesnosense;
#endif
#endif

#include <stdio.h>
#include "test1.h"

void test2() {
	printf("Hello world\n");
}

#endif
