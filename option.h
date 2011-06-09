
// option.h

#ifndef OPTION_H
#define OPTION_H

// includes

#include "util.h"
#include "ini.h"

// defines

#define VarNb    16
#define XBOARD   (1<<0)
#define UCI      (1<<1)
#define PG       (1<<2)
#define XBSEL    (1<<3)
#define OptionNb 256

#define IS_BUTTON(str) (my_string_case_equal(str,"button") ||      \
                        my_string_case_equal(str,"save")   ||      \
                        my_string_case_equal(str,"reset"))         \

#define IS_SPIN(str)   (my_string_case_equal(str,"spin") ||        \
                        my_string_case_equal(str,"slider"))        \

#define IS_STRING(str) (my_string_case_equal(str,"string") ||      \
                        my_string_case_equal(str,"path")   ||      \
                        my_string_case_equal(str,"file"))          \

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

// all non NULL data in an option_list_t should be malloc'ed
// use "my_string_set" to fill it. 

typedef struct {
    option_t options[OptionNb];
    int option_nb;
    int iter;
} option_list_t;

// variables

extern option_list_t Option[1];

// functions


extern void         option_init         (option_list_t *option);

extern void         option_init_pg      ();

extern bool         option_set          (option_list_t *option,
                                         const char var[],
                                         const char val[]);
extern bool         option_set_default  (option_list_t *option,
                                         const char var[],
                                         const char val[]);

extern const char * option_get          (option_list_t *option, const char var[]);
extern const char * option_get_default  (option_list_t *option, const char var[]);

extern bool         option_get_bool     (option_list_t *option, const char var[]);
extern double       option_get_double   (option_list_t *option, const char var[]);
extern int          option_get_int      (option_list_t *option, const char var[]);
extern const char * option_get_string   (option_list_t *option, const char var[]);

extern void         option_from_ini     (option_list_t *option,
                                         ini_t *ini,
                                         const char *section);

extern bool         option_is_ok        (const option_list_t *option);
extern option_t *   option_find         (option_list_t *option, const char var[]);
extern void         option_clear        (option_list_t *option);
extern void         option_insert       (option_list_t *option, option_t *new_option);

extern void         option_start_iter   (option_list_t *option);
extern option_t *   option_next         (option_list_t *option);

extern void         option_free         (option_t *option);

#endif // !defined OPTION_H

// end of option.h

