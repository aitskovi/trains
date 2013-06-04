#ifndef _DASSERT_H_
#define _DASSERT_H_

#include <log.h>

#ifdef DEBUG
    #define dassert(exp, description) if (!(exp)) log("\nExpression Failed: %s\n %s\n", #exp, description)
#else
    #define dassert(exp, description) do {} while(0)
#endif

#endif
