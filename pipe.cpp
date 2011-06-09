#ifdef _WIN32
#include "pipe.h"
#include "util.h"
void PipeStruct::Open(const char *szProcFile) {
  DWORD dwMode;
  HANDLE hStdinRead, hStdinWrite, hStdoutRead, hStdoutWrite;
  SECURITY_ATTRIBUTES sa;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  if (szProcFile == NULL) {
    hInput = GetStdHandle(STD_INPUT_HANDLE);
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    bConsole = GetConsoleMode(hInput, &dwMode);
  } else {
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
    CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0);
    CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0);
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = si.lpDesktop = si.lpTitle = NULL;
    si.dwFlags = STARTF_USESTDHANDLES;
    si.cbReserved2 = 0;
    si.lpReserved2 = NULL;
    si.hStdInput = hStdinRead;
    si.hStdOutput = hStdoutWrite;
    si.hStdError = hStdoutWrite;
    if(!CreateProcess(NULL, (LPSTR) szProcFile, NULL, NULL, TRUE, DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &pi)){
        my_fatal("PipeStruct::Open(): Could not start \"%s\"\n",szProcFile);
    }
	hProcess=pi.hProcess;
    //CloseHandle(pi.hProcess);//not here,baby,but in pipe.close 
    CloseHandle(pi.hThread);
    CloseHandle(hStdinRead);
    CloseHandle(hStdoutWrite);
    hInput = hStdoutRead;
    hOutput = hStdinWrite;
    bConsole = FALSE;
  }
  if (bConsole) {
    SetConsoleMode(hInput, dwMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
    FlushConsoleInputBuffer(hInput);
  } else {
    nBytesLeft = 0;
  }
  nReadEnd = 0;
}

void PipeStruct::Close(void) const {
  CloseHandle(hInput);
  CloseHandle(hOutput);
  DWORD lpexit;

  my_log("POLYGLOT Closing child\n");
  if(GetExitCodeProcess(hProcess,&lpexit)){
	if(lpexit==STILL_ACTIVE)
	  my_log("POLYGLOT Process still active after \"quit\" ");
	//must be java,hammer it down!
	TerminateProcess(hProcess,lpexit);
  }
	CloseHandle(hProcess);
}

void PipeStruct::ReadInput(void) {
  DWORD dwBytes;
  if(!ReadFile(hInput, szBuffer + nReadEnd, LINE_INPUT_MAX_CHAR - nReadEnd, &dwBytes, NULL)){
          // TODO move this comment to a more suitable place
      my_log("POLYGLOT *** EOF from Engine or GUI ***\n");
      exit(EXIT_SUCCESS); // if we are here there should be data!
  }
  nReadEnd += dwBytes;
  if (nBytesLeft > 0) {
    nBytesLeft -= dwBytes;
  }
}

bool PipeStruct::CheckInput(void) {
  DWORD dwEvents, dwBytes;
  if (bConsole) { // a tty, or an un-redirected handle
    GetNumberOfConsoleInputEvents(hInput, &dwEvents);
    if (dwEvents > 1) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else { // a handle redirected to a pipe or a file
    if (nBytesLeft > 0) {
      return TRUE;
    } else {
      if (PeekNamedPipe(hInput, NULL, 0, NULL, &dwBytes, NULL)) {
        nBytesLeft = dwBytes;
        return nBytesLeft > 0; // a pipe
      } else {
        return TRUE; // a file, always TRUE
      }
    }
  }
}

void PipeStruct::LineOutput(const char *szLineStr) const {
  DWORD dwBytes;
  int nStrLen;
  char szWriteBuffer[LINE_INPUT_MAX_CHAR];
  nStrLen = strlen(szLineStr);
  memcpy(szWriteBuffer, szLineStr, nStrLen);
  szWriteBuffer[nStrLen] = '\r';
  szWriteBuffer[nStrLen + 1] = '\n';
  WriteFile(hOutput, szWriteBuffer, nStrLen + 2, &dwBytes, NULL);
}



bool PipeStruct::GetBuffer(char *szLineStr) {
  char *lpFeedEnd;
  int nFeedEnd;
  lpFeedEnd = (char *) memchr(szBuffer, '\n', nReadEnd);
  if (lpFeedEnd == NULL) {
    return FALSE;
  } else {
    nFeedEnd = lpFeedEnd - szBuffer;
    memcpy(szLineStr, szBuffer, nFeedEnd);
    if (szLineStr[nFeedEnd - 1] == '\r') {
      szLineStr[nFeedEnd - 1] = '\0';
    } else {
      szLineStr[nFeedEnd] = '\0';
    }
    nFeedEnd ++;
    nReadEnd -= nFeedEnd;
    memcpy(szBuffer, szBuffer + nFeedEnd, nReadEnd);
    return TRUE;
  }
}

bool PipeStruct::LineInput(char *szLineStr) {
  if (GetBuffer(szLineStr)) {
    return TRUE;
  }
  if (CheckInput()) {
    ReadInput();
    if (GetBuffer(szLineStr)) {
      return TRUE;
    } else {
      if (nReadEnd == LINE_INPUT_MAX_CHAR) {
        memcpy(szLineStr, szBuffer, LINE_INPUT_MAX_CHAR - 1);
        szLineStr[LINE_INPUT_MAX_CHAR - 1] = '\0';
        szBuffer[0] = szBuffer[LINE_INPUT_MAX_CHAR - 1];
        nReadEnd = 1;
        return TRUE;
      } else {
        return FALSE;
      }
    }
  } else {
    return FALSE;
  }
}
#endif
