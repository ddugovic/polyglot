// pipex_win32.c

#ifdef _WIN32

// includes

#include <io.h>
#include <fcntl.h>
#include "pipex.h"
#include "util.h"

// defines

#define ErrorBufferSize 4096
#define dwMaxHandles      32

// variables

static char ErrorBuffer[ErrorBufferSize];

// prototypes

static bool pipex_eof_input(pipex_t *pipex);
static void pipex_set_eof_input(pipex_t *pipex);
static void pipex_set_active(pipex_t *pipex);
static int  pipex_read_data(pipex_t *pipex);
static void pipex_read_input(pipex_t *pipex);

// functions

// win32_error()

static char * win32_error(){
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        LANG_USER_DEFAULT,
        ErrorBuffer,
        ErrorBufferSize,
        NULL);
    return ErrorBuffer;
}

// TreadProc()

static DWORD WINAPI ThreadProc(LPVOID lpParam){ 
    pipex_t *p=(pipex_t *) lpParam;
    while(!pipex_eof_input(p)){
        if(p->nReadEnd<LINE_INPUT_MAX_CHAR-1){
            pipex_read_input(p);
        }else{
                // wait until there is room in buffer
            Sleep(10);
        }
    }
    return 0;
}

// pipex_open()

void pipex_open(pipex_t *pipex,
                const char *szName,
                const char *szWorkingDir,
                const char *szProcFile) {
    DWORD dwMode, dwThreadId;
    HANDLE hStdinRead, hStdinWrite, hStdoutRead, hStdoutWrite, hThread;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    int fdInput;
    char *szCurrentDir;
    pipex->state=0;
    pipex->name=szName;
    pipex->command=szProcFile;
    pipex->quit_pending=FALSE;
    pipex->hProcess=NULL;
    if (szProcFile == NULL) {
        pipex->hInput = GetStdHandle(STD_INPUT_HANDLE);
        pipex->hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        pipex->bConsole = GetConsoleMode(pipex->hInput, &dwMode);
        pipex->bPipe=FALSE;
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
        if((szCurrentDir = (char*)_getcwd( NULL, 0 )) == NULL )
            my_fatal("pipex_open(): no current directory: %s\n",
                     strerror(errno));
        if(_chdir(szWorkingDir)){
            my_fatal("pipex_open(): %s: %s\n",
		     szWorkingDir,
                     strerror(errno));
        }
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
            pipex->hProcess=pi.hProcess;
            CloseHandle(pi.hThread);
            CloseHandle(hStdinRead);
            CloseHandle(hStdoutWrite);
            pipex->hInput = hStdoutRead;
            pipex->hOutput = hStdinWrite;
            pipex->bConsole = FALSE;
            pipex->bPipe=TRUE;
        }else{
	  my_fatal("pipex_open(): %s: %s",szProcFile,win32_error());
        }
        _chdir(szCurrentDir);
    }
    if (pipex->bConsole) {
        SetConsoleMode(pipex->hInput,
                       dwMode & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
        FlushConsoleInputBuffer(pipex->hInput);
    } 
    fdInput=_open_osfhandle((long) pipex->hInput,_O_RDONLY);
    if(fdInput==-1){
        my_fatal("pipex_open(): %s",strerror(errno));
    }
    pipex->fpInput=fdopen(fdInput,"r");
    if(pipex->fpInput==NULL){
        my_fatal("pipex_open(): %s",strerror(errno));
    }
    pipex->nReadEnd = 0;
    pipex->lpFeedEnd = NULL;
    InitializeCriticalSection(&(pipex->CriticalSection));
    pipex->hEvent=CreateEvent(NULL,           // default security
                       FALSE,          // auto reset
                       FALSE,          // not signaled
                       NULL            // nameless
                       );
    if(!(pipex->hEvent)){
        my_fatal("pipex_open(): %s",win32_error());
    }
    hThread=CreateThread(NULL,         // default security
                         0,            // default stacksize
                         ThreadProc,   // worker function
                         pipex,        // tell worker about ourselves
                         0,            // run immediately
                         &dwThreadId   // dropped, but needed for the call to work in Win9x
                         );          
    if(!hThread){
        my_fatal("pipex_open(): %s",win32_error());
    }
    pipex->dwWriteIndex=0;
    pipex_set_active(pipex);
}


