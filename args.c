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

#include "config.h"
#include <popt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <net/if.h>

#include "base64.h"

#define xstr(s) str(s)
#define str(s) #s

#define DEFAULT_ETHERTYPE 0xDEAD
#define DEFAULT_DEST_0    00
#define DEFAULT_DEST_1    01
#define DEFAULT_DEST_2    02
#define DEFAULT_DEST_3    03
#define DEFAULT_DEST_4    04
#define DEFAULT_DEST_5    05
#define DEFAULT_INTERFACE "eth0"
#define DEFAULT_PAYLOAD   "hello world"

#define STATE_RECEIVE 0x01
#define STATE_SEND    0x02

/* Tempory store for popt arguments */
static char *gpch_temp = NULL;
static int gi_ethertype = 0;

/* Globals derived from command line arguments */
uint16_t gui_ethertype = DEFAULT_ETHERTYPE;
uint8_t gui_destmac[] = {
  DEFAULT_DEST_0 ,
  DEFAULT_DEST_1 ,
  DEFAULT_DEST_2 ,
  DEFAULT_DEST_3 ,
  DEFAULT_DEST_4 ,
  DEFAULT_DEST_5 ,
};
char gpch_source_interface[IFNAMSIZ] = {0};
int gb_base64 = false;
int gb_receive = false;
size_t gz_data = 0;
uint8_t *gui_data = NULL;

/*
 * Catch any possibility of unfreed variables.
 */
__attribute__ ((destructor))
static void destroy(void) {
  if (gui_data != NULL) {
    free(gui_data);
    gui_data = NULL;
  }
}

enum ARG_TYPE {
  ARG_ETHERTYPE = (int)1,
  ARG_RECEIVE_ONLY,
  ARG_DEST_MAC,
  ARG_SOURCE_INT,
  ARG_PAYLOAD,
  ARG_PAYLOAD_FILE,
  ARG_BASE64,
};

static const struct poptOption options[] = {
  { "source-int", 's', POPT_ARG_STRING, &gpch_temp, ARG_SOURCE_INT,
    "Interface to send frame from. Defines source mac address."
    " Default: " DEFAULT_INTERFACE,
    "if" },
  { "ethertype", 'e', POPT_ARG_INT, &gi_ethertype, ARG_ETHERTYPE,
    "Value for ethertype field of ethernet header. E.g. 0x0800 for IPv4."
    " See IEEE 802.3."
    " Default: " xstr(DEFAULT_ETHERTYPE),
    "0xZZZZ" },
  { "receive-only", 'r', POPT_ARG_NONE, &gb_receive, ARG_RECEIVE_ONLY,
    "Listen for frames matching the specified ethertype and print them."
    " Incompatible with below arguments.",
    NULL },
  { "dest-mac", 'd', POPT_ARG_STRING, &gpch_temp, ARG_DEST_MAC,
    "MAC address to send the frame to."
    " Default: " xstr(DEFAULT_DEST_0) ":" xstr(DEFAULT_DEST_1) ":"
    xstr(DEFAULT_DEST_2) ":" xstr(DEFAULT_DEST_3) ":" xstr(DEFAULT_DEST_4) ":"
    xstr(DEFAULT_DEST_5),
    "uu:vv:ww:xx:yy:zz" },
  { "payload", 'p', POPT_ARG_STRING, &gpch_temp, ARG_PAYLOAD,
    "String to send as frame data, can not be used with --payload-file option."
    " Binary data can be set with -b64 option."
    " Default: \"" DEFAULT_PAYLOAD "\"",
    "data" },
  { "payload-file", 'f', POPT_ARG_STRING, &gpch_temp, ARG_PAYLOAD_FILE,
    "File to read binary data from to send in frame, can not be used"
    " with --payload. Reads at most 10KiB, however, be careful of the MTU as "
    "no additional checks are performed. Can be given as '-' to read from "
    "stdin. Default: NONE",
    "file-name" },
  { "b64", 'b', POPT_ARG_NONE | POPT_ARGFLAG_ONEDASH, &gb_base64, ARG_BASE64,
    "Decode payload given on command line using base64 codec before"
    " sending frame.",
    NULL },
  POPT_AUTOHELP
  POPT_TABLEEND
};

