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

#ifndef PSH_PARSER_H_
#define PSH_PARSER_H_

/*
 * Enum of the finite state machine parser states.
 */
typedef enum {
  IN_WORD_QUOTED,
  IN_WORD,
  WHITESPACE,
  PIPE,
} ParserState;

/*
 * Holds a single token in a char array.
 */
struct Token {
  char *buffer; /* Holds the token characters. */
  int max_len;  /* Number of chars allocated. */
};

/*
 * Holds a single command as an array of tokens.
 *
 * Tokens are separated by whitespace, e.g the command "ls -la" consists of the
 * tokens "ls" and "-la".
 */
struct Command {
  int max_tokens;        /* Number of tokens allocated. */
  int len;               /* Number of tokens stored. */
  struct Token **tokens; /* Holds pointers to the tokens of the command. */
};

/*
 * Holds the parsed input as an array of commands.
 *
 * After parsing the input with parse_input() which returns a pointer to
 * ParsedInput. This can be passed on to execute_input() to actually run the
 * input.
 */
struct ParsedInput {
  int max_commands; /* Number of commands allocated. */
  int len;          /* Number of commands stored. */
  struct Command **
      commands; /* Holds pointers to the commands that are connected by pipes */
};

/*
 * Creates new ParsedInput.
 */
struct ParsedInput *new_parsed_input();

/*
 * Frees memory allocated to hold ParsedInput.
 *
 * parsed_input: Pointer to the ParsedInput to be freed.
 */
void free_parsed_input(struct ParsedInput *parsed_input);

/*
 * Reallocates more memory to resize token->buffer.
 *
 * token: Pointer to the token to be resized.
 * new_max_len: New number of characters to be stored in token->buffer.
 */
void resize_token(struct Token *token, int new_max_len);

/*
 * Parses the input line buffer into a ParsedInput struct.
 * raw_input: Char array containing the input string
 *
 * Implements a finite state machine parser. Groups the input into *commands*
 * separated by pipes (i.e. |). Each command in turn is a sequence of whitespace
 * separated (one or multiple) tokens. Characters inside a pair of double quotes
 * are interpreted as a single token, even if they include whitespace.
 */
struct ParsedInput *parse_input(char *raw_input);

#endif /* PSH_PARSER_H_ */
