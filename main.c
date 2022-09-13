#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "servidor.h"
#include "cliente.h"

int main(int argc, char *argv[]) {

    struct timeval timeout; //acho que da pra usar a timeeval pra fazer o timeout
    char *tipo = malloc(sizeof(char) * 10);
    int value = 0;

    printf("-------------- Bem vindo ao Trabalho 1 de Redes 1 --------------\n");  

    if(argc < 2) {
        printf("Você esqueceu de passar alguns parâmetros necessários, por favor rode do seguinte modo: \n");
        printf("./main <cliente/servidor> \n");
        exit(1);
    }

    tipo = argv[1];

    value = strcmp(tipo,"servidor");
    if (value == 0){
        servidor();
    } else {
        value = strcmp(tipo,"cliente");

        if (value == 0){
            cliente();
        } else {
            printf("O segundo argumento não está certo, por favor rode do seguinte modo: \n");
            printf("./main <cliente/servidor> \n");
            exit(1);  
        }
    }

   

}
