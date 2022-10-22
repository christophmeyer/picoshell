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

#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char *left_trim(char *ptr) {
  while (isspace(*ptr) && *ptr != '\0') {
    ptr++;
  }
  return ptr;
}

char *right_trim(char *ptr) {
  int pos = strlen(ptr);
  while (isspace(ptr[--pos]))
    ;
  ptr[pos + 1] = '\0';
  return ptr;
}

char *trim(char *ptr) { return right_trim(left_trim(ptr)); }

struct Token *new_token() {
  struct Token *token = handled_malloc(sizeof(struct Token));

  token->buffer = handled_malloc(100 * sizeof(char));
  token->max_len = 100;

  return token;
}

void resize_token(struct Token *token, int new_max_len) {
  token->buffer = handled_realloc(token->buffer, sizeof(char) * new_max_len);
  token->max_len = new_max_len;
}

void free_token(struct Token *token) {
  free(token->buffer);
  free(token);
}

struct Command *new_command() {
  struct Command *command = handled_malloc(sizeof(struct Command));

  command->tokens = handled_malloc(10 * sizeof(struct Token));
  command->len = 0;
  command->max_tokens = 10;

  for (int i = 0; i < 10; i++) {
    command->tokens[i] = new_token();
  }

  return command;
}

void resize_command(struct Command *command, int new_max_tokens) {
  command->tokens =
      handled_realloc(command->tokens, sizeof(struct Token *) * new_max_tokens);

  for (int i = command->max_tokens; i < new_max_tokens; i++) {
    command->tokens[i] = new_token();
  }

  command->max_tokens = new_max_tokens;
}

void free_command(struct Command *command) {
  if (command != NULL) {
    for (int i = 0; i < command->max_tokens; i++) {
      /* Free all allocated tokens, only the NULL termination is freed where it
       * is set.
       */
      if (command->tokens[i] != NULL) {
        free_token(command->tokens[i]);
      }
    }
    free(command->tokens);
    free(command);
  }
}

struct ParsedInput *new_parsed_input() {
  struct ParsedInput *parsed_input = handled_malloc(sizeof(struct ParsedInput));

  parsed_input->commands = handled_malloc(sizeof(struct Command *) * 10);

  for (int i = 0; i < 10; i++) {
    parsed_input->commands[i] = new_command();
  }
  parsed_input->max_commands = 10;
  parsed_input->len = 0;

  return parsed_input;
}

void resize_parsed_input(struct ParsedInput *parsed_input,
                         int new_max_commands) {
  parsed_input->commands = handled_realloc(
      parsed_input->commands, sizeof(struct Command *) * new_max_commands);

  for (int i = parsed_input->max_commands; i < new_max_commands; i++) {
    parsed_input->commands[i] = new_command();
  }
  parsed_input->max_commands = new_max_commands;
}

void free_parsed_input(struct ParsedInput *parsed_input) {
  if (parsed_input != NULL) {
    for (int i = 0; i < parsed_input->max_commands; i++) {
      free_command(parsed_input->commands[i]);
    }
    free(parsed_input->commands);
    free(parsed_input);
  }
}

struct ParsedInput *parse_input(char *raw_input) {
  struct ParsedInput *parsed_input = new_parsed_input();

  /* Ignore all leading and training whitespaces */
  char *input = trim(raw_input);

  /* No parsing necessary for empty input string */
  if (strlen(input) == 0) {
    return parsed_input;
  }

  char current_char;

  /* Start IN_WORD because we have trimmed, and string can't be empty */
  ParserState next_state = IN_WORD;

  int n_command = 0;
  int n_token = 0;
  int n_token_char = 0;

