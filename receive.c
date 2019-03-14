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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include "args.h"

/* CTRL-C sets to true */
static bool gb_shutdown = false;
static void receive_signal_handler(int signo) {
  if (signo == SIGINT) {
    gb_shutdown = true;
  }
}

bool receive() {
  bool b_result = false;
  int i_sockfd;
  struct ifreq s_ifreq;
  int i_ifindex = 0;
  int i_mtu = 0;
  uint8_t *ui_frame_buffer = NULL;
  struct sockaddr_ll s_ingress;
  size_t z_frame = 0;

  /* Open socket to send on */
  if ((i_sockfd = socket(AF_PACKET, SOCK_RAW, htons(gui_ethertype))) == -1) {
    fprintf(stderr, "Unable to open socket\n");
    goto fail;
  }

  /* Lookup interface index */
  memset(&s_ifreq, 0, sizeof(struct ifreq));
  strncpy(s_ifreq.ifr_name, gpch_source_interface, IFNAMSIZ-1);
  if (ioctl(i_sockfd, SIOCGIFINDEX, &s_ifreq) < 0) {
    fprintf(stderr, "Unable to find interface %s\n", gpch_source_interface);
    goto fail;
  }
  i_ifindex = s_ifreq.ifr_ifindex;

  /* Lookup interface MTU */
  memset(&s_ifreq, 0, sizeof(struct ifreq));
  strncpy(s_ifreq.ifr_name, gpch_source_interface, IFNAMSIZ-1);
  if (ioctl(i_sockfd, SIOCGIFMTU, &s_ifreq) < 0) {
    fprintf(stderr, "Unable to find MTU for %s\n", gpch_source_interface);
    goto fail;
  }
  i_mtu = s_ifreq.ifr_mtu;

  /* Lookup source MAC */
  memset(&s_ifreq, 0, sizeof(struct ifreq));
  strncpy(s_ifreq.ifr_name, gpch_source_interface, IFNAMSIZ-1);
  if (ioctl(i_sockfd, SIOCGIFHWADDR, &s_ifreq) < 0) {
    fprintf(stderr,
            "Unable to find mac address for %s\n",
            gpch_source_interface);
    goto fail;
  }
  if (s_ifreq.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
    fprintf(stderr, "Only support on ethernet interfaces\n");
    goto fail;
  }

  printf("\tMTU: %d\n", i_mtu);

  z_frame = i_mtu + sizeof(struct ether_header);
  ui_frame_buffer = (uint8_t*)malloc(i_mtu + sizeof(struct ether_header));

  memset(&s_ingress, 0, sizeof(struct sockaddr_ll));
  s_ingress.sll_ifindex = i_ifindex;
  s_ingress.sll_family = AF_PACKET;
  s_ingress.sll_protocol = htons(gui_ethertype);
  if (bind(i_sockfd,
           (struct sockaddr *)&s_ingress,
           sizeof(struct sockaddr_ll)) < 0) {
    fprintf(stderr, "Unable to bind socket to interface.\n");
    goto fail;
  }

  /* Setup CTRL-C handler */
  struct sigaction act;
  act.sa_handler = receive_signal_handler;
  act.sa_flags = 0;
  sigfillset(&(act.sa_mask));
  act.sa_restorer = NULL;
  sigaction(SIGINT, &act, NULL);

  ssize_t z_r = 0;
  int ctr = 0;
  while (!gb_shutdown &&
         (z_r = recv(i_sockfd, ui_frame_buffer, z_frame, 0)
          ) > ((ssize_t)sizeof(struct ether_header))) {
    struct ether_header *ps_ehdr = (struct ether_header *)ui_frame_buffer;
    printf("Frame %03d: "
           "src=%02X:%02X:%02X:%02X:%02X:%02X "
           "dest=%02X:%02X:%02X:%02X:%02X:%02X "
           "ethertype=0x%04X "
           "len=%zd\n",
           ctr++,
           ps_ehdr->ether_shost[0], ps_ehdr->ether_shost[1],
           ps_ehdr->ether_shost[2], ps_ehdr->ether_shost[3],
           ps_ehdr->ether_shost[4], ps_ehdr->ether_shost[5],
           ps_ehdr->ether_dhost[0], ps_ehdr->ether_dhost[1],
           ps_ehdr->ether_dhost[2], ps_ehdr->ether_dhost[3],
           ps_ehdr->ether_dhost[4], ps_ehdr->ether_dhost[5],
           ntohs(ps_ehdr->ether_type),
           z_r);
    uint8_t *ptr = ui_frame_buffer + sizeof(struct ether_header);
    uint8_t *end = ui_frame_buffer + z_r;
    for (int x = 0; ptr != end; x++) {
      printf(" %04X:", x*16);
      int n = ptr + 16 <= end ? 16 : end - ptr;
      for (int j = 0; j < 16; j++) {
        printf(j > n ? "%s  " : "%s%02x", j % 2 ? "" : " ", ptr[j]);
      }
      for (int j = 0; j < n; j++) {
        printf(j % 8 ? "%c" : " %c", isprint(ptr[j]) ? ptr[j] : '.');
      }
      ptr += n;
      printf("\n");
    }
  }
  if (z_r < 0 && errno != EINTR) {
    fprintf(stderr, "Error while reading from socket\n");
    goto fail;
  }
  if (gb_shutdown) {
    printf("Received SIGINT\n");
  } else {
    fprintf(stderr, "Internal error. Bytes received: %zd\n", z_r);
    goto fail;
  }

  b_result = true;
 fail:
  if (i_sockfd >= 0) {
    close(i_sockfd);
    i_sockfd = -1;
  }
  if (ui_frame_buffer != NULL) {
    free(ui_frame_buffer);
    ui_frame_buffer = NULL;
  }
  return b_result;
}
