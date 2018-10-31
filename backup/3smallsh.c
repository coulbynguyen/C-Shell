#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

char** getArguments(char* userInput, int* argc){
	      /*this section of code tokens the inputed string into arguments so that it can passed on to the correct function*/
	/*      char delim[2] = " ";
	      char* argToken;
	      int totalArg = 0;
	      int i;

	      argToken = strtok(userInput, delim);
	      arguments[totalArg] = malloc(64*sizeof(char));
	      strcpy(arguments[totalArg], argToken);
	      totalArg++;
	      */
	      /*the following while loop will put the rest of the arguments into the argument array including the NULL*/
	      /*while(argToken != NULL){
		 argToken=strtok(NULL, delim);
		 arguments[totalArg] = malloc(64*sizeof(char));
		 strcpy(arguments[totalArg], argToken);
		 totalArg++;
	      }
	      char **argv = malloc(totalArg*sizeof(char*));
	      for(i = 0; i < totalArg; i++){
		 argv[i] = malloc(64*sizeof(char));
		 strcpy(argv[i], arguments[i]);
		 free(arguments[i]);
	      }
		*/

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
	totalArg++;
	
	while(token != NULL){
		token = strtok(NULL, delim);
		if(token != NULL){
		   strcpy(argv[totalArg], token);
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


   while(1){
      printf(": "); fflush(stdout);
      characters = getline(&userInput, &userInputSize, stdin);
      /*this whole section of code operates only if the userInput isnt a comment or blank line*/
      if((userInput[0] != '#') && (strcmp(userInput, "\n") != 0)){

	 userInput[strlen(userInput)-1] = '\0';

	 if(strcmp(userInput, "exit") == 0){
	    exit(0);
	 }
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
	    free(newUserInput);
	    free(pidText);
	 }
	/*get the command line arguments for executing calls*/

	 argv = getArguments(userInput, &argc);
	 /*printf("%d", argc);*/
	 /*change directory command*/
	 if(strcmp(argv[0], "cd") == 0){
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

		   	for(i = 0; i < argc-1; i++){
		   	/*for(i = 0; i < argc; i++){*/
				if(strcmp(argv[i], "<") == 0){
				   sourceFD = open(argv[i+1], O_RDONLY);
			/*	   printf("source == %d\n", sourceFD);*/
				   if(sourceFD == -1){
				      perror("source open()");
				      exit(1);
				   }
				   result = dup2(sourceFD, 0);	
				   if(result == -1){
				      perror("source dup2()");
				      exit(2);
				   }
				   argv[i] = NULL;
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
				   /*
				   strcpy(argv[i], " ");
				   strcpy(argv[i+1], " ");
				   argv[i+1] = NULL;
				   argv[i] = "";
				   argv[i+1] = "";
				   */

				
				}
			}
			execvp(argv[0], argv);
		}
		else{
		   /*parent waits until child completes*/
		   waitpid(spawnPid, &childExitMethod, 0);
		}

	 }
      }
   }
}










