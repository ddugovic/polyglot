
// uci.c

// includes

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "engine.h"
#include "gui.h"
#include "move.h"
#include "move_do.h"
#include "move_legal.h"
#include "option.h"
#include "parse.h"
#include "line.h"
#include "uci.h"


// constants

static const bool UseDebug = FALSE;

#define StringSize ((int)4096)

// variables

uci_t Uci[1];

// Hopefully the following confusion is temporary
// Normally we should check for the engine name but this is a hack anyway
// Some of there where provided by Marc Lacrosse

const char * thread_options[]={
  "number of threads",        // toga
  "number threads",           // Deep Learning Toga
  "threads",                  // glaurung, zappa, cyclone, grapefruit,
                              // Deep Shredder, Deep Junior, bright
  "core threads",             // HIARCS
  "max cpus",                 // rybka
  "cpus",                     // Deep Sjeng, Fruit2.3.5
  "maxthreads",               // Naum 
  NULL
};

// prototypes

static bool uci_is_ok      (const uci_t * uci);

static int  parse_bestmove (uci_t * uci, const char string[]);
static void parse_id       (uci_t * uci, const char string[]);
static int  parse_info     (uci_t * uci, const char string[]);
static void parse_option   (uci_t * uci, const char string[]);
static void parse_score    (uci_t * uci, const char string[]);

static int  mate_score     (int dist);

// functions


// uci_adapt_UCI3()

static void apply_UCI3_heuristics(option_t *opt){
  if(option_get_int(Option,"UCIVersion")>2){
    return;
  }
  if(!my_string_equal(opt->type,"string")){
    return;
  }
  if(!strncmp(opt->name,"UCI_",4)){
    return;
  }
  if(my_string_case_contains(opt->name,"file")){
    my_string_set(&opt->type,"file");
    return;
  }
  if(my_string_case_contains(opt->name,"path")){
    my_string_set(&opt->type,"path");
    return;
  }
}

// uci_set_threads()

void uci_set_threads(uci_t * uci, int n) {
    const char *thread_option=uci_thread_option(uci);
    ASSERT(n>=1);
    if(thread_option){
        uci_send_option(uci,thread_option,"%d",n);
    }
}


const char * uci_thread_option(uci_t * uci){
    const char **p = thread_options;
    const char *thread_option;
    option_t *opt;
    while((thread_option = *(p++))){
        if((opt=option_find(uci->option,thread_option))){
            return opt->name;
            break;
        }
    }
    return NULL;
}

// uci_is_ok()

static bool uci_is_ok(const uci_t * uci) {

   if (uci == NULL) return FALSE;
   if (uci->engine == NULL) return FALSE;
   if (!option_is_ok(uci->option)) return FALSE;
   return TRUE;
}

// uci_open()

void uci_open(uci_t * uci, engine_t * engine) {

   char string[StringSize];
   int event;

   ASSERT(uci!=NULL);
   ASSERT(engine!=NULL);

   // init

   uci->engine = engine;

   uci->name = NULL;
   my_string_set(&uci->name,"<empty>");
   uci->author = NULL;
   my_string_set(&uci->author,"<empty>");
   option_init(uci->option);

   uci->ready_nb = 0;
   uci->searching = 0;
   uci->pending_nb = 0;
   uci->multipv_mode = FALSE;
   board_start(uci->board);
   uci_clear(uci);

   // send "uci" and wait for "uciok"

   engine_send(uci->engine,"uci");

   do {
      engine_get(uci->engine,string);
      // Handle the case that the engine is really a WB engine somewhat gracefully.
      if((strstr(string,"Illegal") || strstr(string,"Error"))
         &&strstr(string,"uci")){
          my_fatal("uci_open(): Not an UCI engine (not found).\n");
      }
      event = uci_parse(uci,string);
   } while (!engine_eof(Engine) && (event & EVENT_UCI) == 0);
}

// uci_close()

