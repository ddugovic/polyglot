// gui.cpp

// includes

#include <cstdarg>
#include <csignal>

#include "gui.h"
#include "main.h"

// constants

static const int StringSize = 4096;

// variables

gui_t GUI[1];

// functions

// sig_quit()

static void sig_quit(int dummy){
    my_log("POLYGLOT *** SIGINT Received ***\n");
    quit();
}


// gui_init()

void gui_init(gui_t *gui){

// the following is nice if the "GUI" is a console!
    signal(SIGINT,sig_quit);
#ifdef _WIN32
    signal(SIGTERM,SIG_IGN);
#ifdef SIGPIPE
    signal(SIGPIPE,SIG_IGN);
#endif
#endif

#ifdef _WIN32
   (gui->io).Open();
#else

   gui->io->in_fd = STDIN_FILENO;
   gui->io->out_fd = STDOUT_FILENO;
   gui->io->name = "GUI";

   io_init(gui->io);
#endif
}


// gui_get_non_blocking()

bool gui_get_non_blocking(gui_t * gui, char string[], int size) {

   ASSERT(gui!=NULL);
   ASSERT(string!=NULL);
   ASSERT(size>=256);
#ifndef _WIN32
   if(io_line_ready(gui->io)){
       gui_get(GUI,string,StringSize);
       return true;
   }else{
       string[0]='\0';
       return false;
   }
#else
   if((gui->io).EOF_()){
        my_log("POLYGLOT *** EOF from GUI ***\n");
        quit();
        return true; // we never get here
   }else if ((gui->io).GetBuffer(string)) {
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
#ifdef _WIN32
    if((gui->io).EOF_()){
        my_log("POLYGLOT *** EOF from GUI ***\n");
        quit();
    }
    (gui->io).LineInput(string);
    my_log("GUI->Adapter: %s\n", string);
#else
    if (!io_get_line(gui->io,string,size)) { // EOF
        my_log("POLYGLOT *** EOF from GUI ***\n");
        quit();
    }
#endif
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
   gui->io.LineOutput(string);
   my_log("Adapter->GUI: %s\n",string);
#endif
}

