// gui.cpp

// includes

#include <cstdarg>

#include "gui.h"
#include "main.h"

// constants

static const int StringSize = 4096;

// variables

gui_t GUI[1];

// functions


// gui_init()

void gui_init(gui_t *gui){
    #ifdef _WIN32
   (gui->pipeStdin).Open();
#else
   
   gui->io->in_fd = STDIN_FILENO;
   gui->io->out_fd = STDOUT_FILENO;
   gui->io->name = "GUI";

   io_init(gui->io);
#endif
}


// gui_get_non_blocking()

// this is only non_blocking on windows!

bool gui_get_non_blocking(gui_t * gui, char string[], int size) {

   ASSERT(gui!=NULL);
   ASSERT(string!=NULL);
   ASSERT(size>=256);
#ifndef _WIN32
   if (!io_get_line(gui->io,string,size)) { // EOF
      my_log("POLYGLOT *** EOF from GUI ***\n");
      quit();
   }
   return true;
#else
   if ((gui->pipeStdin).LineInput(string)) {
       my_log("GUI->Adapter: %s\n", string);
       return true;
   } else {
        string[0]='\0';
        return false;
   }
   
#endif
}

// gui_get()

void gui_get(gui_t * gui, char string[], int size) {
    bool data_available;
    while(true){
        data_available=gui_get_non_blocking(gui, string, size);
        if(!data_available){
            Idle();
        }else{
            break;
        }
    }
}


// gui_send()

void gui_send(gui_t * gui, const char format[], ...) {

   va_list arg_list;
   char string[StringSize];

   ASSERT(gui!=NULL);
   ASSERT(format!=NULL);

   // format

   va_start(arg_list,format);
   vsprintf(string,format,arg_list);
   va_end(arg_list);

   // send

#ifndef _WIN32
   io_send(gui->io,"%s",string);
#else
   printf("%s\n",string);
   fflush(stdout);
   my_log("Adapter->GUI: %s\n",string);
#endif
}

