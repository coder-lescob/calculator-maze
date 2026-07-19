#ifndef ERRORS_H
#define ERRORS_H

#include "rendering.h"
#include "input.h"

/**
 * shows an error screen and waits until the ok key is pressed
 * it also formats the text so that it fits on the screen
 * and shows press [ok] to quit app
 */
void error_screen(char *msg);

#endif