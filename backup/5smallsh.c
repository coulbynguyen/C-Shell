#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int foregroundOnly = 0; /*if foreground only is 0 then its background processes can occur if foregroundONly is 1 then it can only do foreground processes no background processes*/


void catchSIGINT(int signo){
   /*do nothing*/
   /*	char* message = "CTRL-C / SIGINT CAUGHT: STAY IN SHELL\n";
	write(STDOUT_FILENO, message, 39);*/
/*   char message[64];
   sprintf(message, "need to kill %d\n", signo);
   write(STDOUT_FILENO, message, 15); */

}

void catchSIGTSP(int signo){
   	char* message = "\nEntering foreground-only mode (& is now ignored)\n: ";
	char* message2 = "\nExiting foreground-only mode\n: ";
	if(foregroundOnly == 0){
		foregroundOnly = 1;
		write(STDOUT_FILENO, message, 57);
	}
	else{
		foregroundOnly = 0;
		write(STDOUT_FILENO, message2, 34);
	}

}

char** getArguments(char* userInput, int* argc){
   char delim[2] = " ";
   char *token;
   int totalArg = 0;
   int i;
   char* userInput2 = malloc(256*sizeof(char));
   strcpy(userInput2, userInput);

   token = strtok(userInput, delim);
   totalArg++;
   while(token != NULL){
      token = strtok(NULL, delim);
      totalArg++;
   }

   char **argv = malloc(totalArg * sizeof(char*));
   for(i = 0; i < totalArg; i++){
      argv[i] = malloc(32*sizeof(char*));
   }

   totalArg = 0;
   token = strtok(userInput2, delim);
   strcpy(argv[totalArg], token);
   if(foregroundOnly == 1){
      if(strcmp(argv[totalArg], "&") == 0){
	argv[totalArg] = NULL;
      }
   }
   totalArg++; 
   while(token != NULL){
      token = strtok(NULL, delim);

      if(token != NULL){
	 strcpy(argv[totalArg], token);

	 if(foregroundOnly == 1){
	    if(strcmp(argv[totalArg], "&") == 0){
	       argv[totalArg] = NULL;
	    }
	 }

      }
      else{
	 argv[totalArg] = NULL;
      }
      totalArg++;
   }
   *argc = totalArg;
   return argv;
   /*free()*/
}
int main(){
   char* userInput = malloc(256*sizeof(char));
   size_t userInputSize = 255; //to allow space for the null terminator
   size_t characters; //tell how many characters where read in
   char** argv;
   int argc;
   int i;
   int backgroundMode = 0; /* if backgroundMode = 0 program is in normal mode, if backgroundMode = 1 program is only in foreground mode only*/
   char *devPath = "/dev/null";
   int backgroundProcess = 0; /* if backgroundProcess = 0 the program should wait till finished else it should prompt a new command*/
   int status = 0;
   int backgroundPIDS[100];
   int numBackPids = 0;
   pid_t anyComplete;
   int backgroundChildExitMethod = -5;
   int isExit = 1;

   struct sigaction SIGINT_action = {0};
   SIGINT_action.sa_handler = catchSIGINT;
   sigfillset(&SIGINT_action.sa_mask);
   SIGINT_action.sa_flags = SA_RESTART;
   sigaction(SIGINT, &SIGINT_action, NULL);

   struct sigaction SIGTSP_action = {0};
   SIGTSP_action.sa_handler = catchSIGTSP;
   sigfillset(&SIGTSP_action.sa_mask);
   SIGTSP_action.sa_flags = SA_RESTART;
   sigaction(SIGTSTP, &SIGTSP_action, NULL);


   while(1){
      backgroundProcess = 0;
      for(i = 0; i < numBackPids; i++){
	 if(backgroundPIDS[i] != -1){
	    anyComplete = waitpid(backgroundPIDS[i], &backgroundChildExitMethod, WNOHANG);
	    if(anyComplete != 0){
	       if(WIFSIGNALED(backgroundChildExitMethod) != 0){
		  int termSignal = WTERMSIG(backgroundChildExitMethod);
		  printf("background pid %d is done: terminated by signal %d\n", backgroundPIDS[i], termSignal); fflush(stdout);
		  /*only foreground processes need status		     status = termSignal;*/
	       }
	       else{
		  int exitStatus = WEXITSTATUS(backgroundChildExitMethod);
		  printf("background pid %d is done: exit value %d\n", backgroundPIDS[i], exitStatus); fflush(stdout);
		  /*		     status = exitStatus;*/
	       }
	       backgroundPIDS[i] = -1;
	    }
	 }
      }


      /*while(anyComplete = waitpid(-1, &backgroundChildExitMethod, WNOHANG) != 0){
	it checks all processes while a process has been completed
	if(WIFSIGNALED != 0){
	printf(
	}
	}*/
      printf(": "); fflush(stdout);
      characters = getline(&userInput, &userInputSize, stdin);
      /*this whole section of code operates only if the userInput isnt a comment or blank line*/
      if((userInput[0] != '#') && (strcmp(userInput, "\n") != 0)){

	/* userInput[strlen(userInput)-1] = '\0';*/

	 /*This if statement checks to see if the text had any $$ to expand and does it and copies that back to the userInput*/
	 if(strstr(userInput, "$$") != NULL){
	    char* newUserInput = malloc(256*sizeof(char));
	    char spot[2] = "$$";
	    char* token;
	    int pid;
	    char* pidText = malloc(16*sizeof(char));
	    /*these 2 lines of code gets the pid and changes it to a cstring so that it can be strcatted to the string*/
	    pid = getpid();
	    sprintf(pidText, "%d", pid);

	    /*this block of code tokenizes the user inputted string on the $$ and then replaces it with the pid like it should be*/
	    token = strtok(userInput, spot);
	    strcpy(newUserInput, token);
	    strcat(newUserInput, pidText);
	    token = strtok(NULL, spot);
	    strcat(newUserInput,token);

	    /*this while loops adds the pid text if there was more than 1 tok ie tok was found again after therefore it was put into the case*/
	    while(token != NULL){
	       token=strtok(NULL, spot);
	       if(token != NULL){
		  strcat(newUserInput, pidText);
		  strcat(newUserInput, token);
	       }

	    }
	    strcpy(userInput, newUserInput);
	    /*free(newUserInput);
	      free(pidText);*/
	 }
	 if(userInput[strlen(userInput)-1] == '\n'){
	    userInput[strlen(userInput)-1] = '\0';
	 }
	 /*get the command line arguments for executing calls*/
	 if(strcmp(userInput, "exit") == 0){
	    /* wait*/
	    exit(0);
	 }

	 argv = getArguments(userInput, &argc);
	 if(argc >= 2 && argv[argc-2] != NULL && (strcmp(argv[argc-2], "&") == 0)){
	    backgroundProcess = 1;
	 }
	 if(strcmp(argv[0], "status") == 0){
	    if(isExit == 1){
	       printf("exit value %d\n", status);
	    }	   
	    else{
	       printf("terminated by signal %d\n", status);
	    }
	 }
	 else if(strcmp(argv[0], "cd") == 0){
	    if(argv[1] != NULL){
	       /*printf("%s\n", argv[1]);*/
	       chdir(argv[1]);
	       char cwd[256];
	       getcwd(cwd, sizeof(cwd));	   
	       /* printf("%s\n", cwd); */
	    }
	    else{
	       chdir(getenv("HOME"));
	       char cwd[256];
	       getcwd(cwd, sizeof(cwd));	   
	       /*printf("%s\n", cwd); */
	    }
	 }
	 /*built in commands*/
	 else{
	    pid_t spawnPid = -5;
	    int childExitMethod = -5;
	    spawnPid = fork();
	    /*child*/
	    if(spawnPid == 0){
	       int sourceFD;
	       int targetFD;
	       int result = 0;
	       int isSource = 0; /* if there was a source file entered into the command line 0 if no 1 if yes*/
	       int isTarget = 0; /* if there was a target file entered into the command line 0 if no 1 if yes*/
	       for(i = 0; i < argc-1; i++){
		  /*for(i = 0; i < argc; i++){*/
		  if(argv[i] != NULL){
		     if(strcmp(argv[i], "<") == 0){
			sourceFD = open(argv[i+1], O_RDONLY);
			/*	   printf("source == %d\n", sourceFD);*/
			if(sourceFD == -1){
			   perror("source open() ie BAD FILE INPUT");
			   exit(1);
			}
			result = dup2(sourceFD, 0);	
			if(result == -1){
			   perror("source dup2()");
			   exit(2);
			}
			argv[i] = NULL;
			isSource = 1;
			/*
			   argv[i+1] = NULL;
			   strcpy(argv[i], " ");
			   strcpy(argv[i+1], " ");
			   argv[i] = "";
			   argv[i+1] = "";
			   */

		     }
		     else if(strcmp(argv[i], ">") == 0){
			targetFD = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			/*printf("targetFD == %d\n", targetFD);*/
			if(targetFD == -1){
			   perror("target open()");
			   exit(1);
			}
			result = dup2(targetFD, 1);	
			if(result == -1){
			   perror("target dup2()");
			   exit(2);
			}
			argv[i] = NULL;
			isTarget = 1;
			/*
			   strcpy(argv[i], " ");
			   strcpy(argv[i+1], " ");
			   argv[i+1] = NULL;
			   argv[i] = "";
			   argv[i+1] = "";
			   */
		     }
		  }
	       }
	       if((argv[argc-2] != NULL) && (strcmp(argv[argc-2], "&") == 0)){
		  /*if the user intended this program to be run in the background*/
		  if(isSource == 0){
		     /*if there wasnt a designated file to read from*/
		     sourceFD = open(devPath, O_RDONLY);
		     if(sourceFD == -1){
			perror("source open()");
			exit(1);
		     }
		     result = dup2(sourceFD, 0);
		     if(result == -1){
			perror("source dup2()");
			exit(2);
		     }
		  }
		  if(isTarget == 0){
		     targetFD = open(devPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		     if(targetFD == -1){
			perror("target open()");
			exit(1);
		     }
		     result = dup2(targetFD, 1);
		     if(result == -1){
			perror("target dup2()");
			exit(2);
		     }

		  }
		  argv[argc-2] = NULL;
	       }
	       execvp(argv[0], argv);
	       exit(1);
	       }
	       else{
		  int completed;
		  if(backgroundProcess == 1){
		     backgroundPIDS[numBackPids] = spawnPid;
		     numBackPids++;
		     printf("background pid is %d\n", spawnPid); fflush(stdout);
		     /*completed = waitpid(spawnPid, &childExitMethod, WNOHANG);
		       if(completed != 0){
		       if(WIFSIGNALED(childExitMethod) != 0){
		       int termSignal = WTERMSIG(childExitMethod);
		       printf("background pid %d is done: terminated by signal %d\n", spawnPid, termSignal); fflush(stdout);
		       status = termSignal;
		       }
		       else{
		       int exitStatus = WEXITSTATUS(childExitMethod);
		       printf("background pid %d is done: exit value %d\n", spawnPid, termSignal)
		       this would be that the program exited with no signal and should change the status variable
		       }
		       }
		       */
		  }
		  else{
		     /*parent waits until child completes*/
		     waitpid(spawnPid, &childExitMethod, 0);
		     if(WIFSIGNALED(childExitMethod) != 0){
			int termSignal = WTERMSIG(childExitMethod);
			printf("terminated by signal %d\n", termSignal); fflush(stdout);
			status = termSignal;
			isExit = 0;
		     }
		     else{
			int exitStatus = WEXITSTATUS(childExitMethod);
			status = exitStatus;
			isExit = 1;
		     }
		  }
	       }
	    }
	 }
	 for(i = 0; i < numBackPids; i++){
	    if(backgroundPIDS[i] != -1){
	       anyComplete = waitpid(backgroundPIDS[i], &backgroundChildExitMethod, WNOHANG);
	       if(anyComplete != 0){
		  if(WIFSIGNALED(backgroundChildExitMethod) != 0){
		     int termSignal = WTERMSIG(backgroundChildExitMethod);
		     printf("background pid %d is done: terminated by signal %d\n", backgroundPIDS[i], termSignal); fflush(stdout);
		     /*only foreground processes need status		     status = termSignal;*/
		  }
		  else{
		     int exitStatus = WEXITSTATUS(backgroundChildExitMethod);
		     printf("background pid %d is done: exit value %d\n", backgroundPIDS[i], exitStatus); fflush(stdout);
		     /*		     status = exitStatus;*/
		  }
		  backgroundPIDS[i] = -1;
	       }
	    }
	 }
      }
   }










