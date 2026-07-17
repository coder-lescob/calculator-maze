#include "timings.h"

// FUNCTION WRAPPERS

#ifdef PLATFORM_CALCULATOR

#include <eadk.h>

/**
 * sleeps for time µs
 */
void usleep(time_us_t time) {
    eadk_timing_usleep(time);
}

/**
 * sleeps for time ms
 */
void msleep(time_ms_t time) {
    eadk_timing_msleep(time);
}

/**
 * get the current time from the start of the app or from 1970ish in milliseconds
 * what ever it does not really matter however a millisecond must be a millisecond
 * so that `get_time() - old_time` has the same result on both platform (if they are as performant)
 */
time_ms_t get_time() {
    return eadk_timing_millis();
}

#else
#error PLATFORM NOT SUPPORTED
#endif