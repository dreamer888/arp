#include "arp.h"

int main(int argc, char **argv)
{
    int sd;
    uint8_t mac_src[6], ether_frame[IP_MAXPACKET];
    uint32_t ip_src;
    struct sockaddr_in *ipv4;
    struct sockaddr_ll device;
    struct ifreq ifr;
    char *interface = "ens33";
    arp_hdr arphdr;

    if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl()");
        exit (EXIT_FAILURE);
    }

    snprintf (ifr.ifr_name, sizeof(ifr.ifr_name), interface);
    ip_src = get_ipv4 (sd, &ifr);
    print_ipv4(ip_src, stdout);

    get_hwaddr (sd, &ifr, mac_src);
    print_mac (mac_src, stdout);
    close(sd);

    device.sll_ifindex = if_nametoindex (interface);
    device.sll_family = AF_PACKET;
    memset(device.sll_addr, 0xff, 6);
    device.sll_halen = htons (6);
    
    set_gratuitous_arphdr (&arphdr, mac_src, &ip_src);
    set_broadcast_eth (ether_frame, &arphdr, mac_src);
    send_ether_frame (ether_frame, 42, device);

    return 0;
}