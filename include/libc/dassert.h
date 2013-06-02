#ifndef _DASSERT_H_
#define _DASSERT_H_

#include <log.h>

#define dassert(exp, description) if (!(exp)) dlog("\nExpression Failed: %s\n %s\n", #exp, description);

#endif
