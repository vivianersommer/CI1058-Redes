#ifndef __CONEXAO__
#define __CONEXAO__

#define MAX_DADOS 62 // (2^6-1)
#define MAX_SEQ 15
#define MAX_TENTATIVAS 10

//TIPOS:
#define CD 0x06
#define OK 0x01
#define NACK 0x02
#define ERRO 0x011
#define LS 0x07
#define MOSTRA_TELA 0x03f
#define ACK 0x03
#define FIM_TX 0x02e
#define MKDIR 0x08
#define GET 0x09
#define DESCRITOR_DE_ARQUIVO 0x018
#define DADOS 0x020
#define PUT 0x0a

//ERROS
#define A "Diretorio nao existe"
#define B "Sem permissão"
#define C "Diretorio ja existe"
#define D "Arquivo ja existe"
#define E "Sem espaço"
#define F "Erro na paridade"

typedef struct Mensagem{
	unsigned char marcadorInicio;
	unsigned char tamanho : 6;
	unsigned char sequencia : 4;
	unsigned char tipo : 6;
	unsigned char paridade : 8;
	unsigned char dados[MAX_DADOS];
}Mensagem;

int habilitar_rede(); 
void desabilitar_rede();
int ConexaoRawSocket(char *device);
unsigned char paridade(unsigned char* dados, unsigned char tamanho);

#endif