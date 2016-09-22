/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_INPUT 1024 // assume no input line will be longer than 1024 bytes

int main (int argc, char **argv, char **envp) {
  setbuf(stdout, NULL); // disable buffering on stdout so printf works

  int finished = 0;
  char cmd[MAX_INPUT];
  char *startprompt = "[";
  char *prompt = "] thsh> ";
  char *cwd = (char *)malloc(MAX_INPUT);

  while (!finished) {
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
    char *temp;
    temp = strtok(cmd, " \n"); // split input by space and newline
    while (temp != NULL) {
      execute[i] = temp;
      temp = strtok(NULL, " \n");
      i++;
    }
    execute[i] = NULL; // null terminate string

    // Execute the command, handling built-in commands separately 
    // write(1, cmd, strnlen(cmd, MAX_INPUT));

    if (execute[0] == NULL) { // user entered a newline
      continue;
    } else if (strcmp(execute[0], "exit\n") == 0 || strcmp(execute[0], "exit") == 0) {
      exit(0);
    } else if ((strcmp(execute[0], "cd") == 0) && (execute[1] == NULL)) { // cd with no args
      setenv("OLDPWD", getenv("PWD"), 1);
      chdir(getenv("HOME"));
      setenv("PWD", getenv("HOME"), 1);
      continue;
    } else if (strcmp(execute[0], "cd") == 0) {
      setenv("OLDPWD", getenv("PWD"), 1);
      chdir(execute[1]);
      char tempcwd[MAX_INPUT];
      setenv("PWD", getcwd(tempcwd, sizeof(tempcwd)), 1);
      continue;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
      int temp = execvp(execute[0], execute);
      if (temp == -1) {
        printf("Error - command not found\n");
      }
    } else if (pid < 0) {
      printf("Error - fork failed\n");
    } else {
      wait(&rv);
    }
  }

  return 0;
}

