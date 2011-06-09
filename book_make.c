// book_make.c

// includes

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "book_make.h"
#include "move.h"
#include "move_do.h"
#include "move_gen.h"
#include "move_legal.h"
#include "pgn.h"
#include "san.h"
#include "util.h"

// constants

#define COUNT_MAX ((int)16384)

static const int NIL = -1;

// defines

#define opp_search(s) ((s)==BOOK?ALL:BOOK)

// types

typedef struct {
    uint64 key;
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
   int size;
   int alloc;
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
    uint16 moves[1024];
    double probs[1024];
    uint64 keys[1024];
    FILE *output;
} info_t;


// variables

static int MaxPly;
static int MinGame;
static double MinScore;
static bool RemoveWhite, RemoveBlack;
static bool Uniform;
static bool Quiet=FALSE;

static book_t Book[1];

// prototypes

static void   book_clear    ();
static void   book_insert   (const char file_name[]);
static void   book_filter   ();
static void   book_sort     ();
static void   book_save     (const char file_name[]);

static int    find_entry    (const board_t * board, int move);
static void   resize        ();
static void   halve_stats   (uint64 key);

static bool   keep_entry    (int pos);

static int    entry_score    (const entry_t * entry);

static int    key_compare   (const void * p1, const void * p2);

static void   write_integer (FILE * file, int size, uint64 n);
static uint64 read_integer(FILE * file, int size);

static void read_entry_file(FILE *f, entry_t *entry);
static void write_entry_file(FILE * f, const entry_t * entry);

// functions

// book_make()

void book_make(int argc, char * argv[]) {

   int i;
   const char * pgn_file;
   const char * bin_file;

   pgn_file = NULL;
   my_string_set(&pgn_file,"book.pgn");

   bin_file = NULL;
   my_string_set(&bin_file,"book.bin");

   MaxPly = 1024;
   MinGame = 3;
   MinScore = 0.0;
   RemoveWhite = FALSE;
   RemoveBlack = FALSE;
   Uniform = FALSE;

   for (i = 1; i < argc; i++) {

      if (FALSE) {

      } else if (my_string_equal(argv[i],"make-book")) {

         // skip

      } else if (my_string_equal(argv[i],"-pgn")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_make(): missing argument\n");

         my_string_set(&pgn_file,argv[i]);

      } else if (my_string_equal(argv[i],"-bin")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_make(): missing argument\n");

         my_string_set(&bin_file,argv[i]);

      } else if (my_string_equal(argv[i],"-max-ply")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_make(): missing argument\n");

         MaxPly = atoi(argv[i]);
         ASSERT(MaxPly>=0);

      } else if (my_string_equal(argv[i],"-min-game")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_make(): missing argument\n");

         MinGame = atoi(argv[i]);
         ASSERT(MinGame>0);

      } else if (my_string_equal(argv[i],"-min-score")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_make(): missing argument\n");

         MinScore = atof(argv[i]) / 100.0;
         ASSERT(MinScore>=0.0&&MinScore<=1.0);

      } else if (my_string_equal(argv[i],"-only-white")) {

         RemoveWhite = FALSE;
         RemoveBlack = TRUE;

      } else if (my_string_equal(argv[i],"-only-black")) {

         RemoveWhite = TRUE;
         RemoveBlack = FALSE;

      } else if (my_string_equal(argv[i],"-uniform")) {

         Uniform = TRUE;

      } else {

         my_fatal("book_make(): unknown option \"%s\"\n",argv[i]);
      }
   }

   book_clear();

   printf("inserting games ...\n");
   book_insert(pgn_file);

   printf("filtering entries ...\n");
   book_filter();

   printf("sorting entries ...\n");
   book_sort();

   printf("saving entries ...\n");
   book_save(bin_file);

   printf("all done!\n");
}

// book_clear()

