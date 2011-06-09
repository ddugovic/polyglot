// search.h

#ifndef SEARCH_H
#define SEARCH_H

// includes

#include "board.h"
#include "util.h"

// defines

#define DepthMax 63

// functions

extern void search       (const board_t * board, int depth_max, double time_max);
extern void search_perft (const board_t * board, int depth_max);

extern void do_perft     (int argc, char * argv[]);

#endif // !defined SEARCH_H

// end of search.h