void uci_close(uci_t * uci) {

   ASSERT(uci_is_ok(uci));
   engine_close(uci->engine);
   uci->engine = NULL;
   my_string_clear(&uci->name);
   my_string_clear(&uci->author);

   option_clear(uci->option);
}

// uci_clear()

void uci_clear(uci_t * uci) {

   ASSERT(uci_is_ok(uci));

   ASSERT(!uci->searching);

   uci->best_move = MoveNone;
   uci->ponder_move = MoveNone;

   uci->score = 0;
   uci->depth = 0;
   uci->sel_depth = 0;
   line_clear(uci->pv);

   uci->best_score = 0;
   uci->best_depth = 0;
   uci->best_sel_depth = 0;
   line_clear(uci->best_pv);
// make the default 1 instead of 0 so that info lines can be recognized by their node number 0
   uci->node_nb = 1;
   uci->time = 0.0;
   uci->speed = 0.0;
   uci->cpu = 0.0;
   uci->hash = 0.0;
   line_clear(uci->current_line);

   uci->root_move = MoveNone;
   uci->root_move_pos = 0;
   uci->root_move_nb = board_mobility(uci->board);
}

// uci_send_isready()

void uci_send_isready(uci_t * uci) {

   ASSERT(uci!=NULL);

   engine_send(uci->engine,"isready");
   uci->ready_nb++;
}

// uci_send_isready_sync()

void uci_send_isready_sync(uci_t * uci) {

   char string[StringSize];
   int event;

   ASSERT(uci_is_ok(uci));

   // send "isready" and wait for "readyok"

   uci_send_isready(uci);

   do {
      engine_get(uci->engine,string);
      event = uci_parse(uci,string);
   } while (!engine_eof(Engine) && (event & EVENT_READY) == 0);
}

// uci_send_stop()

void uci_send_stop(uci_t * uci) {

   ASSERT(uci_is_ok(uci));

   ASSERT(uci->searching);
   ASSERT(uci->pending_nb>=1);

   engine_send(Engine,"stop");
   uci->searching = FALSE;
}

// uci_send_stop_sync()

void uci_send_stop_sync(uci_t * uci) {

   char string[StringSize];
   int event;

   ASSERT(uci_is_ok(uci));

   ASSERT(uci->searching);
   ASSERT(uci->pending_nb>=1);

   // send "stop" and wait for "bestmove"

   uci_send_stop(uci);

   do {
      engine_get(uci->engine,string);
      event = uci_parse(uci,string);
   } while (!engine_eof(Engine) && (event & EVENT_STOP) == 0);
}

// uci_send_ucinewgame()

void uci_send_ucinewgame(uci_t * uci) {

   ASSERT(uci!=NULL);

   if (option_get_int(Option,"UCIVersion") >= 2) {
      engine_send(uci->engine,"ucinewgame");
   }
}

// uci_send_option()

bool uci_send_option(uci_t * uci, const char option[], const char format[], ...) {

   char value[FormatBufferSize];
   option_t * opt;
   bool found=FALSE;

   ASSERT(uci_is_ok(uci));
   ASSERT(option!=NULL);
   ASSERT(format!=NULL);

   // format

   CONSTRUCT_ARG_STRING(format,value);

   if (UseDebug) my_log("POLYGLOT OPTION %s VALUE %s\n",option,value);

   opt=option_find(uci->option,option);
   if(opt){
       found=TRUE;
       if(!IS_BUTTON(opt->type)){
           if(!my_string_equal(opt->value,value)){
               engine_send(uci->engine,"setoption name %s value %s",
                           opt->name,value);
               my_string_set(&opt->value,value);
           }else{
               my_log("POLYGLOT Not sending option \"%s\" since it "
                      "already has the correct value.\n",opt->name);
           }
       }else{
           engine_send(uci->engine,"setoption name %s",opt->name);
       }
   }
   return found;
}

// uci_parse()