static void book_clear() {

   int index;

   Book->alloc = 1;
   Book->mask = (Book->alloc * 2) - 1;

   Book->entry = (entry_t *) my_malloc(Book->alloc*sizeof(entry_t));
   Book->size = 0;

   Book->hash = (sint32 *) my_malloc((Book->alloc*2)*sizeof(sint32));
   for (index = 0; index < Book->alloc*2; index++) {
      Book->hash[index] = NIL;
   }
}

// book_insert()

static void book_insert(const char file_name[]) {

   pgn_t pgn[1];
   board_t board[1];
   int ply;
   int result;
   char string[256];
   int move;
   int pos;

   ASSERT(file_name!=NULL);

   // init

   pgn->game_nb=1;
   // scan loop

   pgn_open(pgn,file_name);

   while (pgn_next_game(pgn)) {

      board_start(board);
      ply = 0;
      result = 0;

      if (FALSE) {
      } else if (my_string_equal(pgn->result,"1-0")) {
         result = +1;
      } else if (my_string_equal(pgn->result,"0-1")) {
         result = -1;
      }

      while (pgn_next_move(pgn,string,256)) {

         if (ply < MaxPly) {

            move = move_from_san(string,board);

            if (move == MoveNone || !move_is_legal(move,board)) {
               my_fatal("book_insert(): illegal move \"%s\" at line %d, column %d,game %d\n",string,pgn->move_line,pgn->move_column,pgn->game_nb);
            }

            pos = find_entry(board,move);

            Book->entry[pos].n++;
            Book->entry[pos].sum += result+1;

            if (Book->entry[pos].n >= COUNT_MAX) {
               halve_stats(board->key);
            }

            move_do(board,move);
            ply++;
            result = -result;
         }
      }
	  pgn->game_nb++;
      if (pgn->game_nb % 10000 == 0) printf("%d games ...\n",pgn->game_nb);
   }

   pgn_close(pgn);

   printf("%d game%s.\n",pgn->game_nb,(pgn->game_nb>2)?"s":"");
   printf("%d entries.\n",Book->size);

   return;
}

// book_filter()

static void book_filter() {

   int src, dst;

   // entry loop

   dst = 0;

   for (src = 0; src < Book->size; src++) {
      if (keep_entry(src)) Book->entry[dst++] = Book->entry[src];
   }

   ASSERT(dst>=0&&dst<=Book->size);
   Book->size = dst;

   printf("%d entries.\n",Book->size);
}

// book_sort()

static void book_sort() {

   // sort keys for binary search

   qsort(Book->entry,Book->size,sizeof(entry_t),&key_compare);
}

// book_save()

static void book_save(const char file_name[]) {

   FILE * file;
   int pos;

   ASSERT(file_name!=NULL);

   file = fopen(file_name,"wb");
   if (file == NULL) my_fatal("book_save(): can't open file \"%s\" for writing: %s\n",file_name,strerror(errno));

   // entry loop

   for (pos = 0; pos < Book->size; pos++) {

      ASSERT(keep_entry(pos));

      write_integer(file,8,Book->entry[pos].key);
      write_integer(file,2,Book->entry[pos].move);
      write_integer(file,2,entry_score(&Book->entry[pos]));
      write_integer(file,2,0);
      write_integer(file,2,0);
   }

   fclose(file);
}

// find_entry()

static int find_entry(const board_t * board, int move) {

   uint64 key;
   int index;
   int pos;

   ASSERT(board!=NULL);
   ASSERT(move==MoveNone || move_is_ok(move));

   ASSERT(move==MoveNone || move_is_legal(move,board));

   // init

   key = board->key;

   // search

   for (index = key & (uint64) Book->mask; (pos=Book->hash[index]) != NIL; index = (index+1) & Book->mask) {

      ASSERT(pos>=0&&pos<Book->size);

      if (Book->entry[pos].key == key && Book->entry[pos].move == move) {
         return pos; // found
      }
   }

   // not found

   ASSERT(Book->size<=Book->alloc);

   if (Book->size == Book->alloc) {

      // allocate more memory

      resize();

      for (index = key & (uint64) Book->mask; Book->hash[index] != NIL; index = (index+1) & Book->mask)
         ;
   }

   // create a new entry

   ASSERT(Book->size<Book->alloc);
   pos = Book->size++;

   Book->entry[pos].key = key;
   Book->entry[pos].move = move;
   Book->entry[pos].n = 0;
   Book->entry[pos].sum = 0;
   Book->entry[pos].colour = board->turn;

   // insert into the hash table

   ASSERT(index>=0&&index<Book->alloc*2);
   ASSERT(Book->hash[index]==NIL);
   Book->hash[index] = pos;

   ASSERT(pos>=0&&pos<Book->size);

   return pos;
}

