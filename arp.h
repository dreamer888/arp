#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <sys/ioctl.h>        // macro ioctl is defined
#include <net/ethernet.h>
#include <errno.h>            // errno, perror()

#if __APPLE__

#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_dl.h>
#include <sys/sysctl.h>

#define ETH_P_ARP   ETHERTYPE_ARP
#define ETH_P_IP    ETHERTYPE_IP

#endif


#ifdef linux

#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <linux/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_ARP = 0x0806
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)

#endif


#ifndef __ARP_H_
#define __ARP_H_

static const uint8_t arp_broadcast_mac[6] = {0, 0, 0, 0, 0, 0};
static const uint8_t eth_broadcast_mac[6] = {255, 255, 255, 255, 255, 255};

typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t htype;
  uint16_t ptype;
  uint8_t hlen;
  uint8_t plen;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

#define ETH_HDRLEN 14
#define IP4_HDRLEN 20
#define ARP_HDRLEN 28
#define ARPOP_REQUEST 1
#define ARPOP_REPLY 2

void print_ipv4 (uint32_t ipv4, void *where);

uint32_t get_ipv4 (int sd, struct ifreq *ifr);

void print_mac (uint8_t *mac, void *where);

void get_hwaddr (int sd, struct ifreq *ifr, uint8_t *mac);

uint32_t get_ipv4_netmask (int sd, struct ifreq *ifr);

int get_ipv4_mask_num (int sd, struct ifreq *ifr);

void set_arphdr (arp_hdr *arphdr, uint8_t *mac_src, uint32_t *ip_src, uint8_t *mac_dst, uint32_t *ip_dst, int op);

void set_eth(uint8_t *ether_frame, arp_hdr *arphdr, uint8_t *mac_src, uint8_t *mac_dst);

 void send_ether_frame (uint8_t *ether_frame, int frame_length, struct sockaddr_ll device);  //lgl

int get_host_num(uint32_t net_mask);

uint32_t next_ip(uint32_t ip);

uint32_t get_start_ip (uint32_t ip_src, uint32_t net_mask);

uint32_t array2ip (uint8_t *ip);

void get_rand_mac (uint8_t *mac);

#define set_request_arphdr(arp_hdr, mac_src, ip_src, ip_dst)						\
	set_arphdr(arp_hdr, mac_src, ip_src, &arp_broadcast_mac, ip_dst, ARPOP_REQUEST)

#define set_gratuitous_arphdr(arp_hdr, mac_src, ip_src)								\
	set_request_arphdr(arp_hdr, mac_src, ip_src, ip_src)

#define set_reply_arphdr(arp_hdr, mac_src, ip_src, mac_dst, ip_dst)					\
	set_arphdr(arp_hdr, mac_src, ip_src, mac_dst, ip_dst, ARPOP_REPLY)

#define set_broadcast_eth(ether_frame, arp_hdr, mac_src)							\
	set_eth(ether_frame, arp_hdr, mac_src, &eth_broadcast_mac)

#define printf_ipv4(ipv4)															\
	print_ipv4(ipv4, stdout)

#define printf_mac(mac)																\
	print_mac(mac, stdout)

#endif