// pipex_wait_event(pipex)

void pipex_wait_event(pipex_t *pipex[]){
    HANDLE hHandles[dwMaxHandles]; 
    DWORD dwHandleCount=0;
    pipex_t *p;
    while((p=pipex[dwHandleCount])!=NULL){
        ASSERT((p->hEvent)!=0);
        if(dwHandleCount>=dwMaxHandles){
            my_fatal("pipex_wait_event(): Too many objects to wait for");
        }
        hHandles[dwHandleCount++]=p->hEvent;
    }
    WaitForMultipleObjects(dwHandleCount,   // count
                           hHandles,        //
                           FALSE,           // return if one object is signaled
                           INFINITE         // no timeout
                           );
}

// pipex_send_eof()

void pipex_send_eof(pipex_t *pipex)  {
    my_log("Adapter->%s: EOF\n",pipex->name);
    CloseHandle(pipex->hOutput);
}

// pipex_exit()

void pipex_exit(pipex_t *pipex) {
    DWORD lpexit;
    CloseHandle(pipex->hInput);
    CloseHandle(pipex->hOutput);
    if(!pipex->quit_pending){
      // suppress further errors
      pipex->quit_pending=TRUE;
      my_fatal("pipex_exit(): %s: child exited unexpectedly.\n",pipex->command);
    }
    if(GetExitCodeProcess(pipex->hProcess,&lpexit)){
        if(lpexit==STILL_ACTIVE)
                //must be java,hammer it down!
            TerminateProcess(pipex->hProcess,lpexit);
    }
	CloseHandle(pipex->hProcess);
}

// pipex_eof_input()

static bool pipex_eof_input(pipex_t *pipex){ 
    int ret;
    EnterCriticalSection(&(pipex->CriticalSection));
    ret=(pipex->state)&PIPEX_EOF;
    LeaveCriticalSection(&(pipex->CriticalSection));
    return ret;
}

// pipex_set_eof_input()

static void pipex_set_eof_input(pipex_t *pipex){
    EnterCriticalSection(&(pipex->CriticalSection));
    (pipex->state)|=PIPEX_EOF;
    LeaveCriticalSection(&(pipex->CriticalSection));
 }

// pipex_active()

/*
 * This function returns TRUE if and only if the pipes have succesfully
 * been created and the client has been started.
 *
 */

bool pipex_active(pipex_t *pipex){
    int ret;
    EnterCriticalSection(&(pipex->CriticalSection));
    ret=(pipex->state)&PIPEX_ACTIVE;
    LeaveCriticalSection(&(pipex->CriticalSection));
    return ret;
}

// pipex_set_active()

static void pipex_set_active(pipex_t *pipex){
    EnterCriticalSection(&(pipex->CriticalSection));
    (pipex->state)|=PIPEX_ACTIVE;
    LeaveCriticalSection(&(pipex->CriticalSection));
}

// pipex_read_data()

static int pipex_read_data(pipex_t *pipex){
    DWORD dwBytes;
    char * ret;
        // No protection. Access to nReadEnd is atomic.
        // It is not a problem that nReadEnd becomes smaller after the call.
        // This just means we have read less than we could have. 
    ret=fgets(pipex->lpReadBuffer,
              LINE_INPUT_MAX_CHAR-(pipex->nReadEnd),
              pipex->fpInput);
    if(!ret){
        pipex_set_eof_input(pipex);
        (pipex->lpReadBuffer)[0]='\0';
        return 0;
    }
    dwBytes=strlen(pipex->lpReadBuffer);
    (pipex->lpReadBuffer)[dwBytes]='\0';
    return dwBytes;
}

// pipex_read_input()