// rebuild_hash_table

static void rebuild_hash_table(){
    int index,pos;
    for (index = 0; index < Book->alloc*2; index++) {
        Book->hash[index] = NIL;
    }
    for (pos = 0; pos < Book->size; pos++) {
        for (index = Book->entry[pos].key & (uint64) Book->mask; Book->hash[index] != NIL; index = (index+1) & Book->mask)
         ;
        ASSERT(index>=0&&index<Book->alloc*2);
        Book->hash[index] = pos;
    }
}

static void resize() {

   int size;

   ASSERT(Book->size==Book->alloc);

   Book->alloc *= 2;
   Book->mask = (Book->alloc * 2) - 1;

   size = 0;
   size += Book->alloc * sizeof(entry_t);
   size += (Book->alloc*2) * sizeof(sint32);

   if (size >= 1048576) if(!Quiet){
           printf("allocating %gMB ...\n",((double)size)/1048576.0);
       }

   // resize arrays

   Book->entry = (entry_t *) my_realloc(Book->entry,Book->alloc*sizeof(entry_t));
   Book->hash = (sint32 *) my_realloc(Book->hash,(Book->alloc*2)*sizeof(sint32));

   // rebuild hash table

   rebuild_hash_table();
}


// halve_stats()

static void halve_stats(uint64 key) {

   int index;
   int pos;

   // search

   for (index = key & (uint64) Book->mask; (pos=Book->hash[index]) != NIL; index = (index+1) & Book->mask) {

      ASSERT(pos>=0&&pos<Book->size);

      if (Book->entry[pos].key == key) {
         Book->entry[pos].n = (Book->entry[pos].n + 1) / 2;
         Book->entry[pos].sum = (Book->entry[pos].sum + 1) / 2;
      }
   }
}

// keep_entry()

static bool keep_entry(int pos) {

   const entry_t * entry;
   int colour;
   double score;

   ASSERT(pos>=0&&pos<Book->size);

   entry = &Book->entry[pos];

   // if (entry->n == 0) return FALSE;
   if (entry->n < MinGame) return FALSE;

   if (entry->sum == 0) return FALSE;

   score = (((double)entry->sum) / ((double)entry->n)) / 2.0;
   ASSERT(score>=0.0&&score<=1.0);

   if (score < MinScore) return FALSE;

   colour = entry->colour;

   if ((RemoveWhite && colour_is_white(colour))
    || (RemoveBlack && colour_is_black(colour))) {
      return FALSE;
   }

   if (entry_score(entry) == 0) return FALSE; // REMOVE ME?

   return TRUE;
}

// entry_score()

static int entry_score(const entry_t * entry) {

   int score;

   ASSERT(entry!=NULL);

   // score = entry->n; // popularity
   score = entry->sum; // "expectancy"

   if (Uniform) score = 1;

   ASSERT(score>=0);

   return score;
}

// key_compare()

static int key_compare(const void * p1, const void * p2) {

   const entry_t * entry_1, * entry_2;

   ASSERT(p1!=NULL);
   ASSERT(p2!=NULL);

   entry_1 = (const entry_t *) p1;
   entry_2 = (const entry_t *) p2;

   if (entry_1->key > entry_2->key) {
      return +1;
   } else if (entry_1->key < entry_2->key) {
      return -1;
   } else {
      return entry_score(entry_2) - entry_score(entry_1); // highest score first
   }
}

