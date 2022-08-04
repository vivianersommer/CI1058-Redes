#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <linux/if_arp.h>
#include <unistd.h>
#include <sys/time.h>
#include <poll.h>
#include <dirent.h>
#include <sys/statvfs.h>

int main(){
    int soquete;
    struct ifreq ir;
    struct sockaddr_ll endereco;
    struct packet_mreq mr;
    struct pollfd poll;
    char *device = "eth0";

    system("ifconfig eth0 promisc");
	soquete = socket(AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    memset(&endereco, 0, sizeof(struct sockaddr_ll)); 	
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
	endereco.sll_pkttype = PACKET_OTHERHOST;
	endereco.sll_ifindex = 2;
	endereco.sll_hatype = ARPHRD_ETHER;
	endereco.sll_halen = ETH_ALEN;
	endereco.sll_addr[0] = 0x54 ;
	endereco.sll_addr[1] = 0x04 ;
	endereco.sll_addr[2] = 0xA6 ;
	endereco.sll_addr[3] = 0x2C ;
	endereco.sll_addr[4] = 0x57 ;
	endereco.sll_addr[5] = 0x1F ;
	endereco.sll_addr[6] = 0x00 ;   
	endereco.sll_addr[7] = 0x00 ;   

    poll.fd = soquete;
	poll.events = POLLIN;

    sendto(soquete, "OI", 21, 0, (struct sockaddr *) &endereco, sizeof(struct sockaddr_ll));
 	return 1;
}