static void pipex_read_input(pipex_t *pipex) {
  int ret;
  BOOL bSetEvent=FALSE;
      // ReadData is outside the critical section otherwise everything
      // would block during the blocking read
  ret=pipex_read_data(pipex);
  EnterCriticalSection(&(pipex->CriticalSection));
  if(!pipex_eof_input(pipex)){
      if(ret+(pipex->nReadEnd)>=LINE_INPUT_MAX_CHAR){
          my_fatal("pipex_read_input(): Internal error: buffer overflow\n");
      }
      memcpy((pipex->lpBuffer)+(pipex->nReadEnd),(pipex->lpReadBuffer),ret+1);
      (pipex->nReadEnd) += ret;
      if(!(pipex->lpFeedEnd)){
          (pipex->lpFeedEnd) =
              (char *) memchr(pipex->lpBuffer,'\n',pipex->nReadEnd);
      }
      if(pipex->lpFeedEnd){
          bSetEvent=TRUE;
      }else if((pipex->nReadEnd)>=LINE_INPUT_MAX_CHAR-1){
          my_fatal("pipex_read_input(): LINE_INPUT_MAX_CHAR is equal to %d which is too small to contain a full line of engine output or GUI input.\n",LINE_INPUT_MAX_CHAR);
      }
  }
  LeaveCriticalSection(&(pipex->CriticalSection));
  if(pipex_eof_input(pipex) || bSetEvent){
      SetEvent(pipex->hEvent);
  }
}

// pipex_eof()

/*
 * This function returns TRUE if and only if the input buffer does not
 * contain a full line of data and EOF was encountered by
 * the working thread.
 *
 */

bool pipex_eof(pipex_t *pipex){
  int ret;
  EnterCriticalSection(&(pipex->CriticalSection));
  if((pipex->lpFeedEnd) != NULL){
    ret=FALSE;
  }else if(pipex_eof_input(pipex)){
    ret=TRUE;
  }else{
    ret=FALSE;
  }
  LeaveCriticalSection(&(pipex->CriticalSection));
  return ret;
}

// pipex_readln_nb()

/*
 * This function returns FALSE if and only if the asynchronously filled
 * input buffer does not contain a full line of data.
 * In other words it operates in non-blocking mode.
 *
 */

bool pipex_readln_nb(pipex_t *pipex, char *szLineStr) {
  int nFeedEnd;
  int ret;
  int src, dst;
  char c;
  EnterCriticalSection(&(pipex->CriticalSection));
  if ((pipex->lpFeedEnd) == NULL) {
    ret=FALSE;
  } else {
    nFeedEnd = pipex->lpFeedEnd - pipex->lpBuffer;
    memcpy(szLineStr, pipex->lpBuffer, nFeedEnd+1);
    szLineStr[nFeedEnd] = '\0';
    
        // temp hack: stolen from util.c
        // remove CRs and LFs
    src = 0;
    dst = 0;
    while ((c=szLineStr[src++]) != '\0') {
        if (c != '\r' && c != '\n') szLineStr[dst++] = c;
    }
    szLineStr[dst] = '\0';    
    ASSERT(strchr(szLineStr,'\n')==NULL)
    ASSERT(strchr(szLineStr,'\r')==NULL)
        
    nFeedEnd ++;
    pipex->nReadEnd -= nFeedEnd;
    memcpy(pipex->lpBuffer, pipex->lpBuffer + nFeedEnd, pipex->nReadEnd+1);
    pipex->lpFeedEnd =
        (char *) memchr(pipex->lpBuffer, '\n', pipex->nReadEnd);
    ret=TRUE;
  }
  LeaveCriticalSection(&(pipex->CriticalSection));
  if(ret){
      my_log("%s->Adapter: %s\n",pipex->name,szLineStr);
  }
  return ret;
}

// pipex_readln()

/*
 * This function returns FALSE if and only if EOF has been encountered by 
 * the working thread and no full line of data is present in the input buffer.
 *
 * If there is a full line of data present in the input buffer it returns 
 * TRUE. 
 *
 * If none of these conditions is satisfied it blocks.
 *
 * As the name say this function is strictly for line input. 
 * An incomplete line of data (i.e. not ending with \n) is lost when EOF
 * is encountered.
 *
 */

