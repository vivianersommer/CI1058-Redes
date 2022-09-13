#ifndef __CLIENTE__
#define __CLIENTE__

#include "conexao.h"
#define MAX 100

typedef struct TipoComando{
	int tipo; //tipo de comando passado LSR, LSL...
	char* comando; //comando lido do terminal
	char* argumento; //argumento lido do terminal
}TipoComando;

/* int tipo
    LSR - 0
    LSL - 1
    CDR - 2
    CDL - 3
    GET - 4
    PUT - 5
    MKDIRR - 6
    MKDIRL - 7
*/

int habilitar_rede(); 
void desabilitar_rede();
TipoComando *leitura();
Mensagem *cria_mensagem(unsigned char sequencia, TipoComando* tipoComando);
void envia_mensagem(Mensagem *mensagem, int soquete);
void ls_remoto(TipoComando* tipoComando, int soquete);
void comandos(int soquete);
int cliente();

#endif