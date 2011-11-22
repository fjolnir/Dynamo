#include "various.h"
#ifndef WIN32
	#include <sys/time.h>
#else
// Implementation of gettimeofday for windows
#include <time.h>
#include <windows.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
	#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
	#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;

	if(tv != NULL) {
		GetSystemTimeAsFileTime(&ft);

		// The GetSystemTimeAsFileTime returns the number of 100 nanosecond
		// intervals since Jan 1, 1601 in a structure. Copy the high bits to
		// the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// Convert file time to unix epoch
		tmpres /= 10;
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (tz != NULL) {
		if (!tzflag) {
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}
	return 0;
}
#endif

long timeInUsec()
{
	struct timeval elapsedTime;
	gettimeofday(&elapsedTime, NULL);
	return elapsedTime.tv_usec;
}
