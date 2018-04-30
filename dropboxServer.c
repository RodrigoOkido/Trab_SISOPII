/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/


#include "dropboxUtil.h"
#include "dropboxServer.h"


int sockfd, n;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
char buf[BUFFER_TAM];


void sync_server(){

  //TODO

}


void receive_file(char *file) {

	n = recvfrom(sockfd, file, BUFFER_TAM, 0, (struct sockaddr *) &cli_addr, &clilen);
	if (n < 0)
		printf("[ERROR] on recvfrom");
	printf("Received a datagram: %s\n", buf);

	/* send to socket */
	n = sendto(sockfd, "Got your message\n", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
	if (n  < 0)
		printf("[ERROR] Message not received");

}


void send_file(char *file){

  //TODO

}


int main(int argc, char *argv[])
{


  // Executa a operação de abrir o socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("[ERROR] Socket cannot be opened.");


	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);


	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
		printf("[ERROR] Could not Bind");


	clilen = sizeof(struct sockaddr_in);


	while (1) {
		/* receive from socket */
		n = recvfrom(sockfd, buf, BUFFER_TAM, 0, (struct sockaddr *) &cli_addr, &clilen);
		if (n < 0)
			printf("[ERROR] on recvfrom");
		printf("Received a datagram: %s\n", buf);

		/* send to socket */
		n = sendto(sockfd, "Got your message\n", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		if (n  < 0)
			printf("[ERROR] Message not received");
	}

	close(sockfd);
	return 0;
}
