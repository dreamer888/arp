#include "arp.h"
#include "dataio.h"

int main(int argc, char** argv)
{
    uint8_t** hash;
    int sd, status, mask_num, host_num;
    uint8_t* ether_frame = (uint8_t*) malloc (IP_MAXPACKET);
    uint8_t sender_mac[6];
    uint32_t sender_ip;
    struct ifreq ifr;
    arp_hdr *arphdr;

    //if ((sd = socket (PF_INET, SOCK_RAW, htons (ETH_P_ARP))) < 0) {
        if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ARP))) < 0) {
        perror ("socket() failed ...lgl");
        exit (EXIT_FAILURE);  
	//return 0;
    }

    snprintf (ifr.ifr_name, sizeof(ifr.ifr_name), argv[1]);

    sender_ip = get_ipv4(sd, &ifr);
    get_hwaddr(sd, &ifr, sender_mac);
    mask_num = get_ipv4_mask_num (sd, &ifr);
    
    printf("mask_num:   %d\n", mask_num);
    printf("listener mac:  ");
    printf_mac(sender_mac);
    printf("listener ip:  ");
    printf_ipv4(sender_ip);
   
    
    host_num = 1 << mask_num;
    hash = (ipmac_t**) malloc (sizeof(ipmac_t*) * (1 << mask_num));

       /////////////////////把本机的ip和mac先保存起来，因为主机数量少，不方便测试，本机的ip和mac 不会自动生成映射 ，你应该通过发送请求来生成这个映射关系//////////////////////
         ///////////实际应用中，应该屏蔽这两行代码， 这两行代码是为了方便展示演示效果
         add(sender_ip, sender_mac, hash, mask_num);
         save (hash, host_num);
        ///////////////////////////////


     printf("===continous recv    =====下面开始连续接收====\n ");

    arphdr = (arp_hdr*)(ether_frame + 14);
    while (1) {
        if ((status = recv (sd, ether_frame, IP_MAXPACKET, 0)) < 0) {
            if (errno == EINTR) {
                memset (ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
                continue;
            } else {
                perror ("recv() failed:");
                exit (EXIT_FAILURE);
            }
        }
        if (((((ether_frame[12]) << 8) + ether_frame[13]) == ETH_P_ARP) && (ntohs (arphdr->opcode) == ARPOP_REPLY)) {
            memcpy (sender_mac, ether_frame + 22, 6);
            memcpy (&sender_ip, ether_frame + 28, 4);
	        printf("sender mac:  ");
            print_mac(sender_mac, stdout);
	        printf("sender ip:  ");
            print_ipv4(sender_ip, stdout);
            add(sender_ip, sender_mac, hash, mask_num);
            save (hash, host_num);
        }
    }

    return 0;
}