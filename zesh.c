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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "builtins.c"
#include <limits.h>
char* shellname;
int echoon = 1;
long wds;
void init() {
	printf("Zero Shell (c) 2015, Marc Sances\n");
}


void print_prompt() {
	char* curdir = getcwd(malloc(PATH_MAX),PATH_MAX);
	printf("%s:%s=> ",getlogin(),curdir);
}

void run_cmd(int argc, char **argv) {
	if (strcmp(argv[0],"echo")==0) {
			echo(argc,argv);
	} else if (strcmp(argv[0],"cd")==0 || strcmp(argv[0],"chdir")==0) {
			cd(argc,argv);
	} else if (strcmp(argv[0],"exit")==0) {
			exit(0);
	} else if (strcmp(argv[0],"exec")==0) {
			exec(argc,argv);
	} else if (strcmp(argv[0],"pushd")==0) {
			pushd(argc,argv);
	} else if (strcmp(argv[0],"popd")==0) {
			popd(argc,argv);
	} else if (strcmp(argv[0],"dirs")==0) {
			dirs(argc,argv);
	} else {
			run(argc,argv);
	}
}

void parse_cmd(char* command) {
	int bs=2048;
	int s=0;
	char **arr=malloc(bs);
	int i=0;
	arr[i] = strtok(command," ");
	s+=strlen(arr[i]);
	while(arr[i]!=NULL)
	{
   		arr[++i] = strtok(NULL," ");
		// TODO: implement a working buffer expansion
	}
	run_cmd(i,arr);
}

char* read_command() {
	long i = 0;
	char* cmd;
	cmd = (char*)malloc(256 * sizeof(char));
	char c = fgetc(stdin);
	int newline = 0;
	while (newline==0) {
		while (c!='\n' && c!=EOF && i<256) {
			cmd[i]=c;
			i++;
			c = fgetc(stdin);
		}
		if (c=='\n' || c==EOF) {
			newline=1;
			cmd[i]=0;
			
		} else {
			// reallocate buffer and continue
			cmd = (char*) realloc(cmd, i * sizeof(char) + 256);
		}
		
	}
	return cmd;
}

int main(int argc, char **argv) {
	init();
	shellname=malloc(strlen(argv[0])*sizeof(char));
	strcpy(shellname,argv[0]);
	while (1) {
		char* command;
		if (echoon==1) print_prompt();
		command=read_command();
		parse_cmd(command);
	}
}