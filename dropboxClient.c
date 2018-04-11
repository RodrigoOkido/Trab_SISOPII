/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxClient.h"


int login_server(char *host, int port) {

  int sockfd;
	unsigned int length;
	struct sockaddr_in serv_addr, from;
	struct hostent *server;
	char buffer[BUFFER_TAM];

	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);
	}

	server = gethostbyname(argv[1]);

  // Verifica Host
	if (server == NULL) {
        fprintf(stderr,"[ERROR] No host.\n");
        exit(0);
    }

  // Executa a operação de abrir o socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		printf("[ERROR] Socket cannot be opened.");
  }

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

}


void sync_client() {

  //TODO

}


void send_file(char *file){

  //TODO

}


void get_file(char *file){

  //TODO

}


void delete_file(char *file){

  //TODO

}


void close_session(){

  //TODO

}


int main
