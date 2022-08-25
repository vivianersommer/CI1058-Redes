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

struct Mensagem *cria_mensagem(){
    struct Mensagem *mensagem = malloc(sizeof(struct Mensagem));
    mensagem->dados = malloc(sizeof(char) * 10);
    mensagem->marcadorInicio = 00111100;
    mensagem->tamanho = 0;
    mensagem->sequencia = 0;
    mensagem->tipo = 0;
    mensagem->dados = "123456789";
    mensagem->paridade = 0;

    return mensagem;
}

int cliente(){

    int soquete;
    struct Mensagem *mensagem = cria_mensagem();

    soquete = ConexaoRawSocket("lo");
    int escrito = write(soquete, mensagem, sizeof(struct Mensagem) > 16? sizeof(struct Mensagem) : 16);

    if (escrito == -1){
        printf("Mensagem não enviada!\n");
        printf("Erro = %d\n", escrito);
    } else {
        printf("Mensagem enviada com sucesso!\n");
    }

    return 1;

} 