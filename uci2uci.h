
// uci2uci.h

#ifndef UCI2UCI_H
#define UCI2UCI_H

// constants

const char * const PolyglotBookFile="Polyglot BookFile";

// functions

extern void uci_gui_step(char string[]);
extern void uci_engine_step(char string[]);
extern void send_uci_options();

#endif // !defined UCI2UCI_H

// end of uci2uci.h
