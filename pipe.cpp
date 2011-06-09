#ifdef _WIN32
#include "pipe.h"
#include "util.h"

// functions

DWORD WINAPI ThreadProc(LPVOID lpParam){
    PipeStruct *p=(PipeStruct *) lpParam;
    while(!p->EOF_()){
        p->ReadInput();
    }
    return 0;
}



void PipeStruct::Open(const char *szProcFile) {
    DWORD dwMode;
    HANDLE hStdinRead, hStdinWrite, hStdoutRead, hStdoutWrite;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
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
    nReadEnd = 0;
    lpFeedEnd = NULL;
    InitializeCriticalSection(&CriticalSection);
    hEvent=CreateEvent(NULL,           // default security
                       FALSE,          // auto reset
                       FALSE,          // not signaled
                       NULL            // nameless
                       );
    hThread=CreateThread(NULL,         // default security
                         0,            // default stacksize
                         ThreadProc,   // worker function
                         this,         // tell worker about ourselves
                         0,            // run immediately
                         NULL          // nameless
                         );          
    
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
    int ret;
    
    if(!bPipe){
        fgets(lpReadBuffer,LINE_INPUT_MAX_CHAR,stdin);
        dwBytes=strlen(lpReadBuffer);
        if(!dwBytes){
            set_EOF_();
            lpReadBuffer[0]='\0';
            return 0;
        }
    }else{
            // Unfortunately we need to use polling here.
            // Otherwise Windows returns single bytes if
            // the engine runs at low priority.
            // This kills performance.
        while(TRUE){
            ret=PeekNamedPipe(hInput,
                              NULL,    // don't read anything yet
                              0,       // no buffer
                              NULL,    // no we don't read anything!
                              &dwBytes,// now we're talking
                              NULL);   // nono we don't read anything
            if(!ret){
                set_EOF_();
                lpReadBuffer[0]='\0';
                return 0;
            }
            if(dwBytes>0){
                break;
            }else{
                Idle();
            }
            
        }
        ret=ReadFile(hInput,
                     lpReadBuffer,
                     LINE_INPUT_MAX_CHAR,
                     &dwBytes,
                     NULL);
        if(!ret){
            set_EOF_();
            lpReadBuffer[0]='\0';
            return 0;
        }
    }
    lpReadBuffer[dwBytes]='\0';
    return dwBytes;
}

void PipeStruct::ReadInput(void) {
  DWORD dwBytes;
  int ret;
  BOOL bSetEvent=FALSE;
  ret=ReadData();
  EnterCriticalSection(&CriticalSection);
  if(!EOF_()){
      if(ret+nReadEnd>=LINE_INPUT_MAX_CHAR){
          my_fatal("PipeStruct::ReadInput(): buffer overflow\n");
      }
      memcpy(lpBuffer+nReadEnd,lpReadBuffer,ret);
      nReadEnd += ret;
      if(!lpFeedEnd){
          lpFeedEnd = (char *) memchr(lpBuffer, '\n', nReadEnd);
      }
      if(lpFeedEnd){
          bSetEvent=TRUE;
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
