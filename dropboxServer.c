/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxServer.h"
#include "dropboxClient.h"

#include <pthread.h>

// Caso alguma thread comece a executar
pthread_mutex_t exec_mutex     = PTHREAD_MUTEX_INITIALIZER;
// Caso formos usar :D
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;
int thread_no = 0;

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

void delete_file_request(char* file){

	//path = MAXNAME + "./home/sync_dir_" => 273
	char path[273];
	strcpy(path, serverDir);
	strcpy(path, actualClient->userid);
	strcat(path, "/");
	strcat(path, file);

	// struct FILE_INFO file_info;

	if(remove(path) != 0){
	  printf("Error: unable to delete the %s file\n", file);
	}
	else{
		delete_info_file(actualClient, file);
	}
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

	pthread_t threads[MAX_THREADS];

	while(1)
	{
		struct Request *request = (struct Request*)malloc(sizeof(struct Request));
		socklen_t newClilen = sizeof(struct sockaddr_in);

		n = recvfrom(sockfd, request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &newClilen);
		if(n < 0) fprintf(stderr,"[ERROR] receive\n");

		// Cria uma nova Thread para resolver o request
		int rc = pthread_create(&threads[thread_no], NULL, handle_request, (void*)request);
		if(rc)
			fprintf(stderr,"A request could not be processed\n");
		else
			thread_no++;

	}
}

void *handle_request(void *req)
{
	struct Request *request = (struct Request*)req;

	switch(request->cmd){
		case CONNECT:
				actualClient = find_or_createClient(request->user);
				struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
				answer->cmd = ACK;
				n = sendto(sockfd, answer, sizeof(struct Request), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				fprintf(stderr,"User: %s connected\n", actualClient->userid);
				break;

		case UPLOAD:	//IF SERVER CODE IS ONE, THE SERVER PREPARES FOR RECEIVE A FILE
				n = sendto(sockfd, "[SERVER] COMMAND RECEIVED!\n", 26, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				n = recvfrom(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
				if (strncmp(buf,"File not found..",16) == 0){
					fprintf(stderr, "[CLIENT] %s", buf);
					bzero(buf, sizeof(buf));
					n = sendto(sockfd, "[SERVER] ABORTING OPERATION...\n", 30, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				} else {
					filesize = atoi(buf);
					bzero(buf, sizeof(buf));
					n = sendto(sockfd, "[SERVER] CHECKING SIZE....\n", 26, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					n = recvfrom(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);
					parseFile(buf); 		//Parse the filename and file extension (check dropboxUtil.c for this function)
					createNewFile(actualClient, filesize);
					receive_file(actualClient->file_info[actualClient->files_qty-1].name);
				}
				break;

		case DOWNLOAD: break;
		case DELETE:
				bzero(buf, sizeof(buf));

				n = sendto(sockfd, "[SERVER] COMMAND RECEIVED!\n", 26, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				n = recvfrom(sockfd, buf, sizeof(buf), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &clilen);

				delete_file_request(buf); 

				break;
		case LIST_SERVER: break;
		case LIST_CLIENT: break;
		case GET_SYNC_DIR: break;
		case EXIT: break;

		case ERROR:
		default: n = sendto(sockfd, "[SERVER] COMMAND ERROR!\n", 23, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				 break;
	}
}
