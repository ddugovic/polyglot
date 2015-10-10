// book_make.c

// includes

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "book_make.h"
#include "fen.h"
#include "move.h"
#include "move_do.h"
#include "move_gen.h"
#include "move_legal.h"
#include "pgheader.h"
#include "pgn.h"
#include "san.h"
#include "util.h"

// constants

#define COUNT_MAX ((int)16384)

static const int NIL = -1;

// defines

#define opp_search(s) ((s)==BOOK?ALL:BOOK)

// variables

static int MaxPly;
static int MinGame;
static double MinScore;
static bool RemoveWhite, RemoveBlack;
static bool Uniform;
static bool Quiet=FALSE;

static book_t Book[1];
static info_t Info[1];

// prototypes

static void     book_clear    (info_t *info);
static void     book_insert   (const char file_name[]);
static void     book_filter   ();
static void     book_sort     ();
static void     book_save     (const char file_name[]);

static void     write_integer (FILE * file, int size, uint64_t n);
static uint64_t read_integer  (FILE * file, int size);

// functions

// book_make()

void book_make(int argc, char * argv[]) {

   int i;
   const char * pgn_file;
   const char * bin_file;

   info_t *info = &Info[0];
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

   book_clear(info);

   printf("inserting games.");
   book_insert(pgn_file);

   printf("filtering entries.");
   book_filter();

   printf("sorting entries ...\n");
   book_sort();

   printf("saving entries ...\n");
   book_save(bin_file);

   printf("all done!\n");
}

// book_clear()

static void book_clear(info_t *info) {

   int index;

   info->alloc = 1;
   Book->mask = (info->alloc * 2) - 1;

   Book->entry = (entry_t *) my_malloc(info->alloc*sizeof(entry_t));
   Book->size = 0;

   Book->hash = (sint32 *) my_malloc((info->alloc*2)*sizeof(sint32));
   for (index = 0; index < info->alloc*2; index++) {
      Book->hash[index] = NIL;
   }
}

// book_insert()

static void book_insert(const char file_name[]) {

   info_t *info = &Info[0];
   pgn_t pgn[1];
   board_t board[1];
   int ply;
   int result;
   char string[256];
   int move;
   entry_t *entry;

   ASSERT(file_name!=NULL);

   // init

   pgn->game_nb=1;
   // scan loop

   pgn_open(pgn,file_name);

   while (pgn_next_game(pgn)) {

      board_start(board,pgn->fen,pgn->variant);
#ifndef NDEBUG
char fen[256];
board_to_fen(board,fen,256);
fprintf(stderr, "HASH? (%zu) (%zu) [%d] [%s]\n", sizeof(hash_key(NULL)), sizeof(board->key), board->variant, fen);
fprintf(stderr, "HASH? (%zu) (%zu) [%d] [%s]\n", sizeof(hash_key(board)), sizeof(board->key), board->variant, fen);
fprintf(stderr, "HASH? [%"PRIx64"] [%"PRIx64"] [%d] [%s]\n", hash_key(board), board->key, board->variant, fen);
ASSERT(sizeof(board->key)==sizeof(hash_key(board)));
ASSERT(board->key==hash_key(board));
#endif
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

            entry = create_entry(Book,info,board,move);
            entry->n++;
            entry->sum += result+1;

            if (entry->n >= COUNT_MAX) {
               halve_stats(board->key);
            }

            move_do(board,move);
            ply++;
            result = -result;
         }
      }
      pgn->game_nb++;
      if (pgn->game_nb % 1000 == 0) {
         if (pgn->game_nb % 10000 == 0) printf("\n%9zu games", pgn->game_nb);
         printf(".");
         fflush(stdout);
      }
   }
   printf("\n");

   pgn_close(pgn);

   printf("%zu game%s.\n",pgn->game_nb,(pgn->game_nb>1)?"s":"");
   printf("%zu entries.\n",Book->size);

   return;
}

// book_filter()

