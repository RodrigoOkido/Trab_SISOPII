/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxClient.h"
#include <unistd.h>


int n ;
unsigned int length;
struct hostent *server;
char send_buffer[BUFFER_TAM];
char user_cmd[80];
char directory[50]; //
int notifyStart;
int watchList;


struct Request *request;
pthread_t threads[MAX_THREADS];
int thread_num = 0;


CLIENT* cli;

void startNotify () {

	notifyStart = inotify_init();

	watchList = inotify_add_watch (notifyStart, directory, IN_CREATE | IN_DELETE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_FROM); // por enquanto ve apenas se foi criado arquivo, deletado ou modificado

}



int login_server(char *host, int port, CLIENT* cli) {

	server = gethostbyname(host);

	// Verifica Host
	if (server == NULL) {
		fprintf(stderr,"[ERROR] No host.\n");
		exit(0);
	}

	 // Executa a operação de abrir o sockets de requests e socket
	 // para as operações do cliente que deseja logar.
	if (((cli->sockreq = socket(AF_INET, SOCK_DGRAM, 0)) == -1) || ((cli->sockaction = socket(AF_INET, SOCK_DGRAM, 0)) == -1)) {
 		printf("[ERROR] Some socket cannot be opened.");
 	}

	cli->serv_request.sin_family = AF_INET;
	cli->serv_request.sin_port = htons(PORT);
	cli->serv_request.sin_addr = *((struct in_addr *)server->h_addr);

	bzero(&(cli->serv_request.sin_zero), 8);

	return 1;
}



void *sync_client() {

	notifyStart = inotify_init();



	int length, i = 0;
	int fd = 0;
	char buffer[EVENT_BUF_LEN];
	char path[MAXNAME + sizeof(homeDir)];
	memset(path, 0, sizeof(path));

	strcat(path, homeDir);
	strcat(path, cli->userid);
	strcat(path, "/");

	char serverpath[MAXNAME + sizeof(serverDir)];
	memset(serverpath, 0, sizeof(serverpath));

	strcat(serverpath, serverDir);
	strcat(serverpath, cli->userid);
	strcat(serverpath, "/");

	fd = inotify_init();

	if (fd < 0) {
				perror("inotify_init");
	}

	watchList = inotify_add_watch (fd, serverDir, IN_CREATE | IN_DELETE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_FROM); // por enquanto ve apenas se foi criado arquivo, deletado ou modificado

	//while (1) { //fica verificando se alterou o diretorio
		length = read(fd, buffer, EVENT_BUF_LEN);
		fprintf(stderr,"[sync_client]\n");

		if (length < 0) {
			perror( "read" );
		}

		while ( i < length ) {

			struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];

			if ( event->len ) {
				if ( event->mask & IN_CREATE || event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO) {


					strcat(serverpath, event->name);   //teve alteracao no server?
					if( (fopen(path, "r")) == NULL ) {

					   printf("ERROR: File not found.\n");
					}
					else {
						send_file(path);
						// printf( "New file %s created.\n", event->name );
					}
				}

				else if ( event->mask & IN_DELETE  || event->mask & IN_MOVED_FROM) {
					strcat(path, event->name);
					delete_file (path);
					printf( "File %s deleted.\n", event->name );
				}

			}

		i += EVENT_SIZE + event->len;
		}

		i = 0;
	//	sleep(10); //verificar a cada 10 segundos
	//}

}

int send_cmdRequest(int cmd){
		request = (struct Request*)malloc(sizeof(struct Request));
		request->cmd = cmd;
		strcpy(request->user, cli->userid);

		if(DEBUG) fprintf(stderr, "- Enviando comando \n");
		n = sendto(cli->sockreq, request, sizeof(struct Request), 0,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));
		if(n < 0) fprintf(stderr, "[ERROR]\n");

		//RECEIVE THE ACK FOR THE COMMAND
		if(DEBUG) fprintf(stderr, "- Aguardando ACK do comando \n");
		n = recvfrom(cli->sockreq,  request, sizeof(struct Request), 0, (struct sockaddr *) &cli->serv_response, &length);
		if(n < 0) fprintf(stderr, "[ERROR]\n");
		if(request->cmd != ACK){
			fprintf(stderr, "[ERROR] It was not possible execute the command in server\n" );
			return -1;
		}
		return 0;
}


