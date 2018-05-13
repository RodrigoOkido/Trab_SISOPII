/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxServer.h"
#include "dropboxClient.h"
#include <sys/stat.h>
#include <pthread.h>

// Caso alguma thread comece a executar
pthread_mutex_t exec_mutex     = PTHREAD_MUTEX_INITIALIZER;
// Caso formos usar :D
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;
int thread_no = 0;

int sockfd, n;
socklen_t clilen, newClilen;
struct sockaddr_in serv_addr, cli_addr;
char buf[BUFFER_TAM];

CLIENT* actualClient;
int filesize;

void sync_server(){

  //TODO

}


void receive_file() {

	//Server aguardando receber o primeiro pacote
	struct File_package *fileReceive = (struct File_package*)malloc(sizeof(struct File_package));
	n = recvfrom(sockfd, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);

	createNewFile(actualClient, fileReceive);

	//Depois necessita colocar numa pasta para o userid
	char* file_complete = malloc(strlen(fileReceive->name)+EXT+1); /* create space for the file */
	strcpy(file_complete, fileReceive->name); /* copy filename into the new var */
	strcat(file_complete, "."); /* copy filename into the new var */
	strcat(file_complete, fileReceive->extension); /* concatenate extension */

	char* folder = file_complete;
	fprintf(stderr,"%s\n",folder);

	FILE *receiveFile = fopen(folder, "wb");
	int bytesRead = 0;

	do{
		printf("[ESCREVENDO]\n");
		fwrite(fileReceive->buffer , sizeof(char) , sizeof(fileReceive->buffer) , receiveFile );
		bytesRead += fileReceive->package * BUFFER_TAM;
		if(DEBUG){
			printf("\n\nReceived packet from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
			printf("Packet Number: %i\n", fileReceive->package);
			printf("Packet Size Sent: %i\n", (int) strlen(fileReceive->buffer));
			printf("Received a datagram:\n%s", fileReceive->buffer);
		}

		bzero(fileReceive->buffer, sizeof(fileReceive->buffer));
		// Pacote recebido
		struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
		answer->cmd = ACK;
		n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		/* receive from socket */
		if(filesize > bytesRead){
			n = recvfrom(sockfd, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);
			if (n < 0)
					printf("[ERROR] on recvfrom");
		}

	}while (filesize > bytesRead );

	fclose(receiveFile);


	/* send to socket */
	bzero(buf, sizeof(buf));
	n = sendto(sockfd, "File Uploaded\n", 13, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));

	if (n  < 0)
		printf("[ERROR] Message not received");

}


void send_file(char *file){

  	
	int fileSize;
	char path[273];
	strcpy(path, serverDir);
	strcpy(path, actualClient->userid);
	strcat(path, "/");
	strcat(path, file);

	int bytes = 0;
	
	FILE *sendFile;

	if (sendFile = fopen(path, "rb")) {

		fseek(sendFile, 0L, SEEK_END);
		fileSize = ftell(sendFile);

		rewind(sendFile);

		bytes = sendto(sockfd, &fileSize, sizeof(int), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));

		while (!feof(sendFile)) {

		
			fread(buf, BUFFER_TAM,1, sendFile);

			bytes = sendto(sockfd, buf, BUFFER_TAM, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
			if (bytes < 0) {
				printf("Error sending file");
			}
		}
		fclose(sendFile);
	} else { // arquivo nao existe
	
		fileSize = -1;
		bytes = sendto(sockfd, &fileSize, sizeof(int), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
	}
 
}

void delete_file_request(char * user, char * file){

	CLIENT* client = find_or_createClient(user);

	//path = MAXNAME + "./home/sync_dir_" => 273
	char path[273];
	strcpy(path, serverDir);
	strcpy(path, client->userid);
	strcat(path, "/");
	strcat(path, file);

	// struct FILE_INFO file_info;

	if(remove(path) != 0){
	  printf("Error: unable to delete the %s file\n", file);
	}
	else{
		delete_info_file(client, file);
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
		newClilen = sizeof(struct sockaddr_in);

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
	// Ansew with ack
	struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
	answer->cmd = ACK;
	switch(request->cmd){
		case CONNECT:
				actualClient = find_or_createClient(request->user);
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				fprintf(stderr,"User: %s connected\n", actualClient->userid);
				break;

		case UPLOAD:
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				receive_file();
				break;

		case DOWNLOAD: break;
		case DELETE:
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				delete_file_request(request->user, request->buffer); 
				break;
		case LIST_SERVER: 
				memcpy(answer->buffer, &actualClient, sizeof(actualClient));
				answer->buffer[sizeof(actualClient)] = '\0';
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr)); 		
				break;
		case LIST_CLIENT:
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr)); 
				break;
		case GET_SYNC_DIR: break;
		case EXIT: break;

		case ERROR:
		default: n = sendto(sockfd, "[SERVER] COMMAND ERROR!\n", 23, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				 break;
	}
}
