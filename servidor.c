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

int main()
{
    int soquete, lidos;
    static char buff[68];

    soquete = ConexaoRawSocket("lo");
    lidos = recv(soquete, buff, sizeof(buff), 0);
    printf("Mensagem recebida com sucesso!\n");
    printf("Mensagem  = %s!\n", buff);

}