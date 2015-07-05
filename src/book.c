
// book.c

// includes

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "book.h"
#include "move.h"
#include "move_legal.h"
#include "san.h"
#include "util.h"
#include "option.h"

// types

typedef struct {
   uint64 key;
   uint16 move;
   uint16 count;
   uint16 n;
   uint16 sum;
} entry_t;

// variables

static FILE * BookFile;
static int BookSize;

// prototypes

static int    find_pos      (uint64 key);

static void   read_entry    (entry_t * entry, int n);
static void   write_entry   (const entry_t * entry, int n);

static uint64 read_integer  (FILE * file, int size);
static void   write_integer (FILE * file, int size, uint64 n);

// functions

// book_clear()

void book_clear() {

   BookFile = NULL;
   BookSize = 0;
}

bool book_is_open(){
    return BookFile!=NULL;
}

// book_open()

void book_open(const char file_name[]) {

   ASSERT(file_name!=NULL);
   if(option_get_bool(Option,"BookLearn")){
       BookFile = fopen(file_name,"rb+");
   }else{
       BookFile = fopen(file_name,"rb");
   }

//   if (BookFile == NULL) my_fatal("book_open(): can't open file \"%s\": %s\n",file_name,strerror(errno));
   if (BookFile == NULL)  return;

   if (fseek(BookFile,0,SEEK_END) == -1) {
      my_fatal("book_open(): fseek(): %s\n",strerror(errno));
   }

   BookSize = ftell(BookFile) / 16;
//   if (BookSize == 0) my_fatal("book_open(): empty file\n");
   if (BookSize == 0) {
      book_close();
      book_clear();
   };
}

// book_close()

void book_close() {

   if(BookFile==NULL) return;

   if (fclose(BookFile) == EOF) {
      my_fatal("book_close(): fclose(): %s\n",strerror(errno));
   }
}

// is_in_book()

bool is_in_book(const board_t * board) {

   int pos;
   entry_t entry[1];

   if(BookFile==NULL) return FALSE;

   ASSERT(board!=NULL);

   for (pos = find_pos(board->key); pos < BookSize; pos++) {
      read_entry(entry,pos);
      if (entry->key == board->key) return TRUE;
   }

   return FALSE;
}

// book_move()

int book_move(const board_t * board, bool random) {

   int best_move;
   int best_score;
   int pos;
   entry_t entry[1];
   int move;
   int score;
   list_t list[1];
   int i;

   if(BookFile==NULL) return MoveNone;

   ASSERT(board!=NULL);
   ASSERT(random==TRUE||random==FALSE);

   // init

   list_clear(list);

   book_moves(list,board);

   best_move = MoveNone;
   best_score = 0;
   for(i=0; i<list_size(list); i++){

      move = list->move[i];
      score = list->value[i];

      if (move != MoveNone &&
          move_is_legal(move,board) &&
          score>10*option_get_int(Option,"BookTreshold")) {

         // pick this move?

         ASSERT(score>0);

         if (random) {
            best_score += score;
            if (my_random_int(best_score) < score) best_move = move;
         } else {
            if (score > best_score) {
               best_move = move;
               best_score = score;
            }
         }

      } else {

         ASSERT(FALSE);
      }
   }

   return best_move;
}

// book_moves()

void book_moves(list_t * list, const board_t * board) {

   int first_pos;
   double sum;
   int pos;
   entry_t entry[1];
   int move;
   int score;
   uint32 weight[1000]; // [HGM] assumes not more than 1000 book moves per position!
   char move_string[256];

   ASSERT(board!=NULL);
   ASSERT(list!=NULL);

   if(BookFile==NULL) return;

   // init

   list_clear(list);

   first_pos = find_pos(board->key);

   // sum

   sum = 0;

   for (pos = first_pos; pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      weight[pos - first_pos] = 1000 * (uint32)entry->count;
      if(option_get_bool(Option,"BookLearn")) // [HGM] improvised use of learn info
          weight[pos - first_pos] *= ((uint32)entry->n + 10.) /((uint32)entry->sum + 1.);
      sum += weight[pos - first_pos];
   }

   // disp

   for (pos = first_pos; pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      move = entry->move;
      score = (10000.*weight[pos-first_pos])/sum;

      if (move != MoveNone && move_is_legal(move,board)) {
              list_add_ex(list,move,score);
      }
   }

}


