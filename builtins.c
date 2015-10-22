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
#define MAX_DIRECTORY_STACK 50
char* shellname;
void handleerr();
void echo(int argc, char** argv) {
	int i;
	for (i=1; i<argc; i++) {
		printf("%s",argv[i]);
		if (i<(argc-1)) printf(" ");
	}
	printf("\n");
}

void cd(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"Usage: cd <directory>\n");
		return;
	}
	if (chdir(argv[1])==-1) {
		fprintf(stderr,"%s: cannot cd to %s: ",shellname,argv[1]);
		handleerr();
	}
}

void exec(int argc, char** argv) {
	if (argc<2) {
		fprintf(stderr,"Usage: exec <filename>\n");
		return;
	}
	execvp(argv[1],&argv[1]); // removes exec from argv stack
}

void run(int argc, char** argv) {
	int pid=fork();
	int exitc;
	switch (pid) {
		case -1:
			fprintf(stderr,"%s: fork: operation failed\n",shellname);
			break;
		case 0:
			// child process
			execvp(argv[0],argv);
			// if we're here, something went wrong.
			fprintf(stderr,"%s: cannot execute %s: ",shellname,argv[0]);
			handleerr();
			exit(0); // finish child
			break;
		default:
			// shell process
			waitpid(pid,&exitc,0);
			if (exitc!=0) {
				fprintf(stderr,"%s: process %d terminated with status code %d\n",shellname,pid,exitc);
			}
			break;	
	}
}

char *dstack[MAX_DIRECTORY_STACK];
int top=0;

void dirs(int argc, char** argv) {
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
}

void pushd(int argc, char** argv) {
	if (top==MAX_DIRECTORY_STACK) {
		fprintf(stderr,"%s: cannot pushd: directory stack full\n",shellname);
	} else {
		dstack[top]=getcwd(malloc(PATH_MAX),PATH_MAX); // push current directory to stack
		top++;
		cd(argc,argv); // change to requested directory
		dirs(1,NULL);  // stat stack status
	}
}

void popd(int argc, char** argv) {
	// do nothing if stack is empty
	if (top!=0) {
		if (chdir(dstack[top-1])==-1) {
			fprintf(stderr,"%s: cannot cd to %s: ",argv[1],shellname);
			handleerr();
		}
		top--;
	}
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