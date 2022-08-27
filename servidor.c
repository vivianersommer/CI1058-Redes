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
#include <netinet/in.h>
#include <unistd.h>
#include "conexao.h"


int servidor() {

    int soquete;
    struct Mensagem *msg = malloc(sizeof(struct Mensagem));

    soquete = ConexaoRawSocket("lo");
    
    recv(soquete, msg, sizeof(struct Mensagem), 0); 

    printf("Mensagem recebida com sucesso!\n");

    printf("Mensagem  = \n%s \n%c\n", msg->dados, msg->marcadorInicio);

    return 1;
}