void send_file(char *file){

	FILE *sendFile = fopen(file, "r");
	int packet, read_buffer; //packet - Counter of packets needed to send the file.
													 //read_buffer - buffer which will receive the content of the file.
	// Verifica se existe o arquivo
	if (sendFile == NULL){
		fprintf(stderr, "File not found..\n");
		return;
	}

	fseek(sendFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte number.
	int file_size = ftell(sendFile); //Store the file length of the received file.
	fseek(sendFile, 0, SEEK_SET); //Turn the pointer to the beginning.


	if(send_cmdRequest(UPLOAD) == -1){
		return;
	}

	struct File_package *fileSend = (struct File_package*)malloc(sizeof(struct File_package));
	strcpy(fileSend->name, file);
	parseFile(fileSend); 		//Parse the filename and file extension to do the struct (check dropboxUtil.c for this function)
	fileSend->size = file_size;
	fileSend->package = 0;

	//SEND STRUCT FILE WITH INFORMATION
	if(DEBUG) fprintf(stderr, "- Enviando pacote de informações\n");
	n = sendto(cli->sockaction, fileSend, sizeof(struct File_package), 0,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	//RECEIVE THE ACK FOR THE COMMAND
	if(DEBUG) fprintf(stderr, "- Aguardadno ACK do pacote de informações\n");
	n = recvfrom(cli->sockaction,  request, sizeof(struct Request), 0, (struct sockaddr *) &cli->serv_response, &length);
	if(n < 0) fprintf(stderr, "[ERROR]\n");
	if(request->cmd != ACK){
		fprintf(stderr, "[ERROR] in receive ACK\n" );
		return;
	}

	while(!feof(sendFile) && file_size > 0) {

			fileSend->package++;

			//Zero out our send buffer
			bzero(fileSend->buffer, sizeof(fileSend->buffer));

			int s = fread(fileSend->buffer, sizeof(char), sizeof(fileSend->buffer)-1, sendFile);

			//Send data through our socket
			do {
				if(DEBUG) fprintf(stderr, "- Enviado pacote do arquivo\n");
				n = sendto(cli->sockaction, fileSend, sizeof(struct File_package), 0,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));
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
			n = recvfrom(cli->sockaction,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli->serv_response, &length);
			if(request->cmd != ACK){
				fprintf(stderr, "[ERROR] ACK\n" );
				return;
			}
	}

	printf("File ( %s ) uploaded sucessfully!\n", fileSend->name);

	fclose(sendFile);

	char *word = strtok(file,"/");
	char *sync = malloc(strlen(homeDir) + strlen(cli->userid));
	strcpy(sync,"sync_dir_");
	strcat(sync, cli->userid);
	int isSyncDir = 0;
	while(word){
	    if(strcmp(word,sync) == 0) isSyncDir = 1;
	    word = strtok(NULL, "/");
	}

	if(!isSyncDir){
	if(DEBUG) fprintf(stderr, "- copiando arquivo para sync_dir_%s arquivo\n", cli->userid);
	char* cpyfile = malloc(strlen(homeDir) + strlen(fileSend->name)+EXT+1); /* create space for the file */
	strcpy(cpyfile, homeDir); /* copy filename into the new var */
	strcat(cpyfile, cli->userid); /* copy filename into the new var */
	strcat(cpyfile, "/"); /* copy filename into the new var */
	strcat(cpyfile, fileSend->name); /* copy filename into the new var */
	strcat(cpyfile, "."); /* copy filename into the new var */
	strcat(cpyfile, fileSend->extension); /* concatenate extension */
	copy_file(file, cpyfile);

	createNewFile(cli, fileSend);
	}

	if(DEBUG) fprintf(stderr, "=== FIM UPLOAD ===\n");
}



void get_file(char *file)
	{
	struct File_package *fileReceive = (struct File_package*)malloc(sizeof(struct File_package));
	strcpy(fileReceive->name, file);
	parseFile(fileReceive);

	fprintf(stderr, "- %s %s\n", fileReceive->name, fileReceive->extension);


	if(send_cmdRequest(DOWNLOAD) == -1){
		return;
	}

	if(DEBUG) fprintf(stderr, "- Enviando nome do arquivo\n");
	n = sendto(cli->sockaction, fileReceive, sizeof(struct File_package), 0,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));

	if(DEBUG) fprintf(stderr, "- Aguardando o pacote de informações\n");
	n = recvfrom(cli->sockaction, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli->serv_response, &length);
	if(n < 0) fprintf(stderr, "[ERROR]\n");
	if(fileReceive->size == -1){
		fprintf(stderr, "[ERROR] Arquivo não existe no servidor\n" );
		return;
	}

	createNewFile(cli, fileReceive);

	char* file_complete = malloc(strlen(serverDir) + strlen(fileReceive->name)+EXT+1); /* create space for the file */
	strcpy(file_complete, homeDir); /* copy filename into the new var */
	strcat(file_complete, cli->userid); /* copy filename into the new var */
	strcat(file_complete, "/"); /* copy filename into the new var */
	strcat(file_complete, fileReceive->name); /* copy filename into the new var */
	strcat(file_complete, "."); /* copy filename into the new var */
	strcat(file_complete, fileReceive->extension); /* concatenate extension */

	char* folder = file_complete;
	fprintf(stderr,"%s\n",folder);

	FILE *receiveFile = fopen(folder, "w");
	if(receiveFile == NULL){
		fprintf(stderr, "[ERROR] Ao abrir o arquivo\n");
		return;
	}
	int bytesRead = 0;
	int file_size = fileReceive->size;

	if(DEBUG) fprintf(stderr, "- Respondendo com ACK pacote de informações\n");
	n = sendto(cli->sockaction, request, sizeof(struct Request), MSG_CONFIRM,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	while (bytesRead < file_size){

			if(DEBUG) fprintf(stderr, "- Aguardando pacote do arquivo\n");
			n = recvfrom(cli->sockaction, fileReceive, sizeof(struct File_package), 0, (struct sockaddr *) &cli->serv_response, &length);
			if(n < 0) fprintf(stderr, "[ERROR]\n");

			if(DEBUG){
				printf("\n\nReceived packet from %s:%d\n", inet_ntoa(cli->serv_request.sin_addr), ntohs(cli->serv_request.sin_port));
				printf("Packet Number: %i\n", fileReceive->package);
				printf("Packet Size Sent: %i\n", (int) strlen(fileReceive->buffer));
				//printf("Received a datagram:\n%s\n", fileReceive->buffer);
			}

			if(DEBUG) fprintf(stderr, "- Escrevendo buffer no arquivo\n");
			bytesRead += fwrite(fileReceive->buffer , sizeof(char) ,(int) strlen(fileReceive->buffer) , receiveFile );

			bzero(fileReceive->buffer, sizeof(fileReceive->buffer));
			// Pacote recebido
			if(DEBUG) fprintf(stderr, "- Respondendo com ACK pacote do arquivo\n");
			n = sendto(cli->sockaction, request, sizeof(struct Request), MSG_CONFIRM,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));
			if(n < 0) fprintf(stderr, "[ERROR]\n");
	}

	fclose(receiveFile);

	if(DEBUG) fprintf(stderr, "=== FIM DOWNLOAD ===\n");
}

void delete_file(char *file) {

		char path[MAXNAME + sizeof(homeDir)];
		memset(path, 0, sizeof(path));

		strcat(path, homeDir);
		strcat(path, cli->userid);
		strcat(path, "/");
		strcat(path, file);

		// Envia o comando para o servidor, para iniciar o upload
		struct Request *request = (struct Request*)malloc(sizeof(struct Request));
		request->cmd = DELETE;
		strcpy(request->user, cli->userid);

		strcpy(request->buffer, file);
		// request->buffer[sizeof(file)] = '\0';

		n = sendto(cli->sockreq, request, sizeof(struct Request), 0,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr));
		//RECEIVE THE ACK FOR THE COMMAND
		n = recvfrom(cli->sockreq,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli->serv_response, &length);
		if(request->cmd != ACK){
			fprintf(stderr, "[ERROR] It was not possible execute the command in server\n" );
			return;
		}
		else{
			int status = remove(path);
			if(DEBUG) printf("\n sizeof: %i \t path: %s \t status: %i \n",(int) sizeof(path), path, status);
			if(status == 0){ //remove file cliente side
				delete_info_file(cli, file);
				printf("File ( %s ) deleted sucessfully!\n", file);
			}
			else{
				printf("Error: unable to delete the %s file\n", file);
			}
		}

		return;
}

