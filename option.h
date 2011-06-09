
// option.h

#ifndef OPTION_H
#define OPTION_H

// includes

#include "util.h"

// defines

#define VarNb  16
#define XBOARD (1<<0)
#define UCI    (1<<1)
#define PG     (1<<2)

// types

typedef struct {   // TODO: put back in more logical order
    const char * name;
    const char * type;
    const char * min;
    const char * max;
    const char * default_;
    const char * value;
    int var_nb;
    const char * var[VarNb];
    int mode;
} option_t;

// variables

extern option_t Option[];

// functions

extern void         option_init       ();

extern bool         option_set        (const char var[], const char val[]);
extern bool         option_set_default(const char var[], const char val[]);
extern const char * option_get        (const char var[]);
extern const char * option_get_default(const char var[]);

extern bool         option_get_bool   (const char var[]);
extern double       option_get_double (const char var[]);
extern int          option_get_int    (const char var[]);
extern const char * option_get_string (const char var[]);

#endif // !defined OPTION_H

// end of option.h

