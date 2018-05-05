/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/


#include "dropboxUtil.h"
#include "dropboxServer.h"
#include "dropboxClient.h"


int sockfd, n;
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
char buf[BUFFER_TAM];

CLIENT* actualClient;
int filesize;


void sync_server(){

  //TODO

}


void receive_file(char *file) {

		//<<< POR ENQUANTO GAMBIARRA AQUI!!!!!!!!!!! >>>
		//SOBREESCREVE O MESMO arquivo.
		//Depois necessita colocar numa pasta para o userid
		char* file_complete = malloc(strlen(file)+EXT); /* create space for the file */
		strcpy(file_complete, file); /* copy filename into the new var */
		strcat(file_complete, actualClient->file_info[actualClient->files_qty-1].extension); /* concatenate extension */

		char* folder = file_complete;
		fprintf(stderr,"%s\n",folder);

		FILE *receiveFile = fopen(folder, "w");
		int bytesRead = 0;
		bzero(buf, sizeof(buf));
		n = sendto(sockfd, "[SERVER] CHECKING AND PREPARING TO UPLOAD FILE\n", 46, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));


		while (filesize > bytesRead ) {
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
		bzero(buf, sizeof(buf));
		n = sendto(sockfd, "File Uploaded\n", 13, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));

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

		/* MESSAGES EXCHANGES TO ESTABLISH CONNECTION CLIENT/SERVER */
		n = recvfrom(sockfd, buf, UNIQUE_ID, MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
		char* id = buf;
		fprintf(stderr, "[SERVER] RECEIVED USERID %s\n",id);
		actualClient = find_or_createClient(id);
		bzero(buf, sizeof(buf));
		n = sendto(sockfd, "[SERVER] CONNECTION ESTABLISHED!\n", 32, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));


		while (1) {
				/* MESSAGES EXCHANGES TO CHECK THE COMMAND RECEIVED BY USER*/
				n = recvfrom(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
				int server_code = parseCommand(buf); //command_code is the action code wanted by the client.
				fprintf(stderr, "[SERVER] CODE %i\n",server_code);
				bzero(buf, sizeof(buf));


				if(server_code == 1){
						//IF SERVER CODE IS ONE, THE SERVER PREPARES FOR RECEIVE A FILE
						n = sendto(sockfd, "[SERVER] COMMAND RECEIVED!\n", 26, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
						n = recvfrom(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
						filesize = atoi(buf);
						bzero(buf, sizeof(buf));
						 n = sendto(sockfd, "[SERVER] CHECKING SIZE....\n", 26, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				}


				switch(server_code){

						case 1:	n = recvfrom(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
										parseFile(buf); 		//Parse the filename and file extension (check dropboxUtil.c for this function)
										createNewFile(actualClient, filesize);
										receive_file(actualClient->file_info[actualClient->files_qty-1].name); break; //NOT WORKING YET
						case 2: break;
						default: n = sendto(sockfd, "[SERVER] COMMAND ERROR!\n", 23, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
										 break;//fprintf(stderr,"Unrecognized Command...\n"); break;
				}
		}

		close(sockfd);
		return 0;
}
