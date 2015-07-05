
// piece.h

#ifndef PIECE_H
#define PIECE_H

// includes

#include "colour.h"
#include "util.h"

// defines

#define BlackPawnFlag     (1 << 2)
#define WhitePawnFlag     (1 << 3)
#define KnightFlag        (1 << 4)
#define BishopFlag        (1 << 5)
#define RookFlag          (1 << 6)
#define KingFlag          (1 << 7)

#define PawnFlags   (BlackPawnFlag | WhitePawnFlag)
#define QueenFlags  (BishopFlag | RookFlag)

#define PieceNone64     (0)
#define BlackPawn64     (BlackPawnFlag)
#define WhitePawn64     (WhitePawnFlag)
#define Knight64        (KnightFlag)
#define Bishop64        (BishopFlag)
#define Rook64          (RookFlag)
#define Queen64         (QueenFlags)
#define King64          (KingFlag)

#define  PieceNone256      (0)
#define  BlackPawn256      (BlackPawn64 | Black)
#define  WhitePawn256      (WhitePawn64 | White)
#define  BlackKnight256    (Knight64    | Black)
#define  WhiteKnight256    (Knight64    | White)
#define  BlackBishop256    (Bishop64    | Black)
#define  WhiteBishop256    (Bishop64    | White)
#define  BlackRook256      (Rook64      | Black)
#define  WhiteRook256      (Rook64      | White)
#define  BlackQueen256     (Queen64     | Black)
#define  WhiteQueen256     (Queen64     | White)
#define  BlackKing256      (King64      | Black)
#define  WhiteKing256      (King64      | White)

#define BlackPawn12        (0)
#define WhitePawn12        (1)
#define BlackKnight12      (2)
#define WhiteKnight12      (3)
#define BlackBishop12      (4)
#define WhiteBishop12      (5)
#define BlackRook12        (6)
#define WhiteRook12        (7)
#define BlackQueen12       (8)
#define WhiteQueen12       (9)
#define BlackKing12       (10)
#define WhiteKing12       (11)

// functions

extern void piece_init      ();

extern bool piece_is_ok     (int piece);

extern int  piece_make_pawn (int colour);
extern int  piece_pawn_opp  (int piece);

extern int  piece_colour    (int piece);
extern int  piece_type      (int piece);

extern bool piece_is_pawn   (int piece);
extern bool piece_is_knight (int piece);
extern bool piece_is_bishop (int piece);
extern bool piece_is_rook   (int piece);
extern bool piece_is_queen  (int piece);
extern bool piece_is_king   (int piece);

extern bool piece_is_slider (int piece);

extern int  piece_to_12     (int piece);
extern int  piece_from_12   (int piece);

extern int  piece_to_char   (int piece);
extern int  piece_from_char (int c);

extern bool char_is_piece   (int c);

#endif // !defined PIECE_H

// end of piece.h

