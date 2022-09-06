#ifndef __CONEXAO__
#define __CONEXAO__

#define MAX_DADOS 64

typedef struct Mensagem{
	unsigned char marcadorInicio;
	unsigned char tamanho : 6;
	unsigned char sequencia : 4;
	unsigned char tipo : 6;
	unsigned char paridade : 8;
	unsigned char dados[MAX_DADOS];
}Mensagem;

int ConexaoRawSocket(char *device);

#endif