
// move.h

#ifndef MOVE_H
#define MOVE_H

// includes

#include "board.h"
#include "util.h"

// defined

// HACK: a1a1 cannot be a legal move
#define MoveNone (0) 

#define MovePromoteKnight  (1 << 12)
#define MovePromoteBishop  (2 << 12)
#define MovePromoteRook    (3 << 12)
#define MovePromoteQueen   (4 << 12)
#define MoveFlags          (7 << 12)

// types

typedef uint16 move_t;

// functions

extern bool move_is_ok          (int move);

extern int  move_make           (int from, int to);
extern int  move_make_flags     (int from, int to, int flags);

extern int  move_from           (int move);
extern int  move_to             (int move);
extern int  move_promote_hack   (int move);

extern bool move_is_capture     (int move, const board_t * board);
extern bool move_is_promote     (int move);
extern bool move_is_en_passant  (int move, const board_t * board);
extern bool move_is_castle      (int move, const board_t * board);

extern int  move_piece          (int move, const board_t * board);
extern int  move_capture        (int move, const board_t * board);
extern int  move_promote        (int move, const board_t * board);

extern bool move_is_check       (int move, const board_t * board);
extern bool move_is_mate        (int move, const board_t * board);

extern int  move_order          (int move);

extern bool move_to_can         (int move, const board_t * board, char string[], int size);
extern int  move_from_can       (const char string[], const board_t * board);

extern void move_disp           (int move, const board_t * board);

#endif // !defined MOVE_H

// end of move.h