void list_dir(int local) {

		// sync_client();
		if(local == LIST_SERVER){

			if(DEBUG) fprintf(stderr, "- Enviando comando LIST_SERVER\n");

			if(send_cmdRequest(local) == -1){
				return;
			}

			if(DEBUG) fprintf(stderr, "- Aguardando lista de arquivos\n");
			CLIENT* client = (CLIENT*)malloc(sizeof(CLIENT));
			n = recvfrom(cli->sockreq, client->file_info, sizeof(client->file_info), MSG_CONFIRM, (struct sockaddr *) &cli->serv_response, &length);

			strcpy(client->userid, cli->userid);
			client->files_qty = 1;
			show_files(client, 1);
		}
		else{
			show_files(cli, 0);
		}
}



void close_session() {

		if(send_cmdRequest(EXIT) == -1){
			return;
		}

		pthread_exit(&threads[thread_num]);

		close(cli->sockreq);
		close(cli->sockaction);
		fprintf(stderr,"Session ended successfully! \n");
		exit(0);
}

void* command_get_func()
{
	int start = 1;
	while(start){

		showMenu(cli);
		if(DEBUG){
			printf("\t userid : %s \t qnt: %i \n", cli->userid, cli->files_qty);
			show_files(cli, 0);
		}
		printf("cmd > ");
		fgets(user_cmd, sizeof(user_cmd), stdin);
		user_cmd[strlen(user_cmd) -1] = '\0';

		int code = parseCommand(user_cmd);

		char *directory; //Used only if the action is upload/download/delete a file.
						//Takes the directory of the file.

		switch(code){
			case UPLOAD:
				directory = strndup(user_cmd+7, strlen(user_cmd));
				fprintf(stderr, "Uploading File : %s\n", directory);
				send_file(directory);
				break;
			case DOWNLOAD:
				directory = strndup(user_cmd+9, strlen(user_cmd));
				fprintf(stderr, "%s\n", directory);
				get_file(directory); //TO DO
				break;
			case DELETE:
				directory = strndup(user_cmd+7, strlen(user_cmd));
				fprintf(stderr, "%s\n", directory);
				delete_file(directory);
				break;
			case LIST_SERVER:
				list_dir(LIST_SERVER);
				break;
			case LIST_CLIENT:
				list_dir(LIST_CLIENT);
				break;
			case GET_SYNC_DIR: break;
			case EXIT: close_session(); break;
			default: printf("\nINVALID COMMAND \n"); break;
		}

		printf("\n\nPress enter...\n");
		char enter = 0;
		while (enter != '\r' && enter != '\n') {
			enter = getchar();
		}

	}
}

