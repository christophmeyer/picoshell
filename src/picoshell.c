/*
 * picoshell - a rudimentary interactive shell
 *
 * Copyright (C) 2022 Christoph Meyer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <readline/history.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parser.h"
#include "utils.h"

void resolve_env_variables(struct Command *command) {
  for (int i = 0; i < command->len; i++) {
    /* if token starts with $, check if env variable exists and replace token
     * with resolved value. If variable does not exist, resolve to empty string.
     */
    if (command->tokens[i]->buffer[0] == '$') {
      char *env_var = getenv(&(command->tokens[i]->buffer[1]));
      if (env_var != NULL) {
        if (command->tokens[i]->max_len < strlen(env_var)) {
          resize_token(command->tokens[i], strlen(env_var) + 1);
        }
        strcpy(command->tokens[i]->buffer, env_var);
      } else {
        command->tokens[i]->buffer[0] = '\0';
      }
    };
  };
};

char *resolve_path(char *executable) {
  /* If command is a built-in, return it as is */
  char *builtins[] = {"cd", "exit", "pwd"};
  for (int i = 0; i < 3; i++) {
    if (strcmp(executable, builtins[i]) == 0) {
      char *command_copy = strdup(executable);
      return command_copy;
    }
  }

  char *resolved;
  if (strstr(executable, "/") != NULL) {
    /* if command contains / expand to absolute path */
    resolved = realpath(executable, NULL);
  } else {
    /* otherwise iterate through colon separated paths in PATH env variable and
     * check if file can be found. If yes return full path, otherwise return
     * NULL.
     */
    char *path_original = getenv("PATH");
    char *path = strdup(path_original);
    char *ptr = strtok(path, ":");
    while (ptr != NULL) {
      char *full_path =
          handled_malloc(sizeof(char) * (strlen(ptr) + strlen(executable) + 2));
      strcpy(full_path, ptr);
      if (full_path[strlen(full_path) - 1] != '/') {
        strcat(full_path, "/");
      }
      strcat(full_path, executable);
      if (access(full_path, F_OK) == 0) {
        free(path);
        return full_path;
      }
      ptr = strtok(NULL, ":");
      free(full_path);
    }
    free(path);
    resolved = NULL;
  }
  return resolved;
}

char *getprompt() {
  char hostname[30];
  gethostname(hostname, sizeof hostname);
  char *user = getlogin();

  /* assemble prompt, nothing fancy :) */
  char *prompt = handled_malloc(sizeof(char) * (strlen(user) + 35));
  strcpy(prompt, user);
  strcat(prompt, "@");
  strcat(prompt, hostname);
  strcat(prompt, "~> ");

  return prompt;
}

void change_dir(char *dir) {
  char *newpwd = realpath(dir, NULL);
  char *oldpwd = getenv("PWD");
  int res = chdir(dir);
  if (res == 0) {
    /* If chdir was successful, update PWD and OLDPWD and chdir */
    setenv("OLDPWD", oldpwd, 1);
    setenv("PWD", newpwd, 1);
  } else {
    switch (errno) {
      case ENOTDIR:
        printf("cd: not a directory: %s\n", dir);
        break;
      case ENOENT:
        printf("cd: no such file or directory: %s\n", dir);
        break;
      case EACCES:
        printf("cd: permission denied: %s\n", dir);
        break;
      default:
        printf("cd: error %i occurred: %s\n", errno, dir);
    }
  }
  free(newpwd);
}

void execute_input(char *input) {
  if (input[0] == '\0') {
    free(input);
    return;
  }

  /* Add input to readline history. */
  add_history(input);

  struct ParsedInput *parsed_input = parse_input(input);

  /* Do nothing if parsing fails */
  if (parsed_input == NULL) {
    free(input);
    return;
  }

  /* setup pipes */
  int n_pipes = parsed_input->len - 1;
  int *pipefds = malloc(sizeof(int) * 2 * n_pipes);
  for (int n_pipe = 0; n_pipe < n_pipes; n_pipe++) {
    pipe(pipefds + n_pipe * 2);
  }

  /* loop over the commands in the input (which are all piped together) */
  for (int n_command = 0; n_command < parsed_input->len; n_command++) {
    resolve_env_variables(parsed_input->commands[n_command]);

    /* For built-in commands, psh does not fork. Piping has no effect for
     * built-in commands, they are just executed in order. That means, if
     * built-ins are combined with regular commands they break the pipe.
     */
    if (strcmp(parsed_input->commands[n_command]->tokens[0]->buffer, "exit") ==
        0) {
      /* exit */
      exit(EXIT_SUCCESS);
    } else if (strcmp(parsed_input->commands[n_command]->tokens[0]->buffer,
                      "cd") == 0) {
      /* cd */
      if (parsed_input->commands[n_command]->len == 2) {
        change_dir(parsed_input->commands[n_command]->tokens[1]->buffer);
      }
      continue;
    } else if (strcmp(parsed_input->commands[n_command]->tokens[0]->buffer,
                      "pwd") == 0) {
      /* pwd */
      char *pwd = getenv("PWD");
      printf("%s\n", pwd);
      continue;
    } else {
      /* From here on command is a regular one. First resolve its path. */
      char *resolved =
          resolve_path(parsed_input->commands[n_command]->tokens[0]->buffer);
      if (resolved == NULL) {
        printf("psh: no such file or directory %s\n",
               parsed_input->commands[n_command]->tokens[0]->buffer);
        free(resolved);
        break;
      }

      pid_t pid = fork();

      if (pid == 0) {
        /* child process */

        /* change file descriptors to connect stdin and stdout to the pipe */
        if (n_command != 0) {
          dup2(pipefds[2 * n_command - 2], 0);
        }

        if (n_command != parsed_input->len - 1) {
          dup2(pipefds[2 * n_command + 1], 1);
        }
        extern char **environ;

        for (int i = 0; i < 2 * n_pipes; i++) {
          close(pipefds[i]);
        }

        /* collect tokens pointers in array as required for calling execve */
        char **tokens = handled_malloc(
            sizeof(char *) * (parsed_input->commands[n_command]->len + 1));
        for (int i = 0; i < parsed_input->commands[n_command]->len + 1; i++) {
          if (parsed_input->commands[n_command]->tokens[i] != NULL) {
            tokens[i] = parsed_input->commands[n_command]->tokens[i]->buffer;
          } else {
            tokens[i] = NULL;
            break;
          }
        }

        /* execute non-builtin command */
        execve(resolved, tokens, environ);
        free(tokens);
        free(resolved);

        /* execve does not return when successful, so this will only be
         * reached if it errors.
         */
        _exit(127);
      } else if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
      } else {
        /* parent process */

        /* for all but the last command close the write end of the pipe in the
         * parent.
         */
        if (n_command < parsed_input->len - 1) {
          close(pipefds[2 * n_command + 1]);
        }
        if (n_command != 0) {
          close(pipefds[2 * n_command - 2]);
        }

        int status;
        if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1) {
          perror("waitpid");
          exit(EXIT_FAILURE);
        };
      };
      free(resolved);
    };
  }

  free(pipefds);
  free(input);
  free_parsed_input(parsed_input);
}
