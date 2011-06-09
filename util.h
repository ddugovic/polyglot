
// util.h

#ifndef UTIL_H
#define UTIL_H

// includes

#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/stat.h>

// defines

#ifndef EXIT_SUCCES
#define EXIT_SUCCES 0
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif


#undef FALSE
#define FALSE 0

#undef TRUE
#define TRUE 1

#ifdef DEBUG
#  undef DEBUG
#  define DEBUG TRUE
#else
#  define DEBUG FALSE
#endif

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
#  define S64_FORMAT "%I64d"
#  define U64_FORMAT "%016I64X"
#else
#  define S64_FORMAT "%lld"
#  define U64_FORMAT "%016llX"
#endif

// macros

#ifdef _MSC_VER
#  define S64(u) (u##i64)
#  define U64(u) (u##ui64)
#else
#  define S64(u) (u##LL)
#  define U64(u) (u##ULL)
#endif

#undef ASSERT
#if DEBUG
#  define ASSERT(a) { if (!(a)) my_fatal("file \"%s\", line %d, assertion \"" #a "\" failed\n",__FILE__,__LINE__); }
#else
#  define ASSERT(a)
#endif

#ifdef _WIN32
#define snprintf _snprintf
#endif

#define FormatBufferSize 4096
#define StringSize       4096

#ifdef _MSC_VER
#define vsnprintf _vsnprintf
#endif

#define CONSTRUCT_ARG_STRING(format,buf)                                 \
    {                                                                    \
        va_list arg_list;                                                \
        int written;                                                     \
        va_start(arg_list,format);                                       \
        written=vsnprintf(buf,                                           \
                          sizeof(buf),                                   \
                          format,                                        \
                          arg_list);                                     \
        va_end(arg_list);                                                \
        buf[sizeof(buf)-1]='\0';                                         \
        if(written>=sizeof(buf) || written<0){                           \
           my_fatal("write_buffer overflow: file \"%s\", line %d\n",     \
                   __FILE__,__LINE__);                                   \
        }                                                                \
    }                                                                    \

#define TO_BOOL(string) ((my_string_case_equal(string,"false") ||   \
                          my_string_equal(string,"0"))?FALSE:TRUE)

#define IS_BOOL(string) (my_string_case_equal(string,"false")||     \
                         my_string_case_equal(string,"true") ||     \
                         my_string_case_equal(string,"1")    ||     \
                         my_string_case_equal(string,"0"))
// types

typedef signed char sint8;
typedef unsigned char uint8;

typedef signed short sint16;
typedef unsigned short uint16;

typedef signed int sint32;
typedef unsigned int uint32;

typedef int bool;

#ifdef _MSC_VER
  typedef signed __int64 sint64;
  typedef unsigned __int64 uint64;
#else
  typedef signed long long int sint64;
  typedef unsigned long long int uint64;
#endif

typedef struct {
   double start_real;
   double elapsed_real;
   bool running;
} my_timer_t;


// functions

extern void   util_init             ();

extern void   my_random_init        ();
extern int    my_random_int         (int n);
extern double my_random_double      ();

extern sint64 my_atoll              (const char string[]);

extern int    my_round              (double x);

extern void * my_malloc             (size_t size);
extern void * my_realloc            (void * address, size_t size);
extern void   my_free               (void * address);

extern void   my_log_open           (const char file_name[]);
extern void   my_log_close          ();

extern void   my_log                (const char format[], ...);
extern void   my_fatal              (const char format[], ...);

extern bool   my_file_read_line     (FILE * file, char string[], int size);
extern void   my_path_join          (char *join_path, const char *path, const char *file);

extern int    my_mkdir              (const char *path);

extern bool   my_string_empty       (const char string[]);
extern bool   my_string_whitespace  (const char string[]);
extern bool   my_string_equal       (const char string_1[], const char string_2[]);
extern bool   my_string_case_equal  (const char string_1[], const char string_2[]);
extern const char* my_string_case_contains(const char haystack[], 
					   const char needle[]);


extern bool   my_string_to_lower    (char dst[], const char src[]);

extern char * my_strdup             (const char string[]);

extern void   my_string_clear       (const char * * variable);
extern void   my_string_set         (const char * * variable, const char string[]);

extern double now_real              ();

extern void   my_timer_reset        (my_timer_t * timer);
extern void   my_timer_start        (my_timer_t * timer);
extern void   my_timer_stop         (my_timer_t * timer);

extern double my_timer_elapsed_real (const my_timer_t * timer);

extern char * my_error();

extern void my_dequote              (char *out, 
				     const char *in, 
				     const char *special);
extern void my_quote                (char *out, 
				     const char *in, 
				     const char *special);

extern void my_sleep                (int msec);

#endif // !defined UTIL_H

// end of util.h
