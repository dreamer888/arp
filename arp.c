#include "arp.h"

void print_ipv4 (uint32_t ipv4, void *where){
    for (int i = 0; i < 3; i ++)
        fprintf(where, "%d.", (ipv4 >> (i * 8)) & 0xff);
    fprintf(where, "%d\n", (ipv4 >> 24) & 0xff);
}

uint32_t get_ipv4 (int sd, struct ifreq *ifr){
    struct sockaddr_in *ipv4;
    uint32_t ret;
    ioctl (sd, SIOCGIFADDR, ifr); // get src ip
    ipv4 = (struct sockaddr_in *)&ifr->ifr_addr;
    memcpy (&ret, &ipv4->sin_addr, 4);
    return ret;
}

void print_mac (uint8_t *mac, void *where){
    for (int i = 0; i < 5; i ++)
        fprintf(where, "%02x:", mac[i]);
    fprintf(where, "%02x\n", mac[5]);
}


void get_hwaddr (int sd, struct ifreq *ifr, uint8_t *mac){  // referenced from https://stackoverflow.com/questions/10593736/mac-address-from-interface-on-os-x-c
#if linux
    ioctl (sd, SIOCGIFHWADDR, ifr); // get src mac
    memcpy (mac, ifr->ifr_hwaddr.sa_data, 6);
#elif __APPLE__
    int                 mib[6], len;
    char                *buf;
    unsigned char       *ptr;
    struct if_msghdr    *ifm;
    struct sockaddr_dl  *sdl;
    uint8_t             *tmp = mac;  // sysctl seems to corrupt parameters's memory space

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;

    if ((mib[5] = if_nametoindex(ifr->ifr_name)) == 0) {
        perror("if_nametoindex error");
        return;
    }

    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        perror("sysctl: get length error");
        return;
    }
    if ((buf = (char*)malloc(len)) == NULL) {
        perror("malloc error");
        return;
    }

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        perror("sysctl: get hw info error");
        return;
    }

    ifm = (struct if_msghdr *)buf;
    sdl = (struct sockaddr_dl *)(ifm + 1);
    memcpy(tmp, LLADDR(sdl), 6);

    return;

#endif
}


uint32_t get_ipv4_netmask (int sd, struct ifreq *ifr){
    uint8_t net_mask[4];
    ioctl (sd, SIOCGIFNETMASK, ifr); // get net mask
    for (int i = 0; i < 4; i ++)
        memcpy (net_mask + i, ifr->ifr_netmask.sa_data + 4 - i, 1);
    return *(uint32_t*)net_mask;
}

int get_ipv4_mask_num (int sd, struct ifreq *ifr){
    int num = 0;
    uint8_t net_mask[4];
    ioctl (sd, SIOCGIFNETMASK, ifr);
    memcpy (net_mask, ifr->ifr_netmask.sa_data + 1, 4);

    for(int i = 0; i < 4; i ++){
        net_mask[i] = ifr->ifr_netmask.sa_data;
    }

    printf("netmask:");
    printf_ipv4(*(uint32_t*)net_mask);
    
    for(int i = 0; i < 4; i ++){
        net_mask[i] = ~net_mask[i];
        while (net_mask[i] != 0){
            net_mask[i] >>= 1;
            num ++;
        }
    }
    return num;
}

void set_arphdr (arp_hdr *arphdr, uint8_t *mac_src, uint32_t *ip_src, uint8_t *mac_dst, uint32_t *ip_dst, int op){
    arphdr->htype = htons(1);
    arphdr->ptype = htons(ETH_P_IP);
    arphdr->hlen = 6;
    arphdr->plen = 4;
    arphdr->opcode = htons(op);
    memcpy (&arphdr->sender_mac, mac_src, 6);
    memcpy (&arphdr->sender_ip, ip_src, 4);
    memcpy (&arphdr->target_mac, mac_dst, 6);
    memcpy (&arphdr->target_ip, ip_dst, 4);
}

void set_eth(uint8_t *ether_frame, arp_hdr *arphdr, uint8_t *mac_src, uint8_t *mac_dst){
    memcpy (ether_frame, mac_dst, 6);
    memcpy (ether_frame + 6, mac_src, 6);
    ether_frame[12] = ETH_P_ARP / 256;
    ether_frame[13] = ETH_P_ARP % 256;
    memcpy (ether_frame + ETH_HDRLEN, arphdr, ARP_HDRLEN);
}

//lgl 
void send_ether_frame (uint8_t *ether_frame, int frame_length, struct sockaddr_ll device){
    int sd, bytes;
    if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ARP))) < 0) {
        perror ("socket() failed");
        exit (EXIT_FAILURE);
    }
    if ((bytes = sendto (sd, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
        perror ("sendto() failed");
        exit (EXIT_FAILURE);
    }
    close (sd);
}
 

int get_host_num(uint32_t net_mask){
    int num = 1;
    uint32_t temp = ~net_mask;
    while (temp != 0){
        temp <<= 1;
        num <<= 1;
    }
    return num;
}

uint32_t next_ip(uint32_t ipv4){
    uint8_t *ip = &ipv4;
    ip[3] += 1;
    if(ip[3] == 0){
        ip[2] += 1;
        if(ip[2] == 0){
            ip[1] += 1;
            if(ip[1] == 0)
                ip[0] += 1;
        }
    }
    return *(uint32_t*)ip;
}

uint32_t get_start_ip (uint32_t ip_src, uint32_t net_mask){
    return ip_src & net_mask;
}

uint32_t array2ip (uint8_t *ip){
    uint32_t ret = 0;
    for (int i = 0; i < 4; i ++)
        ret |= ((uint32_t)ip[i]) << (8 * i);
    return ret;
}

void get_rand_mac (uint8_t *mac){
    srand (time (NULL));
    int seg1 = rand(), seg2 = rand();
    memcpy (mac, &seg1, 4);
    memcpy (mac + 4, &seg2, 2);
}