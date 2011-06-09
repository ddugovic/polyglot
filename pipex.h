#ifndef PIPEX_H
#define PIPEX_H
#ifdef _WIN32

// WIN32 part

// includes

#include <windows.h>
#include <stdio.h>
#include "util.h"

// defines

#define PIPEX_EOF (1<<0)
#define PIPEX_ACTIVE (1<<1)

// This should be bigger than the maximum length of an engine output or GUI
// input line.

#define LINE_INPUT_MAX_CHAR 4096

// types

typedef struct {
    HANDLE hProcess;
    HANDLE hEvent;
    HANDLE hInput, hOutput;
    FILE *fpInput;
    HANDLE hThread;
    BOOL bConsole;
    BOOL bPipe;
    CRITICAL_SECTION CriticalSection;
    volatile DWORD state;
    volatile char * lpFeedEnd;
    volatile int nReadEnd;
    char lpBuffer[LINE_INPUT_MAX_CHAR];
    char lpReadBuffer[LINE_INPUT_MAX_CHAR];
    char szWriteBuffer[LINE_INPUT_MAX_CHAR];
    DWORD dwWriteIndex;
    const char *name;
    const char *command;
    BOOL quit_pending;

} pipex_t;

#else

// POSIX part

// includes

#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "io.h"
#include "util.h"

// defines

#define PIPEX_EOF (1<<0)
#define PIPEX_ACTIVE (1<<1)

// types

typedef struct {
    io_t io[1];
    pid_t pid;
    int state;
    bool quit_pending;
    const char *command;
} pipex_t;

#endif

// part common to WIN32 and POSIX

// macros

#define PIPEX_MAGIC "!@#$%"
#define WAIT_GRANULARITY 100

// functions 

extern void  pipex_open         (pipex_t *pipex, 
                                 const char *name,
                                 const char *working_dir,
                                 const char *command);
extern bool  pipex_active       (pipex_t *pipex);
extern bool  pipex_readln       (pipex_t *pipex, char *string);
extern bool  pipex_readln_nb    (pipex_t *pipex, char *string);
extern void  pipex_writeln      (pipex_t *pipex, const char *string);
extern void  pipex_write        (pipex_t *pipex, const char *string);
extern char* pipex_get_buffer   (pipex_t *pipex);
extern bool  pipex_eof          (pipex_t *pipex);
extern void  pipex_send_eof     (pipex_t *pipex);
extern void  pipex_exit         (pipex_t *pipex, int kill_timeout);
extern void  pipex_set_priority (pipex_t *pipex, int value);
extern void  pipex_set_affinity (pipex_t *pipex, int value);
extern void  pipex_wait_event   (pipex_t *pipex[]);


// pipex

#endif
