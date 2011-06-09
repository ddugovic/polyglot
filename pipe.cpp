#ifdef _WIN32
#include "pipe.h"
#include "util.h"

// functions

DWORD WINAPI ThreadProc(LPVOID lpParam){ 
    PipeStruct *p=(PipeStruct *) lpParam;
    while(!p->EOF_()){
        if(p->nReadEnd<LINE_INPUT_MAX_CHAR-1){
            p->ReadInput();
        }else{
                // wait until there is room in buffer
            Sleep(10);
        }
    }
    return 0;
}



void PipeStruct::Open(const char *szProcFile) {
    DWORD dwMode, dwThreadId;
    HANDLE hStdinRead, hStdinWrite, hStdoutRead, hStdoutWrite;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int fdInput;
    state=0;
    if (szProcFile == NULL) {
        hInput = GetStdHandle(STD_INPUT_HANDLE);
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        bConsole = GetConsoleMode(hInput, &dwMode);
        bPipe=FALSE;
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
        if(CreateProcess(NULL,
                         (LPSTR) szProcFile,
                         NULL,
                         NULL,
                         TRUE,
                         DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
                         NULL,
                         NULL,
                         &si,
                         &pi)){
            hProcess=pi.hProcess;
            CloseHandle(pi.hThread);
            CloseHandle(hStdinRead);
            CloseHandle(hStdoutWrite);
            hInput = hStdoutRead;
            hOutput = hStdinWrite;
            bConsole = FALSE;
            bPipe=TRUE;
        }else{
            my_fatal("PipeStruct::Open(): %s",my_error());
        }
    }
    if (bConsole) {
        SetConsoleMode(hInput,
                       dwMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(hInput);
    } 
    fdInput=_open_osfhandle((long) hInput,_O_RDONLY);
    if(fdInput==-1){
        my_fatal("PipeStruct::Open(): %s",my_error());
    }
    fpInput=fdopen(fdInput,"r");
    if(fpInput==NULL){
        my_fatal("PipeStruct::Open(): %s",my_error());
    }
    nReadEnd = 0;
    lpFeedEnd = NULL;
    InitializeCriticalSection(&CriticalSection);
    hEvent=CreateEvent(NULL,           // default security
                       FALSE,          // auto reset
                       FALSE,          // not signaled
                       NULL            // nameless
                       );
    if(!hEvent){
        my_fatal("PipeStruct::Open(): %s",my_error());
    }
    hThread=CreateThread(NULL,         // default security
                         0,            // default stacksize
                         ThreadProc,   // worker function
                         this,         // tell worker about ourselves
                         0,            // run immediately
                         &dwThreadId   // dropped, but needed for the call to work in Win9x
                         );          
    if(!hThread){
        my_fatal("PipeStruct::Open(): %s",my_error());
    }
    set_Active();
}


void PipeStruct::Close(void) const {
    CloseHandle(hOutput);
}

void PipeStruct::Kill(void) const {
    CloseHandle(hInput);
    CloseHandle(hOutput);
    DWORD lpexit;
    
    if(GetExitCodeProcess(hProcess,&lpexit)){
        if(lpexit==STILL_ACTIVE)
                //must be java,hammer it down!
            TerminateProcess(hProcess,lpexit);
    }
	CloseHandle(hProcess);
}

bool PipeStruct::EOF_(void){   // EOF is defined
    int ret;
    EnterCriticalSection(&CriticalSection);
    ret=state&PIPE_EOF;
    LeaveCriticalSection(&CriticalSection);
    return ret;
}

void PipeStruct::set_EOF_(void){
    EnterCriticalSection(&CriticalSection);
    state|=PIPE_EOF;
    LeaveCriticalSection(&CriticalSection);
}

bool PipeStruct::Active(void){
    int ret;
    EnterCriticalSection(&CriticalSection);
    ret=state&PIPE_ACTIVE;
    LeaveCriticalSection(&CriticalSection);
    return ret;
}

void PipeStruct::set_Active(void){
    EnterCriticalSection(&CriticalSection);
    state|=PIPE_ACTIVE;
    LeaveCriticalSection(&CriticalSection);
}

int PipeStruct::ReadData(void){
    DWORD dwBytes;
    char * ret;
        // No protection. Access to nReadEnd is atomic.
        // It is not a problem that nReadEnd becomes smaller after the call.
        // This just means we have read less than we could have. 
    ret=fgets(lpReadBuffer,LINE_INPUT_MAX_CHAR-nReadEnd,fpInput);
    if(!ret){
        set_EOF_();
        lpReadBuffer[0]='\0';
        return 0;
    }
    dwBytes=strlen(lpReadBuffer);
    lpReadBuffer[dwBytes]='\0';
    return dwBytes;
}

void PipeStruct::ReadInput(void) {
  DWORD dwBytes;
  int ret;
  BOOL bSetEvent=FALSE;
      // ReadData is outside the critical section otherwise everything
      // would block during the blocking read
  ret=ReadData();
  EnterCriticalSection(&CriticalSection);
  if(!EOF_()){
      if(ret+nReadEnd>=LINE_INPUT_MAX_CHAR){
          my_fatal("PipeStruct::ReadInput(): Internal error: buffer overflow\n");
      }
      memcpy(lpBuffer+nReadEnd,lpReadBuffer,ret+1);
      nReadEnd += ret;
      if(!lpFeedEnd){
          lpFeedEnd = (char *) memchr(lpBuffer, '\n', nReadEnd);
      }
      if(lpFeedEnd){
          bSetEvent=TRUE;
      }else if(nReadEnd>=LINE_INPUT_MAX_CHAR-1){
          my_fatal("PipeStruct::ReadInput(): LINE_INPUT_MAX_CHAR is equal to %d which is too small to contain a full line of engine output or GUI input.\n",LINE_INPUT_MAX_CHAR);
      }
  }
  LeaveCriticalSection(&CriticalSection);
  if(EOF_() || bSetEvent){
      SetEvent(hEvent);
  }
}

bool PipeStruct::GetBuffer(char *szLineStr) {
  int nFeedEnd;
  int ret;
  EnterCriticalSection(&CriticalSection);
  if (lpFeedEnd == NULL) {
    ret=FALSE;
  } else {
    nFeedEnd = lpFeedEnd - lpBuffer;
    memcpy(szLineStr, lpBuffer, nFeedEnd);
    if (szLineStr[nFeedEnd - 1] == '\r') {
      szLineStr[nFeedEnd - 1] = '\0';
    } else {
      szLineStr[nFeedEnd] = '\0';
    }
    nFeedEnd ++;
    nReadEnd -= nFeedEnd;
    memcpy(lpBuffer, lpBuffer + nFeedEnd, nReadEnd);
    lpFeedEnd = (char *) memchr(lpBuffer, '\n', nReadEnd);
    ret=TRUE;
  }
  LeaveCriticalSection(&CriticalSection);
  return ret;
}

void PipeStruct::LineInput(char *szLineStr) {
    while(!EOF_()){
        if (GetBuffer(szLineStr)) {
            break;
        }else{
            WaitForSingleObject(hEvent,INFINITE);
        }
    }
}

void PipeStruct::LineOutput(const char *szLineStr) const {
  DWORD dwBytes;
  int nStrLen;
  char szWriteBuffer[LINE_INPUT_MAX_CHAR];
  if(bPipe){
      nStrLen = strlen(szLineStr);
      memcpy(szWriteBuffer, szLineStr, nStrLen);
      szWriteBuffer[nStrLen] = '\r';
      szWriteBuffer[nStrLen + 1] = '\n';
      WriteFile(hOutput, szWriteBuffer, nStrLen + 2, &dwBytes, NULL);
  }else{
      printf("%s\n",szLineStr);
      fflush(stdout);
  }
}
#endif
