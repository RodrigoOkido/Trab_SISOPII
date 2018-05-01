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

	return;

}



void send_file(char *file){

		FILE *sendFile = fopen(file, "rb");
	  int file_length;
		int packet, read_buffer;

		if (sendFile == NULL){
			printf("File not found..");
			return;
		}

		fseek(sendFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte number.
		file_length = ftell(sendFile); //Store the file length of the received file.
	  fseek(sendFile, 0, SEEK_SET); //Turn the pointer to the beginning.

		packet = 1;

		if (file_length < BUFFER_TAM) {

			n = sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
			if (n < 0)
				printf("ERROR sendto\n");

		} else {

			while(!feof(sendFile)) {

				read_buffer = fread(send_buffer, 1, sizeof(send_buffer)-1, sendFile);

				//Send data through our socket
				do{
					n = sendto(sockfd, send_buffer, read_buffer, MSG_CONFIRM, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
				} while(n < 0);

				printf("\n");
				printf("Packet Number: %i\n",packet);
				printf("Packet Size Sent: %i\n",read_buffer);
				printf("\n");

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
		if (argc < 2) {
			fprintf(stderr, "To connect use: ./dropbox {userid} {user_adress} {PORT}\n");
			exit(0);
		}

		int login = login_server(argv[1], PORT);

	  if(login){

				int start = 1;
				sync_client(); //Sync client files with the server


	      while(start){
	        showMenu();


	        printf("cmd > ");
	        fgets(user_cmd, sizeof(user_cmd), stdin);
					user_cmd[strlen(user_cmd) -1] = '\0';

					int code = parseCommand(user_cmd);
					char *directory; //Used only if the action is upload/download a file.
													 //Takes the directory of the file.

					if (code == 1){
						directory = strndup(user_cmd+7, strlen(user_cmd));
						fprintf(stderr, "%s\n", directory);
					}

					else if (code == 2){
						directory = strndup(user_cmd+9, strlen(user_cmd));
						fprintf(stderr, "%s\n", directory);
					}

					switch(code){
						case 1: send_file(directory); break;
						case 2: get_file(directory); break;
						case 3: break;
						case 4: break;
						case 5: break;
						case 6: break;
						case 7: close_session(); start = 0; break;
						default: printf("\nINVALID COMMAND \n"); break;
					}

					printf("\n\nPress enter...\n");
					char enter = 0;
					while (enter != '\r' && enter != '\n') {
						enter = getchar();
					}

	      }

	  }

		return 0;
}
