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
    int soquete, lidos;
    struct Mensagem *teste = malloc(sizeof(struct Mensagem));

    soquete = ConexaoRawSocket("lo");
    lidos = recv(soquete, teste, sizeof(teste), 0);
    printf("Mensagem recebida com sucesso!\n");
    printf("Mensagem  = %s!\n", teste->dados);

    return 1;
}