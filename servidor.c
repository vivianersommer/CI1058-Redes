#include <poll.h>
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

	arquivo = (char *) malloc(tamanho);

	fread(arquivo, 1, tamanho, file);
	fclose(file);

	return arquivo;
}

Mensagem *cria_mensagem_servidor(unsigned char sequencia, unsigned char tipo, char *dados) {
	
    Mensagem *mensagem = malloc(sizeof(Mensagem));
	mensagem->marcadorInicio = 0x7E; //01111110
	mensagem->tamanho = strlen(dados);
	mensagem->sequencia = sequencia;
	mensagem->tipo = tipo;
	mensagem->paridade = paridade(dados, mensagem->tamanho);

	for (int i = 0; i < MAX_DADOS; i++) {
		mensagem->dados[i] = dados[i];
	}

	return mensagem;
}

void envia_mensagem_servidor(Mensagem *mensagem, int soquete) {
    int result_enviar = send(soquete, mensagem, sizeof(struct Mensagem), 0);
    printf("\nRESULTADO DO SEND, QUANTOS BYTES ELE ENVIOU: %i\n", result_enviar);
}

int processo_poll(Mensagem *mensagem, int soquete) {
	int tenta_enviar = 0, fim = 0, poll_resultado = 0, deu_tuco = 0;
	unsigned char resultado_paridade , *buffer = malloc(sizeof(char) * 21);
	struct pollfd *fds;	
	do {
		poll_resultado = poll(fds, 1, 1000); //espera por algum evento vindo da descricao do arquivo
		//printf("poll_resultado = %d\n", poll_resultado);
		switch (poll_resultado) {
			case -1:
				printf("Erro na função poll, socorro!!\n");
				printf("Ocorreu um erro que não sabemos lidar, desculpa!!\n");
				exit(-1);
			case 0:
				puts("OI GATA");
				tenta_enviar++;
				envia_mensagem_servidor(mensagem, soquete); //tenta enviar
				if (tenta_enviar >= 2) {
					deu_tuco = -1;
					fim = 1; //c h e g a, deu timeout
                    printf("ENTREI  \n");
				}
				break;
			default:
				if (fds->revents == POLLIN) { //tem coisa pra le, corre
					int result_receber = recv(soquete, buffer, MAX_DADOS, 0);
					fsync(soquete); //?
					if (mensagem->dados[0] == 0x7E) {

						//------- refaz a mensagem ---------------------------------------------------------
						mensagem->marcadorInicio = buffer[0];
						mensagem->tamanho = (buffer[1] << 4) >> 4;
						mensagem->sequencia = buffer[1] >> 4;
						mensagem->tipo = (buffer[2] << 4) >> 4;
						mensagem->paridade = buffer[2] >> 4;
						for (poll_resultado = 0; poll_resultado < MAX_DADOS; poll_resultado++) {
							mensagem->dados[poll_resultado] = buffer[poll_resultado + 3];
						}
						//----------------------------------------------------------------------------------

						fim = 1;
						resultado_paridade = paridade(mensagem->dados, mensagem->tamanho); //faz a paridade pra ver se da tudo certo
						if (resultado_paridade == mensagem->paridade) {
                            deu_tuco = 1; //deu tuco, RECEBEU
                        } else {
                            deu_tuco = 0; //NACK
						}
					} else {
                        deu_tuco = 0; //NACK
					}
				}
				break;
			}
	} while (fim == 0);

    return deu_tuco;
}

