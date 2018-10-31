#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int foregroundOnly = 0; /*if foreground only is 0 then its background processes can occur if foregroundONly is 1 then it can only do foreground processes no background processes*/

/*this function catches the sigint signal and does nothing!
 * that way the parent process is still running however
 * if you cast Sigint on a child process that was execd it
 * stops that process!*/
void catchSIGINT(int signo){
   /*do nothing*/
   /*	char* message = "CTRL-C / SIGINT CAUGHT: STAY IN SHELL\n";
	write(STDOUT_FILENO, message, 39);*/
/*   char message[64];
   sprintf(message, "need to kill %d\n", signo);
   write(STDOUT_FILENO, message, 15); */

}

/*this function catches the sigtstp signal and negates the value in 
 * foregroundOnly, if it was 1 then it was 0 and vice versa this 
 * way it creates an off mode for the foreground only thingy*/
void catchSIGTSP(int signo){
   	char* message = "\nEntering foreground-only mode (& is now ignored)\n: \0\0\0\0";
	char* message2 = "\nExiting foreground-only mode\n: \0\0\0\0\0";
	if(foregroundOnly == 0){
		foregroundOnly = 1;
		write(STDOUT_FILENO, message, 57);
	}
	else{
		foregroundOnly = 0;
		write(STDOUT_FILENO, message2, 34);
	}

}
/*this function tokens the user input into arguments
 * for *argv[] to be passed to execvp*/
