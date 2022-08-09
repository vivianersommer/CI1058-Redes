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


int main(){

    int soquete;
    char *mensagem = "123456789012345678901234";
    soquete = ConexaoRawSocket("lo");
    int escrito = write(soquete, mensagem, strlen(mensagem) + 1);
    printf("Mensagem enviada com sucesso!\n");

} 