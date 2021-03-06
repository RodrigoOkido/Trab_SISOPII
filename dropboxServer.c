/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxServer.h"
#include "dropboxClient.h"
#include "election.h"
#include <sys/stat.h>
#include <pthread.h>

// Caso alguma thread comece a executar
pthread_mutex_t exec_mutex     = PTHREAD_MUTEX_INITIALIZER;
// Caso formos usar :D
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

pthread_t threads[MAX_THREADS];
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


void receive_file(char* userid) {

	pthread_mutex_lock(&exec_mutex);
	if(DEBUG) fprintf(stderr, "- Aguardando receber pacote de informações\n");
	//Server aguardando receber o primeiro pacote de informações
	struct File_package *fileReceive = (struct File_package*)malloc(sizeof(struct File_package));
	n = recvfrom(sockfd, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	CLIENT *client = find_or_createClient(userid);

	createNewFile(client, fileReceive);

	char* file_complete = malloc(strlen(serverDir) + strlen(fileReceive->name)+EXT+1); /* create space for the file */
	strcpy(file_complete, serverDir); /* copy filename into the new var */
	strcat(file_complete, userid); /* copy filename into the new var */
	strcat(file_complete, "/"); /* copy filename into the new var */
	strcat(file_complete, fileReceive->name); /* copy filename into the new var */
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
			//printf("Received a datagram:\n%s\n", fileReceive->buffer);
		}

		if(DEBUG) fprintf(stderr, "- Escrevendo buffer no arquivo\n");
		bytesRead += fwrite(fileReceive->buffer , sizeof(char) ,(int) strlen(fileReceive->buffer) , receiveFile );

		bzero(fileReceive->buffer, sizeof(fileReceive->buffer));
		// Pacote recebido
		if(DEBUG) fprintf(stderr, "- Respondendo com ACK pacote do arquivo\n");
		n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		if(n < 0) fprintf(stderr, "[ERROR]\n");
	}
	pthread_mutex_unlock(&exec_mutex);
	fclose(receiveFile);

	if(DEBUG) fprintf(stderr, "=== FIM UPLOAD ===\n");

}


