#ifndef __COMMON_H__
#define __COMMON_H__

enum module_type {
	MODULE_SHARED_LIBRARY,
	MODULE_STATIC_LIBRARY,
	MODULE_EXECUTABLE,
};

enum library_type {
	LIBRARY_NDK,
	LIBRARY_UNSUPPORTED,
	LIBRARY_EXTERNAL,
	LIBRARY_FLAG
};

enum script_type {
	SCRIPT_TOP,
	SCRIPT_SUBDIRECTORY,
};

enum build_type {
	BUILD_NDK,
	BUILD_EXTERNAL
};

enum tags {
	TAG_NONE = 0,
	TAG_USER = 1,
	TAG_ENG = 2,
	TAG_TESTS = 4,
	TAG_OPTIONAL = 8
};

struct generator {

};

struct source {
	char *name;
	struct generator *gen;
};

struct cflag {
	char *flag;
};

struct library {
	char *name;
	enum library_type ltype;
};

struct subdir {
	char *name;
};

struct module {
	char *name; //local_module;
	enum module_type mtype;

	struct source *source;
	int sources;
	struct cflag *cflag;
	int cflags;
	char *ldflags;
	struct library *library;
	int libraries;
	int tags;
};

struct project {
	char *name;
	struct module *module;
	int modules;
	struct subdir *subdir;
	int subdirs;
	enum build_type btype;
	enum script_type stype;
};

#endif /*__COMMON_H__*/
