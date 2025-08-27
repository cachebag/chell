#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define CSH_RL_BUFSIZE 1024
#define CSH_TOK_BUFSIZE 64
#define CSH_TOK_DELIM " \t\r\n\a"

/* 
  Chell commands 
*/ 
int csh_cd(char **args);
int csh_help(char **args);
int csh_exit(char **args);

int main(int argc, char **argv) {
  // config files 
  
  csh_loop();

  // shutdown or cleanup

  return EXIT_SUCCESS;
}

void csh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = csh_read_line();
    args = csh_spit_line(line);
    status = csh_execute(args);

    free(line);
    free(args);
  } while (status);
}

char *csh_read_line(void) {
  int bufsize = cSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;
  
  if (!buffer) {
    fprintf(stderr, "csh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // read 
    c = getchar();

    // replace EOF with a null character and return
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // reallocate space if we've exceeded the buffer 
    if (position >= bufsize) {
      bufsize += CSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "csh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **csh_spit_line(char *line) {
  int bufsize = CSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "csh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, CSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += CSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "csh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, CSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int csh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // child 
    if (execvp(args[0], args) == -1) {
      perror("csh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // forking error 
    perror("csh");
  } else {
    // parent 
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}
