// mainloop.c

// includes

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "engine.h"
#include "gui.h"
#include "option.h"
//#include "ini.h"
#include "xboard2uci.h"
#include "uci2uci.h"

// prototypes

static void mainloop_init            ();
static void mainloop_wait_for_event  ();
static void mainloop_engine_step(char * string);
static void mainloop_gui_step(char * string);

// functions

// mainloop_init()
    
static void mainloop_init(){
    if(!option_get_bool(Option,"UCI")){
        xboard2uci_init();  // the default
    }
}

// mainloop_engine_step()

static void mainloop_engine_step(char * string){
    if(option_get_bool(Option,"UCI")){
        uci2uci_engine_step(string); 
    }else{
        xboard2uci_engine_step(string);
    }
}

// mainloop_gui_step()

static void mainloop_gui_step(char * string){
    if(option_get_bool(Option,"UCI")){
        uci2uci_gui_step(string); 
    }else if(my_string_equal(string,"uci")){ // mode auto detection
        my_log("POLYGLOT *** Switching to UCI mode ***\n");
        option_set(Option,"UCI","true");
        uci2uci_gui_step(string);
    }else{
        xboard2uci_gui_step(string);
    }
}

// mainloop()

void mainloop() {
    char string[StringSize];
    my_log("POLYGLOT *** Mainloop started ***\n");
    mainloop_init();
    while (!engine_eof(Engine)) {
            // process buffered lines
        while(TRUE){
            if(gui_get_non_blocking(GUI,string)){
                mainloop_gui_step(string);
            }else if(!engine_eof(Engine) &&
                     engine_get_non_blocking(Engine,string) ){
                mainloop_engine_step(string);
            }else{
                break;
            }
        }
        mainloop_wait_for_event();
    }
    my_log("POLYGLOT *** Mainloop has ended ***\n");
    // This should be handled better.
    engine_close(Engine);
    my_log("POLYGLOT Calling exit\n");
    exit(EXIT_SUCCESS);

}




// mainloop_wait_for_event()

static void mainloop_wait_for_event(){
    pipex_t *pipex[3];
    pipex[0]=GUI->pipex;
    pipex[1]=Engine->pipex;
    pipex[2]=NULL;
    pipex_wait_event(pipex);
}



