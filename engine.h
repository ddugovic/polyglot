// engine.h

#ifndef ENGINE_H
#define ENGINE_H

// includes

#include "util.h"
#include "pipex.h"

// types

typedef struct {
    pipex_t pipex[1];
} engine_t;



// variables

extern engine_t Engine[1];

// functions

extern bool engine_is_ok      (const engine_t * engine);
extern void engine_open       (engine_t * engine);
extern void engine_close      (engine_t * engine);
extern bool engine_active     (engine_t * engine);
extern bool engine_eof        (engine_t * engine);
extern void engine_send       (engine_t * engine, const char format[], ...);
extern void engine_send_queue (engine_t * engine, const char format[], ...);
extern bool engine_get_non_blocking(engine_t * engine, char string[]);
extern void engine_get        (engine_t * engine, char string[]);
extern void engine_set_nice_value(engine_t * engine, int value);

#endif // !defined ENGINE_H
