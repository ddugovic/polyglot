// gui.h

#ifndef GUI_H
#define GUI_H

// includes

#include "pipex.h"
#include "io.h"

// types

typedef struct {
    pipex_t pipex[1];
} gui_t;

// variables

extern gui_t GUI[1];

// functions

extern void gui_init(gui_t * gui);
extern void gui_send(gui_t * gui, const char *format, ...);
extern void gui_get        (gui_t * gui, char *string); 
extern bool gui_get_non_blocking (gui_t * gui, char *string); 


#endif
