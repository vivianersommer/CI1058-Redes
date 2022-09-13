#ifndef __CLIENTE__
#define __CLIENTE__

#include "conexao.h"
#define MAX 100

typedef struct TipoComando{
	int tipo; //tipo de comando passado LSR, LSL...
	char* comando; //tamanho maximo do comando
	char* argumento; //tamanho maximo do argumento
}TipoComando;

Mensagem *cria_mensagem(unsigned char sequencia, TipoComando* tipoComando);
int habilitar_rede();
TipoComando *leitura();
void comandos();
void desabilitar_rede();
int cliente();

#endif