// write_integer()

static void write_integer(FILE * file, int size, uint64 n) {

   int i;
   int b;

   ASSERT(file!=NULL);
   ASSERT(size>0&&size<=8);
   ASSERT(size==8||n>>(size*8)==0);

   for (i = size-1; i >= 0; i--) {
      b = (n >> (i*8)) & 0xFF;
      ASSERT(b>=0&&b<256);
      fputc(b,file);
   }
}

// read_integer()

static uint64 read_integer(FILE * file, int size) {
   uint64 n;
   int i;
   int b;
   ASSERT(file!=NULL);
   ASSERT(size>0&&size<=8);
   n = 0;
   for (i = 0; i < size; i++) {
      b = fgetc(file);
      if (b == EOF) {
         if (feof(file)) {
            my_fatal("read_integer(): fgetc(): EOF reached\n");
         } else { // error
            my_fatal("read_integer(): fgetc(): %s\n",strerror(errno));
         }
      }
      ASSERT(b>=0&&b<256);
      n = (n << 8) | b;
   }
   return n;
}

// read_entry_file

static void read_entry_file(FILE *f, entry_t *entry){
    uint64 n;
    ASSERT(entry!=NULL);
    n = entry->key   = read_integer(f,8);
    entry->move  = read_integer(f,2);
    entry->count = read_integer(f,2);
    entry->n     = read_integer(f,2);
    entry->sum   = read_integer(f,2);
    ASSERT(n==entry->key); // test for mingw compiler bug with anon structs
}

// write_entry_file

static void write_entry_file(FILE * f, const entry_t * entry) {
   ASSERT(entry!=NULL);
   write_integer(f,8,entry->key);
   write_integer(f,2,entry->move);
   write_integer(f,2,entry->count);
   write_integer(f,2,entry->n);
   write_integer(f,2,entry->sum);
}

static void print_list(const board_t *board, list_t *list){
    int i;
    uint16 move;
    char move_string[256];
    for (i = 0; i < list_size(list); i++) {
        move = list_move(list,i);
        move_to_san(move,board,move_string,256);
        printf("%s",move_string);
    }
    printf("\n");
}

// book_load()
// loads a polyglot book

static void book_load(const char filename[]){
    FILE* f;
    entry_t entry[1];
    int size;
    int i;
    int pos;
    int index;
    ASSERT(filename!=NULL);
    if(!(f=fopen(filename,"rb"))){
        my_fatal("book_load() : can't open file \"%s\" for reading: %s\n",filename,strerror(errno));
    }
    fseek(f,0L,SEEK_END);   // superportable way to get size of book!
    size=ftell(f)/16;
    fseek(f,0,SEEK_SET);
    for(i=0L;i<size;i++){
        read_entry_file(f,entry);
        ASSERT(Book->size<=Book->alloc);
        if (Book->size == Book->alloc) {
                // allocate more memoryx
            resize();
        }
            // insert into the book
        pos = Book->size++;
        Book->entry[pos].key = entry->key;
        ASSERT(entry->move!=MoveNone);
        Book->entry[pos].move = entry->move;
        Book->entry[pos].count = entry->count;
        Book->entry[pos].n = entry->n;
        Book->entry[pos].sum = entry->sum;
        Book->entry[pos].colour = ColourNone;
            // find free hash table spot
        for (index = entry->key & (uint64) Book->mask;
             Book->hash[index] != NIL;
             index = (index+1) & Book->mask);
            // insert into the hash table
        ASSERT(index>=0&&index<Book->alloc*2);
        ASSERT(Book->hash[index]==NIL);
        Book->hash[index] = pos;
        ASSERT(pos>=0&&pos<Book->size);
    }
    fclose(f);
}

