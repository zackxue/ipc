
#include "aval.h"

inline AVal AVV(const char* av_val, int av_len)
{
	AVal aval;
	aval.av_val = av_val;
	aval.av_len = av_len;
	return aval;
}
