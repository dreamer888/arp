
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arp.h"


uint8_t * ip2array( char * ip, uint8_t *  iparray ) 
{
   char* section = strtok(ip,".");  
	//printf ("section = %s \n",section ) ;
   int i=0;
  if(section != NULL ) 
   { 
	iparray[i]  = atoi(section   ) ;
                  i++;

   }

   while ( section != NULL)
   {
	section = strtok(NULL, ".");
	if(section  == NULL )
	break;
	iparray[i]  = atoi(section   ) ;

	
        if(iparray[i] == 0 ) 
	{
                  printf (" please  ensure  valid input !  \n" ) ;
		//break;
		
	}


	i++; 
   }
      

   return  iparray ;
} 

int main(int argv, char** args)
{
    int sd, mask_num, host_num;
    uint8_t mac_src[6], ether_frame[IP_MAXPACKET];
    uint32_t ip_src, net_mask,  ip_dst;
    // struct sockaddr_in *ipv4;
    struct sockaddr_ll device;
    struct ifreq ifr;
    char *interface = args[1];
    arp_hdr arphdr;
        char ipstr[20];

     uint8_t  ip_dst_array[4] = {192, 168, 37, 111};  //可以从 argv[1] 中提取  
     ip_dst = array2ip(ip_dst_array);

    memset(ether_frame, 0, sizeof(ether_frame));

    if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
        perror ("socket() failed to get socket descriptor for using ioctl()");
        exit (EXIT_FAILURE);
    }

    snprintf (ifr.ifr_name, sizeof(ifr.ifr_name), interface);

    ip_src = get_ipv4 (sd, &ifr);
    printf("sorce ip:  ");
    print_ipv4(ip_src, stdout);

    get_hwaddr (sd, &ifr, mac_src);
    printf("sorce mac:  ");
    print_mac (mac_src, stdout);
   
    printf("please input target ip ,and press Enter ：  \n");
    gets(ipstr);

	ip2array(ipstr,ip_dst_array);
	ip_dst = array2ip(ip_dst_array);

    printf("dest ip:  ");
    print_ipv4(ip_dst, stdout);

    //mask_num = get_ipv4_mask_num (sd, &ifr);
    //host_num = 1 << mask_num;
    //net_mask = 0xffffffff >> mask_num;
    close(sd);

    
    device.sll_ifindex = if_nametoindex (interface);
    device.sll_family = AF_PACKET;
    memset(device.sll_addr, 0xff, 6);
    device.sll_halen = htons (6);
 
    set_request_arphdr (&arphdr, mac_src, &ip_src, &ip_dst);
    set_broadcast_eth (ether_frame, &arphdr, mac_src);
    send_ether_frame (ether_frame, 60, device);
    

    return 0;
}