bool pipex_readln(pipex_t *pipex, char *szLineStr) {
  while(!pipex_eof(pipex)){
      if (pipex_readln_nb(pipex,szLineStr)) {
          return TRUE;
      }else{
          WaitForSingleObject(pipex->hEvent,INFINITE);
      }
  }
  my_log("%s->Adapter: EOF\n",pipex->name);
  szLineStr[0]='\0';
  return FALSE;
}

//  GetWin32Priority()

static DWORD GetWin32Priority(int nice)
{
/*
REALTIME_PRIORITY_CLASS     0x00000100
HIGH_PRIORITY_CLASS         0x00000080
ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
NORMAL_PRIORITY_CLASS       0x00000020
BELOW_NORMAL_PRIORITY_CLASS 0x00004000
IDLE_PRIORITY_CLASS         0x00000040
*/
	if (nice < -15) return 0x00000080;
	if (nice < 0)   return 0x00008000;
	if (nice == 0)  return 0x00000020;
	if (nice < 15)  return 0x00004000;
	return 0x00000040;
}

// pipex_set_priority()

void pipex_set_priority(pipex_t *pipex, int value){
    if(pipex->hProcess){
        if(!SetPriorityClass(pipex->hProcess,
                             GetWin32Priority(value))){
            my_log("POLYGLOT Unable to change priority\n");
        }
    }
}

// pipex_set_affinit()

typedef void (WINAPI *SPAM)(HANDLE, int);
void pipex_set_affinity(pipex_t *pipex, int value){
    SPAM pSPAM;

    if(pipex->hProcess) return;
    if(value==-1) return;

    pSPAM = (SPAM) GetProcAddress(
        GetModuleHandle(TEXT("kernel32.dll")), 
        "SetProcessAffinityMask");
    if(NULL != pSPAM){
            // [HGM] avoid crash on Win95 by first checking if API call exists
        pSPAM(pipex->hProcess,value);
    }else{
        my_log("POLYGLOT API call \"SetProcessAffinityMask\" not available\n");
    }
}

// pipex_write()

void pipex_write(pipex_t *pipex, const char *szLineStr) {
    int size,written;
    size=sizeof(pipex->szWriteBuffer)-pipex->dwWriteIndex;
    written=snprintf(pipex->szWriteBuffer + pipex->dwWriteIndex,
                     size,
                     "%s",
                     szLineStr);
        // snprintf returns how many bytes should have been written
        // (not including the trailing zero)
        // old versions of glibc and msvcrt return -1 in
        // case of truncated output.
    if(written>=size || written<0){
        my_fatal("engine_send(): write_buffer overflow\n");
    }
    pipex->dwWriteIndex+=written;
    
}


// pipex_writeln()

void pipex_writeln(pipex_t *pipex, const char *szLineStr) {
  DWORD dwBytes;
  DWORD dwLengthWriteBuffer;
  pipex_write(pipex, szLineStr);
  my_log("Adapter->%s: %s\n",pipex->name,pipex->szWriteBuffer);
  if(pipex->bPipe){
      dwLengthWriteBuffer = strlen(pipex->szWriteBuffer);
      if(dwLengthWriteBuffer>=sizeof(pipex->szWriteBuffer)-3){
          my_fatal("pipex_writeln(): write buffer overflow\n");
      }
      pipex->szWriteBuffer[dwLengthWriteBuffer] = '\r';
      pipex->szWriteBuffer[dwLengthWriteBuffer + 1] = '\n';
          // for easy debugging
      pipex->szWriteBuffer[dwLengthWriteBuffer + 2] = '\0';  
      WriteFile(pipex->hOutput, pipex->szWriteBuffer,
                dwLengthWriteBuffer + 2,
                &dwBytes, NULL);
  }else{
      printf("%s\n",pipex->szWriteBuffer);
      fflush(stdout);
  }
  pipex->dwWriteIndex = 0;
}
#endif