int main(int argc, char *argv[]){
	if (argc < 4) {
		fprintf(stderr, "[ERROR] Use the following syntax : ./dropboxClient {userid} {user_adress} {PORT}\n");
		exit(0);
	}

	cli = create_and_setClient(argv[1], 0);

	int login = login_server(argv[2], PORT, cli);

	if(login)
	{
		struct Request *request = (struct Request*)malloc(sizeof(struct Request));
		request->cmd = CONNECT;
		strcpy(request->user, cli->userid);

		//SEND USER ID TO SERVER
		n = sendto(cli->sockreq, request, sizeof(struct Request), 0,(const struct sockaddr *) &cli->serv_request, sizeof(struct sockaddr_in));
		//RECEIVE THE ANSWER FROM THE SERVER
		n = recvfrom(cli->sockreq,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &cli->serv_response, &length);
		if(request->cmd != ACK)
			fprintf(stderr, "[ERROR] It was not possible connect to this server\n" );
		else{
			//cria diretório local
			int dir = get_sync_dir(argv[1]);
			if(dir == 0) // == 0 Diretório já existe, pode ser sincronizado
			// sync_client(); //Sync client files with the server
			if(dir == -2) exit(dir);

			// Cria a thred de sincronização do diretório
			pthread_t sync_thread;

			// Cria uma nova Thread para resolver o request
			int cli_thread = pthread_create(&threads[thread_num], NULL, command_get_func, NULL);
			if(cli_thread) {
				fprintf(stderr,"A request could not be processed\n");
			} else {
				pthread_join(threads[thread_num], NULL) ;
				thread_num++;
			}
			//LOOP para pegar o comando do client
			//command_get_func();
		}
	}
	return 0;
}
