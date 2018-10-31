#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

char** getArguments(char* userInput){
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
	
	return argv;
}
int main(){
   char* userInput = malloc(256*sizeof(char));
   size_t userInputSize = 255; //to allow space for the null terminator
   size_t characters; //tell how many characters where read in
   char** argv;


   while(1){
      printf(": ");
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

	 argv = getArguments(userInput);
	 if(strcmp(argv[0], "cd") == 0){
	    if(argv[1] != NULL){
	       printf("%s\n", argv[1]);
	       chdir(argv[1]);
	       char cwd[256];
	       getcwd(cwd, sizeof(cwd));	   
	       printf("%s\n", cwd); 
	    }
	    else{
	       chdir(getenv("HOME"));
	       char cwd[256];
	       getcwd(cwd, sizeof(cwd));	   
	       printf("%s\n", cwd); 
	    }
	 }
      }
   }
}










