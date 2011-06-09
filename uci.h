
// uci.h

#ifndef UCI_H
#define UCI_H

// includes

#include "board.h"
#include "engine.h"
#include "line.h"
#include "move.h"
#include "option.h"
#include "util.h"

// macros

// I need to make a uniform string type.

#define UciStringSize 4096

// types

typedef struct {

   engine_t * engine;

   const char * name;
   const char * author;

   option_list_t option[1];

   bool ready;
   int ready_nb;

   bool searching;
   int pending_nb;

   board_t board[1];

   int best_move;
   int ponder_move;

   int score;
   int depth;
   int sel_depth;
   move_t pv[LineSize];

   int best_score;
   int best_depth;
   int best_sel_depth;
   move_t best_pv[LineSize];

   sint64 node_nb;
   double time;
   double speed;
   double cpu;
   double hash;
   move_t current_line[LineSize];

   int root_move;
   int root_move_pos;
   int root_move_nb;
   bool multipv_mode;
   char info[UciStringSize];
} uci_t;

typedef enum {
   EVENT_NONE         = 0,
   EVENT_UCI          = 1 << 0,
   EVENT_READY        = 1 << 1,
   EVENT_STOP         = 1 << 2,
   EVENT_MOVE         = 1 << 3,
   EVENT_PV           = 1 << 4,
   EVENT_DEPTH        = 1 << 5,
   EVENT_DRAW         = 1 << 6,
   EVENT_RESIGN       = 1 << 7,
   EVENT_ILLEGAL_MOVE = 1 << 8,
   EVENT_INFO         = 1 << 9
} dummy_event_t;

// variables

extern uci_t Uci[1];

// functions

extern void uci_open              (uci_t * uci, engine_t * engine);
extern void uci_send_isready      (uci_t * uci);
extern void uci_send_isready_sync (uci_t * uci);
extern void uci_send_stop         (uci_t * uci);
extern void uci_send_stop_sync    (uci_t * uci);
extern void uci_send_ucinewgame   (uci_t * uci);
extern void uci_set_threads       (uci_t * uci, int n);
extern const char * uci_thread_option(uci_t * uci);
extern bool uci_send_option       (uci_t * uci, const char option[], const char format[], ...);
extern void uci_close             (uci_t * uci);
extern void uci_clear             (uci_t * uci);
extern int  uci_parse             (uci_t * uci, const char string[]);

#endif // !defined UCI_H

// end of uci.h

