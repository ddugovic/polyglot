#ifndef _WIN32

// includes

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wordexp.h>
#include <sys/wait.h>
#include "pipex.h"

// prototypes

static void my_close(int fd);
static void my_dup2(int old_fd, int new_fd) ;

// functions

// pipex_open()

void pipex_open(pipex_t *pipex,
                const char *name,
                const char *working_dir,
                const char *command){
    char string[StringSize];
    int argc;
    char * ptr;
    char * argv[256];
    int from_child[2], to_child[2];
    wordexp_t p;
    int i,ret;

    pipex->pid=-1;
    pipex->io->name=name;
    pipex->quit_pending=FALSE;
    pipex->command=command;
    
    if(command==NULL){
        pipex->io->in_fd = STDIN_FILENO;
        pipex->io->out_fd = STDOUT_FILENO;

            // attach standard error to standard output
        
        my_dup2(STDOUT_FILENO,STDERR_FILENO);
        io_init(pipex->io);
    }else{
    
        // parse the command line and create the argument list
#if 0    
        if (strlen(command) >= StringSize) my_fatal("pipex_open(): buffer overflow\n");
        strcpy(string,command);
        argc = 0;
        
        for (ptr = strtok(string," "); ptr != NULL; ptr = strtok(NULL," ")) {
            argv[argc++] = ptr;
        }

        argv[argc] = NULL;
#else
	//printf("command=[%s]\n",command);
	//Buffer overflow alert
	ret=wordexp(command, &p, 0);
	if(ret!=0){
	  my_fatal("pipex_open(): %s: Unable to parse command.\n",command);
	}
	argc = p.we_wordc;
	if(argc>=256-2){
	  my_fatal("pipex_open(): %s: Too many arguments.\n",command);
	}
	for(i=0;i<argc;i++){
	  argv[i] = p.we_wordv[i];
	}
	//	int i;
	//for(i=0;i<argc;i++){
	//  printf("[%s]",argv[i]);
	//}
	//printf("\n");
	argv[argc] = NULL;
#endif        
      // create the pipes
        
        if (pipe(from_child) == -1) {
            my_fatal("pipex_open(): pipe(): %s\n",strerror(errno));
        }
        
        if (pipe(to_child) == -1) {
            my_fatal("pipex_open(): pipe(): %s\n",strerror(errno));
        }
        
            // create the child process 
        
        pipex->pid = fork();
        
        if (pipex->pid == -1) {
            
            my_fatal("pipex_open(): fork(): %s\n",strerror(errno));
            
        } else if (pipex->pid == 0) {
            
                // child 
            
                // close unused pipe descriptors to avoid deadlocks
            
            my_close(from_child[0]);
            my_close(to_child[1]);
            
                // attach the pipe to standard input
            
            my_dup2(to_child[0],STDIN_FILENO);
            my_close(to_child[0]);
            
                // attach the pipe to standard output
            
            my_dup2(from_child[1],STDOUT_FILENO);
            my_close(from_child[1]);
            
                // attach standard error to standard output
                // commenting this out gives error messages on the console
            
            my_dup2(STDOUT_FILENO,STDERR_FILENO); 
            
            if(chdir(working_dir)){
                printf("%s pipex_open(): %s: %s\n",
		       PIPEX_MAGIC,
		       working_dir,
		       strerror(errno));
		goto wait_for_eof;
            }
            
            // launch the new executable file

            execvp(argv[0],&argv[0]);
            
                // execvp() only returns when an error has occured

	    printf("%s pipex_open(): execvp(): %s: %s\n",
		   PIPEX_MAGIC,
		   argv[0],
		   strerror(errno));
	wait_for_eof:
	    while(fgets(string,StringSize,stdin));
	    exit(EXIT_SUCCESS);
            
        } else { // pid > 0
            
            ASSERT(pipex->pid>0);
            
                // parent 
            
                // close unused pipe descriptors to avoid deadlocks
            
            my_close(from_child[1]);
            my_close(to_child[0]);
            
                // fill in the pipex struct
            
            pipex->io->in_fd = from_child[0];
            pipex->io->out_fd = to_child[1];
            pipex->state|=PIPEX_ACTIVE; // can we test if this really TRUE?
            
            io_init(pipex->io);
        } 
    }
}

void pipex_wait_event(pipex_t *pipex[]){

    fd_set set[1];
    pipex_t *p;
    int fd_max;
    int val;
    pipex_t **q;

    q=pipex;

        // init

   FD_ZERO(set);
   fd_max = -1; // HACK
   while((p=*(q++))!=NULL){
       ASSERT(p->io->in_fd>=0);
       FD_SET(p->io->in_fd,set);
       if (p->io->in_fd > fd_max){
           fd_max = p->io->in_fd;
       }
   }
   
   // wait for something to read (no timeout)

   ASSERT(fd_max>=0);
   val = select(fd_max+1,set,NULL,NULL,NULL);
   if (val == -1 && errno != EINTR) my_fatal("pipex_wait_event(): select(): %s\n",strerror(errno));

   q=pipex;
   if (val > 0) {
       while((p=*(q++))!=NULL){
           if (FD_ISSET(p->io->in_fd,set) /*&& !io_line_ready(p->io)*/){
	       io_get_update(p->io); 
	   }
       }
   }    
}

