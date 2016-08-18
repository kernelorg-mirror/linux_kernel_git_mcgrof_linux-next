#ifndef __TOOLS_ASM_GENERIC_BUG
#define __TOOLS_ASM_GENERIC_BUG

#include <stdio.h>
#include <stdlib.h>

#define BUG() do {										\
	fprintf(stderr, "----------------------------------------------------------\n");	\
	fprintf (stderr, "BUG on %s at %s: %i\n", __func__, __FILE__, __LINE__);		\
	abort();										\
}												\
while (0)

#define BUG_ON(cond) do { if (cond) BUG(); } while (0)

#define WARN_ON(__test) do {									\
	if (__test) {										\
		fprintf(stderr, "----------------------------------------------------------\n");\
		fprintf (stderr, "WARN_ON on %s at %s: %i\n", __func__, __FILE__, __LINE__);	\
	}											\
}												\
while (0)

#endif /* __TOOLS_ASM_GENERIC_BUG */
