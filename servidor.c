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

// //Fazendo a paridade vertical em 8 bits, cada char tem 8 bits, entao eh so fazer xor com cada posicao do campo dados
// unsigned char paridade(unsigned char* dados){

//     unsigned char paridades;

//     for(int i=0; i<MAX_DADOS; i++){
//         paridades = paridades ^ dados[i];
//     }

//     return paridades;

// }

int servidor() {

    int soquete;
    unsigned char erro = '0';
    struct timeval timeout;
    struct Mensagem *mensagem = malloc(sizeof(struct Mensagem));

    soquete = ConexaoRawSocket("lo");
    
    recv(soquete, mensagem, sizeof(struct Mensagem), 0); 
    
    // while(timeout.tv_usec != 0){
    //     erro = paridade(mensagem);
    //     if (erro == '0'){
    //         //ack
    //         printf("Mensagem recebida com sucesso!\n");
    //         printf("Mensagem  = \n%s \n%c\n", mensagem->dados, mensagem->marcadorInicio);
    //         return 1;
    //     }else{
    //         //nack
    //         recv(soquete, mensagem, sizeof(struct Mensagem), 0);
    //         timeout.tv_usec--; //acho que n eh isso, mas eh algo assim
    //     }
    // }
    return -1;
}