// pipex_active()

bool pipex_active(pipex_t *pipex){
    return (pipex->state&PIPEX_ACTIVE)!=0;
}

// pipex_eof()

bool pipex_eof(pipex_t *pipex){
    return (pipex->state&PIPEX_EOF)!=0;
}


// pipex_set_priority()

void pipex_set_priority(pipex_t *pipex, int value){
    if(pipex->pid!=-1){
        setpriority(PRIO_PROCESS,pipex->pid,value);
    }
}

// pipex_set_affinity()

void pipex_set_affinity(pipex_t *pipex, int value){
    my_log("POLYGLOT Setting affinity is not yet implemented on posix\n");
}

// pipex_send_eof()

void pipex_send_eof(pipex_t *pipex){
    io_close(pipex->io);
}

// pipex_exit()

/* This routine waits for kill_timeout milliseconds for 
 * the process to exit by itself. If that doesn't
 * happen it will kill the process.
 */



void pipex_exit(pipex_t *pipex, int kill_timeout){
    int status;
    int elapsed_time;
    bool exited;
    int ret;

    my_log("POLYGLOT Waiting for child process to exit.\n");

    elapsed_time=0;
    exited=FALSE;
    ret=0;
    while(elapsed_time<kill_timeout){
      ret=waitpid(pipex->pid,&status,WNOHANG);
      if(ret==0){
	my_log("POLYGLOT Child has not exited yet. Sleeping %dms.\n", WAIT_GRANULARITY);
	my_sleep(WAIT_GRANULARITY);
	elapsed_time+=WAIT_GRANULARITY;
      }else{
	exited=TRUE;
	break;
      }
    }
    if(!exited){
      my_log("POLYGLOT Child wouldn't exit by itself. Terminating it.\n");
      kill(pipex->pid,SIGKILL);
      waitpid(pipex->pid,&status,0);    
    }
    if(WIFEXITED(status)){
      if(pipex->quit_pending){
	my_log("POLYGLOT Child exited with status %d.\n",WEXITSTATUS(status));
      }else{
	// Suppress further messages.
	pipex->quit_pending=TRUE;
	my_fatal("pipex_exit(): %s: child exited with status %d.\n",pipex->command,WEXITSTATUS(status));
      }
    }else if(WIFSIGNALED(status)){
      if(pipex->quit_pending){
	my_log("POLYGLOT pipex_exit(): %s: child terminated with signal %d.\n",pipex->command,WTERMSIG(status));
      }else{
	// Suppress further messages.
	pipex->quit_pending=TRUE;
	  my_fatal("pipex_exit(): %s: child terminated with signal %d.\n",pipex->command,WTERMSIG(status));
      }
    }
    return;
}

// pipex_get_buffer()

char * pipex_get_buffer(pipex_t *pipex){
  return pipex->io->in_buffer;
}

// pipex_readln()

bool pipex_readln(pipex_t *pipex, char *string){
    while (!io_line_ready(pipex->io)) {
	io_get_update(pipex->io);
   }
   if (!io_get_line(pipex->io,string,StringSize)) { // EOF
       string[0]='\0';
       pipex->state|=PIPEX_EOF;
       return FALSE;
   }
   if(strncmp(PIPEX_MAGIC,string,strlen(PIPEX_MAGIC))==0){
     my_fatal("%s\n",string+strlen(PIPEX_MAGIC)+1);
   }

   return TRUE;
}

// pipex_readln_nb()

bool pipex_readln_nb(pipex_t *pipex, char *string){

    while(!pipex->io->in_eof && !io_line_ready(pipex->io) && io_peek(pipex->io)){
      io_get_update(pipex->io);
  }
  
  if(io_line_ready(pipex->io)){
    return pipex_readln(pipex,string);
  }else if(pipex->io->in_eof){
    string[0]='\0';
    pipex->state|=PIPEX_EOF;
    return FALSE;
  }else {  
    string[0]='\0';
    return FALSE;
  }
}

// pipex_write()

void pipex_write(pipex_t *pipex, const char *string){
       io_send_queue(pipex->io,"%s",string);
}


// pipex_writeln()

void pipex_writeln(pipex_t *pipex, const char *string){
       io_send(pipex->io,"%s",string);
}

// my_close()

static void my_close(int fd) {

   ASSERT(fd>=0);

   if (close(fd) == -1) my_fatal("my_close(): close(): %s\n",strerror(errno));
}

// my_dup2()

static void my_dup2(int old_fd, int new_fd) {

   ASSERT(old_fd>=0);
   ASSERT(new_fd>=0);

   if (dup2(old_fd,new_fd) == -1) my_fatal("my_dup2(): dup2(): %s\n",strerror(errno));
}


#endif
