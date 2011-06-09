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

static void mainloop_step      ();
static void mainloop_init();
static void mainloop_engine_step(char * string);
static void mainloop_gui_step(char * string);

// mainloop_init()
    
static void mainloop_init(){
    if(!option_get_bool("UCI")){
        xboard_init();
    }
}

// mainloop_engine_step()

static void mainloop_engine_step(char * string){
    if(option_get_bool("UCI")){
       uci_engine_step(string); 
    }else{
        engine_step(string);
    }
}

// mainloop_gui_step()

static void mainloop_gui_step(char * string){
    if(option_get_bool("UCI")){
        uci_gui_step(string); 
    }else{
        xboard_step(string);
    }  
}



// mainloop()

void mainloop() {
    mainloop_init();
    while (!engine_eof(Engine)) mainloop_step();
    my_log("POLYGLOT *** EOF file received from engine ***");
}

// adapter_step()

#ifdef _WIN32
static void mainloop_step(){  // polling!
    bool xin,ein;
    char string[StringSize];
    xin=gui_get_non_blocking(GUI,string,StringSize);
    if(xin) mainloop_gui_step(string);
    ein=engine_get_non_blocking(Engine,string,StringSize);
    if(ein) mainloop_engine_step(string);
    if(xin==false && ein==false) Idle();//nobody wants me,lets have a beauty nap
}
#else
static void mainloop_step() {

   fd_set set[1];
   int fd_max;
   int val;
   char string[StringSize];

       // process buffered lines

   while (io_line_ready(GUI->io)){
       gui_get(GUI,string,StringSize);
       mainloop_gui_step(string);
   }
   while (!engine_eof(Engine) && io_line_ready(Engine->io)){
       engine_get(Engine,string,StringSize);
       mainloop_engine_step(string);
   }

   // init

   FD_ZERO(set);
   fd_max = -1; // HACK

   // add xboard input

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
}
#endif

