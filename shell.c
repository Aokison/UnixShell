//Project 2 - Unix Shell
//Nicholas Aoki  CWID: 886954221

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

bool at; //checks if input contains &

char* parse(char* s, char* tokens[]) {    //returns file name
  const char break_chars[2] = {' ', '\t'};
  char* tok;    //individual token
  char* file;   //file name

  tok = strtok(s, break_chars); 
  int i = 0;
  tokens[i] = tok;            //storing tokens
  while (tok != NULL) {
    tok = strtok(NULL, break_chars);
    if(tok == NULL) {
      break;
    }
    if(!strncmp(tok, ">", 1)){    //stores command only and returns file name
      tok = strtok(NULL, break_chars);
      file = tok;
      return file;
    }
    if(!strncmp(tok, "<", 1)){    //stores command only and returns file name
      tok = strtok(NULL, break_chars);
      file = tok;
      return file;
    }
    i++;
    tokens[i] = tok;
  }
  //tokens[i] = NULL;
  return file;
  
}

int main(int argc, const char * argv[]) {  
  char input[BUFSIZ];
  char last_command[BUFSIZ];
  char* tokens[BUFSIZ];
  char* file;
  pid_t pid;
  
  memset(input, 0, BUFSIZ * sizeof(char));
  memset(last_command, 0, BUFSIZ * sizeof(char));
  memset(tokens, 0, BUFSIZ * sizeof(char));

  bool finished = false;
  
  while (!finished) {
    printf("osh> ");
    fflush(stdout);
    at = false; 

    if ((fgets(input, BUFSIZ, stdin)) == NULL) {   // or gets(input, BUFSIZ);
        fprintf(stderr, "no command entered\n");
        exit(1);
    }

    input[strlen(input) - 1] = '\0';          // wipe out newline at end of string
    printf("input was: \n'%s'\n", input);

    if (strncmp(input, "exit", 4) == 0) {   // only compare first 4 letters
          finished = true;
    }

    char* waiter = strstr(input, "&");
    if (waiter != NULL){      //checks for &
        *waiter = ' ';
        at = true;
    }

    if (strncmp(input, "!!", 2) != 0) {   //checks for last command
          strcpy(last_command, input);
    }

    pid_t pid = fork();

    if (pid < 0) {
         fprintf(stderr, "Fork failed\n");
         return 1;
    } 
    else if (pid != 0) {
        //printf("the child's pid: %d, and parent's pid: %d\n\n", pid, getpid());
         if(!at){
            wait(NULL);
            wait(NULL);
         }
         printf("************************************* Child complete\n");
    }
    else {
    memset(tokens, 0, BUFSIZ * sizeof(char));

    if(strncmp(input, "!!", 2) == 0) {   // check for history (!!) command
        if (strlen(last_command) == 0) {
            fprintf(stderr, "no last command to execute\n");
        }
        else {
            printf("last command was: %s\n", last_command);
            parse(last_command, tokens);
            execvp(tokens[0], tokens);
            exit(0);
        }
    } else {
        if(strstr(input, ">") != NULL){     //output
            printf("You entered: %s\n", input);
            file = parse(input, tokens);
            printf("Your file: %s\n", file);
            int fd = open(file, O_TRUNC | O_CREAT | O_RDWR);
            dup2(fd, STDOUT_FILENO);
            execvp(tokens[0], tokens);
            exit(0);
        }
        else if(strstr(input, "<") != NULL){    //input
            printf("You entered: %s\n", input);
            file = parse(input, tokens);
             printf("Your file: %s\n", file);
            int fd = open(file, O_RDONLY);
            dup2(fd, STDIN_FILENO);
            execvp(tokens[0], tokens);
            exit(0);
        }
        else if(strstr(input, "|") != NULL){    //pipe
            pid_t pip;
            int pipefd[2];
            char* right[BUFSIZ];    //characters right of the bar
            char* left[BUFSIZ];     //characters left of the bar

            memset(right, 0, BUFSIZ*sizeof(char));
            memset(left, 0, BUFSIZ*sizeof(char));

            parse(input, tokens);

            int bar = 0;
            while(tokens[bar] != '\0'){     //finds bar position
              if(strstr(tokens[bar], "|") != NULL){
                break;
              }
              bar++;
            }

            tokens[bar] = "\0";   //gets rid of bar

            for(int i = 0; i < bar; i++){       //stores characters on left
              left[i] = tokens[i];
            }

            for(int i = 0; i < BUFSIZ; i++){    //stores characters on right
              int after = i + (bar + 1);
              if(tokens[after] == 0){
                break;
              }
              right[i] = tokens[after];
            }

            int p = pipe(pipefd);
            if(p == -1){
              fprintf(stderr, "Pipe failed\n");
              return 1;
            }

            pip = fork();
            if(pip < 0){
              fprintf(stderr, "Fork failed\n");
              return 1;
            }
            if(pip != 0){     //left side of command executed
              dup2(pipefd[1], STDOUT_FILENO);
              close(pipefd[1]);
              execvp(left[0], left);
              exit(0);
            }
            else {      //right side of command executed
              dup2(pipefd[0], STDIN_FILENO);
              close(pipefd[0]);
              execvp(right[0], right);
              exit(0);
            }
            wait(NULL);
            execvp(tokens[0],tokens);
            exit(0);
        }
        else{
            printf("You entered: %s\n", input);   // you will call fork/exec
            parse(input, tokens);
            execvp(tokens[0], tokens);
            exit(0);
        }
     }
    }
  }
  
  printf("osh exited\n");
  printf("program finished\n");
  
  return 0;
}