#ifndef UNWIND_H
#define UNWIND_H

#include <stdbool.h>
#include <setjmp.h>

// a panic handler
typedef void (* panic_handler_t)(void);

// global to panic
extern jmp_buf panic_point;

/**
 * set a panic handler
 */
#define SET_PANIC_HANDLER(code) \
    if (setjmp(panic_point) != 0) code

/**
 * unwinds execution to panic handler
 */
#define PANIC() \
    longjmp(panic_point, 1)

/**
 * asserts a condition
 */
#define ASSERT(cond) \
    if (!cond) PANIC()

#endif