int uci_parse(uci_t * uci, const char string[]) {

   int event;
   parse_t parse[1];
   char command[StringSize];
   char argument[StringSize];

   ASSERT(uci_is_ok(uci));
   ASSERT(string!=NULL);

   // init

   event = EVENT_NONE;

   // parse

   parse_open(parse,string);

   if (parse_get_word(parse,command,StringSize)) {
       
      parse_get_string(parse,argument,StringSize);
      if (UseDebug) my_log("POLYGLOT COMMAND \"%s\" ARGUMENT \"%s\"\n",command,argument);

      if (FALSE) {

      } else if (my_string_equal(command,"bestmove")) {

         // search end

         ASSERT(uci->pending_nb>0);

         if (uci->searching && uci->pending_nb == 1) {

            // current search

            uci->searching = FALSE;
            uci->pending_nb--;

            event = parse_bestmove(uci,argument); // updates uci->best_move and uci->ponder_move

         } else {

            // obsolete search

            if (uci->pending_nb > 0) {
               uci->pending_nb--;
               if (uci->pending_nb == 0) event = EVENT_STOP;
            }
         }

      } else if (my_string_equal(command,"id")) {

         parse_id(uci,argument);

      } else if (my_string_equal(command,"info")) {

         // search information

              if (uci->searching && uci->pending_nb == 1) { // current search
                  event = parse_info(uci,argument);
               }

      } else if (my_string_equal(command,"option")) {

         parse_option(uci,argument);

      } else if (my_string_equal(command,"readyok")) {

         // engine is ready

         ASSERT(uci->ready_nb>0);

         if (uci->ready_nb > 0) {
            uci->ready_nb--;
            if (uci->ready_nb == 0) event = EVENT_READY;
         }

      } else if (my_string_equal(command,"uciok")) {

         event = EVENT_UCI;

      } else {

         if (UseDebug) my_log("POLYGLOT unknown command \"%s\"\n",command);
      }
   }

   parse_close(parse);

   return event;
}

// parse_bestmove()

static int parse_bestmove(uci_t * uci, const char string[]) {

   parse_t parse[1];
   char command[StringSize];
   char option[StringSize];
   char argument[StringSize];
   board_t board[1];

   ASSERT(uci_is_ok(uci));
   ASSERT(string!=NULL);

   // init

   strcpy(command,"bestmove");

   parse_open(parse,string);
   parse_add_keyword(parse,"ponder");

   // bestmove

   if (!parse_get_string(parse,argument,StringSize)) {
       return EVENT_ILLEGAL_MOVE;
//      my_fatal("parse_bestmove(): missing argument\n");
   }

   uci->best_move = move_from_can(argument,uci->board);
   if (uci->best_move == MoveNone) {
       return EVENT_ILLEGAL_MOVE;
//       my_fatal("parse_bestmove(): not a move \"%s\"\n",argument);
   }

   if(!move_is_legal(uci->best_move,uci->board)){
       return EVENT_ILLEGAL_MOVE;
   }
   ASSERT(uci->best_move!=MoveNone);
   ASSERT(move_is_legal(uci->best_move,uci->board));

   // loop

   while (parse_get_word(parse,option,StringSize)) {

      parse_get_string(parse,argument,StringSize);

      if (UseDebug) my_log("POLYGLOT COMMAND \"%s\" OPTION \"%s\" ARGUMENT \"%s\"\n",command,option,argument);

      if (FALSE) {

      } else if (my_string_equal(option,"ponder")) {

         ASSERT(!my_string_empty(argument));

         board_copy(board,uci->board);
         move_do(board,uci->best_move);

         uci->ponder_move = move_from_can(argument,board);
         // if (uci->ponder_move == MoveNone) my_fatal("parse_bestmove(): not a move \"%s\"\n",argument);

         ASSERT(uci->ponder_move!=MoveNone);
         ASSERT(move_is_legal(uci->ponder_move,board));

      } else {

         my_log("POLYGLOT unknown option \"%s\" for command \"%s\"\n",option,command);
      }
   }

   parse_close(parse);

   return EVENT_MOVE;
}

