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
char user_cmd[50];



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

  //TODO

}


void send_file(char *file){

	FILE *sendFile = fopen(file, "rb");
  int file_length;
	int packet, read_buffer;

	if (sendFile == NULL){
		printf("File not found..");
		return;
	}

	fseek(sendFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte.
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

			printf("Packet Number: %i\n",packet);
			printf("Packet Size Sent: %i\n",read_buffer);
			printf(" \n");
			printf(" \n");


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

	printf("Press enter...\n");
	char enter = 0;
	while (enter != '\r' && enter != '\n') { enter = getchar(); }
	close(sockfd);

}


void get_file(char *file){

  //TODO

}


void delete_file(char *file){

  //TODO

}


void close_session(){

  //TODO

}


//FUNCAO TEMPORARIA (EXEMPLO PROFESSOR)
void test(){

  printf("\n\nEnter the message: ");
  bzero(send_buffer, BUFFER_TAM);
  fgets(send_buffer, sizeof(send_buffer), stdin);

  n = sendto(sockfd, send_buffer, strlen(send_buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
  if (n < 0)
    printf("ERROR sendto\n");

  length = sizeof(struct sockaddr_in);
  n = recvfrom(sockfd, send_buffer, BUFFER_TAM, 0, (struct sockaddr *) &from, &length);
  if (n < 0)
    printf("ERROR recvfrom\n");


	printf("Got packet from %s\n",inet_ntoa(serv_addr.sin_addr));
  printf("Packet is %d bytes long\n",n);
  printf("Got an ack: %s\n", send_buffer);

	printf("Press enter...\n");
	char enter = 0;
	while (enter != '\r' && enter != '\n') { enter = getchar(); }
  close(sockfd);

}


int main(int argc, char *argv[]){
	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);
	}

	int login = login_server(argv[1], PORT);

  if(login){
      int start = 1;

      while(start){
        showMenu();


        printf("cmd > ");
        fgets(user_cmd, sizeof(stdin), stdin);

				if(strncmp(user_cmd,"upload",6)== 0){
          send_file("test.txt");
        }

        if(strncmp(user_cmd,"test",4)== 0){
          test();
        }

        else if(strncmp(user_cmd,"exit",4) == 0){
          start = 0;
        }

        else {
            printf("INVALID COMMAND \n");
            printf("Press enter...\n");
            char enter = 0;
            while (enter != '\r' && enter != '\n') { enter = getchar(); }

        }


      }
      printf("SESSION ENDED! \n");
  }

	return 0;
}
