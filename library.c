#include <string.h>
#include <stdio.h>
#include "common.h"
#include "library.h"

/*
	these libraries are supplied in the NDK
*/
static char *libs[] = {
	"c",
	"m",
	"dl",
	"jnigraphics",
	"log",
	"stdc++",
	"thread_db",
	"z",
	NULL
};

enum library_type library_scope(char *name)
{
	int i;

	for (i = 0; libs[i] != NULL; i++) {
		if (strcmp(name, libs[i]) == 0)
			return LIBRARY_NDK;
	}
	return LIBRARY_EXTERNAL;
}

