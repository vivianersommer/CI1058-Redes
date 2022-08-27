#ifndef __CONEXAO__
#define __CONEX

#define MAX_DADOS 200

typedef struct Mensagem{
	unsigned char marcadorInicio;
	unsigned char tamanho : 4;
	unsigned char sequencia : 4;
	unsigned char tipo : 4;
	unsigned char paridade : 4;
	unsigned char dados[MAX_DADOS];
}Mensagem;

int ConexaoRawSocket(char *device);

#endif