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

#include <readline/readline.h>

#include "picoshell.h"

int main() {
  /* Configure readline to auto-complete paths when the tab key is hit. */
  rl_bind_key('\t', rl_complete);

  char *prompt = getprompt();

  while (1) {
    /* Main execution loop reading input lines and executing them. */
    char *input = readline(prompt);
    execute_input(input);
  }

  return 0;
}
