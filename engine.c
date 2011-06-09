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

// defines

#define StringSize 4096

// variables

static int write_index = 0;
static char write_buffer[StringSize];
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
    if(write_index>=StringSize){
        my_fatal("engine_send_queue(): write_buffer overflow\n");
    }
    write_index += vsnprintf(write_buffer + write_index,
                            StringSize-write_index,
                             format,
                            (va_list) (&format + 1));
}

// engine_send()

void engine_send(engine_t * engine, const char *format, ...) {
    if(write_index>=StringSize){
        my_fatal("engine_send(): write_buffer overflow\n");
    }
    vsnprintf(write_buffer + write_index,
              StringSize-write_index,
              format,
              (va_list) (&format + 1));
    pipex_writeln(engine->pipex,write_buffer);
    write_index = 0;
}

// engine_close()

void engine_close(engine_t * engine){
    char string[StringSize];
    pipex_send_eof(engine->pipex);
        // TODO: Timeout
    while (!engine_eof(engine)) { 
      engine_get(Engine,string);
    }
    pipex_exit(engine->pipex);
}

// engine_open()

void engine_open(engine_t * engine){
    int affinity;
    char *my_dir;
    if( (my_dir = my_getcwd( NULL, 0 )) == NULL )
        my_fatal("engine_open(): no current directory: %s\n",strerror(errno));
    if(my_chdir(option_get_string("EngineDir"))){
        my_fatal("engine_open(): cannot change directory: %s\n",strerror(errno));
    }
    pipex_open(engine->pipex,"Engine",option_get_string("EngineCommand"));
    if(pipex_active(engine->pipex)){
            //play with affinity (bad idea)
        affinity=option_get_int("Affinity");
        if(affinity!=-1) set_affinity(engine,affinity); //AAA
            //lets go back
        my_chdir(my_dir);
            // set a low priority
        if (option_get_bool("UseNice")){
            my_log("POLYGLOT Adjust Engine Piority\n");
            engine_set_nice_value(engine, option_get_int("NiceValue"));
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


