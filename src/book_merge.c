
// book_merge.c

// includes

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "book_make.h"
#include "book_merge.h"
#include "util.h"

// variables

static info_t In1[1];
static info_t In2[1];
static info_t Out[1];

// prototypes

static void     book_open     (info_t * book, const char file_name[], const char mode[]);
static void     book_close    (info_t * book);

// functions

// book_merge()

void book_merge(int argc, char * argv[]) {

   int i;
   const char * in_file_1;
   const char * in_file_2;
   const char * out_file;
   int i1, i2;
   bool b1, b2;
   entry_t e1[1], e2[1];

   in_file_1 = NULL;
   my_string_clear(&in_file_1);

   in_file_2 = NULL;
   my_string_clear(&in_file_2);

   out_file = NULL;
   my_string_set(&out_file,"out.bin");

   for (i = 1; i < argc; i++) {

      if (FALSE) {

      } else if (my_string_equal(argv[i],"merge-book")) {

         // skip

      } else if (my_string_equal(argv[i],"-in1")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_merge(): missing argument\n");

         my_string_set(&in_file_1,argv[i]);

      } else if (my_string_equal(argv[i],"-in2")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_merge(): missing argument\n");

         my_string_set(&in_file_2,argv[i]);

      } else if (my_string_equal(argv[i],"-out")) {

         i++;
         if (argv[i] == NULL) my_fatal("book_merge(): missing argument\n");

         my_string_set(&out_file,argv[i]);

      } else {

         my_fatal("book_merge(): unknown option \"%s\"\n",argv[i]);
      }
   }

   init_info(In1);
   init_info(In2);
   init_info(Out);

   book_open(In1,in_file_1,"rb");
   book_open(In2,in_file_2,"rb");
   book_open(Out,out_file,"wb");

   i1 = 0;
   i2 = 0;

   while (TRUE) {

      b1 = read_entry(In1,e1,i1);
      b2 = read_entry(In2,e2,i2);

      if (FALSE) {

      } else if (!b1 && !b2) {

         break;

      } else if (b1 && !b2) {

         write_entry(Out,e1);
         i1++;

      } else if (b2 && !b1) {

         write_entry(Out,e2);
         i2++;

      } else {

         ASSERT(b1);
         ASSERT(b2);

         // ASSUME duplicate entries aren't a problem
         if (key_compare(e1,e2) < 0) {
            write_entry(Out, e1);
            i1++;
         } else {
            write_entry(Out, e2);
            i2++;
         }
      }
   }

   book_close(In1);
   book_close(In2);
   book_close(Out);

   printf("done!\n");
}

// book_open()

static void book_open(info_t * book, const char file_name[], const char mode[]) {

   ASSERT(book!=NULL);
   ASSERT(file_name!=NULL);
   ASSERT(mode!=NULL);

   book->file = fopen(file_name,mode);
   if (book->file == NULL) my_fatal("book_open(): can't open file \"%s\": %s\n",file_name,strerror(errno));

   if (fseek(book->file,0,SEEK_END) == -1) {
      my_fatal("book_open(): fseek(): %s\n",strerror(errno));
   }

   book->alloc = ftell(book->file) / 16;
}

// book_close()

static void book_close(info_t * book) {

   ASSERT(book!=NULL);

   if (fclose(book->file) == EOF) {
      my_fatal("book_close(): fclose(): %s\n",strerror(errno));
   }
}

// end of book_merge.cpp