// book_disp()

void book_disp(const board_t * board) {

   char move_string[256];
   list_t list[1];
   int i;
   int treshold=option_get_int(Option,"BookTreshold");

   ASSERT(board!=NULL);

   if(BookFile==NULL) return;

   book_moves(list,board);

   for(i=0; i<list_size(list); i++){
       move_to_san(list->move[i],board,move_string,256);
       if(list->value[i]>10*treshold){
           printf(" %6s %5.2f%%\n",move_string,list->value[i]/100.0);
       }else{
           printf(" %6s %5.2f%% (below treshold %4.2f%%)\n",
                  move_string,list->value[i]/100.0,treshold/10.0);
       }
   }
   // this is necessary by the xboard protocol
   printf("\n");
}

// book_learn_move()

void book_learn_move(const board_t * board, int move, int result) {

   int pos;
   entry_t entry[1];

   if(BookFile==NULL) return;

   ASSERT(board!=NULL);
   ASSERT(move_is_ok(move));
   ASSERT(result>=-1&&result<=+1);

   ASSERT(move_is_legal(move,board));

   for (pos = find_pos(board->key); pos < BookSize; pos++) {

      read_entry(entry,pos);
      if (entry->key != board->key) break;

      if (entry->move == move) {

         entry->n++;
         entry->sum += result+1;

         write_entry(entry,pos);

         break;
      }
   }
}

// book_flush()

void book_flush() {

   if(BookFile==NULL) return;

   if (fflush(BookFile) == EOF) {
      my_fatal("book_flush(): fflush(): %s\n",strerror(errno));
   }
}

// find_pos()

static int find_pos(uint64 key) {

   int left, right, mid;
   entry_t entry[1];

   // binary search (finds the leftmost entry)

   left = 0;
   right = BookSize-1;

   ASSERT(left<=right);

   while (left < right) {

      mid = (left + right) / 2;
      ASSERT(mid>=left&&mid<right);

      read_entry(entry,mid);

      if (key <= entry->key) {
         right = mid;
      } else {
         left = mid+1;
      }
   }

   ASSERT(left==right);

   read_entry(entry,left);

   return (entry->key == key) ? left : BookSize;
}

// read_entry()

static void read_entry(entry_t * entry, int n) {

   ASSERT(entry!=NULL);
   ASSERT(n>=0&&n<BookSize);

   if (fseek(BookFile,n*16,SEEK_SET) == -1) {
      my_fatal("read_entry(): fseek(): %s\n",strerror(errno));
   }

   entry->key   = read_integer(BookFile,8);
   entry->move  = read_integer(BookFile,2);
   entry->count = read_integer(BookFile,2);
   entry->n     = read_integer(BookFile,2);
   entry->sum   = read_integer(BookFile,2);
}

// write_entry()

static void write_entry(const entry_t * entry, int n) {

   ASSERT(entry!=NULL);
   ASSERT(n>=0&&n<BookSize);

   if (fseek(BookFile,n*16,SEEK_SET) == -1) {
      my_fatal("write_entry(): fseek(): %s\n",strerror(errno));
   }

   write_integer(BookFile,8,entry->key);
   write_integer(BookFile,2,entry->move);
   write_integer(BookFile,2,entry->count);
   write_integer(BookFile,2,entry->n);
   write_integer(BookFile,2,entry->sum);
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

      if (fputc(b,file) == EOF) {
         my_fatal("write_integer(): fputc(): %s\n",strerror(errno));
      }
   }
}

// end of book.cpp

