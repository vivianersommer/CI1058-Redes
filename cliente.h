#ifndef __CLIENTE__
#define __CLIENTE__

#include "conexao.h"
#define MAX 20

typedef struct TipoComando{
	int tipo; //tipo de comando passado LSR, LSL...
	char* comando; //comando lido do terminal
	char* argumento; //argumento lido do terminal
}TipoComando;

void leitura(TipoComando *tipoComando);
void recebe_resposta_ls(Mensagem *mensagem, int soquete);
void recebe_arquivo(Mensagem *mensagem, char *nome, unsigned char tipo, unsigned char prox_enviar, unsigned char prox_receber, int soquete);
void ls_remoto(TipoComando* tipoComando, int soquete);
void ls_local(TipoComando* tipoComando);
void comandos(int soquete);
int cliente();

#endif