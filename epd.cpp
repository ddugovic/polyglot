
// epd.cpp

// includes

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "board.h"
#include "engine.h"
#include "epd.h"
#include "fen.h"
#include "line.h"
#include "main.h"
#include "move.h"
#include "move_legal.h"
#include "option.h"
#include "parse.h"
#include "san.h"
#include "uci.h"
#include "util.h"

// constants

static const bool UseDebug = false;
static const bool UseTrace = false;

static const int StringSize = 4096;

// variables

static int MinDepth;
static int MaxDepth;

static double MaxTime;
static double MinTime;

static int DepthDelta;

static int FirstMove;
static int FirstDepth;
static int FirstSelDepth;
static int FirstScore;
static double FirstTime;
static uint64 FirstNodeNb;
static move_t FirstPV[LineSize];

static int LastMove;
static int LastDepth;
static int LastSelDepth;
static int LastScore;
static double LastTime;
static uint64 LastNodeNb;
static move_t LastPV[LineSize];

static my_timer_t Timer[1];

// prototypes

static void epd_test_file  (const char file_name[]);

static bool is_solution    (int move, const board_t * board, const char bm[], const char am[]);
static bool string_contain (const char string[], const char substring[]);

static bool engine_step    ();

// functions

// epd_test()

void epd_test(int argc, char * argv[]) {

   int i;
   const char * epd_file;

   epd_file = NULL;
   my_string_set(&epd_file,"wac.epd");

   MinDepth = 8;
   MaxDepth = 63;

   MinTime = 1.0;
   MaxTime = 5.0;

   DepthDelta = 3;

   for (i = 1; i < argc; i++) {

      if (false) {

      } else if (my_string_equal(argv[i],"epd-test")) {

         // skip

      } else if (my_string_equal(argv[i],"-epd")) {

         i++;
         if (argv[i] == NULL) my_fatal("epd_test(): missing argument\n");

         my_string_set(&epd_file,argv[i]);

      } else if (my_string_equal(argv[i],"-min-depth")) {

         i++;
         if (argv[i] == NULL) my_fatal("epd_test(): missing argument\n");

         MinDepth = atoi(argv[i]);

      } else if (my_string_equal(argv[i],"-max-depth")) {

         i++;
         if (argv[i] == NULL) my_fatal("epd_test(): missing argument\n");

         MaxDepth = atoi(argv[i]);

      } else if (my_string_equal(argv[i],"-min-time")) {

         i++;
         if (argv[i] == NULL) my_fatal("epd_test(): missing argument\n");

         MinTime = atof(argv[i]);

      } else if (my_string_equal(argv[i],"-max-time")) {

         i++;
         if (argv[i] == NULL) my_fatal("epd_test(): missing argument\n");

         MaxTime = atof(argv[i]);

      } else if (my_string_equal(argv[i],"-depth-delta")) {

         i++;
         if (argv[i] == NULL) my_fatal("epd_test(): missing argument\n");

         DepthDelta = atoi(argv[i]);

      } else {

         my_fatal("epd_test(): unknown option \"%s\"\n",argv[i]);
      }
   }

   if(MinTime>MaxTime){
       MaxTime=MinTime;
   }
  
   epd_test_file(epd_file);
}

// epd_test_file()

