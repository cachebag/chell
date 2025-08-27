#include <sys/wait.h>
#include <unistd.h>
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

/* 
  list of built-in commands and their functions 
*/ 
char *builtin_str[] = {
  "cd", 
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &csh_cd,
  &csh_help,
  &csh_exit,
};

int csh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/* 
  Command implementations
*/ 
int csh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "csh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("csh");
    }
  }
  return 1;
}

int csh_help(char **args) {
  int i;
  printf("chell by cachebag");
  printf("List of supported commands:\n");

  for (i = 0; i < csh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}

int csh_exit(char **args) {
  return 0;
}

char *csh_read_line(void) {
  int bufsize = CSH_RL_BUFSIZE;
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

char **csh_split_line(char *line) {
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

int csh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < csh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return csh_launch(args);
}

void csh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = csh_read_line();
    args = csh_split_line(line);
    status = csh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  // config files 
  
  csh_loop();

  // shutdown or cleanup

  return EXIT_SUCCESS;
}