// gen_book_moves()
// similar signature as gen_legal_moves
static int gen_book_moves(list_t * list, const board_t * board){
    int first_pos, pos, index;
    entry_t entry[1];
    bool found;
    list_clear(list);
    found=FALSE;
    for (index = board->key & (uint64) Book->mask; (first_pos=Book->hash[index]) != NIL; index = (index+1) & Book->mask) {
        ASSERT(first_pos>=0&&first_pos<Book->size);
        if (Book->entry[first_pos].key == board->key) {
            found=TRUE;
            break; // found
        }
    }
    if(!found) return -1;
    if(Book->entry[first_pos].move==MoveNone) return -1;
    for (pos = first_pos; pos < Book->size; pos++) {
        *entry=Book->entry[pos];
        if (entry->key != board->key) break;
        if (entry->count > 0 &&
            entry->move != MoveNone &&
            move_is_legal(entry->move,board)) {
            list_add_ex(list,entry->move,entry->count);
        }
    }
    return first_pos;
}

// gen_opp_book_moves()
// moves to which opponent has a reply in book
// similar signature as gen_legal_moves
static void gen_opp_book_moves(list_t * list, const board_t * board){
    int move;
    list_t new_list[1], legal_moves[1];
    board_t new_board[1];
    int i;
    list_clear(list);
    gen_legal_moves(legal_moves,board);
    for (i = 0; i < list_size(legal_moves); i++) {
        move = list_move(legal_moves,i);
            // scratch_board
        memcpy(new_board, board, sizeof(board_t));
        move_do(new_board,move);
        gen_book_moves(new_list,new_board); // wasteful in time but tested!
        if(list_size(new_list)!=0){
            list_add(list,move);
        }
    }
}

static void print_moves(info_t *info){
    board_t board[1];
    char move_string[256];
    int i;
    int color=White;
    if(!info->output){
        return;
    }
    board_start(board);
    for(i=0;i<info->height;i++){
        if(color==White){
            fprintf(info->output,"%d. ",i/2+1);
            color=Black;
        }else{
            color=White;
        }
        move_to_san(info->moves[i],board,move_string,256);
        fprintf(info->output,"%s", move_string);
        if(color==colour_opp(info->initial_color)){
            fprintf(info->output,"{%.0f%%} ",100*info->probs[i]);
        }else{
            fprintf(info->output," ");
        }
        move_do(board,info->moves[i]);
    }
}

static int search_book(board_t *board, info_t *info, search_t search){
    list_t list[1];
    board_t new_board[1];
    uint16 move;
    int count;
    int ret;
    int i;
    int offset;
    int pos;
    int size;
    int prob_sum;
    double probs[256];
    for(i=0;i<256;i++){
        probs[i]=0.0;  // kill compiler warnings
    }
    for(i=0;i<info->height;i++){
        if(board->key==info->keys[i]){
            if(info->output){
                fprintf(info->output,"%d: ",info->line);
                print_moves(info);
                fprintf(info->output,"{cycle: ply=%d}\n",i);
            }
            info->line++;
            return 1; // end of line because of cycle
        }
    }
    if(!info->book_trans_only || (info->book_trans_only && search==BOOK)){
        info->keys[info->height]=board->key;
        size=Book->size;  // hack
        pos=find_entry(board,MoveNone);
        if(size==Book->size){
            if(info->output){
                fprintf(info->output,"%d: ",info->line);
                print_moves(info);
                fprintf(info->output,"{trans: line=%d, ply=%d}\n",
                        Book->entry[pos].line,
                        Book->entry[pos].height);
            }
            info->line++;
            return 1; // end of line because of transposition
        }else{
            Book->entry[pos].height=info->height;
            Book->entry[pos].line=info->line;
        }
    }
    count=0;
    if(search==BOOK){
        offset=gen_book_moves(list,board);
        if(info->extended_search){
            gen_legal_moves(list,board);
        }
//        ASSERT(offset!=-1);
        if(offset!=-1){ // only FALSE in starting position for black book
            Book->entry[offset].colour=board->turn;
            prob_sum=0;
            if(!info->extended_search){
                for(i=0;i<list_size(list);i++){
                    prob_sum+=((uint16)list_value(list,i));
                }
                for(i=0;i<list_size(list);i++){
                    probs[i]=((double)((uint16)list_value(list,i)))/((double)prob_sum);
                }
            }
        }
    }else{
        gen_opp_book_moves(list,board);
    }
    for (i = 0; i < list_size(list); i++) {
        move = list_move(list,i);
        memcpy(new_board, board, sizeof(board_t));
        ASSERT(move_is_legal(move,new_board));
        move_do(new_board,move);
        ASSERT(search!=opp_search(search));
        info->moves[info->height++]=move;
        if(search==BOOK){
            info->probs[info->height-1]=probs[i];
        }
        ret=search_book(new_board, info, opp_search(search));
        if(ret==0 && search==BOOK){
            if(info->output){
                fprintf(info->output,"%d: ",info->line);
                print_moves(info);
                fprintf(info->output,"\n");
            }
            info->line++;
            ret=1; // end of line book move counts for 1
        }
        info->height--;
        ASSERT(info->height>=0);
        count+=ret;
    }
    return count;
}

