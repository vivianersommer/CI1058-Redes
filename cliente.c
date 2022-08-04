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

#define TENTATIVAS 50

typedef struct sockaddr_ll endereco;

int main(){

    int soquete, i;
    struct pollfd p;
    char *buf;
	char *comandoAux, *argumentoAux, *lComandoAux;
    endereco enderecoSocket;
    char * cwd;

	comandoAux = (char *) malloc(100);
	lComandoAux = (char *) malloc(100);
	argumentoAux = (char *) malloc(100);
    
	
    system("ifconfig eth0 promisc");
    soquete = socket(AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
   
    memset(&enderecoSocket, 0, sizeof(endereco));
	enderecoSocket.sll_family = AF_PACKET;
	enderecoSocket.sll_protocol = htons(ETH_P_ALL);
	enderecoSocket.sll_pkttype = PACKET_OTHERHOST;
	enderecoSocket.sll_ifindex = 2;
	enderecoSocket.sll_hatype = ARPHRD_ETHER;
	enderecoSocket.sll_halen = ETH_ALEN;
	enderecoSocket.sll_addr[0] = 0x54 ;
	enderecoSocket.sll_addr[1] = 0x04 ;
	enderecoSocket.sll_addr[2] = 0xA6 ;
	enderecoSocket.sll_addr[3] = 0x2C ;
	enderecoSocket.sll_addr[4] = 0x57 ;
	enderecoSocket.sll_addr[5] = 0x1F ;
	enderecoSocket.sll_addr[6] = 0x00 ; 
	enderecoSocket.sll_addr[7] = 0x00 ;  
    
    p.fd = soquete;
	p.events = POLLIN;

    buf = getcwd(0, 0);
	strcat(buf, "$ ");
	printf("%s", buf);
    fflush(stdin);

	fgets(lComandoAux, 100, stdin);
	sscanf(lComandoAux, "%s", comandoAux);

    printf("Mensagem recebida = %s \n", comandoAux);

    return 1;
}