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

		//<<< POR ENQUANTO GAMBIARRA AQUI!!!!!!!!!!! >>>
		//Cria um diretorio qualquer para armazenar o arquivo.
		//Depois necessita colocar numa pasta para o userid
		char* file_complete = "/tmp2/";
		file_complete = malloc(strlen(file)+EXT); /* make space for the new string (should check the return value ...) */
		strcpy(file_complete, file); /* copy name into the new var */
		strcat(file_complete, actualClient->file_info[actualClient->files_qty].extension);

		char* folder = file_complete;

		FILE *receiveFile = fopen(folder, "w");
		int bytesRead = 0;


		while (file_length > bytesRead ) {
				/* receive from socket */
				n = recvfrom(sockfd, buf, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
				if (n < 0)
						printf("[ERROR] on recvfrom");

				fwrite(buf , 1 , sizeof(buf) , receiveFile );
				bytesRead += sizeof(buf);

				if(DEBUG){
						printf("\n\nReceived packet from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
						printf("Received a datagram:\n%s", buf);
				}

				bzero(buf, sizeof(buf));
		}

		fclose(receiveFile);


		/* send to socket */
		n = sendto(sockfd, "File Uploaded\n", 13, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		bzero(buf, sizeof(buf));
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
			n = recvfrom(sockfd, buf, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);

			// int server_code = command_code; //command_code is the action code wanted by the client.
			//
			// switch(server_code){
			//
			// 		case 1: receive_file(actualClient->file_info[actualClient->files_qty].name); break; //NOT WORKING YET
			// 		case 2: break;
			// 		default: break;//fprintf(stderr,"Unrecognized Command...\n"); break;
			// }


			if (n < 0)
				printf("[ERROR] on recvfrom");

			printf("\n\nReceived packet from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
			printf("Received a datagram:\n%s", buf);

			/* send to socket */
			n = sendto(sockfd, "File Uploaded\n", 13, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
			bzero(buf, sizeof(buf));
			if (n  < 0)
				printf("[ERROR] Message not received");
		}

		close(sockfd);
		return 0;
}
