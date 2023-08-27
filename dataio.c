#include "dataio.h"

void add (uint32_t ip, uint8_t* mac, ipmac_t** hash, int mask)
{
    ipmac_t *now = (ipmac_t*)malloc(sizeof(ipmac_t));
    now->ip = ip;
    for (int i = 0; i < 6; i ++)
        now->mac[i] = mac[i];

    int index = ip >> (32 - mask);
    if (hash[index] == NULL || hash[index]->mac != ip)
        hash[index] = now;

}

void save (ipmac_t **hash, int host_num)
{
    FILE *fp;
    if ((fp = fopen ("ip_mac.txt", "w")) == NULL){
        printf ("Error occurs while opening the file.");
        exit (1);
    }
    for (int i = 0; i < host_num; i ++){
        if (hash[i] != NULL){
            print_ipv4(hash[i]->ip, fp);
            print_mac(hash[i]->mac, fp);
        }
    }
    fclose (fp);
}