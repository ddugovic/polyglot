#ifndef PIPE_H
#define PIPE_H
#ifdef _WIN32
// includes

#include <windows.h>


// constants

const int LINE_INPUT_MAX_CHAR = 4096;

// types

struct PipeStruct {
  HANDLE hInput, hOutput;
    HANDLE hProcess;
  BOOL bConsole;
  int nBytesLeft;
  int nReadEnd;
  char szBuffer[LINE_INPUT_MAX_CHAR];

  void Open(const char *szExecFile = NULL);
  void Close(void) const;
  void ReadInput(void);
  bool CheckInput(void);
  bool GetBuffer(char *szLineStr);
  bool LineInput(char *szLineStr);
  void LineOutput(const char *szLineStr) const;
}; // pipe

#endif
#endif
