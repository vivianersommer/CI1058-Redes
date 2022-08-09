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

    // int soquete;
    // char *mensagem = "123456789012345678901234";
    // soquete = ConexaoRawSocket("lo");
    // int escrito = write(soquete, mensagem, strlen(mensagem) + 1);
    // printf("Mensagem enviada com sucesso!\n");

    int soquete;
    mensagem *teste = malloc(sizeof(mensagem));
    teste->marcadorInicio = 0;
    teste->tamanho = 0;
    teste->sequencia = 0;
    teste->tipo = 0;
    teste->dados = "123456789012345678901234";
    teste->paridade = 0;

    soquete = ConexaoRawSocket("lo");
    printf("Tamanho = %ld\n", sizeof(teste));
    int escrito = write(soquete, teste, sizeof(teste) > 16? sizeof(teste) : 16);
    if (escrito == -1){
        printf("Mensagem não enviada!\n");
        printf("Erro = %d\n", escrito);
    } else {
        printf("Mensagem enviada com sucesso!\n");
    }

} 