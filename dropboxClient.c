/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxClient.h"


int login_server(char *host, int port) {

  int sockfd,n ;
	unsigned int length;
	struct sockaddr_in serv_addr, from;
	struct hostent *server;
	char buffer[BUFFER_TAM];


	server = gethostbyname(host);

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


	printf("Enter the message: ");
	bzero(buffer, BUFFER_TAM);
	fgets(buffer, BUFFER_TAM, stdin);

	n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if (n < 0) 
		printf("ERROR sendto");
	
	length = sizeof(struct sockaddr_in);
	n = recvfrom(sockfd, buffer, BUFFER_TAM, 0, (struct sockaddr *) &from, &length);
	if (n < 0)
		printf("ERROR recvfrom");

	printf("Got an ack: %s\n", buffer);
	
	close(sockfd);
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


int main(int argc, char *argv[]){
	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);
	}

	int i = login_server(argv[1], PORT);

	return 0;
}