static void epd_test_file(const char file_name[]) {

   FILE * file;
   int hit, tot;
   char epd[StringSize];
   char am[StringSize], bm[StringSize], id[StringSize];
   board_t board[1];
   char string[StringSize];
   int move;
   char pv_string[StringSize];
   bool correct;
   double depth_tot, time_tot, node_tot;
   int line=0;

   ASSERT(file_name!=NULL);

   // init

   file = fopen(file_name,"r");
   if (file == NULL) my_fatal("epd_test_file(): can't open file \"%s\": %s\n",file_name,strerror(errno));

   hit = 0;
   tot = 0;

   depth_tot = 0.0;
   time_tot = 0.0;
   node_tot = 0.0;

   printf("\nEngineName=%s\n",option_get_string("EngineName"));

   printf("\n[Search parameters: MaxDepth=%d   MaxTime=%.1f   DepthDelta=%d   MinDepth=%d   MinTime=%.1f]\n\n",MaxDepth,MaxTime,DepthDelta,MinDepth,MinTime);

   // loop

   while (my_file_read_line(file,epd,StringSize)) {
       line++;
       if(my_string_whitespace(epd)) continue;
      if (UseTrace) printf("%s\n",epd);

      if (!epd_get_op(epd,"am",am,StringSize)) strcpy(am,"");
      if (!epd_get_op(epd,"bm",bm,StringSize)) strcpy(bm,"");
      if (!epd_get_op(epd,"id",id,StringSize)) strcpy(id,"");

      if (my_string_empty(am) && my_string_empty(bm)) {
          my_fatal("epd_test(): no am or bm field at line %d\n",line);
      }

      // init

      uci_send_ucinewgame(Uci);
      uci_send_isready_sync(Uci);

      ASSERT(!Uci->searching);

      // position
      if (!board_from_fen(board,epd)) ASSERT(false);
      if (!board_to_fen(board,string,StringSize)) ASSERT(false);

      engine_send(Engine,"position fen %s",string);

      // search

      my_timer_start(Timer); // also resets
      
      // which ones of the next two alternatives is best?
      engine_send(Engine,"go movetime %.0f depth %d",MaxTime*1000.0,MaxDepth);
      //engine_send(Engine,"go infinite");

      // engine data

      board_copy(Uci->board,board);

      uci_clear(Uci);
      Uci->searching = true;
      Uci->pending_nb++;

      FirstMove = MoveNone;
      FirstDepth = 0;
      FirstSelDepth = 0;
      FirstScore = 0;
      FirstTime = 0.0;
      FirstNodeNb = 0;
      line_clear(FirstPV);

      LastMove = MoveNone;
      LastDepth = 0;
      LastSelDepth = 0;
      LastScore = 0;
      LastTime = 0.0;
      LastNodeNb = 0;
      line_clear(LastPV);

      // parse engine output

      while (!engine_eof(Engine) && engine_step()) {
          bool stop=false;

         // stop search?
//          printf("Uci->time=%.2f time=%.2f\n",Uci->time,my_timer_elapsed_real(Timer));
          if (Uci->depth > MaxDepth){
              my_log("POLYGLOT Maximum depth %d reached\n",MaxDepth);
              stop=true;
          }else if(my_timer_elapsed_real(Timer) >= MaxTime){
              my_log("POLYGLOT Maximum search time %.2fs reached\n",MaxTime);
              stop=true;
          }else if(Uci->depth - FirstDepth >= DepthDelta){
              if(Uci->depth > MinDepth){
                  if(Uci->time >= MinTime){
                      if(is_solution(FirstMove,board,bm,am)){
                          my_log("POLYGLOT Solution found\n",MaxTime);
                          stop=true;
                      }
                  }
              }
          }
          if(stop){
              my_log("POLYGLOT Stopping engine\n");
              engine_send(Engine,"stop");
              break;
          }
      }
      
      move = FirstMove;
      correct = is_solution(move,board,bm,am);

      if (correct) hit++;
      tot++;

      if (correct) {
         depth_tot += double(FirstDepth);
         time_tot += FirstTime;
         node_tot += double(sint64(FirstNodeNb));
      }

      printf("%2d: %-15s %s %4d",tot,id,correct?"OK":"--",hit);

      if (!line_to_san(LastPV,Uci->board,pv_string,StringSize)) ASSERT(false);
      printf(" score=%+6.2f    pv [D=%2d, T=%7.2fs, N=%6dk] =%s\n",double(LastScore)/100.0,FirstDepth,FirstTime,(int)FirstNodeNb/1000,pv_string);
   }

   printf("\nscore=%d/%d",hit,tot);

   if (hit != 0) {

      depth_tot /= double(hit);
      time_tot /= double(hit);
      node_tot /= double(hit);

      printf(" [averages on correct positions: depth=%.1f time=%.2f nodes=%.0f]",depth_tot,time_tot,node_tot);
   }

   printf("\n");

   fclose(file);
   quit();
}

// is_solution()

static bool is_solution(int move, const board_t * board, const char bm[], const char am[]) {

   char move_string[256];
   bool correct;

   ASSERT(move!=MoveNone);
   ASSERT(bm!=NULL);
   ASSERT(am!=NULL);

   if (!move_is_legal(move,board)) {
      board_disp(board);
      move_disp(move,board);
      printf("\n\n");
   }

   ASSERT(move_is_legal(move,board));

   if (!move_to_san(move,board,move_string,256)) ASSERT(false);

   correct = false;
   if (!my_string_empty(bm)) {
      correct = string_contain(bm,move_string);
   } else if (!my_string_empty(am)) {
      correct = !string_contain(am,move_string);
   } else {
      ASSERT(false);
   }

   return correct;
}

// epd_get_op()

bool epd_get_op(const char record[], const char opcode[], char string[], int size) {

   char op[256];
   int len;
   const char *p_start, *p_end;

   ASSERT(record!=NULL);
   ASSERT(opcode!=NULL);
   ASSERT(string!=NULL);
   ASSERT(size>0);

   // find the opcode

   sprintf(op," %s ",opcode); 

   p_start = strstr(record,op);
   if (p_start == NULL){
          sprintf(op,";%s ",opcode); 
          p_start = strstr(record,op);
          if (p_start == NULL){
              return false;
          }
   }

   // skip the opcode

   p_start += strlen(op);

   // find the end
   p_end = strchr(p_start,';');
   if (p_end == NULL) return false;

   // calculate the length

   len = p_end - p_start;
   if (size < len+1) my_fatal("epd_get_op(): size < len+1\n");

   strncpy(string,p_start,len);
   string[len] = '\0';

   return true;
}

// string_contain()

static bool string_contain(const char string[], const char substring[]) {

   char new_string[StringSize], *p;

   strcpy(new_string,string); // HACK

   for (p = strtok(new_string," "); p != NULL; p = strtok(NULL," ")) {
      if (my_string_equal(p,substring)) return true;
   }

   return false;
}

// engine_step()

static bool engine_step() {

   char string[StringSize];
   int event;

   engine_get(Engine,string,StringSize);
   event = uci_parse(Uci,string);

   if ((event & EVENT_MOVE) != 0) {

      return false;
   }

   if ((event & EVENT_PV) != 0) {

      LastMove = Uci->best_pv[0];
      LastDepth = Uci->best_depth;
      LastSelDepth = Uci->best_sel_depth;
      LastScore = Uci->best_score;
      LastTime = Uci->time;
      LastNodeNb = Uci->node_nb;
      line_copy(LastPV,Uci->best_pv);

      if (LastMove != FirstMove) {
         FirstMove = LastMove;
         FirstDepth = LastDepth;
         FirstSelDepth = LastSelDepth;
         FirstScore = LastScore;
         FirstTime = LastTime;
         FirstNodeNb = LastNodeNb;
         line_copy(FirstPV,LastPV);
      }
   }

   return true;
}

// end of epd.cpp

