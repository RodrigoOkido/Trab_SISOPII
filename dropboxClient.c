/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxClient.h"


int sockfd, n ;
unsigned int length;
struct sockaddr_in serv_addr, from;
struct hostent *server;
char send_buffer[BUFFER_TAM];
char user_cmd[80];
char directory[50]; //
int notifyStart;
int watchList;

CLIENT* cli;

void startNotify () {

	notifyStart = inotify_init();

	watchList = inotify_add_watch (notifyStart, directory, IN_CREATE | IN_DELETE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_FROM); // por enquanto ve apenas se foi criado arquivo, deletado ou modificado

}



int login_server(char *host, int port) {

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

	return 1;
}



void *sync_client(void *arg) {

	int length, i = 0;
	char buffer[EVENT_BUF_LEN];
	char path[200];
                     
	while (1) { //fica verificando se alterou o diretorio
		length = read(notifyStart, buffer, EVENT_BUF_LEN);
		fprintf(stderr,"[sync_client]\n");

		if (notifyStart < 0) {
				perror("inotify_init");
		}

		while ( i < length ) {

		struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];

		if ( event->len ) {
			if ( event->mask & IN_CREATE || event->mask & IN_CLOSE_WRITE || event->mask & IN_MOVED_TO) {

				strcpy(path, directory);
				strcat(path, "/");
				strcat(path, event->name);
				if( (fopen(path, "r")) == NULL ) {

				   printf("ERROR: File not found.\n");
				}
				else { 
					send_file(path);
					// printf( "New file %s created.\n", event->name );
				}
			}

			else if ( event->mask & IN_DELETE  || event->mask & IN_MOVED_FROM) {

				strcpy(path, directory);
				strcat(path, "/");
				strcat(path, event->name);
				delete_file (path);
				printf( "File %s deleted.\n", event->name );
			}
			
		}

		i += EVENT_SIZE + event->len;
		}

		i = 0;
		sleep(10); //verificar a cada 10 segundos
	}

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
	file_length = ftell(sendFile); //Store the file length of the received file.
	fseek(sendFile, 0, SEEK_SET); //Turn the pointer to the beginning.

	// Envia o comando para o servidor, para iniciar o upload
	struct Request *request = (struct Request*)malloc(sizeof(struct Request));
	request->cmd = UPLOAD;
	strcpy(request->user, cli->userid);

	if(DEBUG) fprintf(stderr, "- Enviando comando UPLOAD\n");
	n = sendto(sockfd, request, sizeof(struct Request), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	//RECEIVE THE ACK FOR THE COMMAND
	if(DEBUG) fprintf(stderr, "- Aguardando ACK do comando\n");
	n = recvfrom(sockfd,  request, sizeof(struct Request), 0, (struct sockaddr *) &from, &length);
	if(n < 0) fprintf(stderr, "[ERROR]\n");
	if(request->cmd != ACK){
		fprintf(stderr, "[ERROR] It was not possible execute the command in server\n" );
		return;
	}

	struct File_package *fileSend = (struct File_package*)malloc(sizeof(struct File_package));
	strcpy(fileSend->name, file);
	parseFile(fileSend); 		//Parse the filename and file extension to do the struct (check dropboxUtil.c for this function)
	fileSend->size = file_length;
	fileSend->package = 0;

	//SEND STRUCT FILE WITH INFORMATION
	if(DEBUG) fprintf(stderr, "- Enviando pacote de informações\n");
	n = sendto(sockfd, fileSend, sizeof(struct File_package), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	if(n < 0) fprintf(stderr, "[ERROR]\n");

	//RECEIVE THE ACK FOR THE COMMAND
	if(DEBUG) fprintf(stderr, "- Aguardadno ACK do pacote de informações\n");
	n = recvfrom(sockfd,  request, sizeof(struct Request), 0, (struct sockaddr *) &from, &length);
	if(n < 0) fprintf(stderr, "[ERROR]\n");
	if(request->cmd != ACK){
		fprintf(stderr, "[ERROR] in receive ACK\n" );
		return;
	}

	while(!feof(sendFile) && file_length > 0) {

		fileSend->package++;

		//Zero out our send buffer
		bzero(fileSend->buffer, sizeof(fileSend->buffer));

		int s = fread(fileSend->buffer, sizeof(char), sizeof(fileSend->buffer)-1, sendFile);

		//Send data through our socket
		do {
			if(DEBUG) fprintf(stderr, "- Enviado pacote do arquivo\n");
			n = sendto(sockfd, fileSend, sizeof(struct File_package), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
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
		n = recvfrom(sockfd,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &from, &length);
		if(request->cmd != ACK){
			fprintf(stderr, "[ERROR] ACK\n" );
			return;
		}
	}


	printf("File ( %s ) uploaded sucessfully!\n", fileSend->name);
	file_length = 0;

	fclose(sendFile);

	if(DEBUG) fprintf(stderr, "=== FIM UPLOAD ===\n");
}



void get_file(char *file){

  return;

}

void delete_file(char *file){

	char path[MAXNAME + sizeof(homeDir)];
	strcat(path, homeDir);
	strcat(path, cli->userid);
	strcat(path, "/");
	strcat(path, file);

	// Envia o comando para o servidor, para iniciar o upload
	struct Request *request = (struct Request*)malloc(sizeof(struct Request));
	request->cmd = DELETE;
	strcpy(request->user, cli->userid);

	strcpy(request->buffer, file);
	request->buffer[sizeof(file)] = '\0';

	n = sendto(sockfd, request, sizeof(struct Request), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	//RECEIVE THE ACK FOR THE COMMAND
	n = recvfrom(sockfd,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &from, &length);
	if(request->cmd != ACK){
		fprintf(stderr, "[ERROR] It was not possible execute the command in server\n" );
		return;
	}
	else{

		int status = remove(path);
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

void list_dir(int local){

	// Envia o comando para o servidor, para iniciar o upload
	struct Request *request = (struct Request*)malloc(sizeof(struct Request));
	request->cmd = local;
	strcpy(request->user, cli->userid);

	n = sendto(sockfd, request, sizeof(struct Request), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	//RECEIVE THE ACK FOR THE COMMAND
	n = recvfrom(sockfd,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &from, &length);
	if(request->cmd != ACK){
		fprintf(stderr, "[ERROR] It was not possible execute the command in server\n" );
		return;
	}
	else{
		if(local == LIST_CLIENT){
			show_files(cli, 0);		
		}
		else{
			CLIENT *serverClient = (CLIENT *) request->buffer;
			show_files(serverClient, 1);		
		}
	}
}



void close_session(){

		printf("closing conection with socket...\n");
		close(sockfd);
		printf("Session ended successfully! \n");
		exit(0);

}

void command_get_func()
{
	int start = 1;
	while(start){

		showMenu(cli);
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
				// get_file(directory); //TO DO
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
			case EXIT: break;
			default: printf("\nINVALID COMMAND \n"); break;
		}
		/*
		// SENDO MESSAGE OF THE USER COMMAND
		n = sendto(sockfd, user_cmd, strlen(user_cmd) -1, MSG_CONFIRM,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		// RECEIVE THE MESSAGE FROM THE SERVER ABOUT THE USER COMMAND
		n = recvfrom(sockfd, send_buffer, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &from, &length);
		fprintf(stderr, "%s\n", send_buffer);
		bzero(send_buffer, BUFFER_TAM);

		switch(code){
			case UPLOAD:  break;
			case DOWNLOAD: get_file(directory); break;
			case DELETE: delete_file(directory); break;
			case LIST_SERVER: break;
			case LIST_CLIENT: break;
			case GET_SYNC_DIR: break;
			case EXIT: close_session(); start = 0; break;
			case ERROR:
			default: printf("\nINVALID COMMAND \n"); break;
		}

		memset(user_cmd, 0, sizeof user_cmd);
		command_code = 0;
		bzero(send_buffer, BUFFER_TAM);
		*/
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

	cli = find_or_createClient(argv[1]);

	int login = login_server(argv[2], PORT);

	if(login)
	{
		int dir = get_sync_dir(argv[1]);
		if(dir == 0) // == 0 Diretório já existe, pode ser sincronizado
		 //sync_client(); //Sync client files with the server
		if(dir == -2) exit(dir);

		struct Request *request = (struct Request*)malloc(sizeof(struct Request));
		request->cmd = CONNECT;
		strcpy(request->user, cli->userid);

		//SEND USER ID TO SERVER
		n = sendto(sockfd, request, sizeof(struct Request), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		//RECEIVE THE ANSWER FROM THE SERVER
		n = recvfrom(sockfd,  request, sizeof(struct Request), MSG_CONFIRM, (struct sockaddr *) &from, &length);
		if(request->cmd != ACK) fprintf(stderr, "[ERROR] It was not possible connect to this server\n" );

		// Cria a thred de sincronização do diretório
		pthread_t sync_thread;

		//int rc = pthread_create(&sync_thread, NULL, sync_client, NULL);
		//if(rc)
		//	fprintf(stderr,"A request could not be processed\n");

		//LOOP para pegar o comando do client
		command_get_func();
	}
	return 0;
}
