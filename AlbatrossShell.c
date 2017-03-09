#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "dirent.h"

//Moves the input string into different parts
//Argument Vector
//Argument Count (including command)
//Flags
void parseInput(char *input, char **argv, int *argc, int *pipeFlag, int *bgFlag, char *outputFlag, char *inputFlag, char **pipeArgs)
{
  //Get argv
  char *token;
  int i = 0; //argv index
  int j = 0;
  bool piping = false;
  token = strtok(input, " ");
  while (token != NULL)
  {
    //printf("%s \n", token);
    if (i != 0)
      token = strtok(NULL, " ");

    if (token != NULL)
    {
      if (strcmp(token, "&") == 0)
      {
        *bgFlag = 1;
      }
      else if (strcmp(token, "<") == 0)
      {
        token = strtok(NULL, " "); //take the next token
        strcpy(inputFlag, token);
      }
      else if (strcmp(token, ">") == 0)
      {
        token = strtok(NULL, " "); //take the next token
        strcpy(outputFlag, token);
      }
      else //else its an actual argument
      {
        if (strcmp(token, "|") == 0 || piping)
        {
          if (!piping)
          {
            piping = true;
            *pipeFlag = 1;
          }
          else
          {
            strcpy((pipeArgs[j]), token);
            //(*argc)++;
            j++;
          }
        }
        else if (piping == false)
        {
          strcpy((argv[i]), token);
          (*argc)++;
          i++;
        }
      }
    }
  }

  argv[i] = NULL;
  pipeArgs[j] = NULL;
}

