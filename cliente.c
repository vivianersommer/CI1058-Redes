#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "conexao.h"
#include "cliente.h"

// leitura dos comandos a partir de um "falso" terminal
void leitura(TipoComando* tipoComando){

    char *linha_terminal = malloc(sizeof(char) * MAX);

    char *diretorio_atual = getcwd(0, 0);
	strcat(diretorio_atual, "$ ");
	printf("%s", diretorio_atual);
    fflush(stdin);
    fflush(stdout); //limpa entrada padrao

    fgets(linha_terminal, MAX, stdin); //linha a ser lida do terminal
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
                tipoComando->tipo = -1;
                puts("Comando inválido!!");
                puts("Tente novamente!!");
            }
        }
	}
    free(linha_terminal);
}

void recebe_resposta_ls(Mensagem *mensagem, int soquete){
    do {
        recv(soquete, mensagem, sizeof(struct Mensagem), 0); // leitura do soquete
        if ((mensagem->marcadorInicio == 0x7E) && (paridade(mensagem->dados, mensagem->tamanho) == mensagem->paridade)) {
            if (mensagem->tipo == OK) { //se servidor devolveu um OK, está tudo certo
                break;
            } else if (mensagem->tipo == ERRO){ // se servidor devolveu um ERRO, pede pro usuário redigitar o comando
                printf("Erro: ");
                for(int i = 0; i<mensagem->tamanho; i++){
                    printf("%c", mensagem->dados[i]);
                }
                printf("\n");
                printf("Por favor, digite o comando novamente!!\n");
                break;
            }
        }
    }while(1);
}

void recebe_arquivo(Mensagem *mensagem, char *nome, unsigned char tipo, unsigned char prox_enviar, unsigned char prox_receber, int soquete) {
	
    int fim = 0;
	FILE *arquivo;

	if (tipo == DADOS) {
		arquivo = fopen(nome, "w"); 
		// chmod(nome, (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)); //seta as permissões
	}

	do {
		int deu_tuco = espera_mensagem(mensagem, soquete); //espera uma resposta
		if (deu_tuco == 1) {
			if (mensagem->sequencia == prox_receber) { //se a sequência for a esperada
				if ((mensagem->tipo == MOSTRA_TELA && tipo == MOSTRA_TELA) || (mensagem->tipo == DADOS && tipo == DADOS)) {
					if (tipo == MOSTRA_TELA) { 
						fwrite(mensagem->dados, mensagem->tamanho, 1, stdout); //escreve os dados na tela
					} else { 
						fwrite(mensagem->dados, mensagem->tamanho, 1, arquivo); //escreve os dados no arquivo
					}
					mensagem = cria_mensagem(prox_enviar, ACK, ""); //cria um ACK
                    envia_mensagem(mensagem, soquete); //envia o ACK
					
                    //tratamento de sequencias após envio-----------------------------------------------------------
                    prox_enviar = sequencia(prox_enviar);
                    prox_receber = sequencia(prox_receber);
                    // -------------------------------------------------------------------------------

				} else if (mensagem->tipo == FIM_TX) {
					mensagem = cria_mensagem(prox_enviar, ACK, "");
					envia_mensagem(mensagem, soquete);
                    fim = 1;
				} else if (mensagem->tipo == NACK) { //se for do tipo NACK
                    printf("entrei no caso de nack\n");
					envia_mensagem(mensagem, soquete); //envia a mesma mensagem novamente
					//tratamento de sequencia --------------------------------------------------------
                    if (prox_receber < MAX_SEQ) {
                        prox_receber++;
                    } else {
                        prox_receber = 0x00; //se passar do limite zeramos o contador
                    }
                    // -------------------------------------------------------------------------------
				} else if (mensagem->tipo == ERRO) {
                    printf("entrei no caso de erro\n");
					mensagem = cria_mensagem(prox_enviar, ACK, "");
					envia_mensagem(mensagem, soquete);
					fim = 1;
					printf("Erro: %s\n", mensagem->dados);
				}
			} else if (mensagem->sequencia != prox_receber) {
                fim = 0;
				// printf("Erro: %s\n", G);
                // fim = 1; 
			}
		} else if (deu_tuco == 2) { //TODO: NUNCA RETORNA TIMEOUT
			printf("Timeout!!\n");
            printf("Por favor, digite o comando novamente!!\n");
            fim = 1; 
		} else if (deu_tuco == 1) {
             printf("entrei no caso de nack 2\n");
			mensagem = cria_mensagem(prox_enviar, NACK, "");
			envia_mensagem(mensagem, soquete);
            //tratamento de sequencias -----------------------------------------------------------
            prox_enviar = sequencia(prox_enviar);
            prox_receber = sequencia(prox_receber);
            // -------------------------------------------------------------------------------

		}
	} while (!fim);

	if (tipo == DADOS) {
		fclose(arquivo);
	}
}

