#include "arp.h"

int main(int argc, char **argv){
    int sd;
    char* interface = argv[1];
    struct sockaddr_ll device;
    uint8_t ether_frame[IP_MAXPACKET];
    memset(ether_frame, 0, sizeof(ether_frame));
    uint8_t mac_src[6] = {0x34, 0xa8, 0xeb, 0x4d, 0xce, 0x8d};
    uint8_t mac_dst[6] = {0x20, 0x3c, 0xae ,0x92, 0xff, 0x65};
    uint8_t ip_src_array[4] = {192, 168, 37, 130}, ip_dst_array[4] = {192, 168, 37, 111};
    uint32_t ip_src = array2ip(ip_src_array), ip_dst = array2ip(ip_dst_array);
    arp_hdr arphdr;
    print_ipv4 (ip_src, stdout);
    // get_rand_mac (mac_src);
    print_mac (mac_src, stdout);

    device.sll_ifindex = if_nametoindex (interface);
    device.sll_family = AF_PACKET;
    memset(device.sll_addr, 0xff, 6);
    device.sll_halen = htons (6);

    if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl()");
        exit (EXIT_FAILURE);
    }

    set_reply_arphdr (&arphdr, mac_src, &ip_src, mac_dst, &ip_dst);
    set_eth (ether_frame, &arphdr, mac_src, mac_dst);
    while(1){
        send_ether_frame (ether_frame, 60, device);
        sleep(5);
    }

    close (sd);
}