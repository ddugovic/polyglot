
// colour.h

#ifndef COLOUR_H
#define COLOUR_H

// includes

#include "util.h"

// defines

#define BlackFlag (1 << 0)
#define WhiteFlag (1 << 1)
#define ColourNone 0
#define Black BlackFlag
#define White WhiteFlag
#define ColourNb 3

// functions

extern bool colour_is_ok    (int colour);

extern bool colour_is_white (int colour);
extern bool colour_is_black (int colour);
extern bool colour_equal    (int colour_1, int colour_2);

extern int  colour_opp      (int colour);

#endif // !defined COLOUR_H

// end of colour.h

