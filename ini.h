// ini.h

#ifndef INI_H
#define INI_H

// defines

#define IniEntriesNb 256

// includes

#include "option.h"

// types

typedef struct {
    const char *section;
    const char *name;
    const char *value;
    const char *comment;
} ini_entry_t;

typedef struct {
    ini_entry_t entries[IniEntriesNb];
    int index;
    int iter;
} ini_t;

typedef enum {
    SYNTAX_ERROR,
    EMPTY_LINE,
    NAME_VALUE,
    EMPTY_VALUE,
    SECTION
} line_type_t;

// functions

extern void         ini_init              (ini_t *ini);
extern void         ini_clear             (ini_t *ini);
extern void         ini_copy              (ini_t *dst, ini_t *src);
extern int          ini_parse             (ini_t *ini, const char *filename);
extern void         ini_disp              (ini_t *ini);
extern void         ini_insert            (ini_t *ini, ini_entry_t *entry);
extern void         ini_insert_ex         (ini_t *ini,
                                           const char *section,
                                           const char *name,
                                           const char *value);
extern void         ini_start_iter        (ini_t *ini);
extern ini_entry_t *ini_next              (ini_t *ini);
extern ini_entry_t *ini_find              (ini_t *ini,
                                           const char *section,
                                           const char *name);
extern line_type_t ini_line_parse         (const char *line,
                                           char *section,
                                           char *name,
                                           char *value);

extern const char * ini_specials;

#endif // !defined INI_H

// end of ini.h