// parse_id()

static void parse_id(uci_t * uci, const char string[]) {

   parse_t parse[1];
   char command[StringSize];
   char option[StringSize];
   char argument[StringSize];

   ASSERT(uci!=NULL);
   ASSERT(string!=NULL);

   // init

   strcpy(command,"id");

   parse_open(parse,string);
   parse_add_keyword(parse,"author");
   parse_add_keyword(parse,"name");

   // loop

   while (parse_get_word(parse,option,StringSize)) {

      parse_get_string(parse,argument,StringSize);
      if (UseDebug) my_log("POLYGLOT COMMAND \"%s\" OPTION \"%s\" ARGUMENT \"%s\"\n",command,option,argument);

      if (FALSE) {
      } else if (my_string_equal(option,"author")) {
         ASSERT(!my_string_empty(argument));
         my_string_set(&uci->author,argument);
      } else if (my_string_equal(option,"name")) {
         ASSERT(!my_string_empty(argument));
         my_string_set(&uci->name,argument);
      } else {
         my_log("POLYGLOT unknown option \"%s\" for command \"%s\"\n",option,command);
      }
   }

   parse_close(parse);

   if (UseDebug) my_log("POLYGLOT engine name \"%s\" author \"%s\"\n",uci->name,uci->author);
}

// parse_info()