void init_info(info_t *info){
    info->line=1;
    info->height=0;
    info->output=NULL;
    info->initial_color=White;
    info->book_trans_only=FALSE;
}

// book_clean()
// remove MoveNone entries from book and rebuild hash table
void book_clean(){
    int read_ptr,write_ptr;
    write_ptr=0;
    for(read_ptr=0;read_ptr<Book->size;read_ptr++){
        if(Book->entry[read_ptr].move!=MoveNone){
            Book->entry[write_ptr++]=Book->entry[read_ptr];
        }
    }
    Book->size=write_ptr;
    rebuild_hash_table();
}

// book_dump()

void book_dump(int argc, char * argv[]) {
    const char * bin_file=NULL;
    const char * txt_file=NULL;
    char string[StringSize];
    int color=ColourNone;
    board_t board[1];
    info_t info[1];
    int i;
    FILE *f;
    my_string_set(&bin_file,"book.bin");
    for (i = 1; i < argc; i++) {
        if (FALSE) {
        } else if (my_string_equal(argv[i],"dump-book")) {
                // skip
        } else if (my_string_equal(argv[i],"-bin")) {
            i++;
            if (i==argc) my_fatal("book_dump(): missing argument\n");
            my_string_set(&bin_file,argv[i]);
        } else if (my_string_equal(argv[i],"-out")) {
            i++;
            if (i==argc) my_fatal("book_dump(): missing argument\n");
            my_string_set(&txt_file,argv[i]);
        } else if (my_string_equal(argv[i],"-color") || my_string_equal(argv[i],"-colour")) {
            i++;
            if (i == argc) my_fatal("book_dump(): missing argument\n");
            if(my_string_equal(argv[i],"white")){
                color=White;
            }else if (my_string_equal(argv[i],"black")){
                color=Black;
            }else{
                my_fatal("book_dump(): unknown color \"%s\"\n",argv[i]);
            }
        } else {
            my_fatal("book_dump(): unknown option \"%s\"\n",argv[i]);
        }
    }
    if(color==ColourNone){
        my_fatal("book_dump(): you must specify a color\n");
    }
    if(txt_file==NULL){
        snprintf(string,StringSize,"book_%s.txt",(color==White)?"white":"black");
        my_string_set(&txt_file,string);
    }

    book_clear();
    if(!Quiet){printf("loading book ...\n");}
    book_load(bin_file);
    board_start(board);
    init_info(info);
    info->initial_color=color;
    if(!(f=fopen(txt_file,"w"))){
        my_fatal("book_dump(): can't open file \"%s\" for writing: %s",
                 txt_file,strerror(errno));
    }
    info->output=f;
    fprintf(info->output,"Dump of \"%s\" for %s.\n",
            bin_file,color==White?"white":"black");
    if(color==White){
        if(!Quiet){printf("generating lines for white...\n");}
        search_book(board,info, BOOK);
    }else{
        if(!Quiet){printf("generating lines for black...\n");}
        search_book(board,info, ALL);
    }
}