static void book_filter() {

   int src, dst;

   // entry loop

   dst = 0;

   for (src = 0; src < Book->size; src++) {
      if (keep_entry(src)) Book->entry[dst++] = Book->entry[src];
      if (src+1 % 1000 == 0) {
         if (src+1 % 10000 == 0) printf("\n%9d entries", src+1);
         printf(".");
         fflush(stdout);
      }
   }
   printf("\n");

   ASSERT(dst>=0&&dst<=Book->size);
   Book->size = dst;

   printf("%zu entries.\n",Book->size);
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
   char *header, *raw_header;
   unsigned int size;
   int i;

   ASSERT(file_name!=NULL);

   file = fopen(file_name,"wb");
   if (file == NULL) my_fatal("book_save(): can't open file \"%s\" for writing: %s\n",file_name,strerror(errno));

   pgheader_create(&header,"normal","Created by PolyGlot.");
   pgheader_create_raw(&raw_header,header,&size);
   free(header);

   // write header

   for(i=0;i<size;i++){
       fputc(raw_header[i],file);
   }
   free(raw_header);

   // entry loop

   for (pos = 0; pos < Book->size; pos++) {

      ASSERT(keep_entry(pos));

      /* null keys are reserved for the header */
      if(Book->entry[pos].key==U64(0x0)) continue;

      write_entry_file(file,&Book->entry[pos]);
   }

   fclose(file);
}

// find_entry()

entry_t * find_entry(book_t *book, info_t *info, const board_t *board, int move, size_t *index) {

   size_t pos;

   ASSERT(board!=NULL);
   ASSERT(move==MoveNone || move_is_ok(move));
   ASSERT(move==MoveNone || move_is_legal(move,board));

   for (*index = board->key & (uint64_t) book->mask; (pos=book->hash[*index]) != NIL; *index = (*index+1) & book->mask) {

      ASSERT(pos>=0&&pos<book->size);

      if (book->entry[pos].key == board->key && book->entry[pos].move == move) {
         return &book->entry[pos]; // found
      }
   }

   return NULL;
}

// create_entry()

entry_t * create_entry(book_t *book, info_t *info, const board_t *board, int move) {

   size_t index;
   entry_t * entry;
   size_t pos;

   entry = find_entry(book, info, board, move, &index);
   if (entry!=NULL)
      return entry;

   // not found

   ASSERT(book->size<=info->alloc);

   if (book->size == info->alloc) {

      // allocate more memory

      resize(info);

      for (index = board->key & (uint64_t) book->mask; book->hash[index] != NIL; index = (index+1) & book->mask)
         ;
   }

   // create a new entry

   ASSERT(book->size<=info->alloc);
   pos = book->size++;

   book->entry[pos].key = board->key;
   book->entry[pos].move = move;
   book->entry[pos].n = 0;
   book->entry[pos].sum = 0;
   book->entry[pos].colour = board->turn;

   // insert into the hash table

   ASSERT(index>=0&&index<info->alloc*2);
   ASSERT(book->hash[index]==NIL);
   book->hash[index] = pos;

   ASSERT(pos>=0&&pos<book->size);

   return &book->entry[pos];
}

// rebuild_hash_table

static void rebuild_hash_table(info_t *info){
    int index,pos;
    for (index = 0; index < info->alloc*2; index++) {
        Book->hash[index] = NIL;
    }
    for (pos = 0; pos < Book->size; pos++) {
        for (index = Book->entry[pos].key & (uint64_t) Book->mask; Book->hash[index] != NIL; index = (index+1) & Book->mask)
         ;
        ASSERT(index>=0&&index<info->alloc*2);
        Book->hash[index] = pos;
    }
}

void resize(info_t *info) {

   size_t size;

   info->alloc *= 2;
   Book->mask = (info->alloc * 2) - 1;

   size = info->alloc * sizeof(entry_t);
   size += (info->alloc*2) * sizeof(sint32);

   if (!Quiet && size >= 1048576) {
      printf("(allocating %gMB)",((double)size)/1048576.0);
      fflush(stdout);
   }

   // resize arrays

   Book->entry = (entry_t *) my_realloc(Book->entry,info->alloc*sizeof(entry_t));
   Book->hash = (sint32 *) my_realloc(Book->hash,(info->alloc*2)*sizeof(sint32));

   // rebuild hash table

   rebuild_hash_table(info);
}


