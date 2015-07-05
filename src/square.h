
// square.h

#ifndef SQUARE_H
#define SQUARE_H

// includes

#include "util.h"

// defines

#define SquareNb (16 * 12)

#define FileA 0
#define FileB 1
#define FileC 2
#define FileD 3
#define FileE 4
#define FileF 5
#define FileG 6
#define FileH 7

#define Rank1 0
#define Rank2 1
#define Rank3 2
#define Rank4 3
#define Rank5 4
#define Rank6 5
#define Rank7 6
#define Rank8 7

#define SquareNone 0

#define A1 0x24
#define B1 0x25
#define C1 0x26
#define D1 0x27
#define E1 0x28
#define F1 0x29
#define G1 0x2A
#define H1 0x2B
#define A2 0x34
#define B2 0x35
#define C2 0x36
#define D2 0x37
#define E2 0x38
#define F2 0x39
#define G2 0x3A
#define H2 0x3B
#define A3 0x44
#define B3 0x45
#define C3 0x46
#define D3 0x47
#define E3 0x48
#define F3 0x49
#define G3 0x4A
#define H3 0x4B
#define A4 0x54
#define B4 0x55
#define C4 0x56
#define D4 0x57
#define E4 0x58
#define F4 0x59
#define G4 0x5A
#define H4 0x5B
#define A5 0x64
#define B5 0x65
#define C5 0x66
#define D5 0x67
#define E5 0x68
#define F5 0x69
#define G5 0x6A
#define H5 0x6B
#define A6 0x74
#define B6 0x75
#define C6 0x76
#define D6 0x77
#define E6 0x78
#define F6 0x79
#define G6 0x7A
#define H6 0x7B
#define A7 0x84
#define B7 0x85
#define C7 0x86
#define D7 0x87
#define E7 0x88
#define F7 0x89
#define G7 0x8A
#define H7 0x8B
#define A8 0x94
#define B8 0x95
#define C8 0x96
#define D8 0x97
#define E8 0x98
#define F8 0x99
#define G8 0x9A
#define H8 0x9B

#define Dark  0
#define Light 1

// functions

extern void square_init        ();

extern bool square_is_ok       (int square);

extern int  square_make        (int file, int rank);

extern int  square_file        (int square);
extern int  square_rank        (int square);
extern int  square_side_rank   (int square, int colour);

extern int  square_from_64     (int square);
extern int  square_to_64       (int square);

extern bool square_is_promote  (int square);
extern int  square_ep_dual     (int square);

extern int  square_colour      (int square);

extern bool char_is_file       (int c);
extern bool char_is_rank       (int c);

extern int  file_from_char     (int c);
extern int  rank_from_char     (int c);

extern int  file_to_char       (int file);
extern int  rank_to_char       (int rank);

extern bool square_to_string   (int square, char string[], int size);
extern int  square_from_string (const char string[]);

#endif // !defined SQUARE_H

// end of square.h

