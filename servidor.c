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
#include <dirent.h>
#include <sys/stat.h>

#include "conexao.h"
#include "servidor.h"


char *le_arquivo(char *nome){

	char *arquivo;
	FILE *file = fopen(nome, "r");

	fseek(file, 0L, SEEK_END);
	long long tamanho = ftell(file);
	rewind(file);

	arquivo = malloc(tamanho * sizeof(char));

	fread(arquivo, 1, tamanho, file);
	fclose(file);

	return arquivo;
}

//comando get envia arquivo que cliente está solicitando
void comando_get(Mensagem *mensagem, int soquete) {

	char *nome_arquivo = malloc(sizeof(char) * (MAX_DADOS + 1));
	char *tamHexa = malloc(sizeof(char) * MAX_DADOS + 1);
	int i = 0, fim = 0, errors = 1;
	unsigned int tam_arq = 0x00;
	unsigned char prox_enviar = 0x00, prox_receber = 0x01;
	FILE *arquivo;
	struct stat fileStat;

	for (i=0; i < mensagem->tamanho; i++) {
		nome_arquivo[i] = mensagem->dados[i];
	}
	
	nome_arquivo[i] = '\0';

	// pega arquivo gerado no LS ---------------------------------------------------------
	char *arquivo_dados; 
	arquivo_dados = le_arquivo(nome_arquivo);
	int tamArquivo = strlen(arquivo_dados);
	// -----------------------------------------------------------------------------------

	// copia os 62 primeiros caracteres do arquivo para a variável -----------------------
	char *buffer = malloc(MAX_DADOS * sizeof(char));
	
	// -----------------------------------------------------------------------------------

	printf("Executou: get %s\n", nome_arquivo);

	arquivo = fopen(nome_arquivo, "r"); //tenta ler o arquivo
	if (arquivo) {
		for (i = 0; i < MAX_DADOS && i < tamArquivo; i++) { 
			buffer[i] = arquivo_dados[i];
		}

		//j=i;
		buffer[i] = '\0'; //adciona um valor de fim no buffer
		fstat(fileno(arquivo), &fileStat);
		tam_arq = fileStat.st_size; //recebe tamanho do arquivo

		if (tam_arq > MAX_ARQUIVO) { //se tamanho for maior que o máximo possível
			mensagem = cria_mensagem(prox_enviar, ERRO, "");
			envia_mensagem(mensagem, soquete);
			prox_enviar = sequencia(prox_enviar);
			errors = 1;
			printf("Tamanho do arquivo maior que o máximo!!\n");

		} else {
			mensagem = cria_mensagem(prox_enviar, DESCRITOR_DE_ARQUIVO, buffer); //cria mensagem como o tamanho do arquivo e envia
			envia_mensagem(mensagem, soquete);
			prox_enviar = sequencia(prox_enviar);
			errors = 0; 
		}

	//se arquivo não existe
	} else { 
		mensagem = cria_mensagem(prox_enviar, ERRO, "");
		envia_mensagem(mensagem, soquete);
		prox_enviar = sequencia(prox_enviar);
		errors = 1;
		printf("Arquivo inexistente\n");
	}

	// se deu erro ao tentar enviar primeira mensagem, ja nem envia as proximas
	if (errors == 1){
		return;
	}

	do {
		int deu_tuco = espera_mensagem(mensagem, soquete);
		printf("TIPO = %d\n", mensagem->tipo);
		if (deu_tuco == 1){	
			if (mensagem->sequencia == prox_receber) {
				if (mensagem->tipo == ACK) {

					fim = 1; //se deu erro é só pra esperar o ack e encerrar
					if (!errors) { //se não deu erro, envia o arquivo
						fim = 1;	
						
					}
				} else if (mensagem->tipo == NACK) {
					prox_receber = sequencia(prox_receber);
					envia_mensagem(mensagem, soquete);
				} else if (mensagem->tipo == ERRO) {
					fim = 1;
					printf("%s\n", E); //tem espaco nao
				}
			} else if (mensagem->sequencia > prox_receber) {
				fim = 1; //sai do while
				printf("%s\n", G); //deu ruim na sequencia
			}
		} else if (deu_tuco == -1) { //se o evento for um timeout
			fim = 1; //sai do while
			printf("Timeout, comando não executado...\n");
		} else if (deu_tuco == 0) {
			mensagem = cria_mensagem(prox_enviar, NACK, "");
			envia_mensagem(mensagem, soquete);
			prox_enviar = sequencia(prox_enviar); //incrementa prox_enviar e prox_receber
			prox_receber = sequencia(prox_receber);
		}
	} while (!fim);

	if (arquivo) {
		fclose(arquivo);
	}
}

