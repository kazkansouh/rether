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

#include "args.h"
#include "send.h"
#include "receive.h"

#define AUTHOR "Karim Kanso"
#define YEAR "2019"

int main(int argc, const char** argv) {
  printf(PACKAGE_STRING " - Copyright (C) " YEAR " " AUTHOR "\n");

  /* Parse command line arguments */
  if (!args_parse(argc, argv)) {
    return 1;
  }
  printf("Running with config:\n"
         "\tSource Interface: %s\n"
         "\tEtherType: %04X\n",
         gpch_source_interface,
         gui_ethertype);

  if (gb_receive) {
    printf("\tMode: Receive\n");
    receive();
  } else {
    printf("\tMode: Send\n"
           "\tDestination LLA: %02X:%02X:%02X:%02X:%02X:%02X\n"
           "\tPayload length: %zu\n",
           gui_destmac[0], gui_destmac[1], gui_destmac[2],
           gui_destmac[3], gui_destmac[4], gui_destmac[5],
           gz_data);
    send_one_shot();
  }

  return 0;
}
