#pragma once

#ifndef NDEBUG
#define ABORT_IF_FN(ConditionFn, b) \
if(ConditionFn == b) \
{ \
	abort(); \
}
#else
#define ABORT_IF_FN_TRUE(ConditionFn, b) ConditionFn
#endif // DEBUG
