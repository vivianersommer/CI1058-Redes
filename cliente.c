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

#define TENTATIVAS 50


int main(){

    int soquete, i;
    char *mensagem = (unsigned char *) calloc(32, sizeof(unsigned char));
    
    // CRIAR SOCKET ----------------------------------------------------------------------------
	soquete = socket(AF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;
    if(soquete < 0){
		printf("Erro ao criar Socket!\n");
		exit(-1);
	} else {
        printf("Socket criado com sucesso!\n");
    }
    //------------------------------------------------------------------------------------------

    // ALOCA ESPAÇO DA MENSAGEM ----------------------------------------------------------------
    if (mensagem == NULL) {
        printf("Erro ao alocar espaço para a mensagem!\n");
        printf("Motivo:  %s \n",strerror(errno));
        exit(-1);
    } else {
        printf("Sucesso ao alocar espaço para a mensagem!\n");
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