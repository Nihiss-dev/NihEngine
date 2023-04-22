#pragma once

#include "Config.h"

#if defined(ENABLE_ASSERT)
/*
* Make sure the condition is true
* If the condition is not met, the programm will assert
* It will then create a breakpoint on the faulty code
*/
#define NIH_ASSERT(condition) \
	do \
	{ \
		if (!(condition)) \
			{ \
				__debugbreak(); \
			} \
	} while (false)
#else
#define NIH_ASSERT(condition) ((void)0)
#endif // ENABLE_ASSERT