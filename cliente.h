#ifndef __CLIENTE__
#define __CLIENTE__

#include "conexao.h"
#define MAX 100

typedef struct TipoComando{
	int tipo; //tipo de comando passado LSR, LSL...
	char* comando; //comando lido do terminal
	char* argumento; //argumento lido do terminal
}TipoComando;

TipoComando *leitura();
Mensagem* cria_mensagem_cliente(unsigned char sequencia, TipoComando* tipoComando, unsigned char tipo);
void envia_mensagem_cliente(Mensagem *mensagem, int soquete);
void ls_remoto(TipoComando* tipoComando, int soquete);
void comandos(int soquete);
int cliente();

#endif