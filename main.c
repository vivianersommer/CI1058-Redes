#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "servidor.h"
#include "cliente.h"

int main(int argc, char *argv[]) {

    char *tipo = malloc(sizeof(char) * 10);

    printf("-------------- Bem vindo ao Trabalho 1 de Redes 1 --------------\n");  

    if(argc < 2) {
        printf("Você esqueceu de passar alguns parâmetros necessários, por favor rode do seguinte modo: \n");
        printf("./main <cliente/servidor> \n");
        exit(1);
    }

    tipo = argv[1];
    if (!strcmp(tipo,"servidor")){
        printf("---------------------- Rodando o Servidor ----------------------\n");
        servidor();
    } else {
        if (!strcmp(tipo,"cliente")){
            printf("---------------------- Rodando o Cliente -----------------------\n");
            cliente();
        } else {
            printf("O segundo argumento não está certo, por favor rode do seguinte modo: \n");
            printf("./main <cliente/servidor> \n");
            exit(1);  
        }
    }

    return 1;
}
