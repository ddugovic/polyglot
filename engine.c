// engine.c

// includes

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "engine.h"
#include "option.h"
#include "pipex.h"
#include "util.h"

// variables

engine_t Engine[1];

// functions

// set_affinity()

void set_affinity(engine_t *engine, int value){
    pipex_set_affinity(engine->pipex,value);
}

// engine_set_nice_value()

void engine_set_nice_value(engine_t *engine, int value){
    pipex_set_priority(engine->pipex,value);
}

// engine_send_queue()

void engine_send_queue(engine_t * engine, const char *format, ...) {
    char buf[FormatBufferSize];
    CONSTRUCT_ARG_STRING(format,buf);
    pipex_write(engine->pipex,buf);
}

// engine_send()

void engine_send(engine_t * engine, const char *format, ...) {
    char buf[FormatBufferSize];
    CONSTRUCT_ARG_STRING(format,buf);
    pipex_writeln(engine->pipex,buf);
}

// engine_close()

void engine_close(engine_t * engine){
    char string[StringSize];
    int elapsed_time;
    int ret;
    int close_timeout=500;
    my_log("POLYGLOT Closing engine.\n");
    pipex_send_eof(engine->pipex);
        // TODO: Timeout

    elapsed_time=0;
    while (!engine_eof(engine) && (elapsed_time<close_timeout)) { 
      ret=engine_get_non_blocking(engine,string);
      if(!ret  && !engine_eof(engine)){
	my_log("POLYGLOT Engine does not reply. Sleeping %dms.\n", WAIT_GRANULARITY);
	my_sleep(WAIT_GRANULARITY);
	elapsed_time+=WAIT_GRANULARITY;
      }
    }
    if(elapsed_time>=close_timeout){
      my_log("POLYGLOT Waited more than %dms. Moving on.\n",close_timeout); 
    }
    pipex_exit(engine->pipex,200);
}

// engine_open()

void engine_open(engine_t * engine){
    int affinity;
    pipex_open(engine->pipex,
               "Engine",
               option_get_string(Option,"EngineDir"),
               option_get_string(Option,"EngineCommand"));
    if(pipex_active(engine->pipex)){
            //play with affinity (bad idea)
        affinity=option_get_int(Option,"Affinity");
        if(affinity!=-1) set_affinity(engine,affinity); //AAA
            // set a low priority
        if (option_get_bool(Option,"UseNice")){
            my_log("POLYGLOT Adjust Engine Piority\n");
            engine_set_nice_value(engine, option_get_int(Option,"NiceValue"));
        }
    }
    
}

bool engine_active(engine_t *engine){
    return pipex_active(engine->pipex);
}

bool engine_eof(engine_t *engine){
    return pipex_eof(engine->pipex);
}

bool engine_get_non_blocking(engine_t * engine, char *string){
    return pipex_readln_nb(engine->pipex,string);
 }

void engine_get(engine_t * engine, char *string){
    pipex_readln(engine->pipex,string);
}


