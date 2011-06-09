#ifndef PIPE_H
#define PIPE_H
#ifdef _WIN32
// includes

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

// constants

// This should be bigger than the maximum length of an engine output or GUI
// input line.
const int LINE_INPUT_MAX_CHAR = 40960;

// defines

#define PIPE_EOF 1
#define PIPE_ACTIVE 2

// types

class  PipeStruct {
  friend DWORD WINAPI ThreadProc(LPVOID lpParam);
 public:
    HANDLE hProcess;
    HANDLE hEvent;
    bool GetBuffer(char *szLineStr);
    void LineInput(char *szLineStr);
    void LineOutput(const char *szLineStr) const;
    void Open(const char *szExecFile = NULL);
    void Close(void) const;
    void Kill(void) const;
    bool Active(void);
    bool EOF_(void);
 private:
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

    bool EOF_input(void);
    void set_EOF_input(void);
    void set_Active(void);
    void ReadInput(void);
    int ReadData(void);

};

// pipe

#endif
#endif
