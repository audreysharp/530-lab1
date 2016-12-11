/* thsh.c
Tar Heel SHell 
COMP 530 Lab 1

Audrey Sharp (aud), PID 720473458
Joshua Smith (joshuals), PID 720457161

UNC Honor Pledge: I certify that no unauthorized assistance has been received or
given in the completion of this work
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_INPUT 1024 //assume no input line will be longer than 1024 bytes

int main (int argc, char **argv, char **envp) { //same as "int main (int argc, char *argv[], char *envp[])"
  setbuf(stdout, NULL); // disable buffering on stdout so printf works
  
  //char *testing = "hello";
  //char testing[2][15];
  //strcpy(testing[0], "hello");
  //printf("%c\n", testing[0][1]);
  //printf("%d\n", strlen(testing));
  //printf("%s\n", testing+1);
  
  int debug = 0;
  int finished = 0;
  char cmd[MAX_INPUT];
  char *startprompt = "[";
  char *prompt = "] thsh> ";
  char *cwd = (char *)malloc(MAX_INPUT); //or just "malloc(MAX_INPUT);"?
  char tempStr[MAX_INPUT]; //for storing environment variable user input
  char *vPath;
  char tempVar[MAX_INPUT]; //for storing environment variable user input
  char rv_char[2]; //store return value of child shell
  
  //add support for $? environment variable
  setenv("?", "0", 1);

  if (argv[1] != NULL) {
    if (strcmp(argv[1], "-d") == 0) {
      debug = 1;
    }
  }
  
  while (!finished) {
    memset(tempVar, 0, sizeof(tempVar));  //reset the char array to all null chars
    memset(rv_char, 0, sizeof(rv_char));  //reset the char array to all null chars
    char *cursor;
    char last_char;
    int rv;
    int count;

    cwd = getcwd(cwd, MAX_INPUT);
    char *cwdprompt = malloc(strlen(startprompt) + strlen(cwd) + strlen(prompt));
    strcpy(cwdprompt, startprompt);
    strcat(cwdprompt, cwd);
    strcat(cwdprompt, prompt);

    // Print the prompt
    rv = write(1, cwdprompt, strlen(cwdprompt));
    if (!rv) { 
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1; 
      rv && (++count < (MAX_INPUT-1)) && (last_char != '\n'); cursor++) { 
      rv = read(0, cursor, 1);
        last_char = *cursor;
    } 
    *cursor = '\0';

    if (!rv) { 
      finished = 1;
      break;
    }

    char *execute[MAX_INPUT];
    int i = 0;
    char *input[MAX_INPUT];
    char *output[MAX_INPUT];
    char *pipeCommands[MAX_INPUT];
    int numPipes; // counter for pipe

    // reset input/output/pipe vars
    input[0] = NULL;
    output[0] = NULL;
    int k = 0;
    for (k=0; k<20; k++) {
      pipeCommands[k] = NULL;
    }

    numPipes = 0;
    int in = 0; // boolean values for exec() later on
    int out = 0;
    int pipeFlag = 0;
    int changeOutput = 0;

    char *temp;
    temp = strtok(cmd, " =\n\t"); // split input by space, =, newline, and tab
      while (temp != NULL) {
       // printf("temp:%s\n", temp);
        if (strstr(temp, "<") != NULL || strstr(temp, ">") != NULL || strstr(temp, "|") != NULL) {
          if (strstr(temp, "<") != NULL) {
            in = 1;
            if (temp[1] != '\0') { // no space between < and input file
              input[0] = temp+1; // temp+1 to get rid of < at beginning of string
            } else {
              temp = strtok(NULL, " =\n\t");
              continue;
            }
          } else if (strstr(temp, ">") != NULL) {
            out = 1;
            if (temp[0] != '>') { // number in front of >
              if (temp[0] == '2') {
                changeOutput = 1; // redirect to stderr
              }
              if (temp[2] != '\0') {
                output[0] = temp+2;  
              }
            } else if (temp[1] != '\0') {
              output[0] = temp+1;
            } else {
              temp = strtok(NULL, " =\n\t");
              continue;
            }
          } else if (strstr(temp, "|") != NULL) {
            pipeFlag = 1;
            numPipes++;
            execute[i] = "\0"; // delimit commands with + for parsing later
            i++;
            if (temp[1] != '\0') {
              execute[i] = temp+1;
              i++;
            } else {
              temp = strtok(NULL, " =\n\t");
              continue;
            }
          }
        }
        if (in && input[0] == NULL) { // space between < and input file; put file in input[0]
          input[0] = temp;
        } else if (out && output[0] == NULL) {
          output[0] = temp;
        } else if (in || out) {
        } else {
          execute[i] = temp;
          i++;
        }
        temp = strtok(NULL, " =\n\t"); // NULL used to get token from previous string
      }
    execute[i] = NULL; // null terminate string

    // Execute the command, handling built-in commands separately 
    // write(1, cmd, strnlen(cmd, MAX_INPUT));

    if (debug) {
      fprintf(stderr, "RUNNING: %s\n", execute[0]);
    }

    if (execute[0] == NULL) { // user entered a newline
      continue;
    } else if (strcmp(execute[0], "exit\n") == 0 || strcmp(execute[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(execute[0], "goheels\n") == 0 || strcmp(execute[0], "goheels") == 0) {
        goheels();
        continue;
    } else if (strcmp(execute[0], "cd") == 0) { 
        if ((execute[1] == NULL) || (strcmp(execute[1], "~") == 0)) { // cd with no args
            setenv("OLDPWD", getenv("PWD"), 1);
            chdir(getenv("HOME"));
            setenv("PWD", getenv("HOME"), 1);
        } else if (strcmp(execute[1], "-") == 0) { //change to last directory user was in
            strcpy(tempVar, getenv("PWD"));
            chdir(getenv("OLDPWD"));
            setenv("PWD", getenv("OLDPWD"), 1);
            setenv("OLDPWD", tempVar, 1);
        } else {
            setenv("OLDPWD", getenv("PWD"), 1);
            chdir(execute[1]);
            char tempcwd[MAX_INPUT];
            setenv("PWD", getcwd(tempcwd, sizeof(tempcwd)), 1);
        }   
        continue;   
    } else if (strcmp(execute[0], "echo") == 0) {
        // print empty line of input after "echo"
        if (execute[1]==NULL) {
            printf("\n");
        } else if (execute[1]!=NULL) {    //print environment variable value
            //if first character of input variable = $
            if (execute[1][0] == '$') {
                strncpy(tempVar, execute[1]+1, strlen(execute[1])-1);
                vPath = getenv(tempVar); 
                
                //prints out the correct value if the variable exists
                if (vPath != NULL) {
                    printf("%s\n", vPath);
                } else {        //print blank string if the variable doesn't exist
                    printf("\n");
                }
                
            } else {
                fprintf(stderr, "ECHO error\n");
            }       
        }
        continue;
    } else if (strcmp(execute[0], "set") == 0) {
        //print all environment variables if input is just "set"
        if (execute[1]==NULL) {
            char **env;
            for (env = envp; *env!=0; env++) {
                printf("%s\n", *env);
            }
        } else if (strcmp(execute[1], "*") == 0 || strcmp(execute[1], "@") == 0
                   || strcmp(execute[1], "#") == 0 || strcmp(execute[1], "-") == 0
                   || strcmp(execute[1], "$") == 0 || strcmp(execute[1], "!") == 0
                   || strcmp(execute[1], "0") == 0 || strcmp(execute[1], "_") == 0) {
            fprintf(stderr, "Error: invalid variable (*,@,#,-,$,!,0, and _ are not supported here)\n");
        } else if ((execute[1]!=NULL) && (execute[2]!=NULL) && (execute[3]==NULL)) {
            vPath = getenv(execute[1]); 
            // assigns new value to existing variable or makes a new variable with a value
            setenv(execute[1], execute[2], 1);
        }
        continue;
    }/* else if (strcmp(execute[0], "jobs") == 0) {
      //execute[0] = "ps";
      //execute[1] = '\0';
    } else if (strcmp(execute[0], "fg") == 0) { // make job x go to foreground

    } else if (strcmp(execute[0], "bg") == 0) { // make job x go to background

    }*/
    
    if (pipeFlag) {
      // I don't know enough about computer science to get piping to work. I give up.
      printf("Piping...\n");
      //runPipes(execute, numPipes);
      //runPipes(execute, pipeCommands, numPipes);
      //int pipes[numPipes*2];

      /*int b;
      for (b=0; b<numPipes; b++) { // set up pipes and parse commands
        int tempB = b*2;
        pipe(pipes[tempB]);
      }*/

      /*int a;
      int c; // for loop to close pipes
      for (a=0; a<numPipes+1; a++) { // # of commands to execute = pipes+1
        pipe(pipes[a*2]);
        pid_t pid = fork();
        if (pid == 0) {
          if (in) {
            int inputTemp = open(input[0], O_RDONLY);
            dup2(inputTemp, STDIN_FILENO);
            close(inputTemp);
          }
          if (out) {
            int outputTemp = creat(output[0], 777);
            if (changeOutput) { // output STDERR
              dup2(outputTemp, STDERR_FILENO);
            } else {
              dup2(outputTemp, STDOUT_FILENO);
            }
            close(outputTemp);
          }
          if (a == 0) {
            dup2(pipes[a+1], STDOUT_FILENO);
            for (c=0; c<numPipes*2; c++) {
              close(pipes[c]);
            }
          } else if (numPipes > 1 && a > 0) {
            dup2(pipes[a], STDIN_FILENO);
            dup2(pipes[a+2], STDOUT_FILENO);
            for (c=0; c<numPipes*2; c++) {
              close(pipes[c]);
            }
          } else if (a == numPipes) {
            dup2(pipes[numPipes], STDIN_FILENO);
            for (c=0; c<numPipes*2; c++) {
              close(pipes[c]);
            }
          }

          // parse commands
          int d;
          if (a>0) { // get second/third/etc set of commands
            for (d=0; d<i; d++) {
              if (execute[d] == '\0') {
                execute[0] = execute[d+1];
                break;
              }
            }
          }
          for (d=0; d<i; d++) { // cut off commands to go to exec()
            if (execute[d] == '+') {
              execute[d] == '\0';
              break;
            }
          }

          printf("%s %s \n", execute[0], execute[1]);
          int temp = execvp(execute[0], execute);
          if (temp == -1) {
            fprintf(stderr, "Error - command not found\n");
            break;
          }
        } else if (pid < 0) {
          fprintf(stderr, "Error - fork failed\n");
          break;
        } else {
          wait(&rv);
        }
      }*/
    }
      pid_t pid = fork();
      if (pid == 0) {
        if (in) {
          int inputTemp = open(input[0], O_RDONLY);
          dup2(inputTemp, STDIN_FILENO);
          close(inputTemp);
        }
        if (out) {
          int outputTemp = creat(output[0], 777);
          if (changeOutput) { // output STDERR
            dup2(outputTemp, STDERR_FILENO);
          } else {
            dup2(outputTemp, STDOUT_FILENO);
          }
          close(outputTemp);
        }

        //printf("%s %s \n", execute[0], execute[1]);
        int temp = execvp(execute[0], execute);
        if (temp == -1) {
          fprintf(stderr, "Error - command not found\n");
          break;
        }
      } else if (pid < 0) {
        fprintf(stderr, "Error - fork failed\n");
        break;
      } else {
        wait(&rv);
      }
    
      
      //change the value of the int return variable to a char array in order to use setenv
      sprintf(rv_char, "%d", WEXITSTATUS(rv));
      setenv("?", rv_char, 1);

      if (debug) {
        fprintf(stderr, "ENDED: %s (ret=%d)\n", execute[0], WEXITSTATUS(rv));
      }
  }

  return 0;
}