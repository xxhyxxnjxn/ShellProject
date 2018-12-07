#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>

#define TRUE	1
#define FALSE	0

typedef struct { // 커맨드 구조체
    char* name;
    char* desc;
    int ( *func )( int argc, char* argv[] ); // 함수포인터. 사용할 함수들의 매개변수를 맞춰줌
} COMMAND;

struct sigaction act;

int cmd_cd( int argc, char* argv[] ){ //cd : change directory
    if( argc == 1 ){
      chdir( getenv( "HOME" ) );
    }

    else if( argc == 2 ){
        if( chdir( argv[1] ) )
            printf( "No directory\n" );
    }
    else{
      printf( "USAGE: cd [dir]\n" );
    }
    return TRUE;
}

static COMMAND builtin_cmds[] =
{
  { "cd", "change directory", cmd_cd },
};

int makeargv(char *s, const char *delimiters, char** argvp, int MAX_LIST)
{
	int i = 0;
	int numtokens = 0;
	char *snew = NULL;

	if( (s==NULL) || (delimiters==NULL) )
	{
		return -1;
	}

	snew = s+strspn(s, delimiters);

	argvp[numtokens]=strtok(snew, delimiters);

	if( argvp[numtokens] !=NULL)
		for(numtokens=1; (argvp[numtokens]=strtok(NULL, delimiters)) != NULL; numtokens++)
		{
			if(numtokens == (MAX_LIST-1)) return -1;
		}

	if( numtokens > MAX_LIST) return -1;

	return numtokens;
}

void fatal(char *str)
{
	perror(str);
	exit(1);
}

int getargs(char *cmd, char **argv) {
  int narg = 0;
  while (*cmd) {
    if (*cmd == ' ' || *cmd == '\t')
      *cmd++ = '\0';
    else {
      argv[narg++] = cmd++;

      while (*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
        cmd++;
      }
    }
    argv[narg] = NULL;

    return narg;
}

void commend_execvp(char *cmdlist)
{
    char* cmdargs[10];

    if(makeargv(cmdlist, " \t", cmdargs, 10) <= 0)
		fatal("makeargv_cmdargs error");

    execvp(cmdargs[0], cmdargs); // 5
    fatal("exec error");
}

void commend_grp(char *cmdgrp){
	commend_execvp(cmdlist[i]);
}

void zombie_handler(int signo)
{
    pid_t pid ;
    int stat ;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated normaly\n", pid) ;
}

void commend(char* cmdline){
  int narg;
  pid_t pid;

  int i=0, j=0;
  char* cmdgrp[10];
  char* cmdvector[10];
  char cmdgrptemp[BUFSIZ];
  int numtokens = 0;

  narg = makeargv(cmdline, ";", cmdgrp, 10);

  for(i=0; i<narg; ++i)
  {
      memcpy(cmdgrptemp, cmdgrp[i], strlen(cmdgrp[i]) + 1);
      numtokens = makeargv(cmdgrp[i], " \t", cmdvector, 10);

      for( j = 0; j < sizeof( builtin_cmds ) / sizeof( COMMAND ); j++ ){
        if( strcmp( builtin_cmds[j].name, cmdvector[0] ) == 0 ){
          builtin_cmds[j].func( numtokens , cmdvector );
          return;
        }
      }

      switch(pid=fork())
      {
          case -1:
              perror("fork failed");
          case  0:
              commend_grp(cmdgrptemp);
          default:
      }
    }
}

int main() {
  int i;
	sigset_t set;

	sigfillset(&set);
	sigdelset(&set,SIGCHLD);
	sigprocmask(SIG_SETMASK,&set,NULL);

	act.sa_flags = SA_RESTART;
	sigemptyset(&act.sa_mask);
	act.sa_handler = zombie_handler;
	sigaction(SIGCHLD, &act, 0);

  char buf[BUFSIZ];

  while (1) {
    printf("shell [%s]$ ", get_current_dir_name() );
    fgets(buf, BUFSIZ, stdin);
    buf[ strlen(buf) -1] ='\0';
    commend(buf);
  }
  return 0;
}
