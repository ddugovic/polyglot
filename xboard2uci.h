
// xboard2uci.h

#ifndef XBOARD2UCI_H
#define XBOARD2UCI_H

// includes

#include "util.h"

// types

// functions

extern void xboard2uci_init           ();
extern void xboard2uci_gui_step       (char string[]);
extern void xboard2uci_engine_step    (char string[]);
extern void xboard2uci_send_options   ();


#endif // !defined XBOARD2UCI_H

// end of xboard2uci.h

