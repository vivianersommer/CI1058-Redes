#ifndef __SERVIDOR__
#define __SERVIDOR__

#include "conexao.h"

//void envia_mensagem_servidor(Mensagem *mensagem, int soquete);
//int espera_mensagem_servidor(Mensagem *mensagem, int soquete);
char *le_arquivo(char *nome);
void envia_arquivo(char *nome, unsigned char tipo, unsigned char prox_enviar, unsigned char prox_receber, int soquete);
void comando_ls(Mensagem *mensagem, int soquete);
void roda_servidor(int soquete);
int servidor();
#endif