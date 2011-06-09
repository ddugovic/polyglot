// search.c

// includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attack.h"
#include "board.h"
#include "colour.h"
#include "engine.h"
#include "fen.h"
#include "line.h"
#include "list.h"
#include "move.h"
#include "move_do.h"
#include "move_gen.h"
#include "move_legal.h"
#include "option.h"
#include "parse.h"
#include "san.h"
#include "search.h"
#include "uci.h"
#include "util.h"

// variables

static int Depth;

static int BestMove;
static int BestValue;
static move_t BestPV[LineSize];

static sint64 NodeNb;
static sint64 LeafNb;
static double Time;

static int Move;
static int MovePos;
static int MoveNb;

// prototypes

static bool depth_is_ok (int depth);
static void perft       (const board_t * board, int depth);

// functions

// depth_is_ok()

static bool depth_is_ok(int depth) {

   return depth >= 0 && depth < DepthMax;
}

// search()

void search(const board_t * board, int depth_max, double time_max) {

   char string[256];

   ASSERT(board_is_ok(board));
   ASSERT(depth_max>=1&&depth_max<DepthMax);
   ASSERT(time_max>=0.0);

   // engine

   Depth = 0;

   BestMove = MoveNone;
   BestValue = 0;
   line_clear(BestPV);

   NodeNb = 0;
   LeafNb = 0;
   Time = 0.0;

   Move = MoveNone;
   MovePos = 0;
   MoveNb = 0;

   // init

   uci_send_ucinewgame(Uci);
   uci_send_isready_sync(Uci);

   // position

   if (!board_to_fen(board,string,256)) ASSERT(FALSE);
   engine_send(Engine,"position fen %s",string);

   // search

   engine_send_queue(Engine,"go");

   engine_send_queue(Engine," movetime %.0f",time_max*1000.0);
   engine_send_queue(Engine," depth %d",depth_max);

   engine_send(Engine,""); // newline

   // wait for feed-back

   while (!engine_eof(Engine)) {

      engine_get(Engine,string);

      if (FALSE) {

      } else if (match(string,"bestmove * ponder *")) {

         BestMove = move_from_can(Star[0],board);
         ASSERT(BestMove!=MoveNone&&move_is_legal(BestMove,board));

         break;

      } else if (match(string,"bestmove *")) {

         BestMove = move_from_can(Star[0],board);
         ASSERT(BestMove!=MoveNone&&move_is_legal(BestMove,board));

         break;
      }
   }

   printf("\n");
}

// do_perft()

void do_perft(int argc,char * argv[]){
    const char * fen=NULL;
    int depth=1;
    board_t board[1];
    int i;
    for (i = 1; i < argc; i++) {
        if (FALSE) {
        } else if (my_string_equal(argv[i],"perft")) {
                // skip
        } else if (my_string_equal(argv[i],"-fen")) {
            i++;
            if (argv[i] == NULL) my_fatal("do_perft(): missing argument\n");
            my_string_set(&fen,argv[i]);
        } else if (my_string_equal(argv[i],"-max-depth")){
            i++;
            if (argv[i] == NULL) my_fatal("do_perft(): missing argument\n");
            depth=atoi(argv[i]);
            if(depth<1) my_fatal("do_perft(): illegal depth %d\n",depth);
        } else {
            my_fatal("do_perft(): unknown option \"%s\"\n",argv[i]);
        }
    }
    if(fen==NULL){
        my_string_set(&fen,StartFen);
    }
    board_from_fen(board,fen);
    search_perft(board,depth);
}

// search_perft()

void search_perft(const board_t * board, int depth_max) {

   int depth;
   my_timer_t timer[1];
   double time, speed;
   char node_string[StringSize];
   char leafnode_string[StringSize];

   ASSERT(board_is_ok(board));
   ASSERT(depth_max>=1&&depth_max<DepthMax);

   // init

   board_disp(board);

   // iterative deepening

   for (depth = 1; depth <= depth_max; depth++) {

      // init

      NodeNb = 0;
      LeafNb = 0;

      my_timer_reset(timer);

      my_timer_start(timer);
      perft(board,depth);
      my_timer_stop(timer);

      time = my_timer_elapsed_real(timer);//my_timer_elapsed_cpu(timer);
      speed = (time < 0.01) ? 0.0 : ((double)NodeNb) / time;

      snprintf(node_string,StringSize,S64_FORMAT,NodeNb);
      snprintf(leafnode_string,StringSize,S64_FORMAT,LeafNb);

      printf("depth=%2d nodes=%12s leafnodes=%12s time=%7.2fs nps=%8.0f\n",depth,node_string,leafnode_string,time,speed);
   }

}

// perft()

static void perft(const board_t * board, int depth) {

   int me;
   list_t list[1];
   int i, move;
   board_t new_board[1];

   ASSERT(board_is_ok(board));
   ASSERT(depth_is_ok(depth));

   ASSERT(!is_in_check(board,colour_opp(board->turn)));

   // init

   NodeNb++;

   // leaf

   if (depth == 0) {
      LeafNb++;
      return;
   }

   // more init

   me = board->turn;

   // move loop

   gen_moves(list,board);

   for (i = 0; i < list_size(list); i++) {

      move = list_move(list,i);

      board_copy(new_board,board);
      move_do(new_board,move);

      if (!is_in_check(new_board,me)) perft(new_board,depth-1);
   }
}

// end of search.cpp