void envia_arquivo(char *nome, unsigned char tipo, unsigned char prox_enviar, unsigned char prox_receber, int soquete) {
	
	int i = 0, j = 0, fim = 0, enviouTudo = 0, enviouFim = 0;
	long tamArquivo = 0; 
    int deu_tuco = 0;

	// pega arquivo gerado no LS ---------------------------------------------------------
	char *arquivo; 
	arquivo = le_arquivo(nome);
	tamArquivo = strlen(arquivo);
	// -----------------------------------------------------------------------------------

	// copia os 62 primeiros caracteres do arquivo para a variável -----------------------
	char *buffer = malloc(MAX_DADOS * sizeof(char));
	
	for (i = 0; i < MAX_DADOS && i < tamArquivo; i++) { 
		buffer[i] = arquivo[i];
	}

	j=i;
	buffer[i] = '\0'; //adciona um valor de fim no buffer
	// -----------------------------------------------------------------------------------

	// tratamento de mensagem ------------------------------------------------------------
	Mensagem *mensagem = cria_mensagem(prox_enviar, tipo, buffer);
	envia_mensagem(mensagem, soquete);
	// -----------------------------------------------------------------------------------
    	
    // tratamento da sequencia -----------------------------------------------------------
	prox_enviar = sequencia(prox_enviar);
    // -----------------------------------------------------------------------------------

	do {
		// verifica se mandou todo o arquivo ---------------------------------------------
		if (i < tamArquivo) {
			enviouTudo = 0; //NAO mandou tudo, tamanho do arquivo é maior do que aquilo que é permitido
		} else {
			enviouTudo = 1;
		}
		// -------------------------------------------------------------------------------

		deu_tuco = processo_poll(mensagem, soquete); //espera uma resposta ---------------

		if (deu_tuco == 1) { 
			if (mensagem->sequencia == prox_receber) { //se a sequência ta certa

				//se for um ACK e ainda não enviou todo o arquivo, envia os próximos caracteres
                if (mensagem->tipo == ACK && !enviouTudo) { 
					j = 0;
					while (j < MAX_DADOS && i < tamArquivo) {
						buffer[j++] = arquivo[i++];
					}
					buffer[j] = '\0';

					mensagem = cria_mensagem(prox_enviar, tipo, buffer); 
					envia_mensagem(mensagem, soquete); //envia o que sobrou

					//tratamento de sequencias após envio-----------------------------------------------------------
					prox_enviar = sequencia(prox_enviar);
					prox_receber = sequencia(prox_receber);
					// -------------------------------------------------------------------------------

				} else if (mensagem->tipo == ACK && (enviouTudo == 1) && !enviouFim) { //se for ACK E enviou todo arquivo E não enviou fim de arquivo
					mensagem = cria_mensagem(prox_enviar, FIM_TX, ""); //cria mensagem sinalizando fim do arquivo FIM_TX
					envia_mensagem(mensagem, soquete); //envia mensagem
				
					fim = 1; //sai do while
				
				} else if (mensagem->tipo == ACK && enviouTudo && enviouFim) { //se for ACK E envitou todo arquivo E enviou fim de arquivo				
					fim = 1; //sai do while
				
				} else if (mensagem->tipo == NACK) { //se for do tipo NACK
					envia_mensagem(mensagem, soquete); //envia a mesma mensagem novamente
					
					//tratamento de sequencias após envio-----------------------------------------------------------
					prox_enviar = sequencia(prox_enviar);
					// -------------------------------------------------------------------------------
				} 
				
			} else if (mensagem->sequencia > prox_receber) { //se a sequência for maior do que a esperada
				printf("%s\n", G); //problema na sequencia
				
				fim = 1; //sai do while
			}

		// ERRO ----------------------------------------------------------------------------------
		} else if (deu_tuco == -1) { //se o evento for um timeout
			printf("Tivemos um timeout, indo embora!!\n");
			fim = 1; //sai do while
        // ---------------------------------------------------------------------------------------

        //NACK -----------------------------------------------------------------------------------
		} else if (deu_tuco == 0) { //se rolou algum erro			
			cria_mensagem(prox_enviar, NACK, ""); // crio NHAQUE
			envia_mensagem(mensagem, soquete); // manda famoso NHAQUE
			
            //tratamento de sequencias após envio-----------------------------------------------------------
            prox_enviar = sequencia(prox_enviar);
			prox_receber = sequencia(prox_receber);
            // -------------------------------------------------------------------------------
		}
		// ---------------------------------------------------------------------------------------
	} while(!fim); //sai do while quando enviar todo o arquivo ou quando der algum erro sinistro
}

