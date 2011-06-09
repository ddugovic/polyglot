#ifndef PIPE_H
#define PIPE_H
#ifdef _WIN32
// includes

#include <windows.h>

// constants

const int LINE_INPUT_MAX_CHAR = 10*4096;

// defines

#define PIPE_EOF 1
#define PIPE_ACTIVE 2

// types

struct PipeStruct {

    HANDLE hInput, hOutput;
    HANDLE hProcess;
    HANDLE hThread;
    HANDLE hEvent;
    BOOL bConsole;
    BOOL bPipe;

    CRITICAL_SECTION CriticalSection;

    volatile DWORD state;
    volatile int nReadEnd;
    char lpBuffer[LINE_INPUT_MAX_CHAR];
    char lpReadBuffer[LINE_INPUT_MAX_CHAR];

    void Open(const char *szExecFile = NULL);
    void Close(void) const;
    void Kill(void) const;
    bool EOF_(void);
    void set_EOF_(void);
    bool Active(void);
    void set_Active(void);
    void ReadInput(void);
    int ReadLine(void);
    bool CheckInput(void);
    bool GetBuffer(char *szLineStr);
    void LineInput(char *szLineStr);
    void LineOutput(const char *szLineStr) const;

};

// pipe

#endif
#endif
