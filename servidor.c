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
#include <netinet/in.h>
#include <unistd.h>
#include "conexao.h"
#include "servidor.h"


char *le_arquivo(char *nome){

	char *arquivo;
	FILE *file = fopen(nome, "r");

	fseek(file, 0L, SEEK_END);
	long long tamanho = ftell(file);
	rewind(file);

	arquivo = malloc(tamanho * sizeof(char)); //mexiaki

	fread(arquivo, 1, tamanho, file);
	fclose(file);

	return arquivo;
}

void envia_arquivo(char *nome, unsigned char tipo, unsigned char prox_enviar, unsigned char prox_receber, int soquete) {
	
	int i = 0, j = 0, fim = 0, enviouTudo = 0, enviouFim = 0;
	long tamArquivo = 0; 
    int deu_tuco = 0;

	// pega arquivo gerado no LS ---------------------------------------------------------
	char *arquivo; 
	arquivo = le_arquivo(nome); //ok aqui, esta retornando certo
	tamArquivo = strlen(arquivo);
	// -----------------------------------------------------------------------------------

	// copia os 62 primeiros caracteres do arquivo para a variável -----------------------
	char *buffer = malloc(MAX_DADOS * sizeof(char));
	
	int w;
	for (w = 0; w < MAX_DADOS && w < tamArquivo; w++) { 
		buffer[w] = arquivo[w];
	}

	i=w;
	j=w;
	buffer[w] = '\0';
	// -----------------------------------------------------------------------------------

	// tratamento de mensagem ------------------------------------------------------------
	Mensagem *mensagem = cria_mensagem(prox_enviar, tipo, buffer);
	envia_mensagem(mensagem, soquete);
	// -----------------------------------------------------------------------------------
    	
    // tratamento da sequencia -----------------------------------------------------------
	prox_enviar = sequencia(prox_enviar);
    // -----------------------------------------------------------------------------------

	printf("SERVIDOR: prox_enviar = %hhu e prox_receber %hhu\n", prox_enviar, prox_receber);
	do {
		// verifica se mandou todo o arquivo ---------------------------------------------
		if (i < tamArquivo) {
			enviouTudo = 0; //NAO mandou tudo, tamanho do arquivo é maior do que aquilo que é permitido
		} else {
			enviouTudo = 1;
		}
		// -------------------------------------------------------------------------------

		deu_tuco = processo_poll(mensagem, soquete); //espera uma resposta ---------------

		printf("Recebi uma mensagem do tipo %hhu\n", mensagem->tipo);
		if (deu_tuco == 1) { //se recebeu mensagem
			if (mensagem->sequencia == prox_receber) { //se a sequência ta certa
                if (mensagem->tipo == ACK && !enviouTudo) { //se for um ACK e ainda não enviou todo o arquivo, envia os próximos caracteres
					
					printf("Sequencia da mensagem:%hhu  Sequencia do prox_receber:%hhu \n\n", mensagem->sequencia, prox_receber);

					j = 0;
					while (j < MAX_DADOS && i < tamArquivo) {
						buffer[j++] = arquivo[i++];
					}
					buffer[j] = '\0';

					mensagem = cria_mensagem(prox_enviar, tipo, buffer); //cria mensagem do tipo EXIBE //AKI NGM TAVA RECEBENDO O QUE TAVA VOLTANDO
					printf("-SERVIDOR\nDADOS RESTANTES:\n");
					for(int i=0; i<mensagem->tamanho; i++){
						printf("%c", mensagem->dados[i]);
					}
					printf("\n");
					printf("prox_enviar = %d\n", prox_enviar);
					printf("prox_receber = %d\n", prox_receber);

					envia_mensagem(mensagem, soquete); //envia

					//tratamento de sequencias após envio-----------------------------------------------------------
					prox_enviar = sequencia(prox_enviar);
					prox_receber = sequencia(prox_receber);
					// -------------------------------------------------------------------------------

				} else if (mensagem->tipo == ACK && (enviouTudo == 1) && !enviouFim) { //se for ACK E enviou todo arquivo E não enviou fim de arquivo
					printf("entrei no caso de fim\n");
					cria_mensagem(prox_enviar, FIM_TX, ""); //cria mensagem sinalizando fim do arquivo FIM_TX
					envia_mensagem(mensagem, soquete); //envia mensagem
					fim = 1;
				} else if (mensagem->tipo == ACK && enviouTudo && enviouFim) { //se for ACK E envitou todo arquivo E enviou fim de arquivo
					printf("entrei no caso de ack\n");
					fim = 1;
				} else if (mensagem->tipo == NACK) { //se for do tipo NACK
					printf("entrei no caso de nack\n");
					envia_mensagem(mensagem, soquete); //envia a mesma mensagem novamente
					
					//tratamento de sequencias após envio-----------------------------------------------------------
					prox_enviar = sequencia(prox_enviar);
					// -------------------------------------------------------------------------------
				} else {
					printf("Cai no tipo %hhu, ignorando ...\n", mensagem->tipo);
				}
			} else if (mensagem->sequencia > prox_receber) { //se a sequência for maior do que a esperada
				fim = 1; //sai do while
				printf("Mensagem com sequência maior do que a esperada, encerrando a transmissão...\n seq: %d e esperava: %d\n", mensagem->sequencia, prox_receber);
			}

		// ERRO ----------------------------------------------------------------------------------
		} else if (deu_tuco == -1) { //se o evento for um timeout
			printf("Tivemos um timeout, indo embora!!\n");
			fim = 1; //sai do while
        // ---------------------------------------------------------------------------------------

        //NACK -----------------------------------------------------------------------------------
		} else if (deu_tuco == 0) { //se rolou algum erro
			printf("entrei no caso de nack 2\n");
			
			cria_mensagem(prox_enviar, NACK, ""); // crio NHAQUE
			envia_mensagem(mensagem, soquete); // manda famoso NHAQUE
			
            //tratamento de sequencias após envio-----------------------------------------------------------
            prox_enviar = sequencia(prox_enviar);
			prox_receber = sequencia(prox_receber);
            // -------------------------------------------------------------------------------
		}
		// ---------------------------------------------------------------------------------------
        printf("NAO TERMINEI\n");
	} while(!fim);
	//sai do while quando enviar todo o arquivo (ou nao)
    printf("TERMINEI\n");
}

