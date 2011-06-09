// gui.h

#ifndef GUI_H
#define GUI_H

// includes

#include "pipe.h"
#include "io.h"

struct gui_t {
#ifndef _WIN32
    io_t io[1];
#else
    PipeStruct io;
#endif
};

// variables

extern gui_t GUI[1];

// functions

extern void gui_init(gui_t * gui);
extern void gui_send(gui_t * gui, const char format[], ...);
extern void gui_get        (gui_t * gui, char string[], int size); 
extern bool gui_get_non_blocking (gui_t * gui, char string[], int size); 


#endif
