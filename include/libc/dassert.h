#ifndef _DASSERT_H_
#define _DASSERT_H_

#include <log.h>

#ifdef DEBUG
    #define duassert(exp, description) if (!(exp)) ulog("\nExpression Failed: %s\n %s\n", #exp, description)
    #define dassert(exp, description) if (!(exp)) log("\nExpression Failed: %s\n %s\n", #exp, description)
#else
    #define duassert(exp, description) do {} while(0)
    #define dassert(exp, description) do {} while(0)
#endif

#ifdef INFO
    #define iuassert(exp, description) if (!(exp)) ulog("\nExpression Failed: %s\n %s\n", #exp, description)
    #define iassert(exp, description) if (!(exp)) log("\nExpression Failed: %s\n %s\n", #exp, description)
#else
    #define iuassert(exp, description) do {} while(0)
    #define iassert(exp, description) do {} while(0)
#endif

#ifdef WARNING
    #define wuassert(exp, description) if (!(exp)) ulog("\nExpression Failed: %s\n %s\n", #exp, description)
    #define wkassert(exp, description) if (!(exp)) log("\nExpression Failed: %s\n %s\n", #exp, description)
#else
    #define wuassert(exp, description) do {} while(0)
    #define wkassert(exp, description) do {} while(0)
#endif

#ifdef CRITICAL
    #define cuassert(exp, description) if (!(exp)) ulog("\nExpression Failed: %s\n %s\n", #exp, description)
    #define ckassert(exp, description) if (!(exp)) log("\nExpression Failed: %s\n %s\n", #exp, description)
#else
    #define cuassert(exp, description) do {} while(0)
    #define ckassert(exp, description) do {} while(0)
#endif


#endif