// book_info()

void book_info(int argc,char* argv[]){
    const char *bin_file=NULL;
    board_t board[1];
    info_t info[1];
    uint64 last_key;
    int pos;
    int white_pos,black_pos,total_pos,white_pos_extended,
        black_pos_extended,white_pos_extended_diff,black_pos_extended_diff;
    int s;
    bool extended_search=FALSE;
    int i;
    Quiet=TRUE;
    my_string_set(&bin_file,"book.bin");

    for (i = 1; i < argc; i++) {
        if (FALSE) {
        } else if (my_string_equal(argv[i],"info-book")) {
                // skip
        } else if (my_string_equal(argv[i],"-bin")) {
            i++;
            if (i==argc) my_fatal("book_info(): missing argument\n");
            my_string_set(&bin_file,argv[i]);
        } else if (my_string_equal(argv[i],"-exact")) {
            extended_search=TRUE;
        } else {
            my_fatal("book_info(): unknown option \"%s\"\n",argv[i]);
        }
    }
    book_clear();
    if(!Quiet){printf("loading book ...\n");}
    book_load(bin_file);
    s=Book->size;

    board_start(board);
    init_info(info);
    info->book_trans_only=FALSE;
    info->initial_color=White;
    info->extended_search=FALSE;
    search_book(board,info, BOOK);
    printf("Lines for white                : %8d\n",info->line-1);


    info->line=1;
    info->height=0;
    info->initial_color=Black;
    book_clean();
    ASSERT(Book->size==s);
    board_start(board);
    search_book(board,info, ALL);
    printf("Lines for black                : %8d\n",info->line-1);

    book_clean();
    ASSERT(Book->size==s);
    white_pos=0;
    black_pos=0;
    total_pos=0;
    last_key=0;
    for(pos=0;pos<Book->size;pos++){
        if(Book->entry[pos].key==last_key){
            ASSERT(Book->entry[pos].colour==ColourNone);
            continue;
        }
        last_key=Book->entry[pos].key;
        total_pos++;
        if(Book->entry[pos].colour==White){
            white_pos++;
        }else if(Book->entry[pos].colour==Black){
            black_pos++;
        }
    }
    printf("Positions on lines for white   : %8d\n",white_pos);
    printf("Positions on lines for black   : %8d\n",black_pos);

    
    if(extended_search){
        init_info(info);
        info->book_trans_only=TRUE;
        info->initial_color=White;
        info->extended_search=TRUE;
        book_clean();
        board_start(board);
        search_book(board,info, BOOK);

        init_info(info);
        info->book_trans_only=TRUE;
        info->initial_color=Black;
        info->extended_search=TRUE;
        book_clean();
        board_start(board);
        search_book(board,info, ALL);
        book_clean();
        ASSERT(Book->size==s);
        white_pos_extended=0;
        black_pos_extended=0;
        last_key=0;
        for(pos=0;pos<Book->size;pos++){
            if(Book->entry[pos].key==last_key){
                ASSERT(Book->entry[pos].colour==ColourNone);
                continue;
            }
            last_key=Book->entry[pos].key;
            if(Book->entry[pos].colour==White){
                white_pos_extended++;
            }else if(Book->entry[pos].colour==Black){
                black_pos_extended++;
            }
        }
        white_pos_extended_diff=white_pos_extended-white_pos;
        black_pos_extended_diff=black_pos_extended-black_pos;
        printf("Unreachable white positions(?) : %8d\n",
               white_pos_extended_diff);
        printf("Unreachable black positions(?) : %8d\n",
               black_pos_extended_diff);

    }
    if(extended_search){
        printf("Isolated positions             : %8d\n",
               total_pos-white_pos_extended-black_pos_extended);
    }else{
        printf("Isolated positions             : %8d\n",
               total_pos-white_pos-black_pos);
    }
}



// end of book_make.cpp