// halve_stats()

void halve_stats(uint64_t key) {

   int index;
   int pos;

   // search

   for (index = key & (uint64_t) Book->mask; (pos=Book->hash[index]) != NIL; index = (index+1) & Book->mask) {

      ASSERT(pos>=0&&pos<Book->size);

      if (Book->entry[pos].key == key) {
         Book->entry[pos].n = (Book->entry[pos].n + 1) / 2;
         Book->entry[pos].sum = (Book->entry[pos].sum + 1) / 2;
      }
   }
}

// keep_entry()

bool keep_entry(int pos) {

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

int entry_score(const entry_t * entry) {

   int score;

   ASSERT(entry!=NULL);

   // score = entry->n; // popularity
   score = entry->sum; // "expectancy"

   if (Uniform) score = 1;

   ASSERT(score>=0);

   return score;
}

// key_compare()

int key_compare(const void * p1, const void * p2) {

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

static void write_integer(FILE * file, int size, uint64_t n) {

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

static uint64_t read_integer(FILE * file, int size) {
   uint64_t n;
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

// read_entry()

bool read_entry(info_t *info, entry_t *entry, int n) {

   ASSERT(info!=NULL);
   ASSERT(entry!=NULL);

   if (n < 0 || n >= info->alloc) return FALSE;

   ASSERT(n>=0&&n<info->alloc);

   if (fseek(info->file,n*sizeof(entry_t),SEEK_SET) == -1) {
      my_fatal("read_entry(): fseek(): %s\n",strerror(errno));
   }

   read_entry_file(info->file, entry, n);

   return TRUE;
}

// write_entry()

void write_entry(info_t *info, const entry_t *entry) {

   ASSERT(info!=NULL);
   ASSERT(entry!=NULL);

   write_entry_file(info->file, entry);
}

// read_entry_file

void read_entry_file(FILE *f, entry_t *entry, int n){

    ASSERT(entry!=NULL);
    entry->key   = read_integer(f,8);
    entry->move  = read_integer(f,2);
    entry->count = read_integer(f,2);
    entry->n     = entry->count;//read_integer(f,2);
    entry->sum   = entry->count;//read_integer(f,2);

    ASSERT(entry->move!=MoveNone);
}

// write_entry_file

void write_entry_file(FILE * f, const entry_t * entry) {

   ASSERT(entry!=NULL);
   write_integer(f,8,entry->key);
   write_integer(f,2,entry->move);
   write_integer(f,2,entry_score(entry));
   write_integer(f,2,0);//write_integer(f,2,entry->n);
   write_integer(f,2,0);//write_integer(f,2,entry->sum);
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

    info_t *info = &Info[0];
    entry_t entry[1];
    size_t n;
    size_t m;
    int pos;
    int index;
    ASSERT(filename!=NULL);
    if(!(info->file=fopen(filename,"rb"))){
        my_fatal("book_load() : can't open file \"%s\" for reading: %s\n",filename,strerror(errno));
    }
    fseek(info->file,0L,SEEK_END);   // superportable way to get size of book!
    m = ftell(info->file)/sizeof(entry_t);
    while(info->alloc<m) {
        // allocate more memory
        resize(info);
    }
    for(n=0L;n<m;n++){
        read_entry(info,entry,n);
        ASSERT(Book->size<=info->alloc);
            // insert into the book
        pos = Book->size++;
        Book->entry[pos].key = entry->key;
        Book->entry[pos].move = entry->move;
        Book->entry[pos].count = entry->count;
        Book->entry[pos].n = entry->n;
        Book->entry[pos].sum = entry->sum;
        Book->entry[pos].colour = ColourNone;
            // find free hash table spot
        for (index = entry->key & (uint64_t) Book->mask;
             Book->hash[index] != NIL;
             index = (index+1) & Book->mask);
            // insert into the hash table
        ASSERT(index>=0&&index<info->alloc*2);
        ASSERT(Book->hash[index]==NIL);
        Book->hash[index] = pos;
        ASSERT(pos>=0&&pos<Book->size);
    }
    fclose(info->file);
}

// gen_book_moves()
// similar signature as gen_legal_moves
static int gen_book_moves(list_t * list, const board_t * board){
    int first_pos, pos, index;
    entry_t entry[1];
    bool found;
    list_clear(list);
    found=FALSE;
    for (index = board->key & (uint64_t) Book->mask; (first_pos=Book->hash[index]) != NIL; index = (index+1) & Book->mask) {
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
    if(!info->file){
        return;
    }
    board_start(board,info->fen,info->variant);
    for(i=0;i<info->height;i++){
        if(color==White){
            fprintf(info->file,"%d. ",i/2+1);
            color=Black;
        }else{
            color=White;
        }
        move_to_san(info->moves[i],board,move_string,256);
        fprintf(info->file,"%s", move_string);
        if(color==colour_opp(info->initial_color)){
            fprintf(info->file,"{%.0f%%} ",100*info->probs[i]);
        }else{
            fprintf(info->file," ");
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
    size_t index;
    entry_t *entry;
    int prob_sum;
    double probs[256];
    for(i=0;i<256;i++){
        probs[i]=0.0;  // kill compiler warnings
    }
    for(i=0;i<info->height;i++){
        if(board->key==info->keys[i]){
            if(info->file){
                fprintf(info->file,"%d: ",info->line);
                print_moves(info);
                fprintf(info->file,"{cycle: ply=%d}\n",i);
            }
            info->line++;
            if(!Quiet && info->line%1000==0){printf("."); fflush(stdout);}
            return 1; // end of line because of cycle
        }
    }
    if(!info->book_trans_only || (info->book_trans_only && search==BOOK)){
        info->keys[info->height]=board->key;
        entry=find_entry(Book,info,board,MoveNone,&index);
        if(entry!=NULL){
            if(info->file){
                fprintf(info->file,"%d: ",info->line);
                print_moves(info);
                fprintf(info->file,"{trans: line=%d, ply=%d}\n",
                        entry->line,entry->height);
            }
            info->line++;
            if(!Quiet && info->line%1000==0){printf("."); fflush(stdout);}
            return 1; // end of line because of transposition
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
            if(info->file){
                fprintf(info->file,"%d: ",info->line);
                print_moves(info);
                fprintf(info->file,"\n");
            }
            info->line++;
            if(!Quiet && info->line%1000==0){printf("."); fflush(stdout);}
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
    info->file=NULL;
    info->initial_color=White;
    info->book_trans_only=FALSE;
}

// book_clean()
// remove MoveNone entries from book and rebuild hash table
void book_clean(info_t *info){
    int read_ptr,write_ptr;
    write_ptr=0;
    for(read_ptr=0;read_ptr<Book->size;read_ptr++){
        if(Book->entry[read_ptr].move!=MoveNone){
            Book->entry[write_ptr++]=Book->entry[read_ptr];
        }
    }
    Book->size=write_ptr;
    rebuild_hash_table(info);
}

// book_dump()

void book_dump(int argc, char * argv[]) {

    info_t *info = Info;
    const char * bin_file=NULL;
    const char * txt_file=NULL;
    char string[StringSize];
    int color=ColourNone;
    board_t board[1];
    int i;
    int n;
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

    book_clear(info);
    if(!Quiet){printf("Loading book %s...\n",bin_file);}
    book_load(bin_file);
    init_info(info);
    info->initial_color=color;
    if(!(f=fopen(txt_file,"w"))){
        my_fatal("book_dump(): can't open file \"%s\" for writing: %s",
                 txt_file,strerror(errno));
    }
    info->file=f;
    fprintf(info->file,"Dump of \"%s\" for %s.\n",
            bin_file,color==White?"white":"black");
    if(color==White){
        if(!Quiet){printf("generating lines for white...\n");}
        board_start(board,info->fen=StartFen,info->variant);
        if (!board_to_fen(board,string,StringSize)) ASSERT(FALSE);
        fprintf(info->file,"[FEN \"%s\"]\n",string);
        info->line = 1;
        search_book(board,info,BOOK);
        for (n=0; n<sizeof(StartFen960)/sizeof(StartFen960[0]); n++) {
            board_start(board,info->fen=StartFen960[n],info->variant);
            if (!board_to_fen(board,string,StringSize)) ASSERT(FALSE);
            fprintf(info->file,"[FEN \"%s\"]\n",string);
            info->line = 1;
            search_book(board,info,BOOK);
        }
    }else{
        if(!Quiet){printf("generating lines for black...\n");}
        board_start(board,info->fen=StartFen,info->variant);
        if (!board_to_fen(board,string,StringSize)) ASSERT(FALSE);
        fprintf(info->file,"[FEN \"%s\"]\n",string);
        info->line = 1;
        search_book(board,info,ALL);
        for (n=0; n<sizeof(StartFen960)/sizeof(StartFen960[0]); n++) {
            board_start(board,info->fen=StartFen960[n],info->variant);
            if (!board_to_fen(board,string,StringSize)) ASSERT(FALSE);
            fprintf(info->file,"[FEN \"%s\"]\n",string);
            info->line = 1;
            search_book(board,info,ALL);
        }
    }
}

// book_info()

void book_info(int argc,char* argv[]){

    info_t *info = Info;
    const char *bin_file=NULL;
    board_t board[1];
    uint64_t last_key;
    int pos;
    int white_pos,black_pos,total_pos,white_pos_extended,
        black_pos_extended,white_pos_extended_diff,black_pos_extended_diff;
    int s;
    bool extended_search=FALSE;
    int i;
    int n;
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
    book_clear(info);
    if(!Quiet){printf("Loading book %s...",bin_file);}
    book_load(bin_file);
    if(!Quiet){printf("\n");}
    s=Book->size;

    init_info(info);
    info->book_trans_only=FALSE;
    info->initial_color=White;
    info->extended_search=FALSE;
    if(!Quiet){printf("Scanning book.");}
    board_start(board,info->fen=StartFen,info->variant=NORMAL);
    search_book(board,info,BOOK);
    for (n=0; n<sizeof(StartFen960)/sizeof(StartFen960[0]); n++) {
        board_start(board,info->fen=StartFen960[n],info->variant=FISCHER);
        search_book(board,info,BOOK);
    }
    printf("\nLines for white                : %8d\n",info->line-1);

    info->line=1;
    info->height=0;
    info->initial_color=Black;
    book_clean(info);
    ASSERT(Book->size==s);
    if(!Quiet){printf("Scanning book.");}
    board_start(board,info->fen=StartFen,info->variant=NORMAL);
    search_book(board,info,ALL);
    for (n=0; n<sizeof(StartFen960)/sizeof(StartFen960[0]); n++) {
        board_start(board,info->fen=StartFen960[n],info->variant=FISCHER);
        search_book(board,info,ALL);
    }
    printf("\nLines for black                : %8d\n",info->line-1);

    book_clean(info);
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
        book_clean(info);
        if(!Quiet){printf("Scanning book.");}
        board_start(board,info->fen=StartFen,info->variant=NORMAL);
        search_book(board,info,BOOK);
        for (n=0; n<sizeof(StartFen960)/sizeof(StartFen960[0]); n++) {
            board_start(board,info->fen=StartFen960[n],info->variant=FISCHER);
            search_book(board,info,BOOK);
        }
        init_info(info);
        info->book_trans_only=TRUE;
        info->initial_color=Black;
        info->extended_search=TRUE;
        book_clean(info);
        board_start(board,info->fen=StartFen,info->variant=NORMAL);
        search_book(board,info,ALL);
        for (n=0; n<sizeof(StartFen960)/sizeof(StartFen960[0]); n++) {
            board_start(board,info->fen=StartFen960[n],info->variant=FISCHER);
            search_book(board,info,ALL);
        }
        book_clean(info);
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
        printf("\nUnreachable white positions(?) : %8d\n",
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