void sendFile(char* userid){
	pthread_mutex_lock(&exec_mutex);
	//Receive the download request
	struct Request *answer = (struct Request*)malloc(sizeof(struct Request));
	answer->cmd = ACK;

	if(DEBUG) fprintf(stderr, "- Aguardando o nome do arquivo\n");
	struct File_package *fileSend = (struct File_package*)malloc(sizeof(struct File_package));
	n = recvfrom(sockfd, fileSend, sizeof(struct File_package), 0, (struct sockaddr *) &cli_addr, &newClilen);

	//Prepara o caminho do arquivo a ser enviado
	char* file = malloc(strlen(serverDir) + strlen(fileSend->name)+EXT+1); /* create space for the file */
	strcpy(file, serverDir); /* copy filename into the new var */
	strcat(file, userid); /* copy filename into the new var */
	strcat(file, "/"); /* copy filename into the new var */
	strcat(file, fileSend->name); /* copy filename into the new var */
	strcat(file, "."); /* copy filename into the new var */
	strcat(file, fileSend->extension); /* concatenate extension */

	FILE *sendFile = fopen(file, "r");
	if(sendFile == NULL){
		fprintf(stderr, "[ERROR] Ao abrir o arquivo\n");
		if(DEBUG) fprintf(stderr, "- Enviando pacote ERROR\n");
		fileSend->size = -1;
		n = sendto(sockfd, fileSend, sizeof(struct File_package), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		return;
	}
	int packet, read_buffer; //packet - Counter of packets needed to send the file.
							//read_buffer - buffer which will receive the content of the file.

	fseek(sendFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte number.
	int file_length = ftell(sendFile); //Store the file length of the received file.
	fseek(sendFile, 0, SEEK_SET); //Turn the pointer to the beginning.

	fileSend->size = file_length;

	if(DEBUG) fprintf(stderr, "- Enviando pacote de informações\n");
	n = sendto(sockfd, fileSend, sizeof(struct File_package), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
	if(n<0) printf("[ERROR] in Pacote de informações\n");

	if(DEBUG) fprintf(stderr, "- Aguardadno ACK do pacote de informações\n");
	n = recvfrom(sockfd,  answer, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &newClilen);
	if(answer->cmd != ACK){
		fprintf(stderr, "[ERROR] ACK\n" );
		return;
	}

	while(!feof(sendFile) && file_length > 0) {

		fileSend->package++;

		//Zero out our send buffer
		bzero(fileSend->buffer, sizeof(fileSend->buffer));

		int s = fread(fileSend->buffer, sizeof(char), sizeof(fileSend->buffer)-1, sendFile);

		//Send data through our socket
		if(DEBUG) fprintf(stderr, "- Enviado pacote do arquivo\n");
		n = sendto(sockfd, fileSend, sizeof(struct File_package), 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
		if(n<0) printf("[ERROR] in package %d\n", fileSend->package);

		if(DEBUG){
			printf("\n");
			printf("Packet Number: %i\n", fileSend->package);
			printf("Packet Size Sent: %i\n",s);
			printf("\n");
		}

		if(DEBUG) fprintf(stderr, "- Aguardando ACK do pacote do arquivo\n");
		n = recvfrom(sockfd,  answer, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli_addr, &newClilen);
		if(answer->cmd != ACK){
			fprintf(stderr, "[ERROR] ACK\n" );
			return;
		}
	}
	pthread_mutex_unlock(&exec_mutex);
	fclose(sendFile);

	if(DEBUG) fprintf(stderr, "=== FIM DOWNLOAD ===\n");
}

void delete_file_request(char * user, char * file){

	pthread_mutex_lock(&exec_mutex);

	char path[MAXNAME + sizeof(serverDir)];
	memset(path, 0, sizeof(path));

	strcat(path, serverDir);
	strcat(path, actualClient->userid);
	strcat(path, "/");
	strcat(path, file);

	int status = remove(path);
	if(DEBUG) printf("\n sizeof: %i \t path: %s_ \t status: %i \t file: %s_ \n",(int) sizeof(path), path, status, file);

	pthread_mutex_unlock(&exec_mutex);

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
	if(argc < 3){printf("\n\n---------------| to use |--------------\n\n ./dropboxServer b {IP PRIMARY} \n\n--------------------------------------------------\n"); exit(1);}
	// Executa a operação de abrir o socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("[ERROR] Socket cannot be opened.");

	int port = 4000;
	int type;
		printf("type = %d \n", type);

	THREAD_ARG argument[1]; 
	if(strcmp("p", argv[1])==0){
		type = PRIMARY;
	}
	if(strcmp("b", argv[1])==0){
		type = BACKUP;
	}

	argument[0].type = type;
	argument[0].address = argv[2];

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
		printf("[ERROR] Could not Bind\n");

	pthread_create(&threads[thread_no], NULL, handle_replication, &(argument[0]));
	thread_no++;

	// Cria uma nova Thread para resolver o request
	int rc = pthread_create(&threads[thread_no], NULL, handle_request, NULL);
	if(rc)
		fprintf(stderr,"A request could not be processed\n");
	else
		pthread_join(threads[thread_no], NULL) ;
		thread_no++;


}

void *handle_replication(void *type)
{
	p_THREAD_ARG args = (p_THREAD_ARG) type;
	printf("type = %d \n", args->type);
	if(DEBUG) fprintf(stderr, "[Thread replication] init \n");
	initConfigOfElection(args->type, args->address);
}

void *handle_request(void *req)
{
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

					if (logged_device(actualClient, 1)){
						fprintf(stderr,"User: %s connected\n", actualClient->userid);
					}
					else{
						fprintf(stderr, "\n \t Unable to login more devices \n");
						answer->cmd = ERROR;
						fprintf(stderr,"User: %s ERROR to connect\n", actualClient->userid);
					}

					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					break;

			case UPLOAD:
					if(DEBUG) fprintf(stderr, "- UPLOAD: Respondendo o comando com ACK\n");
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					receive_file(request->user);
					break;

			case DOWNLOAD:
					if(DEBUG) fprintf(stderr, "- DOWNLOAD: Respondendo o comando com ACK\n");
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					sendFile(request->user);
					break;
			case DELETE:
					if(DEBUG) fprintf(stderr, "- DELETE: Respondendo o comando com ACK\n");
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					delete_file_request(request->user, request->buffer);
					break;
			case LIST_SERVER:
					if(DEBUG) fprintf(stderr, "- LIST_SERVER: Respondendo o comando com ACK\n");
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));

					CLIENT* client = find_or_createClient(request->user);

					if(DEBUG) show_files(client, 1);

					if(DEBUG) fprintf(stderr, "- LIST_SERVER: Enviando lista de arquivos\n");
					n = sendto(sockfd, client->file_info, sizeof(client->file_info), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					break;
			case LIST_CLIENT:
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					break;
			case GET_SYNC_DIR: break;
			case EXIT:
					actualClient = find_or_createClient(request->user);
					logged_device(actualClient, 0);
					n = sendto(sockfd, answer, sizeof(struct Request), MSG_CONFIRM,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					fprintf(stderr,"User: %s disconnected!\n", actualClient->userid);
			 		break;

			case ERROR:
			default: n = sendto(sockfd, "[SERVER] COMMAND ERROR!\n", 23, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr));
					 break;
		}
	}
}
