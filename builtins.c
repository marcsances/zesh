/*
  Zero Shell
  Copyright (c) 2015, Marc Sances
  All rights reserved.
 
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
  3. Neither the name of copyright holders nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#define MAX_DIRECTORY_STACK 50
#define SIGDONE 42
char* shellname;
void handleerr();
int echo(int argc, char** argv) {
	int i;
	for (i=1; i<argc; i++) {
		printf("%s",argv[i]);
		if (i<(argc-1)) printf(" ");
	}
	printf("\n");
	return 0;
}

int cd(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"Usage: cd <directory>\n");
		return 2;
	}
	if (chdir(argv[1])==-1) {
		fprintf(stderr,"%s: cannot cd to %s: ",shellname,argv[1]);
		handleerr();
	}
	return 0;
}

int exec(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"Usage: exec <filename>\n");
		return 2;
	}
	execvp(argv[1],&argv[1]); // removes exec from argv stack
	return 0;
}

int cpid=-1;

static void int_catch(int sig) {
	if (cpid!=-1) {
		kill(cpid,SIGINT); // send SIGINT to child
	}
}
int exitcode=256;
int lastpid=-1;
int run(int argc, char** argv) {
	lastpid=fork();

	switch (lastpid) {
		case -1:
			fprintf(stderr,"%s: fork: operation failed\n",shellname);
			return 1;
			break;
		case 0:
			// child process
			execvp(argv[0],argv);
			// if we're here, something went wrong.
			fprintf(stderr,"%s: cannot execute %s: ",shellname,argv[0]);
			handleerr();
			exit(1); // finish child
			break;
		default:
			// shell process
			cpid=lastpid;
			if (signal(SIGINT,int_catch)==SIG_ERR) {
				fprintf(stderr,"%s: error while setting up signal handler\n",shellname);
			}
			waitpid(lastpid,&exitcode,0);
			cpid=-1;
			return exitcode;
			break;	
	}
	return 0;
}

char *dstack[MAX_DIRECTORY_STACK];
int top=0;

int dirs(int argc, char** argv) {
	if (argc>1 && (strcmp(argv[1],"-c")==0)) {
		top=0; // clear stack
	} else {
		// display directory stack
		int i;
		for (i=0; i<top; i++) {
			printf("%s ",dstack[i]);
		}
		printf("\n");
	}
	return 0;
}

int pushd(int argc, char** argv) {
	if (top==MAX_DIRECTORY_STACK) {
		fprintf(stderr,"%s: cannot pushd: directory stack full\n",shellname);
		return 1;
	} else {
		dstack[top]=getcwd(malloc(PATH_MAX),PATH_MAX); // push current directory to stack
		top++;
		cd(argc,argv); // change to requested directory
		dirs(1,NULL);  // stat stack status
	}
	return 0;
}

int popd(int argc, char** argv) {
	// do nothing if stack is empty
	if (top!=0) {
		if (chdir(dstack[top-1])==-1) {
			fprintf(stderr,"%s: cannot cd to %s: ",argv[1],shellname);
			handleerr();
			return 1;
		}
		top--;
	}
	return 0;
}

int _c(char* a, char* b) {
	return strcmp(a,b)==0;
}

int helpw(int argc, char** argv) {
	if (argc==8 && _c(argv[1],"my") && _c(argv[2],"os") && _c(argv[3],"defaulted") && _c(argv[4],"me") && _c(argv[5],"to")
	&& _c(argv[6],"this") && _c(argv[7],"shell")) {
		printf("Blame the developer of your OS.\n");
		return 1;
	} else return 0;
}


int help(int argc, char** argv) {
	if (helpw(argc,argv)) return 42;
	// real help command starts here
	printf("List of currently supported builtins:\n");
	printf("cd, chdir, dirs, echo, exec, exit, exitc, help, popd, pushd, pwd, runpipe, setpipe\n");
	return 0;
}

int exitc(int argc, char** argv) {
	if (lastpid!=-1 && exitcode!=256) {
		printf("Last process executed with PID %d and exit code %d\n",lastpid,exitcode);
		return 0;
	} else {
		printf("Couldn't execute last process or unknown exit code.\n");
		return 32;
	}
}

int pwd(int argc, char** argv) {
	printf("%s\n",getcwd(malloc(PATH_MAX),PATH_MAX));
	return 0;
}


void handleerr() {
	switch (errno){
					case E2BIG:
						fprintf(stderr,"list of arguments too large\n");
						break;
					case EACCES:
						fprintf(stderr,"permission denied\n");
						break;
					case EFAULT:
						fprintf(stderr,"segmentation fault\n");
						break;
					case EIO:
						fprintf(stderr,"I/O error\n");
						break;
					case EISDIR:
						fprintf(stderr,"is a directory\n");
						break;
					case ENOTDIR:
						fprintf(stderr,"not a directory\n");
						break;
					case ELIBBAD:
						fprintf(stderr,"not a valid executable\n");
						break;
					case ELOOP:
						fprintf(stderr,"too many redirections\n");
						break;
					case EMFILE:
						fprintf(stderr,"file limit for this shell exceeded\n");
						break;
					case ENAMETOOLONG:
						fprintf(stderr,"file name too long\n");
						break;
					case ENFILE:
						fprintf(stderr,"file limit for this system exceeded\n");
						break;
					case ENOENT:
						fprintf(stderr,"file or directory does not exist\n");
						break;
					default:
						fprintf(stderr,"unknown error\n");
						break;
	}
}

int pipeargc=0;
char** pipeargv=NULL;

int setpipe(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"Usage: setpipe <command>\n");
		return 2;
	}
	pipeargc=argc-1;
	pipeargv=&argv[1];
	return 1;
}

void sd_hd(int sig) {
	
}
int _startpipe(int* fd, int argc1, char **argv1, int argc2, char **argv2) {
	int pid1 = fork();
	int pid2;
	int exitc1;
	int exitc2;
	signal(SIGDONE,sd_hd);
	switch (pid1) {
		case -1:
			fprintf(stderr,"%s: fork: operation failed",shellname);
			kill(pid2,SIGKILL);
			return 1;
		case 0:
			// child process
			close(0);	// close stdin
			dup(fd[0]); // connect stdin with pipe read end
			close(fd[1]); // close write end of pipe
			int c2 = run_cmd(argc2,argv2);
			close(fd[0]); // close pipe read end
			close(1); // close stdout
			exit(c2); // terminate child
			break;
		default:
			// parent process
			pid2=fork();
			switch (pid2) {
				case -1:
					fprintf(stderr,"%s: fork: operation failed",shellname);
					kill(pid1,SIGKILL);
					return 1;
				case 0:
					// child process
					close(1); // close stdout
					dup(fd[1]); // connect stdout with pipe write end
					close(fd[0]); // close read end of pipe 
					int c = run_cmd(argc1,argv1);
					close(fd[1]); // close pipe write end
					close(0); // close stdin
					kill(pid1,SIGTERM); // terminate child 1
					kill(getppid(),SIGDONE);
					exit(c); // terminate child 
					
					break;
			
			}
			
	}
	
	pause();
	fflush(stdout);
	lastpid=pid2;
	exitcode=exitc2; 
	pipeargc=0;
	pipeargv=NULL;
	return exitc2;
}

int runpipe(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"Usage: runpipe <command>\n");
		return 2;
	} else if (pipeargc==0 || pipeargv==NULL ){
		fprintf(stderr,"%s: cannot create pipe: no left command set, use setpipe to set left command\n",shellname);
		return 2;
	} else {
		int fd[2];
		pipe(fd);
		return _startpipe(fd,pipeargc,pipeargv,argc-1,&argv[1]);
	}
}

int run_cmd(int argc, char **argv) {
	if (strcmp(argv[0],"echo")==0) {
			return echo(argc,argv);
	} else if (strcmp(argv[0],"cd")==0 || strcmp(argv[0],"chdir")==0) {
			return cd(argc,argv);
	} else if (strcmp(argv[0],"exit")==0) {
			fprintf(stderr,"exit\n");
			exit(0);
			return 0;
	} else if (strcmp(argv[0],"exec")==0) {
			return exec(argc,argv);
	} else if (strcmp(argv[0],"pushd")==0) {
			return pushd(argc,argv);
	} else if (strcmp(argv[0],"popd")==0) {
			return popd(argc,argv);
	} else if (strcmp(argv[0],"dirs")==0) {
			return dirs(argc,argv);
	} else if (strcmp(argv[0],"help")==0) {
			return help(argc,argv);
	} else if (strcmp(argv[0],"exitc")==0) {
			return exitc(argc,argv);
	} else if (strcmp(argv[0],"pwd")==0) {
			return pwd(argc,argv);
	} else if (strcmp(argv[0],"setpipe")==0) {
			return setpipe(argc,argv);
	} else if (strcmp(argv[0],"runpipe")==0) {
			return runpipe(argc,argv);
	} else {
			return run(argc,argv);
	}
}