void recebe_resposta_cd(Mensagem* mensagem, int soquete) {
	unsigned char prox_receber = 0;
	int fim = 0;

	//cria_mensagem(&m, 0, CDR, argumento); //cria solicitação do cd
	//envia_mensagem(&m); //envia a mensagem

	do {
		int deu tuco = espera_mensagem(mensagem, soquete);
		if (deu_tuco == 1) {
			if (mensagem->sequencia == prox_receber) {
				if (mensagem->tipo == OK || mensagem->tipo == ERRO) {
					fim = 1;
					mensagem = cria_mensagem(mensagem, 1, ACK, "");
					envia_mensagem(mensagem);
				} else if (mensagem->tipo == NACK) {
					prox_receber = sequencia(prox_receber);
					envia_mensagem(mensagem);
				}
			} else if (mensagem->sequencia > prox_receber) {
                printf("-CD CAI NA CONDIÇÃO DE SEQUENCIA\n");
				fim = 1; //sai do while
				printf("Mensagem com sequência maior do que a esperada, comando não executado...\n");
			}
		} else if (deu_tuco == 0) { //se o evento for um timeout
			printf("-CD CAI NA CONDIÇÃO DE TIMEOUT\n");
            fim = 1; //sai do while
			printf("Timeout, comando não executado...\n");
		}
	} while (!fim);

	if (mensagem->tipo == ERRO) {
		printf("Erro ao executar comando: ");
		exibe_erro(mensagem);
		//fwrite(r.dados, r.tamanho, 1, stdout); //escreve os dados na tela
		//printf("\n");
	} else if (mensagem->tipo == ACK) {
		printf("Comando executado com sucesso.\n");
	}
}

void ls_remoto(TipoComando* tipoComando, int soquete) {

	Mensagem *mensagem = cria_mensagem(0x00, LS, tipoComando->argumento); //cria mensagem do tipo lsr
	envia_mensagem(mensagem, soquete); //envia mensagem do lsr
    // recebe_resposta_ls(mensagem, soquete);
	recebe_arquivo(mensagem, "", MOSTRA_TELA, 0x01, 0x00, soquete);
}

void ls_local(TipoComando* tipoComando) {
    if (!strcmp(tipoComando->argumento, "-l")) {
		system("ls -l");
	} else if (!strcmp(tipoComando->argumento, "-a")) {
		system("ls -a");
	} else {
        system("ls");
    }
}

void cd_remoto(TipoComando* tipoComando, int soquete){
    Mensagem *mensagem = cria_mensagem(0x00, CD, tipoComando->argumento); //cria mensagem do tipo CD
    envia_mensagem(mensagem, soquete); //envia mensagem da requisicao CD
    recebe_resposta_cd(mensagem, soquete);
}

void cd_local(TipoComando* tipoComando){
    chdir(tipoComando->argumento);
}

void mkdir_local(TipoComando* tipoComando){
    struct stat st = {0};

    if (stat(tipoComando->argumento, &st) == -1) {
        mkdir(tipoComando->argumento, 0700);
    } else {
        printf("Erro: esse diretório já existe!!\n");
        fflush(stdin);
        fflush(stdout);
    }
}

void comandos(int soquete){

	int acaba = 0;
    TipoComando* tipoComando = malloc(sizeof(TipoComando)); //criação de tipoComando
    tipoComando->tipo = -1; //inicia em -1 (inválido)
    tipoComando->argumento = malloc(sizeof(char) * MAX);
    tipoComando->comando = malloc(sizeof(char) * MAX);
	do {
		leitura(tipoComando);
		switch (tipoComando->tipo) {
            case 0: //LS Remoto 
                ls_remoto(tipoComando, soquete);
                break;
            case 1: //LS Local
                ls_local(tipoComando);
                break;
            case 2: //CD Remoto
                 cd_remoto(comando->argumento);
                 break;
            case 3: //CD Local
                cd_local(tipoComando);
                break;
        //     case 4: //GET
        //         comando_get(comando->argumento);
        //         break;
        //     case 5: //PUT
        //         comando_put(comando->argumento);
        //         break;
        //     case 6: //MKDIR Remoto
        //         comando_cat(comando->argumento);
        //         break;
            case 7: //MKDIR Local
                mkdir_local(tipoComando);
                break;
            default:
                break;
         }
	} while (!acaba);    

    free(tipoComando);
}

int cliente(){

    int soquete = habilitar_rede();
    comandos(soquete);
    desabilitar_rede(soquete);

    return 0;
} 