void envia_arquivo(char *nome, unsigned char tipo, unsigned char prox_enviar, unsigned char prox_receber, int soquete) {
	
	char *buffer = malloc(MAX_DADOS * sizeof(char));
	int i = 0, j = 0, fim = 0, enviouTudo = 0, enviouFim = 0;
	long tamArquivo = 0; 
    int deu_tuco = 0;

	// pega arquivo gerado no LS ---------------------------------------------------------
	char *arquivo = le_arquivo(nome);
	tamArquivo = strlen(arquivo);
	// -----------------------------------------------------------------------------------

	// copia os 62 primeiros caracteres do arquivo para a variável -----------------------
	for (int w = 0; w < MAX_DADOS && w < tamArquivo; w++) {
		buffer[j++] = arquivo[w];
        i = w;
	}
	buffer[j] = '\0';
	// -----------------------------------------------------------------------------------

	// tratamento de mensagem ------------------------------------------------------------
	Mensagem *mensagem = cria_mensagem_servidor(prox_enviar, tipo, buffer);
	envia_mensagem_servidor(mensagem, soquete);
	// -----------------------------------------------------------------------------------
    
    // tratamento da sequencia -----------------------------------------------------------
	if (prox_enviar < MAX_SEQ) {
		prox_enviar++;
	} else {
		prox_enviar = 0x00; //se passar do limite zeramos o contador
	}
    // -----------------------------------------------------------------------------------

	do {
        //ACHO QUE ISSO AQUI NAO PRESTA PRA NADA
		// verifica se mandou todo o arquivo ---------------------------------------------
		if (i >= tamArquivo) {
			enviouTudo = 1; //mandou
		}
		// -------------------------------------------------------------------------------

		deu_tuco = processo_poll(mensagem, soquete); //espera uma resposta ---------------
		if (deu_tuco == 1) { //se recebeu mensagem
			if (mensagem->sequencia == prox_receber) { //se a sequência ta certa
				printf("if da mensagem recebida\n");
                if (mensagem->tipo == ACK && !enviouTudo) { //se for um ACK e ainda não enviou todo o arquivo, envia os próximos caracteres
					j = 0;
					while (j < MAX_DADOS && i < tamArquivo) {
						buffer[j++] = arquivo[i++];
					}
					buffer[j] = '\0';

					cria_mensagem_servidor(prox_enviar, tipo, buffer); //cria mensagem do tipo EXIBE
					envia_mensagem_servidor(mensagem, soquete); //envia
					//tratamento de sequencias -----------------------------------------------------------
					if (prox_enviar < MAX_SEQ) {
						prox_enviar++;
					} else {
						prox_enviar = 0x00; //se passar do limite zeramos o contador
					}

					if (prox_receber < MAX_SEQ) {
						prox_receber++;
					} else {
						prox_receber = 0x00; //se passar do limite zeramos o contador
					}
					// -------------------------------------------------------------------------------
				} else if (mensagem->tipo == ACK && enviouTudo && !enviouFim) { //se for ACK E enviou todo arquivo E não enviou fim de arquivo
					cria_mensagem_servidor(prox_enviar, FIM_TX, ""); //cria mensagem sinalizando fim do arquivo FIM_TX
					envia_mensagem_servidor(mensagem, soquete); //envia mensagem
					fim = 1;
				} else if (mensagem->tipo == ACK && enviouTudo && enviouFim) { //se for ACK E envitou todo arquivo E enviou fim de arquivo
					fim = 1;
				} else if (mensagem->tipo == NACK) { //se for do tipo NACK
					envia_mensagem_servidor(mensagem, soquete); //envia a mesma mensagem novamente
					//tratamento de sequencias -----------------------------------------------------------
					if (prox_enviar < MAX_SEQ) {
						prox_enviar++;
					} else {
						prox_enviar = 0x00; //se passar do limite zeramos o contador
					}
					// -------------------------------------------------------------------------------
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
			cria_mensagem_servidor(prox_enviar, NACK, ""); // crio NHAQUE
			envia_mensagem_servidor(mensagem, soquete); // manda famoso NHAQUE
			
            //tratamento de sequencias -----------------------------------------------------------
            if (prox_enviar < MAX_SEQ) {
                prox_enviar++;
            } else {
                prox_enviar = 0x00; //se passar do limite zeramos o contador
            }

            if (prox_receber < MAX_SEQ) {
                prox_receber++;
            } else {
                prox_receber = 0x00; //se passar do limite zeramos o contador
            }
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
		system("ls -a > .lsResult");
	} else if (!strcmp(mensagem->dados, "-l")) {
        printf("Executando comando: ls -l\n");
		system("ls -l > .lsResult");
	} else {
        printf("Executando comando: ls\n");
		system("ls > .lsResult");
	}

	envia_arquivo(".lsResult", MOSTRA_TELA, 0, 1, soquete); //envia o arquivo com a saída do ls
	system("rm .lsResult"); //remove o arquivo temporário
}

void espera_mensagem(Mensagem *mensagem, int soquete) {
	unsigned char paridade = 0x00;
	unsigned char buffer[21];
	int i = 0;

	recv(soquete, buffer, 21, 0);

	if (buffer[0] == 0x7E) {
		//------- recupera a mensagem ------------
		r->inicio = buffer[0];
		r->tamanho = (buffer[1] << 4) >> 4;
		r->sequencia = buffer[1] >> 4;
		r->tipo = (buffer[2] << 4) >> 4;
		r->paridade = buffer[2] >> 4;
		i = 0;
		for (; i < MAX_DADOS; i++) {
			r->dados[i] = buffer[i + 3];
		}
		//------ fim recupera mensagem -----------
		paridade = calcula_paridade(r);
		if (paridade == r->paridade) {
			*evento = mensagemRecebida;
		} else {
			*evento = error;
		}
	} else {
		*evento = lixo;
	}
}

void roda_servidor(int soquete){
    Mensagem *mensagem = malloc(sizeof(Mensagem));

    do {
		processo_poll(mensagem, soquete);

		if (mensagem->sequencia == 0) {
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