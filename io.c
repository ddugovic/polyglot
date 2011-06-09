#ifndef _WIN32

// io.c

// includes

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>

#include "io.h"
#include "util.h"

// constants

static const bool UseDebug = FALSE;
static const bool UseCR = FALSE;

static const char LF = '\n';
static const char CR = '\r';

// prototypes

static int  my_read  (int fd, char string[], int size);
static void my_write (int fd, const char string[], int size);

// functions

// io_is_ok()

bool io_is_ok(const io_t * io) {

   if (io == NULL) return FALSE;

   if (io->name == NULL) return FALSE;

   if (io->in_eof != TRUE && io->in_eof != FALSE) return FALSE;

   if (io->in_size < 0 || io->in_size > BufferSize) return FALSE;
   if (io->out_size < 0 || io->out_size > BufferSize) return FALSE;

   return TRUE;
}

// io_init()

void io_init(io_t * io) {

   ASSERT(io!=NULL);

   io->in_eof = FALSE;

   io->in_size = 0;
   io->out_size = 0;

   ASSERT(io_is_ok(io));
}

// io_peek()

bool io_peek(io_t * io){
    fd_set set[1];
    int fd_max;
    int ret;
    struct timeval tv;
    tv.tv_sec=0;
    tv.tv_usec=0;
  
    FD_ZERO(set);
    FD_SET(io->in_fd,set);
    fd_max=io->in_fd;
    ret=select(fd_max+1,set,NULL,NULL,&tv);
    if(ret>0){
	return TRUE;
    }else{
	return FALSE;
    }
}

// io_close()

void io_close(io_t * io) {

   ASSERT(io_is_ok(io));

   ASSERT(io->out_fd>=0);

   my_log("Adapter->%s: EOF\n",io->name);

   if (close(io->out_fd) == -1) {
      my_fatal("io_close(): close(): %s\n",strerror(errno));
   }

   io->out_fd = -1;
}

// io_get_update()

void io_get_update(io_t * io) {

   int pos, size;
   int n;

   ASSERT(io_is_ok(io));

   ASSERT(io->in_fd>=0);
   ASSERT(!io->in_eof);

   // init

   pos = io->in_size;

   size = BufferSize - pos;


   if (size <= 0){
       //  io->in_buffer[FormatBufferSize-20]='\0';
       //  my_log("%s","io_get_update(): buffer too small; content starts with:\n");
       //  my_log("[%s...]\n",io->in_buffer);
       my_fatal("io_get_update(): buffer overflow\n");
   }

   // read as many data as possible
   n = my_read(io->in_fd,&io->in_buffer[pos],size);
   if (UseDebug) my_log("POLYGLOT read %d byte%s from %s\n",n,(n>1)?"s":"",io->name);

   if (n > 0) { // at least one character was read

      // update buffer size

      ASSERT(n>=1&&n<=size);

      io->in_size += n;
      ASSERT(io->in_size>=0&&io->in_size<=BufferSize);

   } else { // EOF

      ASSERT(n==0);

      io->in_eof = TRUE;
   }

}

// io_line_ready()

bool io_line_ready(const io_t * io) {

   ASSERT(io_is_ok(io));

   if (io->in_eof) return TRUE;

   if (memchr(io->in_buffer,LF,io->in_size) != NULL) return TRUE; // buffer contains LF

   return FALSE;
}

// io_get_line()

bool io_get_line(io_t * io, char string[], int size) {

   int src, dst;
   int c;

   ASSERT(io_is_ok(io));
   ASSERT(string!=NULL);
   ASSERT(size>=256);

   src = 0;
   dst = 0;

   while (TRUE) {

      // test for end of buffer

      if (src >= io->in_size) {
         if (io->in_eof) {
            my_log("%s->Adapter: EOF\n",io->name);
            return FALSE;
         } else {
            my_fatal("io_get_line(): no EOL in buffer\n");
         }
      }

      // test for end of string

      if (dst >= size) my_fatal("io_get_line(): buffer overflow\n");

      // copy the next character

      c = io->in_buffer[src++];

      if (c == LF) { // LF => line complete
         string[dst] = '\0';
         break;
      } else if (c != CR) { // skip CRs
         string[dst++] = c;
      }
   }

   // shift the buffer

   ASSERT(src>0);

   io->in_size -= src;
   ASSERT(io->in_size>=0);

   if (io->in_size > 0) memmove(&io->in_buffer[0],&io->in_buffer[src],io->in_size);

   // return

   my_log("%s->Adapter: %s\n",io->name,string);

   return TRUE;
}

// io_send()

void io_send(io_t * io, const char format[], ...) {

   char string[FormatBufferSize];
   int len;

   ASSERT(io_is_ok(io));
   ASSERT(format!=NULL);

   ASSERT(io->out_fd>=0);

   // format

   CONSTRUCT_ARG_STRING(format,string);

   // append string to buffer

   len = strlen(string);
   if (io->out_size + len > BufferSize-2) my_fatal("io_send(): buffer overflow\n");

   memcpy(&io->out_buffer[io->out_size],string,len);
   io->out_size += len;

   ASSERT(io->out_size>=0&&io->out_size<=BufferSize-2);

   // log

   io->out_buffer[io->out_size] = '\0';
   my_log("Adapter->%s: %s\n",io->name,io->out_buffer);
    
   // append EOL to buffer

   if (UseCR) io->out_buffer[io->out_size++] = CR;
   io->out_buffer[io->out_size++] = LF;

   ASSERT(io->out_size>=0&&io->out_size<=BufferSize);

   // flush buffer

   if (UseDebug) my_log("POLYGLOT writing %d byte%s to %s\n",io->out_size,(io->out_size>1)?"s":"",io->name);
   my_write(io->out_fd,io->out_buffer,io->out_size);

   io->out_size = 0;
}

// io_send_queue()

void io_send_queue(io_t * io, const char format[], ...) {

   char string[FormatBufferSize];
   int len;

   ASSERT(io_is_ok(io));
   ASSERT(format!=NULL);

   ASSERT(io->out_fd>=0);

   // format

   CONSTRUCT_ARG_STRING(format,string);

   // append string to buffer

   len = strlen(string);
   if (io->out_size + len > BufferSize-2) my_fatal("io_send_queue(): buffer overflow\n");

   memcpy(&io->out_buffer[io->out_size],string,len);
   io->out_size += len;

   ASSERT(io->out_size>=0&&io->out_size<=BufferSize-2);
}

// my_read()

static int my_read(int fd, char string[], int size) {

   int n;

   ASSERT(fd>=0);
   ASSERT(string!=NULL);
   ASSERT(size>0);

   do {
      n = read(fd,string,size);
   } while (n == -1 && errno == EINTR);

   if (n == -1) my_fatal("my_read(): read(): %s\n",strerror(errno));

   ASSERT(n>=0);

   return n;
}

// my_write()

static void my_write(int fd, const char string[], int size) {

   int n;

   ASSERT(fd>=0);
   ASSERT(string!=NULL);
   ASSERT(size>0);

   do {

      n = write(fd,string,size);

      // if (n == -1 && errno != EINTR && errno != EPIPE) my_fatal("my_write(): write(): %s\n",strerror(errno));

      if (n == -1) {
         if (FALSE) {
         } else if (errno == EINTR) {
            n = 0; // nothing has been written
         } else if (errno == EPIPE) {
            n = size; // pretend everything has been written
         } else {
            my_fatal("my_write(): write(): %s\n",strerror(errno));
         }
      }

      ASSERT(n>=0);

      string += n;
      size -= n;

   } while (size > 0);

   ASSERT(size==0);
}

// end of io.cpp

#endif
