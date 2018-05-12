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



void sync_client() {

		int length, i = 0;
		char buffer[EVENT_BUF_LEN];

		while (1) { //fica verificando se alterou o diretorio
				length = read(notifyStart, buffer, EVENT_BUF_LEN);


				if (notifyStart < 0) {
						perror("inotify_init");
				}

				while ( i < length ) {

						struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];

						if ( event->len ) {
							if ( event->mask & IN_CREATE ) {
									printf( "New file %s created.\n", event->name );
							}

						else if ( event->mask & IN_DELETE  || event->mask & IN_MOVED_FROM) {
							printf( "File %s deleted.\n", event->name );
						}
						else if ( event->mask & IN_MODIFY || event->mask & IN_CLOSE_WRITE) {
							printf( "File %s modified.\n", event->name );
						}
						}

						i += EVENT_SIZE + event->len;
				}

		i = 0;
		sleep(10); //verificar a cada 10 segundos
	}
	return;

}



void send_file(char *file){

		FILE *sendFile = fopen(file, "rb");
		int packet, read_buffer; //packet - Counter of packets needed to send the file.
														 //read_buffer - buffer which will receive the content of the file.

		if (sendFile == NULL){
			printf("File not found..");
			return;
		}

		fseek(sendFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte number.
		file_length = ftell(sendFile); //Store the file length of the received file.
		fseek(sendFile, 0, SEEK_SET); //Turn the pointer to the beginning.

		char *filesize = "475";


		//SEND MESSAGE TO CHECK THE FILE LENGTH
		n = sendto(sockfd, filesize, sizeof(filesize), MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		n = recvfrom(sockfd, send_buffer, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &from, &length);
		fprintf(stderr, "%s\n", send_buffer);
		bzero(send_buffer, BUFFER_TAM);


		//SEND MESSAGE TO CHECK THE FILENAME AND EXTENSION.
		n = sendto(sockfd, file, strlen(file), MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
		n = recvfrom(sockfd, send_buffer, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &from, &length);
		fprintf(stderr, "%s\n", send_buffer);
		bzero(send_buffer, BUFFER_TAM);

		fprintf(stderr, "FILE LENGTH %i\n", file_length);
		packet = 1; //Packet number.

		if (file_length < BUFFER_TAM) {
				read_buffer = fread(send_buffer, 1, file_length, sendFile);
				n = sendto(sockfd, send_buffer, read_buffer, MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));

				if (n < 0)
						printf("ERROR sendto\n");

				if(DEBUG){
						printf("\n");
						printf("Packet Number: %i\n",packet);
						printf("Packet Size Sent: %i\n",read_buffer);
						printf("\n");
				}

				bzero(send_buffer, sizeof(send_buffer));
		} else {

				while(!feof(sendFile)) {

						read_buffer = fread(send_buffer, 1, sizeof(send_buffer)-1, sendFile);

						//Send data through our socket
						do {
								n = sendto(sockfd, send_buffer, read_buffer, MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
						} while(n < 0);


						if(DEBUG){
								printf("\n");
								printf("Packet Number: %i\n",packet);
								printf("Packet Size Sent: %i\n",read_buffer);
								printf("\n");
						}

						packet++;

						//Zero out our send buffer
						bzero(send_buffer, sizeof(send_buffer));
				}

		}


		length = sizeof(struct sockaddr_in);
		n = recvfrom(sockfd, send_buffer, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &from, &length);

		if (n < 0)
			printf("ERROR recvfrom\n");

		printf("File ( %s ) uploaded sucessfully!\n", file);
		file_length = 0;

		fclose(sendFile);

		close(sockfd);

}



void get_file(char *file){

  return;

}



void delete_file(char *file){

  return;

}



void close_session(){

		printf("closing conection with socket...\n");
		close(sockfd);
		printf("Session ended successfully! \n");
		exit(0);

}



int main(int argc, char *argv[]){
		if (argc < 4) {
			fprintf(stderr, "[ERROR] Use the following syntax : ./dropboxClient {userid} {user_adress} {PORT}\n");
			exit(0);
		}

		cli = find_or_createClient(argv[1]);

		int login = login_server(argv[2], PORT);

	  if(login){

				int start = 1;
				int dir = get_sync_dir(argv[1]);
				if(dir == 0) // == 0 Diretório já existe, pode ser sincronizado
				 //sync_client(); //Sync client files with the server
				if(dir == -2) exit(dir);

				//SEND USER ID TO SERVER
				n = sendto(sockfd, cli->userid, UNIQUE_ID, MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
				//RECEIVE THE ANSWER FROM THE SERVER
				n = recvfrom(sockfd, send_buffer, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &from, &length);
				fprintf(stderr, "%s\n", send_buffer);
				bzero(send_buffer, BUFFER_TAM);

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
								break;
							case DOWNLOAD: 
								directory = strndup(user_cmd+9, strlen(user_cmd));
								fprintf(stderr, "%s\n", directory);
								break;
							case DELETE: 
								directory = strndup(user_cmd+7, strlen(user_cmd));
								fprintf(stderr, "%s\n", directory);
								break;
							case LIST_SERVER: break;
							case LIST_CLIENT: break;
							case GET_SYNC_DIR: break;
							case EXIT: break;
							default: printf("\nINVALID COMMAND \n"); break;
						}

						// SENDO MESSAGE OF THE USER COMMAND
						n = sendto(sockfd, user_cmd, strlen(user_cmd) -1, MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
						// RECEIVE THE MESSAGE FROM THE SERVER ABOUT THE USER COMMAND
						n = recvfrom(sockfd, send_buffer, BUFFER_TAM, MSG_CONFIRM, (struct sockaddr *) &from, &length);
						fprintf(stderr, "%s\n", send_buffer);
						bzero(send_buffer, BUFFER_TAM);

						switch(code){
							case UPLOAD: send_file(directory); break;
							case DOWNLOAD: get_file(directory); break;
							case DELETE: break;
							case LIST_SERVER: break;
							case LIST_CLIENT: break;
							case GET_SYNC_DIR: break;
							case EXIT: close_session(); start = 0; break;
							case ERROR:
							default: printf("\nINVALID COMMAND \n"); break;
						}

						memset(user_cmd, 0, sizeof user_cmd);
						command_code = 0;

						printf("\n\nPress enter...\n");
						char enter = 0;
						while (enter != '\r' && enter != '\n') {
							enter = getchar();
						}

	      }

	  }

		return 0;
}