char** getArguments(char* userInput, int* argc){
   char delim[2] = " ";
   char *token;
   int totalArg = 0;
   int i;
   char* userInput2 = malloc(256*sizeof(char));
   strcpy(userInput2, userInput);

   /*this section of code gets the total count of arguments*/
   token = strtok(userInput, delim);
   totalArg++;
   while(token != NULL){
      token = strtok(NULL, delim);
      totalArg++;
   }

   /*this section of code creates the correct amount of space to hold the counted arguments*/
   char **argv = malloc(totalArg * sizeof(char*));
   for(i = 0; i < totalArg; i++){
      argv[i] = malloc(32*sizeof(char*));
   }

   totalArg = 0;
   /*this section of code tokens the first sub string to the delimiter aka the space between the words
    * and stores that into the array*/
   token = strtok(userInput2, delim);
   strcpy(argv[totalArg], token);
   /*this block of code checks to see if it is in foreground mode or not
    * if it is then if the string is the ampersand it changes that value to null*/
   if(foregroundOnly == 1){
      if(strcmp(argv[totalArg], "&") == 0){
	argv[totalArg] = NULL;
      }
   }
   totalArg++; 
   /*this for loop iterates through so that all the arguments are in the array
    * and places a NULL at the very end so that execvp knows there arent any more arguements*/
   while(token != NULL){
      token = strtok(NULL, delim);

      if(token != NULL){
	 strcpy(argv[totalArg], token);

   /*this block of code checks to see if it is in foreground mode or not
    * if it is then if the string is the ampersand it changes that value to null*/
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
   /*this block of code sets up the signal handler for sigint
    * it points to the function, as well as making sure that getline resumes
    * even if it is interrupted*/
   struct sigaction SIGINT_action = {0};
   SIGINT_action.sa_handler = catchSIGINT;
   sigfillset(&SIGINT_action.sa_mask);
   SIGINT_action.sa_flags = SA_RESTART;
   sigaction(SIGINT, &SIGINT_action, NULL);

   /*this block of code sets up the signal handler for sigint
    * it points to the sigtsp function and its flags are also set
    * to reseet so that getline is still functionable*/
   struct sigaction SIGTSP_action = {0};
   SIGTSP_action.sa_handler = catchSIGTSP;
   sigfillset(&SIGTSP_action.sa_mask);
   SIGTSP_action.sa_flags = SA_RESTART;
   sigaction(SIGTSTP, &SIGTSP_action, NULL);


   while(1){
      backgroundProcess = 0;

      /*this section of code waits for any background processes so that there are no zombie children out there
       * it goes through the list of background pids and if it was complete it sets the pid in the array to -1 
       * so that it doesnt keep responding that a process is complete*/
      for(i = 0; i < numBackPids; i++){
	 if(backgroundPIDS[i] != -1){
	    anyComplete = waitpid(backgroundPIDS[i], &backgroundChildExitMethod, WNOHANG);
	    if(anyComplete != 0){
	       if(WIFSIGNALED(backgroundChildExitMethod) != 0){
		  int termSignal = WTERMSIG(backgroundChildExitMethod);
		  printf("background pid %d is done: terminated by signal %d\n", backgroundPIDS[i], termSignal); fflush(stdout);
	       }
	       else{
		  int exitStatus = WEXITSTATUS(backgroundChildExitMethod);
		  printf("background pid %d is done: exit value %d\n", backgroundPIDS[i], exitStatus); fflush(stdout);
	       }
	       backgroundPIDS[i] = -1;
	    }
	 }
      }
      
      /*this section of code displays that semicolon so that the user knows its ok to enter a command and flushes that to the screen
       * and gets the user input stored into the correct variables*/
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
	/*this section of code checks to see if there is a newline character at the end of the string and changes that to a null terminator
	 * so that it is easier to compare string*/
	 if(userInput[strlen(userInput)-1] == '\n'){
	    userInput[strlen(userInput)-1] = '\0';
	 }
	/*this function exits the program with a exit value of 0 and the exit function kills all other processes before exiting itself*/
	 if(strcmp(userInput, "exit") == 0){
	    exit(0);
	 }

	 /*get the command line arguments for executing calls*/
	 argv = getArguments(userInput, &argc);

	 /*this block of code checks to see if the user entered a ampersand and if they did
	  * it means that the process should be a background process and sets the background process
	  * variable to true*/
	 if(argc >= 2 && argv[argc-2] != NULL && (strcmp(argv[argc-2], "&") == 0)){
	    backgroundProcess = 1;
	 }

	 /*this block of code compares to see if the user inputed status as their command
	  * it doesnt even look at the rest of the arguments and outputs the status variable
	  * if the status was received as an exit value it prints a different string
	  * if the status was received as a signal value*/
	 if(strcmp(argv[0], "status") == 0){
	    if(isExit == 1){
	       printf("exit value %d\n", status); fflush(stdout);
	    }	   
	    else{
	       printf("terminated by signal %d\n", status); fflush(stdout);
	    }
	 }
	 /*this block of code changes the directory to that of what is in argv[1]
	  * if argv[1] is NULL then it changes directory to the parent directory*/
	 else if(strcmp(argv[0], "cd") == 0){
	    if(argv[1] != NULL){
	       chdir(argv[1]);
	       char cwd[256];
	       getcwd(cwd, sizeof(cwd));	   
	    }
	    else{
	       chdir(getenv("HOME"));
	       char cwd[256];
	       getcwd(cwd, sizeof(cwd));	   
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

	       /*this block of code handles I/O so it parses the arguments to see if there are < > and redirects
		* the streams accordingly*/
	       for(i = 0; i < argc-1; i++){
		  /*this section opens the source file descriptor and then duplicates that stdin points to where
		   * the source file descriptor points and sets the < to null since there shouldnt be any more arguments
		   * past that point*/
		  if(argv[i] != NULL){
		     if(strcmp(argv[i], "<") == 0){
			sourceFD = open(argv[i+1], O_RDONLY);
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
		     }
		     /*this section opens the target file descriptor and then duplicates that stdout points to where
		      * the target file descriptor points and sets the > to null since there shouldnt be any more arguments
		      * past that point*/
		     else if(strcmp(argv[i], ">") == 0){
			targetFD = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
		     }
		  }
	       }
	       /*this section of code handles the redirection of background processes*/
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
		     /*if there wasnt a designated file to output to*/
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
	       
	       /*this one line of code does most of the shell work by inputing the name of the 
		* command, and then all of the arguments. however if the exec fails it should print out
		* no such file or directory and then exit with a exit status of 2*/
	       execvp(argv[0], argv);
	       printf("%s: no such file or directory\n", argv[0]);
	       exit(2);
	       }
	       else{
		  int completed;
		  /*this block of code adds the spawnpid to the array of background pids and increments the total number
		   * it also prints out the PID of that process to stdout*/
		  if(backgroundProcess == 1){
		     backgroundPIDS[numBackPids] = spawnPid;
		     numBackPids++;
		     printf("background pid is %d\n", spawnPid); fflush(stdout);
		  }
		  else{
		     /*parent waits until child completes*/
		     waitpid(spawnPid, &childExitMethod, 0);
		     /*this section of code sets the status code if the signal was the cause of exit
		      * then this if statement is exectuted and isExit is set to 0 since
		      * it wasnt an exit status. And vice versa for the else statement*/
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
      /*this section of code waits for any background processes so that there are no zombie children out there
       * it goes through the list of background pids and if it was complete it sets the pid in the array to -1 
       * so that it doesnt keep responding that a process is complete
       * this code is ran at the end of the code as well just in case a process above kills a process that was in the background
       * you gotta reap that child!*/
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










