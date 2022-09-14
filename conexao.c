#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include "cliente.h"

int habilitar_rede(){

    // system("ifconfig eth0 promisc");
    int soquete = ConexaoRawSocket("lo"); // TODO: quando for para o cabo, usar eth0
    return soquete;
}

void desabilitar_rede(int soquete){

    close(soquete);
	// system("ifconfig eth0 -promisc");
}

int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }

  return soquete;
}

unsigned char paridade(unsigned char* dados, unsigned char tamanho){

  unsigned char paridades = 0x00;

  for(int i=0; i < tamanho; i++){
    paridades = paridades ^ dados[i];
  }

  return paridades;
}

Mensagem *cria_mensagem(unsigned char sequencia, unsigned char tipo, char *dados) {
	
  Mensagem *mensagem = malloc(sizeof(Mensagem));
	mensagem->marcadorInicio = 0x7E; //01111110
	mensagem->tamanho = strlen(dados);
	mensagem->sequencia = sequencia;
	mensagem->tipo = tipo;
	mensagem->paridade = paridade(dados, mensagem->tamanho);

	for (int i = 0; i < MAX_DADOS; i++) {
		mensagem->dados[i] = dados[i];
	}

	return mensagem;
}

void envia_mensagem(Mensagem *mensagem, int soquete) {
    int result_enviar = send(soquete, mensagem, sizeof(struct Mensagem), 0);
    puts("Enviando...");
}

int espera_mensagem(Mensagem *mensagem, int soquete) {

	  recv(soquete, mensagem, sizeof(struct Mensagem), 0); // leitura do soquete
    // checa o marcador de início e a paridade da mensagem recebida -------------------------------------------------------
    if ((mensagem->marcadorInicio == 0x7E)) {
        if (paridade(mensagem->dados, mensagem->tamanho) == mensagem->paridade){
            // mensagem = cria_mensagem(0x00, OK, "");
            // envia_mensagem(mensagem, soquete);
            return 1; //mensagem recebida com paridade certa
        } else {
            // mensagem = cria_mensagem(0x00, ERRO, F);
            // envia_mensagem(mensagem, soquete);
            return 0; //erro na paridade
        }
	  }
    // --------------------------------------------------------------------------------------------------------------------
	
    return -1; //lixo da placa de rede
}