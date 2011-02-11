#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "common.h"
#include "library.h"

enum mode {
	MODE_UNDEFINED,
	MODE_PROJECT,
	MODE_MODULE,
	MODE_SOURCES,
	MODE_CFLAGS,
	MODE_LDFLAGS,
	MODE_TAGS,
	MODE_SUBDIR,
	MODE_END
};

static char *add_slashes(char *in)
{
	int newlen = 0;
	char *ptr, *out, *outptr;

	ptr = in;
	while (*ptr) {
		if (*ptr == '"');
			newlen++;
		newlen++; ptr++;
	}
	out = malloc(sizeof(char) * newlen + 1);
	ptr = in;
	outptr = out;
	while (*ptr) {
		if (*ptr == '"') {
			*(outptr++) = '\\';
		}
		*(outptr++) = *(ptr++);
	}
	*outptr = 0;
	return out;
}

static struct project *new_project(char *name, enum script_type stype, enum build_type btype)
{
	struct project *out = calloc(1, sizeof(struct project));
	out->name = strdup(name);
	out->stype = stype;
	out->btype = btype;
	return out;
}

static struct module *new_module(char *name, enum module_type mtype)
{
	struct module *out = calloc(1, sizeof(struct module));
	out->name = strdup(name);
	out->mtype = mtype;
	return out;
}

void add_tag(struct module *m, char *name)
{
	enum tags tag = TAG_NONE;

	if (strcmp("user", name) == 0)
		tag = TAG_USER;

	if (strcmp("eng", name) == 0)
		tag = TAG_ENG;

	if (strcmp("tests", name) == 0)
		tag = TAG_TESTS;

	if (strcmp("optional", name) == 0)
		tag = TAG_OPTIONAL;

	m->tags |= tag;
}

static void add_cflag(struct module *m, char *flag)
{
	if (strcmp("-Werror", flag) == 0)
		return;
	m->cflags++;
	m->cflag = realloc(m->cflag, m->cflags * sizeof(struct cflag));
	m->cflag[m->cflags - 1].flag = strdup(flag);
}

static void add_source(struct module *m, char *name, struct generator *g)
{
	m->sources++;
	m->source = realloc(m->source, m->sources * sizeof(struct source));
	m->source[m->sources - 1].name = strdup(name);
	m->source[m->sources - 1].gen = g;
}

static void add_library(struct module *m, char *name, enum library_type ltype)
{
	m->libraries++;
	m->library = realloc(m->library, m->libraries * sizeof(struct library));
	m->library[m->libraries - 1].name = strdup(name);
	m->library[m->libraries - 1].ltype = ltype;
}

static void add_ldflag(struct module *m, char *flag, enum build_type btype)
{
	enum library_type ltype;
	int len = strlen(flag);

	if (len < 2)  //this is probably a WTF condition...
		return;

	if (flag[0] == '-') {
		if (flag[1] == 'L') //we eat that for BREAKFAST
			return;
		if (flag[1] == 'l') {// actually figure out what libtype...
			ltype = library_scope(flag + 2);
			add_library(m, flag+2, ltype);
			return;
		}
		if (strcmp(flag, "-pthread") == 0) //yum
			return;
		add_library(m, flag, LIBRARY_FLAG);
	}
	//else add an ldflag directly...
}

static void add_module(struct project *p, struct module *m)
{
	p->modules++;
	p->module = realloc(p->module, p->modules * sizeof(struct module));
	p->module[p->modules - 1] = *m;
}

static void add_subdir(struct project *p, char *name)
{
	p->subdirs++;
	p->subdir = realloc(p->subdir, p->subdirs * sizeof(struct subdir));
	p->subdir[p->subdirs - 1].name = strdup(name);
}


static enum mode get_mode(char *arg)
{
	int len = strlen(arg);

	if (len < 3)
		return MODE_UNDEFINED;
	if ((arg[0] != '-') || (arg[1] != ':'))
		return MODE_UNDEFINED;

	if (strcmp("-:PROJECT", arg) == 0)
		return MODE_PROJECT;

	if (strcmp("-:MODULE", arg) == 0)
		return MODE_MODULE;

	if (strcmp("-:SOURCES", arg) == 0)
		return MODE_SOURCES;

	if (strcmp("-:CFLAGS", arg) == 0)
		return MODE_CFLAGS;

	if (strcmp("-:LDFLAGS", arg) == 0)
		return MODE_LDFLAGS;

	if (strcmp("-:TAGS", arg) == 0)
		return MODE_TAGS;

	if (strcmp("-:SUBDIR", arg) == 0)
		return MODE_SUBDIR;

	if (strcmp("-:END", arg) == 0)
		return MODE_END;

	return MODE_UNDEFINED;
}

struct project *options_parse(int argc, char **args)
{
	enum mode mode = MODE_UNDEFINED;
	char *arg;
	int i;
	enum build_type bt;
	struct project *p = NULL;
	struct module *m = NULL;

	if (getenv("ANDROGENIZER_NDK"))
		bt = BUILD_NDK;
	else bt = BUILD_EXTERNAL;
/*
	p = new_project("pname", SCRIPT_TOP);
	m = new_module("foo", MODULE_SHARED_LIBRARY);
	add_tag(m, TAG_OPTIONAL);
	add_source(m, "flah.c", NULL);
	add_source(m, "foop.c", NULL);
	add_source(m, "gnarg.c", NULL);
	add_source(m, "plah.c", NULL);
	add_source(m, "dung.c", NULL);
	add_library(m, "dl", LIBRARY_NDK);
	add_library(m, "media", LIBRARY_UNSUPPORTED);
	add_library(m, "nice", LIBRARY_EXTERNAL);
	add_module(p, m);
*/

	if (argc < 2) {
//print help!
		return NULL;
	}
	for (i = 1; i < argc; i++) {
		enum mode nm;
		arg = add_slashes(args[i]);
		nm = get_mode(arg);
		if (nm == MODE_UNDEFINED) {
			switch (mode) {
			case MODE_UNDEFINED:
				assert(!!!"OH NOES!!!");
				break;
			case MODE_PROJECT:
				p = new_project(arg, SCRIPT_SUBDIRECTORY, bt);
				break;
			case MODE_SUBDIR:
				add_subdir(p, arg);
				break;
			case MODE_MODULE:
				if (m)
					add_module(p, m);
				m = new_module(arg, MODULE_SHARED_LIBRARY);
				break;
			case MODE_SOURCES:
				add_source(m, arg, NULL);
				break;
			case MODE_LDFLAGS:
				add_ldflag(m, arg, p->btype);
				break;
			case MODE_CFLAGS:
				add_cflag(m, arg);
				break;
			case MODE_TAGS:
				add_tag(m, arg);
				break;
			case MODE_END:
				break;
			}
		} else mode = nm;
	}
	if (p && m)
		add_module(p, m);
	return p;
}
