// gui.c

// includes

#include <stdarg.h>
#include <signal.h>

#include "gui.h"
#include "main.h"

// variables

gui_t GUI[1];

// functions

// sig_int()

static void sig_int(int dummy){
    my_log("POLYGLOT *** SIGINT Received ***\n");
    quit();
}

// sig_pipe()

static void sig_pipe(int dummy){
    my_log("POLYGLOT *** SIGPIPE Received ***\n");
    quit();
}

// sig_term()

static void sig_term(int dummy){
    my_log("POLYGLOT *** SIGTERM Received ***\n");
    quit();
}


// gui_init()

void gui_init(gui_t *gui){

// the following is nice if the "GUI" is a console!
    signal(SIGINT,sig_int);
#ifdef SIGTERM
    signal(SIGTERM,sig_term);
#endif 
#ifdef SIGPIPE
    signal(SIGPIPE,sig_pipe);
#endif
    pipex_open(gui->pipex,"GUI",NULL,NULL);
}


// gui_get_non_blocking()

bool gui_get_non_blocking(gui_t * gui, char *string) {

   ASSERT(gui!=NULL);
   ASSERT(string!=NULL);

   if(pipex_eof(gui->pipex)){
        quit();
        return TRUE; // we never get here
   }
   return pipex_readln_nb(gui->pipex,string);
}

// gui_get()

void gui_get(gui_t * gui, char *string) {
    if(pipex_eof(gui->pipex)){
        quit();
    }
    pipex_readln(gui->pipex,string);
}


// gui_send()

void gui_send(gui_t * gui, const char format[], ...) {

   char string[FormatBufferSize];

   ASSERT(gui!=NULL);
   ASSERT(format!=NULL);

   // format

   CONSTRUCT_ARG_STRING(format,string);

   // send

   pipex_writeln(gui->pipex,string);

}