void main(int argc, char *argv[], char **env)
{
  bool runningBatch;
  //Variable Setup
  char cwd[1024];                                      //Current Dir
  char *input = (char *)malloc(sizeof(char) * 256);    //Input
  char **argv2 = (char **)malloc(sizeof(char *) * 64); //Argument Vector
  int argc2 = 0;                                       //Argument Count
  int F_BG = 0;
  int F_PIPE = 0;                                         //Background Flag
  char *F_IN = (char *)malloc(sizeof(char) * 32);         //Input flag
  char *F_OUT = (char *)malloc(sizeof(char) * 32);        //output flag
  char **pipeArgs = (char **)malloc(sizeof(char *) * 64); //Pipe flag

  FILE *batchFP;
  char *batchLine;
  size_t linelength = 0;

  int ct = 0;
  for (ct = 0; ct < 64; ct++)
  {
    argv2[ct] = (char *)malloc(sizeof(char) * 128);
  }

  for (ct = 0; ct < 64; ct++)
  {
    pipeArgs[ct] = (char *)malloc(sizeof(char) * 128);
  }

  //If there is a batch file to be ran
  if (argc > 1)
  {
    printf("%s Running \n", argv[1]);
    runningBatch = true;
    batchFP = fopen(argv[1], "r");
  }
  else
  {
    runningBatch = false;
    //printf("argc is  %d \n", argc);
    getcwd(cwd, sizeof(cwd));
    printf("Albatross < %s > >", cwd);
  }

  while (true)
  {
    //Clear all variables
    memset(input, '\0', sizeof(input));
    argc2 = 0;
    //CLEAR THE ARGV2 MEMORY OR IT WONT WORK!!
    for (ct = 0; ct < 64; ct++)
    {
      argv2[ct] = (char *)malloc(sizeof(char) * 128);
    }

    for (ct = 0; ct < 64; ct++)
    {
      pipeArgs[ct] = (char *)malloc(sizeof(char) * 128);
    }

    F_BG = 0;
    F_PIPE = 0;
    memset(F_IN, '\0', sizeof(F_IN));
    memset(F_OUT, '\0', sizeof(F_OUT));

    if (runningBatch)
    {
      if (getline(&batchLine, &linelength, batchFP) != -1)
      {
        //printf("Line: \'%s\' \n", batchLine);
        char *str = strtok(batchLine, "\n");
        strcpy(input, str);
      }
      else
      {
        fclose(batchFP);
        exit(0);
      }
    }
    else
    {
      gets(input);
    }

    //printf("You entered: %s \n", input);
    parseInput(input, argv2, &argc2, &F_PIPE, &F_BG, F_OUT, F_IN, pipeArgs);

    //If the input is okay
    if (!(strcmp(input, "\0") == 0 || argc2 == 0))
    {
      //quit
      if (strcmp(argv2[0], "quit") == 0 && argc2 == 1)
      {
        printf("Quitting Albatross Shell! \n");
        exit(0);
      }
      if (strcmp(argv2[0], "pause") == 0 && argc2 == 1)
      {
        printf("Click Enter to Unpause...");
        char pauseString[512];
        gets(pauseString); //Just wait for input
      }
      //cd
      else if (strcmp(argv2[0], "cd") == 0 && argc2 == 1)
      {
        printf("%s \n", cwd);
      }
      //cd <directory>
      else if (strcmp(argv2[0], "cd") == 0 && argc2 == 2)
      {
        chdir(argv2[1]);
      }
      //clr
      else if (strcmp(argv2[0], "clr") == 0 && argc2 == 1)
      {
        printf("\033[2J");
      }
      //dir <directory>
      else if (strcmp(argv2[0], "dir") == 0 && argc2 == 2)
      {
        int def_out = dup(1);
        if (strcmp(F_OUT, "\0"))
        {
          int fd = open(F_OUT, O_CREAT | O_WRONLY, S_IRWXU);
          dup2(fd, 1);
          //close(fd);
        }

        DIR *dir = opendir(argv2[1]);
        struct dirent *ent;
        if (dir != NULL)
        {
          //print all the files and directories within directory
          ent = readdir(dir);
          while (ent != NULL)
          {
            printf("%s \n", ent->d_name);
            ent = readdir(dir);
          }
          closedir(dir);
        }
        else
        {
          printf("Cannot Open Directory Specified \n");
        }

        if (strcmp(F_OUT, "\0"))
        {
          dup2(def_out, 1);
          close(def_out);
        }
      }
      //environ
      else if (strcmp(argv2[0], "environ") == 0 && argc2 == 1)
      {

        int def_out = dup(1);
        if (strcmp(F_OUT, "\0"))
        {
          int fd = open(F_OUT, O_CREAT | O_WRONLY, S_IRWXU);
          dup2(fd, 1);
          //close(fd);
        }

        int i = 0;
        char *str = *(env);
        while (str != NULL)
        {
          printf("%s\n", str);
          str = *(env + i);
          i++;
        }

        if (strcmp(F_OUT, "\0"))
        {
          dup2(def_out, 1);
          close(def_out);
        }
      }
      //echo <comment>
      else if (strcmp(argv2[0], "echo") == 0 && argc2 > 1)
      {
        int def_out = dup(1);
        if (strcmp(F_OUT, "\0"))
        {
          int fd = open(F_OUT, O_CREAT | O_WRONLY, S_IRWXU);
          dup2(fd, 1);
          //close(fd);
        }

        int i = 1;
        for (i = 1; i < argc2; i++)
        {
          printf("%s ", argv2[i]);
        }
        printf("\n");

        if (strcmp(F_OUT, "\0"))
        {
          dup2(def_out, 1);
          close(def_out);
        }
        else
        {
          printf("\n");
        }
      }
      //help
      else if (strcmp(argv2[0], "help") == 0 && argc2 == 1)
      {
        int def_out = dup(1);
        if (strcmp(F_OUT, "\0"))
        {
          int fd = open(F_OUT, O_CREAT | O_WRONLY, S_IRWXU);
          dup2(fd, 1);
          //close(fd);
        }

        FILE *fp = fopen("readme", "r"); //Open the File to read
        char line[256];                  //line buffer
        while (fgets(line, sizeof(line), fp) != NULL)
        {
          printf("%s", line);
        }
        printf("\n");
        fclose(fp);

        if (strcmp(F_OUT, "\0"))
        {
          dup2(def_out, 1);
          close(def_out);
        }
      }
      //Some other command
      else
      {
        int def_in = dup(0);
        int def_out = dup(1);

        if (strcmp(F_OUT, "\0"))
        {
          int fd = open(F_OUT, O_CREAT | O_WRONLY, S_IRWXU);
          dup2(fd, 1);
          //close(fd);
        }

        if (strcmp(F_IN, "\0"))
        {
          int fd = open(F_IN, O_RDONLY);
          dup2(fd, 0);
          //close(fd);
        }

        pid_t pid = fork();
        if (pid == 0)
        {
          int pipefd[2];
          if (F_PIPE)
          {
            //printf("PIPING \n");
            pipe(pipefd);

            pid_t currPid = getpid();
            //The child that will accept the input from the pid1
            pid_t pid2 = fork();
            if (pid2 == 0)
            {
              close(pipefd[0]);
              //input is whatever the input flag is or stdin
              dup2(pipefd[1], 1); //output replaced by pipe

              //printf("Execing %s \n", argv2[0]);
              execvp(argv2[0], argv2);

              printf("%s: Command Not Found \n", argv2[0]);
              exit(0);
            }
            else
            {
              close(pipefd[1]);
              //output is whatever the output flag is or stdout default
              dup2(pipefd[0], 0); //input replaced by pipe

              //printf("Waiting...");
              waitpid(pid2);

              //printf("Execing %s \n", pipeArgs[0]);
              execvp(pipeArgs[0], pipeArgs);

              printf("%s: Command Not Found \n", pipeArgs[0]);
              exit(0);
            }
          }
          else
          {
            //printf("Execing %s \n", argv2[0]);
            execvp(argv2[0], argv2);

            printf("%s: Command Not Found \n", argv2[0]);
            exit(0);
          }
        }
        else
        {
          //if bg flag set, wait
          if (F_BG == 0)
          {
            wait();
            //int status = 0;
            //wait(&status);
          }

          //reset the stdout
          if (strcmp(F_OUT, "\0") /* || F_PIPE */)
          {
            dup2(def_out, 1);
            close(def_out);
          }

          //reset the stdin
          if (strcmp(F_IN, "\0"))
          {
            dup2(def_in, 0);
            close(def_in);
          }
        }
      }
    }

    if (!runningBatch)
    {
      getcwd(cwd, sizeof(cwd));
      printf("Albatross < %s > >", cwd);
    }
  }
}
