#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "conexao.h"


int cliente(){

    int soquete;
    struct timeval timeout;
    struct Mensagem *mensagem = malloc(sizeof(struct Mensagem));

    mensagem->marcadorInicio = 0x7E; // hex 7E = 01111110
    strncpy(mensagem->dados, "AAAAAAAAAAAAAAAAAAAAAAA", MAX_DADOS);;

    soquete = ConexaoRawSocket("lo");
    
    int escrito = send(soquete, mensagem, sizeof(struct Mensagem), 0); //que isso? escrito?

    while (escrito == -1 && timeout.tv_usec != 0){
        printf("Mensagem não enviada!\n");
        printf("Erro = %d\n", escrito);
        printf("TENTANDO NOVAMENTE!\n");
        escrito = send(soquete, mensagem, sizeof(struct Mensagem), 0);

    } 

    if(timeout.tv_usec != 0){
        printf("TIMEOUT !!!\n");
        exit(1);
    }

    printf("Mensagem enviada com sucesso!\n");
    printf("Mensagem  = \n%s \n%c\n", mensagem->dados, mensagem->marcadorInicio);
    

    return 1;

} 