void comando_ls(Mensagem *mensagem, int soquete){
    
	if (!strcmp(mensagem->dados, "-a")) {
        printf("Executando comando: ls -a\n");
		system("ls -a > .comandoLS");
	} else if (!strcmp(mensagem->dados, "-l")) {
        printf("Executando comando: ls -l\n");
		system("ls -l > .comandoLS");
	} else {
        printf("Executando comando: ls\n");
		system("ls > .comandoLS");
	}

	envia_arquivo(".comandoLS", MOSTRA_TELA, 0x00, 0x01, soquete); //envia o arquivo com a saída do ls
	system("rm .comandoLS"); //remove o arquivo temporário
}

void roda_servidor(int soquete){
    Mensagem *mensagem = malloc(sizeof(Mensagem));

    do {
        int resultado_espera_mensagem = espera_mensagem(mensagem, soquete);
		if (mensagem->sequencia == 0x00 && resultado_espera_mensagem == 1) {
			switch (mensagem->tipo) {
				case LS: //ls
					comando_ls(mensagem, soquete);
					break;
				// case 2: //cd
				// 	comando_cd(mensagem);
				// 	break;
				// case 4: //get
				// 	comando_get(mensagem);
				// 	break;
				// case 5: //put
				// 	comando_put(mensagem);
				// 	break;
				// case 6: //mkdir remoto
				// 	comando_mkdir(mensagem);
				// 	break;
				default:
					break;
			}
		}
	} while (1);
}

int servidor() {
	int soquete = habilitar_rede();
	roda_servidor(soquete);
	desabilitar_rede(soquete);
	return 0;
}