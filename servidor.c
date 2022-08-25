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


struct Mensagem *recebe_mensagem(){
    struct Mensagem *mensagem = malloc(sizeof(struct Mensagem));
    mensagem->dados = malloc(sizeof(char) * 10);
    return mensagem;
}

int servidor() {

    int soquete, lidos;
    struct Mensagem *mensagem = recebe_mensagem();

    soquete = ConexaoRawSocket("lo");
    
    recv(soquete, mensagem, sizeof(struct Mensagem), 0);

    printf("Mensagem recebida com sucesso!\n");
    printf("Mensagem  = %d!\n", mensagem->marcadorInicio);

    return 1;
}