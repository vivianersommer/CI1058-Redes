#ifndef __CONEXAO__
#define __CONEX


typedef struct Mensagem{
   int  marcadorInicio : 8;
   int  tamanho:6;
   int  sequencia:4;
   int  tipo:6;
   char *dados;
   int  paridade:8;  
}Mensagem;

int ConexaoRawSocket(char *device);

#endif