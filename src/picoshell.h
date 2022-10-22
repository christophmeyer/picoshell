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

#ifndef PSH_PICOSHELL_H_
#define PSH_PICOSHELL_H_

#include "parser.h"

/*
 * Resolves environment variables by replacing tokens starting with $ by the
 * value of the corresponding environment variable if it exists. Otherwise
 * replaces it with the empty string.
 */
void resolve_env_variables(struct Command *command);

/*
 * Resolves full path of executable. If executable contains / it calls realpath
 * to expand the path. If executable does not contain /, e.g. "ls", resolve_path
 * searches the paths in the PATH environment variable. If successful, the
 * executable is replaced with the full path , e.g. "/usr/bin/ls", otherwise the
 * NULL pointer is returned.
 */
char *resolve_path(char *executable);

/*
 * Generates the prompt in the format:
 * username@host~>
 * and returns a pointer to it.
 */
char *getprompt();

/*
 * Calls chdir and updates PWD and OLDPWD environment variables if chdir is
 * successful.
 */
void change_dir(struct Command *command);

/*
 * Executes one line of input.
 */
void execute_input(char *input);

#endif /* PSH_PICOSHELL_H_ */