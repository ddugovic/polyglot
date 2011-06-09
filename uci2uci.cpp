// uci2uci.cpp

// includes

#include <cstring>
#include <cstdlib>

#include "util.h"
#include "adapter.h"
#include "board.h"
#include "engine.h"
#include "fen.h"
#include "gui.h"
#include "move.h"
#include "move_do.h"
#include "move_legal.h"
#include "parse.h"
#include "option.h"
#include "book.h"
#include "main.h"
#include "uci.h"

// constants

static const int StringSize = 4096;

// variables 

static board_t UCIboard[1];
static bool Init=true;
static int SavedMove=MoveNone;

// defines

#ifdef _WIN32
#define strcasecmp lstrcmpi
#endif

// parse_position()

static void parse_position(const char string[]) {

/* This is borrowed from Toga II. This code is quite hacky and will be
   rewritten using the routines in parse.cpp.
*/
                                                   
   const char * fen;
   char * moves;
   const char * ptr;
   char move_string[256];
   int move;
   char * string_copy;

   // init

   string_copy=my_strdup(string);
   
   fen = strstr(string_copy,"fen ");
   moves = strstr(string_copy,"moves ");

   // start position

   if (fen != NULL) { // "fen" present

      if (moves != NULL) { // "moves" present
         ASSERT(moves>fen);
         moves[-1] = '\0'; // dirty, but so is UCI
      }

      board_from_fen(UCIboard,fen+4); // CHANGE ME

   } else {

      // HACK: assumes startpos

      board_from_fen(UCIboard,StartFen);
   }

   // moves

   if (moves != NULL) { // "moves" present

      ptr = moves + 6;

      while (*ptr != '\0') {

         while (*ptr == ' ') ptr++;

         move_string[0] = *ptr++;
         move_string[1] = *ptr++;
         move_string[2] = *ptr++;
         move_string[3] = *ptr++;

         if (*ptr == '\0' || *ptr == ' ') {
            move_string[4] = '\0';
         } else { // promote
            move_string[4] = *ptr++;
            move_string[5] = '\0';
         }
         move = move_from_can(move_string,UCIboard);

         move_do(UCIboard,move);

      }
   }
   free(string_copy);
}


// send_book_move()

static void send_book_move(int move){
    char move_string[256];
    my_log("POLYGLOT *BOOK MOVE*\n");
    move_to_can(move,UCIboard,move_string,256);
        // bogus info lines
    gui_send(GUI,"info depth 1 time 0 nodes 0 nps 0 cpuload 0");
    gui_send(GUI,"bestmove %s",move_string);
}

// send_uci_options()

void send_uci_options() {
    int i;
    char option_line[StringSize]="";
    char option_string[StringSize]="";
    gui_send(GUI,"id name %s", Uci->name);
    gui_send(GUI,"id author %s", Uci->author);
    for(i=0;i<Uci->option_nb;i++){
        strcat(option_line,"option");
        if(strcmp(Uci->option[i].name,"<empty>")){
            sprintf(option_string," name %s",Uci->option[i].name);
            strcat(option_line,option_string);
        }
        if(strcmp(Uci->option[i].type,"<empty>")){
            sprintf(option_string," type %s",Uci->option[i].type);
            strcat(option_line,option_string);
        }
        if(strcmp(Uci->option[i].value,"<empty>")){
            sprintf(option_string," default %s",Uci->option[i].value);
            strcat(option_line,option_string);
        }
        if(strcmp(Uci->option[i].min,"<empty>")){
            sprintf(option_string," min %s",Uci->option[i].min);
            strcat(option_line,option_string);
        }
        if(strcmp(Uci->option[i].max,"<empty>")){
            sprintf(option_string," max %s",Uci->option[i].max);
            strcat(option_line,option_string);
        }
        if(strcmp(Uci->option[i].var,"<empty>")){
            sprintf(option_string," var %s",Uci->option[i].var);
            strcat(option_line,option_string);
        }
        gui_send(GUI,"%s",option_line);
        strcpy(option_line,"");
    }
    gui_send(GUI,"uciok");
}

// parse_setoption()



static void parse_setoption(const char string[]) {
    parse_t parse[1];
    char option[StringSize];
    char name[StringSize];
    char value[StringSize];
    char * string_copy;
    string_copy=my_strdup(string);
    parse_open(parse,string_copy);
    parse_add_keyword(parse,"setoption");
    parse_add_keyword(parse,"name");
    parse_add_keyword(parse,"value");
    parse_get_word(parse,option,StringSize);
    ASSERT(my_string_equal(option,"setoption"));
    if(parse_get_word(parse,option,StringSize)){
        if(my_string_equal(option,"name")){
            parse_get_string(parse,name,StringSize);
            if(parse_get_word(parse,option,StringSize)){
                if(my_string_equal(option,"value")){
                    parse_get_string(parse,value,StringSize);
                    if(my_string_case_equal(name,PolyglotBookFile)){
                        my_log("POLYGLOT *** SETTING BOOK ***\n");
                        my_log("POLYGLOT BOOK \"%s\"\n",value);
                        book_close();
                        book_clear();
                        book_open(value);
                        if(!book_is_open()){
                            my_log("POLYGLOT Unable to open book \"%s\"\n",value);
                        }
                    }else{
                        engine_send(Engine,"%s",string);       
                    }
                }
            }
        }
    }
    parse_close(parse);
    free(string_copy);
}


// uci_gui_step()

void uci_gui_step(char string[]) {
    int move;
     if(false){
     }else if(match(string,"uci")){
         send_uci_options();
         return;
     }else if(match(string,"setoption *")){
         parse_setoption(string);
         return;
     }else if(match(string,"position *")){
         parse_position(string);
         Init=false;
     }else if(match(string,"go *")){
         if(Init){
             board_from_fen(UCIboard,StartFen);
             Init=false;
         }
         SavedMove=MoveNone;
         if(!strstr(string,"infinite")){
             move=book_move(UCIboard,option_get_bool("BookRandom"));
             if (move != MoveNone && move_is_legal(move,UCIboard)) {
                 if(strstr(string,"ponder")){
                     SavedMove=move;
                     return;
                 }else{
                     send_book_move(move);
                     return;
                 }
             }
         }
     }else if(match(string,"ponderhit") || match(string,"stop")){
         if(SavedMove!=MoveNone){
         	send_book_move(SavedMove);
         	SavedMove=MoveNone;
         	return;
         }
     }else if(match(string,"quit")){
         my_log("POLYGLOT *** \"quit\" from GUI ***\n");
         quit();
     }
     engine_send(Engine,"%s",string);
}

void uci_engine_step(char string[]) {
    gui_send(GUI,string);
}

// end of uci2uci.cpp
