#include "emit.h"
#include "options.h"
#include "common.h"

int main(int argc, char **argv)
{
	struct project *p;
	int err;

	p = options_parse(argc, argv);
	if (p)
		err = emit_file(p);
	return 0;
}
