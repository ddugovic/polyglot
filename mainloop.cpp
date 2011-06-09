// mainloop.cpp

// constants

static const int StringSize = 4096;

// includes

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef _WIN32
#include <sys/select.h>
#include <sys/types.h> // Mac OS X needs this one
#include <unistd.h>
#endif

#include "main.h"
#include "engine.h"
#include "gui.h"
#include "option.h"
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
    if(!option_get_bool("UCI")){
        xboard2uci_init();  // the default
    }
}

// mainloop_engine_step()

static void mainloop_engine_step(char * string){
    if(option_get_bool("UCI")){
        uci2uci_engine_step(string); 
    }else{
        xboard2uci_engine_step(string);
    }
}

// mainloop_gui_step()

static void mainloop_gui_step(char * string){
    if(option_get_bool("UCI")){
        uci2uci_gui_step(string); 
    }else if(my_string_equal(string,"uci")){ // mode auto detection
        my_log("POLYGLOT *** Switching to UCI mode ***\n");
        option_set("UCI","true");
        uci2uci_gui_step(string);
    }else{
        xboard2uci_gui_step(string);
    }
}

// mainloop()

void mainloop() {
    char string[StringSize];
    mainloop_init();
    while (!engine_eof(Engine)) {
            // process buffered lines
        while(TRUE){
            if(gui_get_non_blocking(GUI,string,StringSize)){
                mainloop_gui_step(string);
            }else if(!engine_eof(Engine) &&
                     engine_get_non_blocking(Engine,string,StringSize) ){
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
#ifdef _WIN32
    HANDLE hHandles[2];
    char string[StringSize];
    hHandles[0]=(GUI->io).hEvent;
    hHandles[1]=(Engine->io).hEvent;
    WaitForMultipleObjects(2,               // count
                           hHandles,        //
                           FALSE,           // return if one object is signaled
                           INFINITE         // no timeout
                           );
#else
   fd_set set[1];
   int fd_max;
   int val;
   char string[StringSize];
   // init

   FD_ZERO(set);
   fd_max = -1; // HACK

   // add gui input

   ASSERT(GUI->io->in_fd>=0);

   FD_SET(GUI->io->in_fd,set);
   if (GUI->io->in_fd > fd_max) fd_max = GUI->io->in_fd;

   // add engine input

   ASSERT(Engine->io->in_fd>=0);

   FD_SET(Engine->io->in_fd,set);
   if (Engine->io->in_fd > fd_max) fd_max = Engine->io->in_fd;

   // wait for something to read (no timeout)

   ASSERT(fd_max>=0);
   val = select(fd_max+1,set,NULL,NULL,NULL);
   if (val == -1 && errno != EINTR) my_fatal("adapter_step(): select(): %s\n",strerror(errno));

   if (val > 0) {
      if (FD_ISSET(GUI->io->in_fd,set)) io_get_update(GUI->io); // read some xboard input
      if (FD_ISSET(Engine->io->in_fd,set)) io_get_update(Engine->io); // read some engine input
   }    
#endif
}



