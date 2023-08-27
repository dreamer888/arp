#include "arp.h"

int main(int argv, char** args)
{
    int sd, mask_num, host_num;
    uint8_t mac_src[6], ether_frame[IP_MAXPACKET];
    uint32_t ip_src, net_mask, now_ip;
    // struct sockaddr_in *ipv4;
    struct sockaddr_ll device;
    struct ifreq ifr;
    char *interface = args[1];
    arp_hdr arphdr;

    memset(ether_frame, 0, sizeof(ether_frame));

    if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl()");
        exit (EXIT_FAILURE);
    }

    snprintf (ifr.ifr_name, sizeof(ifr.ifr_name), interface);

    ip_src = get_ipv4 (sd, &ifr);
    print_ipv4(ip_src, stdout);

    get_hwaddr (sd, &ifr, mac_src);
    print_mac (mac_src, stdout);

    mask_num = get_ipv4_mask_num (sd, &ifr);
    host_num = 1 << mask_num;
    net_mask = 0xffffffff >> mask_num;
    close(sd);

    
    device.sll_ifindex = if_nametoindex (interface);
    device.sll_family = AF_PACKET;
    memset(device.sll_addr, 0xff, 6);
    device.sll_halen = htons (6);

    now_ip = get_start_ip (ip_src, net_mask);
    ip_src = next_ip(now_ip);
    print_ipv4(ip_src, stdout);
    
    for (int i = 0; i < host_num - 1; i ++){
        now_ip = next_ip(now_ip);
        //print_ipv4(ip_src, stdout);
        //print_ipv4(now_ip, stdout);
        //set_request_arphdr (&arphdr, mac_src, &ip_src, &now_ip);
        //set_broadcast_eth (ether_frame, &arphdr, mac_src);
        //send_ether_frame (ether_frame, 60, device);
    }

    return 0;
}