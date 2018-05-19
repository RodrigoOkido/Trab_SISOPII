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

	if(DEBUG) fprintf(stderr, "- Aguardando receber pacote de informações\n");
	//Server aguardando receber o primeiro pacote de informações
	struct File_package *fileReceive = (struct File_package*)malloc(sizeof(struct File_package));
	n = recvfrom(sockfd, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	createNewFile(actualClient, fileReceive);

	//Depois necessita colocar numa pasta para o userid
	char* file_complete = malloc(strlen("receiveFile")+EXT+1); /* create space for the file */
	strcpy(file_complete, "receiveFile"); /* copy filename into the new var */
	strcat(file_complete, "."); /* copy filename into the new var */
	strcat(file_complete, fileReceive->extension); /* concatenate extension */

	char* folder = file_complete;
	fprintf(stderr,"%s\n",folder);

	FILE *receiveFile = fopen(folder, "w");
	int bytesRead = 0;

	if(DEBUG) fprintf(stderr, "- Respondendo ACK do pacote de informações\n");

	struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
	answer->cmd = ACK;
	n = sendto(sockfd, answer, sizeof(struct Request), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	int file_size = fileReceive->size;

	while (bytesRead < file_size){

		if(DEBUG) fprintf(stderr, "- Aguardando pacote do arquivo\n");
		n = recvfrom(sockfd, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);
		if(n < 0) fprintf(stderr, "[ERROR]\n");

		if(DEBUG){
			printf("\n\nReceived packet from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
			printf("Packet Number: %i\n", fileReceive->package);
			printf("Packet Size Sent: %i\n", (int) strlen(fileReceive->buffer));
			printf("Received a datagram:\n%s\n", fileReceive->buffer);
		}

		if(DEBUG) fprintf(stderr, "- Escrevendo buffer no arquivo\n");
		bytesRead += fwrite(fileReceive->buffer , sizeof(char) ,(int) strlen(fileReceive->buffer) , receiveFile );

		bzero(fileReceive->buffer, sizeof(fileReceive->buffer));
		// Pacote recebido
		if(DEBUG) fprintf(stderr, "- Respondendo com ACK pacote do arquivo\n");
		n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		if(n < 0) fprintf(stderr, "[ERROR]\n");
	}

	fclose(receiveFile);

	if(DEBUG) fprintf(stderr, "=== FIM UPLOAD ===\n");

}


void sendFile(){

	//Receive the download request
	struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
	answer->cmd = ACK;

	struct File_package *fileSend = (struct File_package*)malloc(sizeof(struct File_package));
	n = recvfrom(sockfd, fileSend, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);

	//Depois necessita colocar numa pasta para o userid
	char* file = malloc(strlen(fileSend->name)+EXT+1); /* create space for the file */
	strcpy(file, fileSend->name); /* copy filename into the new var */
	strcat(file, "."); /* copy filename into the new var */
	strcat(file, fileSend->extension); /* concatenate extension */

	FILE *sendFile = fopen(file, "r");
	int packet, read_buffer; //packet - Counter of packets needed to send the file.
													 //read_buffer - buffer which will receive the content of the file.

	fseek(sendFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte number.
	int file_length = ftell(sendFile); //Store the file length of the received file.
	fseek(sendFile, 0, SEEK_SET); //Turn the pointer to the beginning.


	int file_size = fileSend->size;

	while(!feof(sendFile) && file_length > 0) {

		fileSend->package++;

		//Zero out our send buffer
		bzero(fileSend->buffer, sizeof(fileSend->buffer));

		int s = fread(fileSend->buffer, sizeof(char), sizeof(fileSend->buffer)-1, sendFile);

		//Send data through our socket
		do {
			if(DEBUG) fprintf(stderr, "- Enviado pacote do arquivo\n");
			n = sendto(sockfd, fileSend, sizeof(struct File_package), 0,(const struct sockaddr *) &cli_addr, sizeof(struct sockaddr_in));
			if(n<0) printf("[ERROR] in package %d\n", fileSend->package);
		} while(n < 0);

		if(DEBUG){
			printf("\n");
			printf("Packet Number: %i\n", fileSend->package);
			printf("Packet Size Sent: %i\n",s);
			printf("\n");
		}

		if(DEBUG) fprintf(stderr, "- Aguardando ACK do pacote do arquivo\n");
		//RECEIVE THE ACK FOR THE PACKAGE
		n = recvfrom(sockfd,  answer, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &newClilen);
		if(answer->cmd != ACK){
			fprintf(stderr, "[ERROR] ACK\n" );
			return;
		}
	}

	file_length = 0;

	fclose(sendFile);



	if(DEBUG) fprintf(stderr, "=== FIM DOWNLOAD ===\n");

}

void delete_file_request(char * user, char * file){

	//CLIENT* client = find_or_createClient(user);

	char path[MAXNAME + sizeof(serverDir)];
	memset(path, 0, sizeof(path));

	strcat(path, serverDir);
	strcat(path, actualClient->userid);
	strcat(path, "/");
	strcat(path, file);

	int status = remove(path);
	if(DEBUG) printf("\n sizeof: %i \t path: %s_ \t status: %i \t file: %s_ \n",(int) sizeof(path), path, status, file);
	if(status == 0){ //remove file cliente side
		delete_info_file(actualClient, file);
		printf("File ( %s ) deleted sucessfully!\n", file);
	}
	else{
		printf("Error: unable to delete the %s file\n", file);
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

		if(DEBUG) fprintf(stderr, "- Aguardando receber Request\n");
		n = recvfrom(sockfd, request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &newClilen);
		if(n < 0) fprintf(stderr,"[ERROR] main(): receive\n");

		struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
		answer->cmd = ACK;
		switch(request->cmd){
			case CONNECT:
					actualClient = find_or_createClient(request->user);
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					fprintf(stderr,"User: %s connected\n", actualClient->userid);
					break;

			case UPLOAD:
					if(DEBUG) fprintf(stderr, "- Respondendo o comando com ACK\n");
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					receive_file();
					break;

			case DOWNLOAD:
					if(DEBUG) fprintf(stderr, "- Respondendo o comando com ACK\n");
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					sendFile();
					break;
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
			case EXIT:
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					fprintf(stderr,"User: %s disconnected!\n", actualClient->userid);
			 		break;

			case ERROR:
			default: n = sendto(sockfd, "[SERVER] COMMAND ERROR!\n", 23, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					 break;
		}


		/*
		// Cria uma nova Thread para resolver o request
		int rc = pthread_create(&threads[thread_no], NULL, handle_request, (void*)request);
		if(rc)
			fprintf(stderr,"A request could not be processed\n");
		else
			thread_no++;
		*/
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
				if(DEBUG) fprintf(stderr, "- Respondendo o comando com ACK\n");
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				receive_file();
				break;

		case DOWNLOAD: if(DEBUG) fprintf(stderr, "- Respondendo o comando com ACK\n");
				n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				sendFile();
				break;
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
		case EXIT: fprintf(stderr, "- Respondendo o comando com ACK\n");break;

		case ERROR:
		default: n = sendto(sockfd, "[SERVER] COMMAND ERROR!\n", 23, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
				 break;
	}
}