  /* loop through all chars of the input */
  for (int i = 0; i <= strlen(input); i++) {
    /* reallocate larger (double size) buffers if necessary */
    if (n_command == parsed_input->max_commands - 1) {
      resize_parsed_input(parsed_input, 2 * parsed_input->max_commands);
    }

    if (n_token == parsed_input->commands[n_command]->max_tokens - 1) {
      resize_command(parsed_input->commands[n_command],
                     2 * parsed_input->commands[n_command]->max_tokens);
    }

    if (n_token > -1 &&
        n_token_char ==
            parsed_input->commands[n_command]->tokens[n_token]->max_len - 1) {
      resize_token(
          parsed_input->commands[n_command]->tokens[n_token],
          2 * parsed_input->commands[n_command]->tokens[n_token]->max_len);
    }

    current_char = input[i];
    switch (next_state) {
      case IN_WORD:
        if (current_char == ' ') {
          /* Terminate current word and switch to WHITESPACE state. */
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = '\0';
          next_state = WHITESPACE;

        } else if (current_char == '|') {
          /* Terminate current word */
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = '\0';
          /* Terminate tokens with NULL pointer, to signify end of command. Need
           * to free token before overwriting pointer to it.
           */
          free_token(parsed_input->commands[n_command]->tokens[n_token + 1]);
          parsed_input->commands[n_command]->tokens[n_token + 1] = NULL;
          /* Terminate current command */
          parsed_input->commands[n_command]->len = n_token + 1;
          n_token = -1;
          n_command++;
          /* Switch to PIPE state */
          next_state = PIPE;
        } else if (current_char == '"') {
          next_state = IN_WORD_QUOTED;
        } else if (current_char == '\0') {
          /* Terminate current word */
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = '\0';
          /* Terminate tokens with NULL pointer, to signify end of command. Need
           * to free token before overwriting pointer to it.
           */
          free_token(parsed_input->commands[n_command]->tokens[n_token + 1]);
          parsed_input->commands[n_command]->tokens[n_token + 1] = NULL;
          /* Terminate current command */
          parsed_input->commands[n_command]->len = n_token + 1;

        } else {
          /* For all other chars, append to current word and stay in WORD state
           */
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = current_char;
          n_token_char++;
        }
        break;

      case IN_WORD_QUOTED:
        if (current_char == '"') {
          next_state = IN_WORD;
        } else if (current_char == '\0') {
          free_parsed_input(parsed_input);
          return NULL;
        } else {
          /* For all other chars append to current word and stay in
           * IN_WORD_QUOTED state
           */
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = current_char;
          n_token_char++;
        }
        break;

      case WHITESPACE:
        if (current_char == ' ') {
          continue;
        } else if (current_char == '|') {
          /* Terminate tokens with NULL pointer, to signify end of command. Need
           * to free token before overwriting pointer to it.
           */
          free_token(parsed_input->commands[n_command]->tokens[n_token + 1]);
          parsed_input->commands[n_command]->tokens[n_token + 1] = NULL;
          /* Terminate current command */
          parsed_input->commands[n_command]->len = n_token + 1;
          n_token = -1;
          n_command++;
          /* Switch to PIPE state */
          next_state = PIPE;
        } else if (current_char == '"') {
          /* Start new word with current_char. */
          n_token++;
          n_token_char = 0;
          next_state = IN_WORD_QUOTED;
        } else {
          /* Start new word with current_char. */
          n_token++;
          n_token_char = 0;
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = current_char;
          n_token_char++;
          next_state = IN_WORD;
        }
        break;

      case PIPE:
        if (current_char == ' ') {
          next_state = WHITESPACE;
        } else if (current_char == '|') {
          printf("psh: parse error near ||\n\n");
          free_parsed_input(parsed_input);
          return NULL;
        } else if (current_char == '"') {
          /* Start new command with new word with current_char. */
          n_token++;
          n_token_char = 0;
          next_state = IN_WORD_QUOTED;
        } else {
          /* Start new command with new word with current_char. */
          n_token++;
          n_token_char = 0;
          parsed_input->commands[n_command]
              ->tokens[n_token]
              ->buffer[n_token_char] = current_char;
          n_token_char++;
          next_state = IN_WORD;
        }
        break;

      default:
        break;
    }
  }
  parsed_input->len = n_command + 1;

  return parsed_input;
}
