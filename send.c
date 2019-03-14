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

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#include "args.h"

#define MINIMUM_FRAME_SIZE 64

bool send_one_shot() {
  bool b_result = false;
  int i_sockfd;
  struct ifreq s_ifreq;
  int i_ifindex = 0;
  int i_mtu = 0;
  size_t z_padding = 0, z_data = 0, z_frame = 0;
  uint8_t *ui_frame_buffer = NULL;
  struct ether_header *ps_ehdr = NULL;
  struct sockaddr_ll s_egress;

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

  z_data = gz_data > i_mtu ? i_mtu : gz_data;
  z_padding = gz_data < 46 ? 46 - gz_data : 0;
  z_frame = sizeof(struct ether_header) + z_padding + z_data;

  printf("\tSource LLA: %02X:%02X:%02X:%02X:%02X:%02X\n"
         "\tMTU: %d\n"
         "\tFrame size (excl. FCS): %zu %s%s\n",
         (uint8_t)(s_ifreq.ifr_hwaddr.sa_data)[0],
         (uint8_t)(s_ifreq.ifr_hwaddr.sa_data)[1],
         (uint8_t)(s_ifreq.ifr_hwaddr.sa_data)[2],
         (uint8_t)(s_ifreq.ifr_hwaddr.sa_data)[3],
         (uint8_t)(s_ifreq.ifr_hwaddr.sa_data)[4],
         (uint8_t)(s_ifreq.ifr_hwaddr.sa_data)[5],
         i_mtu,
         z_frame,
         z_padding ? "(padded)" : "",
         z_data != gz_data ? "(truncated)" : "");

  ui_frame_buffer = (uint8_t*)malloc(z_frame);
  memset(ui_frame_buffer, 0, z_frame);
  ps_ehdr = (struct ether_header*)ui_frame_buffer;

  memcpy(ps_ehdr->ether_shost, s_ifreq.ifr_hwaddr.sa_data, ETH_ALEN);
  memcpy(ps_ehdr->ether_dhost, gui_destmac, ETH_ALEN);
  ps_ehdr->ether_type = htons(gui_ethertype);
  ps_ehdr = NULL;

  memcpy(ui_frame_buffer + sizeof(struct ether_header), gui_data, z_data);

  /* Specify egress information */
  memset(&s_egress, 0, sizeof(struct sockaddr_ll));
  s_egress.sll_family = AF_PACKET;
  s_egress.sll_ifindex = i_ifindex;
  /* From man for packet(7):
   *
   * When you send packets, it is enough to specify sll_family,
   * sll_addr, sll_halen, sll_ifindex, and sll_protocol.  The other
   * fields should be 0.
   *
   * However, it appears that only sll_ifindex needs to be set.
   */

  /* Send packet */
  if (sendto(i_sockfd,
             ui_frame_buffer,
             z_frame,
             0,
             (struct sockaddr*)&s_egress,
             sizeof(struct sockaddr_ll)) != z_frame) {
    fprintf(stderr, "Did not send frame\n");
    goto fail;
  } else {
    printf("Frame sent.\n");
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