static int parse_info(uci_t * uci, const char string[]) {

   int event;
   parse_t parse[1];
   char command[StringSize];
   char option[StringSize];
   char argument[StringSize];
   int n;
   int multipvline=0;
   sint64 ln;


   
   ASSERT(uci_is_ok(uci));
   ASSERT(string!=NULL);

   // init

   event = EVENT_NONE;

   strcpy(command,"info");

   parse_open(parse,string);
   parse_add_keyword(parse,"cpuload");
   parse_add_keyword(parse,"currline");
   parse_add_keyword(parse,"currmove");
   parse_add_keyword(parse,"currmovenumber");
   parse_add_keyword(parse,"depth");
   parse_add_keyword(parse,"hashfull");
   parse_add_keyword(parse,"multipv");
   parse_add_keyword(parse,"nodes");
   parse_add_keyword(parse,"nps");
   parse_add_keyword(parse,"pv");
   parse_add_keyword(parse,"refutation");
   parse_add_keyword(parse,"score");
   parse_add_keyword(parse,"seldepth");
   parse_add_keyword(parse,"string");
   parse_add_keyword(parse,"tbhits");
   parse_add_keyword(parse,"time");

   // loop

   while (parse_get_word(parse,option,StringSize)) {

      parse_get_string(parse,argument,StringSize);

      if (UseDebug) my_log("POLYGLOT COMMAND \"%s\" OPTION \"%s\" ARGUMENT \"%s\"\n",command,option,argument);

      if (FALSE) {

      } else if (my_string_equal(option,"cpuload")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=0);

         if (n >= 0) uci->cpu = ((double)n) / 1000.0;

      } else if (my_string_equal(option,"currline")) {

         ASSERT(!my_string_empty(argument));

         line_from_can(uci->current_line,uci->board,argument,LineSize);

      } else if (my_string_equal(option,"currmove")) {

         ASSERT(!my_string_empty(argument));

         uci->root_move = move_from_can(argument,uci->board);
         ASSERT(uci->root_move!=MoveNone);

      } else if (my_string_equal(option,"currmovenumber")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=1&&n<=uci->root_move_nb);

         if (n >= 1 && n <= uci->root_move_nb) {
            uci->root_move_pos = n - 1;
            ASSERT(uci->root_move_pos>=0&&uci->root_move_pos<uci->root_move_nb);
         }

      } else if (my_string_equal(option,"depth")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=1);

         if (n >= 0) {
            if (n > uci->depth) event |= EVENT_DEPTH;
            uci->depth = n;
         }

      } else if (my_string_equal(option,"hashfull")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=0);

         if (n >= 0) uci->hash = ((double)n) / 1000.0;

      } else if (my_string_equal(option,"multipv")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
		 if(Uci->multipv_mode) multipvline=n;
        
         ASSERT(n>=1);

      } else if (my_string_equal(option,"nodes")) {

         ASSERT(!my_string_empty(argument));

         ln = my_atoll(argument);
         ASSERT(ln>=0);

         if (ln >= 0) uci->node_nb = ln;

      } else if (my_string_equal(option,"nps")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=0);

         if (n >= 0) uci->speed = ((double)n);

      } else if (my_string_equal(option,"pv")) {

         ASSERT(!my_string_empty(argument));

         line_from_can(uci->pv,uci->board,argument,LineSize);
         event |= EVENT_PV;

      } else if (my_string_equal(option,"refutation")) {

         ASSERT(!my_string_empty(argument));

         line_from_can(uci->pv,uci->board,argument,LineSize);

      } else if (my_string_equal(option,"score")) {

         ASSERT(!my_string_empty(argument));

         parse_score(uci,argument);

      } else if (my_string_equal(option,"seldepth")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=0);

         if (n >= 0) uci->sel_depth = n;

      } else  if (my_string_equal(option,"string")) {
		  if(my_string_case_equal(argument,"DrawOffer")){
			  event |= EVENT_DRAW;
          }else if(my_string_case_equal(argument,"Resign")){
			  event |= EVENT_RESIGN;
          }else{
              snprintf(uci->info,sizeof(uci->info),"%s",argument);
              uci->info[sizeof(uci->info)-1]='\0';
              event|=EVENT_INFO;
          }
         // TODO: argument to EOS

         ASSERT(!my_string_empty(argument));

      } else if (my_string_equal(option,"tbhits")) {

         ASSERT(!my_string_empty(argument));

         ln = my_atoll(argument);
         ASSERT(ln>=0);

      } else if (my_string_equal(option,"time")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n>=0);

         if (n >= 0) uci->time = ((double)n) / 1000.0;

      } else {

         my_log("POLYGLOT unknown option \"%s\" for command \"%s\"\n",option,command);
             // This should probably be protected
             // by a "WorkAround" option.
         snprintf(uci->info,sizeof(uci->info),"%s %s",option,argument);
         uci->info[sizeof(uci->info)-1]='\0';
         event|=EVENT_INFO;
      }
   }

   parse_close(parse);

   // update display
   //lousy uci,filter out lower depth multipv lines that have been repeated from the engine 
   if(multipvline>1 && uci->depth<uci->best_depth) event &= ~EVENT_PV;
   if ((event & EVENT_PV) != 0) {
      uci->best_score = uci->score; 
	  uci->best_depth = uci->depth;
	  if(multipvline==1)uci->depth=-1; //HACK ,clears the engine outpout window,see send_pv in adapter.cpp 
      uci->best_sel_depth = uci->sel_depth;
      line_copy(uci->best_pv,uci->pv);
   }
   return event;
}

// parse_option()

