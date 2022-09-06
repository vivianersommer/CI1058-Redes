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

    struct Mensagem *msg = malloc(sizeof(struct Mensagem));

    msg->marcadorInicio = 0x7E; // hex 7E = 01111110
    strncpy(msg->dados, "AAAAAAAAAAAAAAAAAAAAAAA", MAX_DADOS);;

    soquete = ConexaoRawSocket("lo");
    
    int escrito = send(soquete, msg, sizeof(struct Mensagem), 0); //que isso?

    if (escrito == -1){
        printf("Mensagem não enviada!\n");
        printf("Erro = %d\n", escrito);
    } else {
        printf("Mensagem enviada com sucesso!\n");
        printf("Mensagem  = \n%s \n%c\n", msg->dados, msg->marcadorInicio);
    }

    return 1;

} 