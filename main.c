/*
 * Copyright (c) 2019 Karim Kanso. All Rights Reserved.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(HAVE_CONFIG_H)
 #include "config.h"
#else
 #define PACKAGE_STRING "rether ?"
#endif

#include <stdio.h>
#include "base64.h"

#define AUTHOR "Karim Kanso"
#define YEAR "2019"

const char b64[] = "aGVsbG8gd29ybGQK";

int main(int argc, char** argv) {
  printf(PACKAGE_STRING " - Copyright (C) " YEAR " " AUTHOR "\n");

  char buff[1024];
  size_t buff_len = sizeof(buff);
  if (!base64_decode(b64, sizeof(b64) - 1, buff, &buff_len)) {
    fprintf(stderr, "Unable to decode b64\n");
  } else {
    printf("result: %s", buff);
  }
  return 0;
}