static void parse_option(uci_t * uci, const char string[]) {

   option_t opt[1];
   parse_t parse[1];
   char command[StringSize];
   char option[StringSize];
   char argument[StringSize];

   ASSERT(uci!=NULL);
   ASSERT(string!=NULL);

   // init

   strcpy(command,"option");

   memset(opt,0,sizeof(option_t));
   
   my_string_set(&opt->value,"<empty>");
   my_string_set(&opt->name,"<empty>");
   my_string_set(&opt->default_,"<empty>");
   my_string_set(&opt->max,"<empty>");
   my_string_set(&opt->min,"<empty>");
   my_string_set(&opt->type,"<empty>");
   opt->var_nb=0;
   opt->mode=0;
   
   parse_open(parse,string);
   parse_add_keyword(parse,"default");
   parse_add_keyword(parse,"max");
   parse_add_keyword(parse,"min");
   parse_add_keyword(parse,"name");
   parse_add_keyword(parse,"type");
   parse_add_keyword(parse,"var");

   // loop

   while (parse_get_word(parse,option,StringSize)) {
      parse_get_string(parse,argument,StringSize);
      if (UseDebug) my_log("POLYGLOT COMMAND \"%s\" OPTION \"%s\" ARGUMENT \"%s\"\n",command,option,argument);

      if (FALSE) {

      } else if (my_string_equal(option,"default")) {

         // ASSERT(!my_string_empty(argument)); // HACK for Pepito

         if (!my_string_empty(argument)) {
            my_string_set(&opt->default_,argument);
            my_string_set(&opt->value,argument);
         }

      } else if (my_string_equal(option,"max")) {

         ASSERT(!my_string_empty(argument));
         my_string_set(&opt->max,argument);

      } else if (my_string_equal(option,"min")) {

         ASSERT(!my_string_empty(argument));
         my_string_set(&opt->min,argument);

      } else if (my_string_equal(option,"name")) {

         ASSERT(!my_string_empty(argument));

         if (!my_string_empty(argument)) {
            my_string_set(&opt->name,argument);
         }

      } else if (my_string_equal(option,"type")) {

         ASSERT(!my_string_empty(argument));
         my_string_set(&opt->type,argument);

      } else if (my_string_equal(option,"var")) {

         ASSERT(!my_string_empty(argument));
         my_string_set(&opt->var[opt->var_nb++],argument);
         if(opt->var_nb==VarNb) break;

      } else {

         my_log("POLYGLOT unknown option \"%s\" for command \"%s\"\n",option,command);
      }
   }

   parse_close(parse);

   apply_UCI3_heuristics(opt);
   option_insert(uci->option,opt);
   option_free(opt);

   if (UseDebug) my_log("POLYGLOT option name \"%s\" default \"%s\"\n",opt->name,opt->default_);
}

// parse_score()

static void parse_score(uci_t * uci, const char string[]) {

   parse_t parse[1];
   char command[StringSize];
   char option[StringSize];
   char argument[StringSize];
   int n;

   ASSERT(uci_is_ok(uci));
   ASSERT(string!=NULL);

   // init

   strcpy(command,"score");

   parse_open(parse,string);
   parse_add_keyword(parse,"cp");
   parse_add_keyword(parse,"lowerbound");
   parse_add_keyword(parse,"mate");
   parse_add_keyword(parse,"upperbound");

   // loop

   while (parse_get_word(parse,option,StringSize)) {

      parse_get_string(parse,argument,StringSize);

      if (UseDebug) my_log("POLYGLOT COMMAND \"%s\" OPTION \"%s\" ARGUMENT \"%s\"\n",command,option,argument);

      if (FALSE) {

      } else if (my_string_equal(option,"cp")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);

         uci->score = n;

      } else if (my_string_equal(option,"lowerbound")) {

         ASSERT(my_string_empty(argument));

      } else if (my_string_equal(option,"mate")) {

         ASSERT(!my_string_empty(argument));

         n = atoi(argument);
         ASSERT(n!=0);

         uci->score = mate_score(n);

      } else if (my_string_equal(option,"upperbound")) {

         ASSERT(my_string_empty(argument));

      } else {

         my_log("POLYGLOT unknown option \"%s\" for command \"%s\"\n",option,command);
      }
   }

   parse_close(parse);
}

// mate_score()

static int mate_score(int dist) {

   ASSERT(dist!=0);

   if (FALSE) {
   } else if (dist > 0) {
       return +option_get_int(Option,"MateScore") - (+dist) * 2 + 1;
   } else if (dist < 0) {
       return -option_get_int(Option,"MateScore") + (-dist) * 2;
   }

   return 0;
}

// end of uci.cpp

