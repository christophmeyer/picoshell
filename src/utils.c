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

#include <stdio.h>
#include <stdlib.h>

void *handled_malloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    /* malloc failed */
    perror("malloc");
    exit(EXIT_FAILURE);
  } else {
    return ptr;
  }
}

void *handled_realloc(void *ptr, size_t size) {
  void *return_ptr = realloc(ptr, size);
  if (return_ptr == NULL) {
    /* realloc failed */
    perror("realloc");
    exit(EXIT_FAILURE);
  } else {
    return return_ptr;
  }
}
