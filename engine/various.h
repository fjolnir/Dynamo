#include <stdio.h>

#ifndef _VARIOUS_H_
#define _VARIOUS_H_

#ifdef MAX
#undef MAX
#endif
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

#ifdef MIN
#undef MIN
#endif
#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )

#define CLAMP(x, min, max) MAX(min, MIN(x, max))

#ifdef WIN32
struct timezone {
	int  tz_minuteswest;
	int  tz_dsttime;
};
int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

#pragma mark - Debug logging

#ifdef DEBUG
	#define debug_log(fmt, ...) fprintf(stderr, "%s:%u (%s): " fmt "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
	#define debug_log(fmt, ...)
#endif

#endif
