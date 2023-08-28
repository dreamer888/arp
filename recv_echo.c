#include "arp.h"


/**** 
recv_echo.c.c

this program  should run on another pc whith the  arp_send  !


***/

int main(int argc, char** argv)
{
    uint8_t** hash;
    int sd, status, mask_num, host_num;
    uint8_t* ether_frame = (uint8_t*) malloc (IP_MAXPACKET);
    uint8_t* ether_frame1 = (uint8_t*) malloc (IP_MAXPACKET);
    uint8_t sender_mac[6];
    uint32_t sender_ip;
    struct ifreq ifr;
    arp_hdr *arphdr;

 struct sockaddr_ll device;
 char *interface = argv[1];
 char * strMac =NULL;
   char mac[100];

uint32_t ip_src;
uint8_t mac_src[6];
char strIp[100];

struct sockaddr_ll sll;
bzero(&sll,sizeof(sll));
snprintf (ifr.ifr_name, sizeof(ifr.ifr_name), interface);

memset(strIp,0 ,sizeof(strIp));

    //if ((sd = socket (PF_INET, SOCK_RAW, htons (ETH_P_ARP))) < 0) {
        if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ARP))) < 0) {

        perror ("socket() failed ...lgl");
	if(ether_frame)   free(ether_frame); 
	if (ether_frame1  )  free(ether_frame1);
	ether_frame = NULL;
	ether_frame1 =NULL;

        exit (EXIT_FAILURE);  
	 
    }


get_hwaddr (sd, &ifr, mac_src);
print_mac (mac_src, stdout);

ip_src = get_ipv4 (sd, &ifr);
 printf("sorce ip:  ");
 print_ipv4(ip_src, stdout);



    sll.sll_ifindex = ifr.ifr_ifindex;
   printf("listening .....  \n ");

   
    while (1) {
        if ((status = recv (sd, ether_frame, IP_MAXPACKET, 0)) < 0) {
            if (errno == EINTR) {
                memset (ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
                              printf(" EINTR  \n   ");

                continue;
            } else {
                perror ("recv() failed:");
		free(ether_frame);
		free(ether_frame1);
                exit (EXIT_FAILURE);
            }
        }

	

  	arphdr = (arp_hdr*)(ether_frame + 14);


        if (((((ether_frame[12]) << 8) + ether_frame[13]) == ETH_P_ARP) && (ntohs (arphdr->opcode) == ARPOP_REQUEST)) {

	
            
            memcpy (sender_mac, ether_frame + 22, 6);
            memcpy (&sender_ip, ether_frame + 28, 4);

                //printf("  \n   b4 convertIp  \n  ");
                memset(strIp,0 ,sizeof(strIp));
              
                memset(mac,0 ,sizeof(mac));
                 char * pIp = convertIp(sender_ip,strIp);
                  //printf("  \n   strIp =pIp = %s \n  ", strIp);
             //strcpy(strIp, pIp);
             //printf("  \n   after convertIp  \n  ");
             strMac= findMac("ip_mac.txt", strIp,mac);  //  find  ip address  in the ip-mac map file
              //printf("  \n  strMac=  %s \n  ",strMac);
             if (strMac ==NULL) 
		continue;  

             //int   tail = (sender_ip >> 24) & 0xff ; //  pass  192.168.0.1

	            printf("sender mac: \n ");  //if (tail !=1)
                   print_mac(sender_mac, stdout);
          
                   printf("sender ip: ");
                    print_ipv4(sender_ip, stdout);
                   printf("\n ");

	////////begin to echo  the REQUEST //////

        memset(ether_frame1, 0, sizeof(ether_frame));
	memcpy(ether_frame1,ether_frame,IP_MAXPACKET);
  
    	//device.sll_ifindex = if_nametoindex (interface);
    	//device.sll_family = AF_PACKET;

      	
	memset(ether_frame1,0xff,6); /*Ŀ  mac  ַ*/
	//memcpy(ether_frame1,ether_frame+6,6);   /*Ŀ  mac  ַ*/
	memcpy(ether_frame1+6,ether_frame+32,6);/*Դmac  ַ */
	memcpy(ether_frame1+12,ether_frame+12,8);

	ether_frame1[20] = 0x00;//// ARPOP_REPLY =0x0002
	ether_frame1[21] = 0x02; // ARPOP_REPLY =0x0002 
/*if u want to simiulate ARP, u had better to custom your own code instead of 0x0002 , such as 0x0008
  otherwise the peer can not know who send  this packae ,system program or your program 
*/
	memcpy(ether_frame1+22,ether_frame+32,6); //source mac
	memcpy(ether_frame1+28,ether_frame+38,4); //source ip
	memcpy(ether_frame1+32,ether_frame+22,6); //dest mac
	memcpy(ether_frame1+38,ether_frame+28,4); //det ip

	//printf("\n sendto: \n ");
        sendto(sd,ether_frame1, 42, 0, (struct sockaddr *)&sll, sizeof(sll));  
 

        } //if is ARPOP_REQUEST
	//printf("  \n not  REQUEST   \n  ");


    }  //  while (true) {

	free(ether_frame);
	free(ether_frame1);
	ether_frame = NULL;
	ether_frame1 =NULL;

    return 0;
}
