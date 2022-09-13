#ifndef __CLIENTE__
#define __CLIENTE__

#define MAX 100

typedef struct TipoComando{
	int tipo; //tipo de comando passado LSR, LSL...
	char* comando; //tamanho maximo do comando
	char* argumento; //tamanho maximo do argumento
}TipoComando;

struct Mensagem *cria_mensagem();
int habilitar_rede();
TipoComando *leitura();
void comandos();
void desabilitar_rede();
int cliente();

#endif