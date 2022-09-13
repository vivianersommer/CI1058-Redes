#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "conexao.h"
#include "cliente.h"

    // int escrito = send(soquete, mensagem, sizeof(struct Mensagem), 0); //que isso? escrito?
    // while (escrito == -1 && timeout.tv_usec != 0){
    //     printf("Mensagem não enviada!\n");
    //     printf("Erro = %d\n", escrito);
    //     printf("TENTANDO NOVAMENTE!\n");
    //     escrito = send(soquete, mensagem, sizeof(struct Mensagem), 0);
    // } 
    // if(timeout.tv_usec == 0){
    //     printf("TIMEOUT !!!\n");
    //     exit(1);
    // }
    // printf("Mensagem enviada com sucesso!\n");
    // printf("Mensagem  = \n%s \n%c\n", mensagem->dados, mensagem->marcadorInicio);

int habilitar_rede(){

    // system("ifconfig eth0 promisc");
    int soquete = ConexaoRawSocket("lo"); // TODO: quando for para o cabo, usar eth0

    return soquete;
}

void desabilitar_rede(int soquete){

    close(soquete);
	// system("ifconfig eth0 -promisc");
}

// leitura dos comandos a partir de um "falso" terminal
TipoComando *leitura(){ 

    TipoComando* tipoComando = malloc(sizeof(TipoComando)); //criação de tipoComando
    tipoComando->tipo = -1; //inicia em -1 (inválido)
    tipoComando->argumento = malloc(sizeof(char) * MAX);
    tipoComando->comando = malloc(sizeof(char) * MAX);

    char* linha_terminal = malloc(sizeof(char) * MAX);
    char * diretorio_atual = malloc(sizeof(char) * MAX);

    diretorio_atual = getcwd(0, 0); //obtem diretorio atual
    strcat(diretorio_atual, "$ ");
    printf("%s", diretorio_atual);

    fflush(stdin); //limpa entrada padrao

    fgets(linha_terminal, (MAX * 2) + 1, stdin); //linha a ser lida do terminal, +1 pro espaco vazio
    sscanf(linha_terminal, "%s %s", tipoComando->comando, tipoComando->argumento);

	if (strlen(tipoComando->comando) > 0) {
		if (strlen(tipoComando->comando) > MAX || strlen(tipoComando->argumento) > MAX) {
			printf("Por favor, o tamanho do comando e seu argumento devem ter no máximo tamanho %d!!\n" , MAX);
            puts("Tente novamente!!");
		} else {
                if (strcmp(tipoComando->comando, "lsr") == 0){
                    tipoComando->tipo = 0; //ls remoto
                } else if (strcmp(tipoComando->comando, "lsl") == 0) {
                    tipoComando->tipo = 1; //ls local
                } else if (strcmp(tipoComando->comando, "cdr") == 0) {
                    tipoComando->tipo = 2; //cd remoto
                } else if (strcmp(tipoComando->comando, "cdl") == 0) {
                    tipoComando->tipo = 3; //cd local
                } else if (strcmp(tipoComando->comando, "get") == 0) {
                    tipoComando->tipo = 4; //get
                } else if (strcmp(tipoComando->comando, "put") == 0) {
                    tipoComando->tipo = 5; //put
                } else if (strcmp(tipoComando->comando, "mkdirr") == 0){
                    tipoComando->tipo = 6; //mkdir remoto
                } else if (strcmp(tipoComando->comando, "mkdirl") == 0){
                    tipoComando->tipo = 7; //mkdir local
                } else{
                    puts("Comando inválido!!");
                    puts("Tente novamente!!");
                }
            }
	}

    free(diretorio_atual);
    free(linha_terminal);

    return tipoComando;
}

Mensagem* cria_mensagem(unsigned char sequencia, TipoComando* tipoComando) {

    Mensagem *mensagem = malloc(sizeof(Mensagem));
	mensagem->marcadorInicio =  0x7E; // 0x7E = 01111110
	mensagem->tamanho = strlen(tipoComando->argumento);
	mensagem->sequencia = sequencia;
	mensagem->tipo = tipoComando->tipo;
	mensagem->paridade = paridade(tipoComando->argumento, mensagem->tamanho); 

	for (int i = 0; i < MAX_DADOS; i++) {
		mensagem->dados[i] = tipoComando->argumento[i];
	}

    return mensagem;
}

void ls_remoto(TipoComando* tipoComando) {

	Mensagem *mensagem = cria_mensagem('0', tipoComando); //cria mensagem do tipo lsr
	//enviar_mensagem(mensagem); //envia mensagem do lsr
	//recebe_arquivo(mensagem);
}

void comandos(){

	int acaba = 0;
	TipoComando* tipocomando = malloc(sizeof(TipoComando));

	do {
		tipocomando = leitura();

		 switch (tipocomando->tipo) {
             case 0: //LS Remoto 
                ls_remoto(tipocomando);
                break;
        //     case 1: //LS Local
        //         comando_ls_local(comando);
        //         break;
        //     case 2: //CD Remoto
        //         comando_cd_remoto(comando->argumento);
        //         break;
        //     case 3: //CD Local
        //         comando_cd_local(comando->argumento);
        //         break;
        //     case 4: //GET
        //         comando_get(comando->argumento);
        //         break;
        //     case 5: //PUT
        //         comando_put(comando->argumento);
        //         break;
        //     case 6: //MKDIR Remoto
        //         comando_cat(comando->argumento);
        //         break;
        //     case 7: //MKDIR Local
        //         comando_cat(comando->argumento);
        //         break;
            default:
                break;
         }

	} while (!acaba);    
}

int cliente(){

    int soquete = habilitar_rede();
    comandos();
    desabilitar_rede(soquete);
    
    return 0;
} 