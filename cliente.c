#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <errno.h>

#define TENTATIVAS 50000


int main(){

    int soquete, i = 0;
    struct ifreq ir;
    struct sockaddr_ll endereco;
    struct packet_mreq mr;
    char *device = "lo", *mensagem;

    // CRIAR SOCKET ----------------------------------------------------------------------------
	soquete = socket(AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    if(soquete < 0){
		printf("Erro ao criar Socket!\n");
		exit(-1);
	} else {
        printf("Socket criado com sucesso!\n");
    }
    //------------------------------------------------------------------------------------------

    // CONECTAR AO DISPOSITIVO -----------------------------------------------------------------
    memset(&ir, 0, sizeof(struct ifreq));  	
    int device_size = sizeof(device);
    memcpy(ir.ifr_name, device, device_size);
    if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
        printf("Erro em ioctl!\n");
        printf("Motivo:  %s \n",strerror(errno));
     	exit(-1);
    } else {
        printf("Ioctl executado com sucesso!\n");
    }
    //------------------------------------------------------------------------------------------

    // IP DO DISPOSITIVO -----------------------------------------------------------------------
    memset(&endereco, 0, sizeof(endereco)); 	
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ir.ifr_ifindex;
    if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
        printf("Erro no bind!\n");
        exit(-1);
    } else {
        printf("Bind executado com sucesso!\n");
    }
    //------------------------------------------------------------------------------------------

    // MODO PROMISCUO --------------------------------------------------------------------------
    memset(&mr, 0, sizeof(mr));         
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
        printf("Erro ao fazer setsockopt!\n");
        exit(-1);
    } else {
        printf("Setsockopt executado com sucesso!\n");
    }
    //------------------------------------------------------------------------------------------

    // LEITURA DA MENSAGEM ---------------------------------------------------------------------
    for (i = 0; (recv(soquete, mensagem, 32, 0) == -1) && (i < TENTATIVAS); i++) {
        printf("Erro ao receber mensagem!\n");
        printf("Motivo:  %s \n",strerror(errno));
        exit(-1);
    } 
    printf("Sucesso ao receber mensagem!\n");

    if (i >= TENTATIVAS) {
        printf("Erro , número máximo de tentativas feitas!\n");
        exit(-1);
    }
    //------------------------------------------------------------------------------------------

    printf("Mensagem recebida = %s \n", mensagem);
    return 1;
} 