void comando_mkdir(Mensagem *mensagem, int soquete){
	
	int fim = 0;
	unsigned char prox_receber = 1;
	struct stat st = {0};
	char buffer[100] = "mkdir ";
	strcat(buffer, mensagem->dados);

    if (stat(mensagem->dados, &st) == -1) {
        mensagem = cria_mensagem(0, OK, ""); //se recebi um nome de diretorio valido crio um OK
		printf("%s\n", buffer);
		
		system(buffer);
		system("ls");
    } else {
		printf("%s\n", C);
        fflush(stdin);
        fflush(stdout);
    }

	envia_mensagem(mensagem, soquete); //envio resultado
	
	do {
		int deu_tuco = espera_mensagem(mensagem, soquete);
		if (deu_tuco == 1) {
			if (mensagem->sequencia == prox_receber) {
				if (mensagem->tipo == ACK) {
					fim = 1;
				} else if (mensagem->tipo == NACK) {
					prox_receber = sequencia(prox_receber);
					envia_mensagem(mensagem, soquete);
				}
			} else if (mensagem->sequencia > prox_receber) {
				fim = 1; //sai do while
				printf("%s\n", G);
			}
		} else if (deu_tuco == 0) { //se o evento for um timeout
			fim = 1; //sai do while
			printf("Timeout, comando não executado...\n");
		}
	} while (!fim);
}


//cd remoto
void comando_cd(Mensagem* mensagem, int soquete){

	int i = 0, fim = 0;
	char *nome_arquivo = malloc(sizeof(char) * (MAX_DADOS));
	unsigned char prox_receber = 1;

	// copia o nome do diretório a ser criado ---------------------------------------------
	for(i = 0; i < mensagem->tamanho; i++) {
		nome_arquivo[i] = mensagem->dados[i];
	}
	// ------------------------------------------------------------------------------------

	// ve se tem permissão para acessar o diretório ---------------------------------------
	DIR *diretorio = opendir(nome_arquivo);	
	struct stat st = {0};
	stat(nome_arquivo, &st);
	// ------------------------------------------------------------------------------------

	// roda o CD se tiver permissão e se não der erro ao rodar o comando ------------------
	if (diretorio >= 0) {
		printf("Executando: cdr %s\n", nome_arquivo);
		int entraste = chdir(nome_arquivo);
		if (entraste >= 0) {
			char *diretorio_atual = getcwd(0,0);
			printf("Diretório atual após CD: %s!!\n", diretorio_atual);
			mensagem = cria_mensagem(0x00, OK, diretorio_atual); //se recebi o diretorio crio um OK
		} else {
			mensagem = cria_mensagem(0x00, ERRO, A);
			printf("Diretório: %s não encontrado!!\n", nome_arquivo);
		}
	} else {
		mensagem = cria_mensagem(0x00, ERRO, B);
		printf("%s\n", B);
		
	}
	// ------------------------------------------------------------------------------------

	// envio do resultado -----------------------------------------------------------------
	envia_mensagem(mensagem, soquete); //envio resultado
	// ------------------------------------------------------------------------------------

	// espera cliente dizer que recebeu resposta ------------------------------------------
	do {
		int deu_tuco = espera_mensagem(mensagem, soquete);
		if (deu_tuco == 1) {
			if (mensagem->sequencia == prox_receber) {
				if (mensagem->tipo == ACK) {
					fim = 1;
				} else if (mensagem->tipo == NACK) {
					prox_receber = sequencia(prox_receber);
					envia_mensagem(mensagem, soquete);
				}
			} else if (mensagem->sequencia > prox_receber) {
				fim = 1; //sai do while
			}
		} else { //se o evento for um timeout
			fim = 1; //sai do while
		}
	} while (!fim);
	// ------------------------------------------------------------------------------------

	free(nome_arquivo);
	free(diretorio);
}

//ls remoto
void comando_ls(Mensagem *mensagem, int soquete){
    
	//tipos de ls
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
				 case CD: //cd
				 	comando_cd(mensagem, soquete);
				 	break;
				 case GET: //get
				 	comando_get(mensagem, soquete);
				 	break;
				// case PUT: //put
				// 	comando_put(mensagem, soquete);
				// 	break;
				 case MKDIR: //mkdir remoto
				 	comando_mkdir(mensagem, soquete);
				 	break;
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