bool args_parse(int argc, const char** argv) {
  poptContext ctx = poptGetContext("rether", argc, argv, options, 0);
  if (ctx == NULL) {
    fprintf(stderr, "Unable to parse command line arguments\n");
    return false;
  }

  int state = 0;
  int rc = 0;
  while ((rc = poptGetNextOpt(ctx)) > 0) {
    /* Check send/receive state */
    switch (rc) {
      /* Neutral args */
    case ARG_ETHERTYPE:
    case ARG_SOURCE_INT:
      break;
      /* Receive only args */
    case ARG_RECEIVE_ONLY:
      if (state & STATE_SEND) {
        fprintf(stderr,
                "--receive-only not compatible with given command line"
                " arguments\n");
        goto fail;
      }
      state |= STATE_RECEIVE;
      break;
      /* Send only arguments */
    case ARG_DEST_MAC:
    case ARG_PAYLOAD:
    case ARG_PAYLOAD_FILE:
    case ARG_BASE64:
      if (state & STATE_RECEIVE) {
        fprintf(stderr,
                "--receive-only not compatible with given command line"
                " arguments\n");
        goto fail;
      }
      state |= STATE_SEND;
      break;
      break;
    default:
      fprintf(stderr, "Internal error\n");
      goto fail;
    }

    switch (rc) {
    case ARG_ETHERTYPE:
      if (gi_ethertype < 0x0600 || gi_ethertype > 0xFFFF) {
        fprintf(stderr, "Out of range ethertype specified, 0x0600-0xFFFF\n");
        goto fail;
      } else {
        gui_ethertype = gi_ethertype & 0xFFFF;
      }
      break;
    case ARG_RECEIVE_ONLY:
      break;
    case ARG_DEST_MAC:
      if (sscanf(gpch_temp,
                 "%02" SCNx8 ":"
                 "%02" SCNx8 ":"
                 "%02" SCNx8 ":"
                 "%02" SCNx8 ":"
                 "%02" SCNx8 ":"
                 "%02" SCNx8,
                 &gui_destmac[0],
                 &gui_destmac[1],
                 &gui_destmac[2],
                 &gui_destmac[3],
                 &gui_destmac[4],
                 &gui_destmac[5]) != 6) {
        fprintf(stderr, "Invalid destination MAC: %s\nShould be of the format "
                "uu:vv:ww:xx:yy:zz\n", gpch_temp);
        goto fail;
      }
      break;
    case ARG_SOURCE_INT:
      if (gpch_source_interface[0] != '\0') {
        fprintf(stderr, "Source interface can only be given once.\n");
        goto fail;
      }
      if (strlen(gpch_temp) > IFNAMSIZ-1) {
        fprintf(stderr, "Interface name too long.\n");
        goto fail;
      }
      strcpy(gpch_source_interface, gpch_temp);
      break;
    case ARG_PAYLOAD:
      if (gui_data != NULL) {
        fprintf(stderr,
                "Option --payload can only be given once "
                "and it is incompatible with --payload-file.\n");
        goto fail;
      }
      if (gb_base64) {
        bool ok = base64_decode_alloc(gpch_temp, strlen(gpch_temp),
                                      (char**)&gui_data, &gz_data);
        if (!ok) {
          fprintf(stderr, "Invalid base64.\n");
          goto fail;
        }
        if (gui_data == NULL) {
          fprintf(stderr, "Memory allocation error while decoding.\n");
          goto fail;
        }
      } else {
        gui_data = (uint8_t*)malloc(strlen(gpch_temp)+1);
        strcpy((char*)gui_data, gpch_temp);
        gz_data = strlen(gpch_temp);
      }
      break;
    case ARG_PAYLOAD_FILE:
      if (gui_data != NULL) {
        fprintf(stderr,
                "Option --payload-file can only be given once "
                "and it is incompatible with --payload.\n");
        goto fail;
      }
      FILE* f = NULL;
      if (strcmp("-", gpch_temp) == 0) {
        f = stdin;
      } else {
        f = fopen(gpch_temp, "r");
      }
      if (f == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", gpch_temp);
        goto fail;
      }
      void *buff = malloc(10240);
      size_t i = fread(buff, 1, 10240, f);
      fclose(f);
      if (i == 0) {
        fprintf(stderr, "Unable to read file: %s\n", gpch_temp);
        free(buff);
        buff = NULL;
        goto fail;
      }
      gui_data = (uint8_t*)malloc(i);
      memcpy(gui_data, buff, i);
      gz_data = i;
      free(buff);
      buff = NULL;
      break;
    case ARG_BASE64:
      if (gui_data != NULL) {
        fprintf(stderr, "-b64 option must be given before --payload option\n");
        goto fail;
      }
      break;
    default:
      fprintf(stderr, "Internal error, unknown option argument\n");
      goto fail;
    }
    if (gpch_temp != NULL) {
      free(gpch_temp);
      gpch_temp = NULL;
    }
  }
  if (rc != -1) {
    fprintf(stderr,
            "Unable to parse command line arg \"%s\" because %s\n",
            poptBadOption(ctx, 0),
            poptStrerror(rc));
    goto fail;
  }

  if (gpch_source_interface[0] == '\0') {
    strcpy(gpch_source_interface, DEFAULT_INTERFACE);
  }

  if (gui_data == NULL) {
    gui_data = (uint8_t*)malloc(strlen(DEFAULT_PAYLOAD)+1);
    strcpy((char*)gui_data, DEFAULT_PAYLOAD);
    gz_data = strlen(DEFAULT_PAYLOAD);
  }

  poptFreeContext(ctx);
  return true;
 fail:
  if (gpch_temp != NULL) {
    free(gpch_temp);
    gpch_temp = NULL;
  }
  poptFreeContext(ctx);
  destroy();
  return false;
}
