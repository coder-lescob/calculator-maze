#ifndef TIMINGS_H
#define TIMINGS_H

#include "platform.h"

// TYPE WRAPPERS

#ifdef PLATFORM_CALCULATOR

#include <eadk.h>

typedef uint64_t time_ms_t;
typedef uint64_t time_us_t;

#else
#error PLATFORM NOT SUPPORTED
#endif

// FUNCTION WRAPPERS

/**
 * sleeps for time µs
 */
void usleep(time_us_t time);

/**
 * sleeps for time ms
 */
void msleep(time_ms_t time);

/**
 * get the current time from the start of the app or from 1970ish in milliseconds
 * what ever it does not really matter however a millisecond must be a millisecond
 * so that `get_time() - old_time` has the same result on both platform (if they are as performant)
 */
time_ms_t get_time();

#endif