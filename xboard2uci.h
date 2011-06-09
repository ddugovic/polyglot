
// xboard2uci.h

#ifndef XBOARD2UCI_H
#define XBOARD2UCI_H

// includes

#include "util.h"
#include "uci2uci.h"

// types





// functions

extern void adapter_loop ();
extern void xboard_init           ();
extern void xboard_step           (char string[]);
extern void engine_step           (char string[]);

#endif // !defined XBOARD2UCI_H

// end of xboard2uci.h

