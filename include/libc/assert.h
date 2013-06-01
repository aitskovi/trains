#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <log.h>

#define dassert(exp, description) if (!(exp)) dlog("\nExpression Failed: %s\n %s\n", #exp, description);

#endif
