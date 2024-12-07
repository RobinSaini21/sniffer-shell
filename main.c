#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <bits/waitflags.h>
#include <unistd.h>   
#include <sys/wait.h>

#define EXIT_SUCCESS 0
#define EXIT_ERROR 1
#define MAX_BUFFER_SIZE 1024
#define SNIFFER_TOK_BUFSIZE 64
#define SNIFFER_TOK_DELIM " \t\r\n\a"

int sniffer_cd(char **args);
int sniffer_help(char **args);
int sniffer_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &sniffer_cd,
  &sniffer_help,
  &sniffer_exit
};

int sniffer_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int sniffer_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int sniffer_help(char **args)
{
  int i;
  printf("Stephen Brennan's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < sniffer_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int sniffer_exit(char **args)
{
  return 0;
}




int sniffer_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int sniffer_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < sniffer_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return sniffer_launch(args);
}


char *sniffer_read_line()
{
    int bufferSize = MAX_BUFFER_SIZE;
    char *buffer = malloc(sizeof(char) * bufferSize);
    char c;
    int postion = 0;

    if (!buffer)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();
        if (c == EOF || c == '\n')
        {
            buffer[postion] = c;
            return buffer;
        }
        else
        {
            buffer[postion] = c;
        }

        postion++;

        if (postion >= bufferSize)
        {
            bufferSize += MAX_BUFFER_SIZE;
            buffer = realloc(buffer, bufferSize);
            if (!buffer)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    return buffer;
}

char **sniffer_split_line(char *line){
   int bufsize = SNIFFER_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  token = strtok(line, SNIFFER_TOK_DELIM);

   while (token != NULL) {
    tokens[position] = token;
    position++;
      printf("Token: %s\n", token);
    if (position >= bufsize) {
      bufsize += SNIFFER_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SNIFFER_TOK_DELIM);
  }
  tokens[position] = NULL;
   return tokens;
}

void sniffer_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = sniffer_read_line();
    args = sniffer_split_line(line);
    status = sniffer_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main()
{
   
    sniffer_loop();
    
    return EXIT_SUCCESS;
}
