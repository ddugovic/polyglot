
// book_make.h

#ifndef BOOK_MAKE_H
#define BOOK_MAKE_H

// includes

#include "board.h"
#include "pgn.h"
#include "util.h"

// types

typedef struct {
    uint64_t key;
    uint16 move;
    uint16 count;
// Unfortunately the minggw32 cross compiler [4.2.1-sjlj (mingw32-2)]
// seems to have a bug with anon structs contained in unions when using -O2.
// See the ASSERT below in "read_entry_file"...
// To be fair this seems to be illegal in C++
// although it is hard to understand why, and the compiler does not complain
// even with -Wall.
//    union {
//        struct {
            uint16 n;
            uint16 sum;
//        };
//        struct {
            uint8 height;
            int line;
//        };
//   };
    uint8 colour;
} entry_t;

typedef struct {
    size_t size;
    uint32 mask;
    entry_t * entry;
    sint32 * hash;
} book_t;

typedef enum {
    BOOK,
    ALL
} search_t;

typedef struct {
    int height;
    int line;
    int initial_color;
    bool book_trans_only;
    bool extended_search;
    const char *fen;
    variant_t variant;
    uint16 moves[1024];
    double probs[1024];
    uint64_t keys[1024];
    FILE *file;
    size_t alloc;
} info_t;

// functions

extern void book_make (int argc, char *argv[]);
extern void book_dump (int argc, char *argv[]);
extern void book_info (int argc, char *argv[]);

extern entry_t * find_entry   (book_t *book, info_t *info, const board_t *board, int move, size_t *index);
extern entry_t * create_entry (book_t *book, info_t *info, const board_t *board, int move);
extern void   resize       (info_t *info);
extern void   halve_stats  (uint64_t key);

extern bool keep_entry  (int pos);
extern int  entry_score (const entry_t *entry);

extern int  key_compare (const void *p1, const void *p2);

extern bool read_entry  (info_t *info, entry_t *entry, int n);
extern void write_entry (info_t *info, const entry_t *entry);
extern void read_entry_file  (FILE *f, entry_t *entry, int n);
extern void write_entry_file (FILE *f, const entry_t * entry);
extern void init_info (info_t *info);

#endif // !defined BOOK_MAKE_H

// end of book_make.h

