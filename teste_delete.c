#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/*#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/inotify.h>
#include <arpa/inet.h>
#include <time.h>*/

#define PORT 		4000
#define BUFFER_TAM 	256

#define MAXNAME		256
#define MAXFILES	20

#define UNIQUE_ID 10
#define EXT 6
#define DATE 18
#define MAXCLIENTS 150

//parseCommands 
#define UPLOAD  1
#define DOWNLOAD  2
#define DELETE  3
#define LIST_SERVER  4
#define LIST_CLIENT  5
#define GET_SYNC_DIR  6
#define EXIT  7
#define CONNECT 99
#define ERROR  -1

#define MAX_THREADS 100
#define ACK 100

#define DEBUG 1 //DEBUGGING PURPOSE

extern int command_code;
extern int mkdir();

// Local & Remote Diretory
static const char homeDir[] = "/tmp/sync_dir_";
static const char serverDir[] = "/home/dlazarosps/sync_dir_";


/**
  name[MAXNAME] refere-se ao nome do arquivo.
  extension[MAXNAME] refere-se ao tipo de extensão do arquivo.
  last_modified [MAXNAME] refere-se a data da última
  mofidicação no arquivo.
  size indica o tamanho do arquivo, em bytes.
*/
typedef struct file_info	{
	char name[MAXNAME];
	char extension[EXT];
	char last_modified[DATE];
	int	size;
} FILE_INFO;

/**
  int devices[2] – associado aos dispositivos do usuário
  userid[MAXNAME] – id do usuário no servidor, que deverá ser
  único. Informado pela linha de comando.
  file_info[MAXFILES] – metadados de cada arquivo que o cliente
  possui no servidor.
  logged_in – cliente está logado ou não.
*/
typedef struct client {
  int devices[2];
  char userid[UNIQUE_ID];
  FILE_INFO file_info[MAXFILES];
  int files_qty;
  int logged_in;
} CLIENT;

void show_files(CLIENT * client, int isServer);
void delete_info_file(CLIENT* client, char* namefile);

CLIENT * localClient; 
CLIENT * serverClient; 


CLIENT* create_and_setClient(char* user_id) {

    CLIENT* newClient = malloc(sizeof(CLIENT)); //Create new client

    //Inicialization of this new Client.
    newClient->devices[0]= 0;
    newClient->devices[1]= 0;
    newClient->devices[2]= 0;
    char id [sizeof(user_id)];
    memcpy(id, &user_id[0], sizeof(user_id));
    strncpy(newClient->userid , id , sizeof(id));
    newClient->userid[sizeof(id)+1] = '\0';
    newClient->files_qty = 2;
    newClient->logged_in = 0;

    return newClient;

}

void init_client(){
	/*
	{{0,1}, "server",(FILE_INFO *) &filesClient, 2, 1};
	{{0,1}, "local",(FILE_INFO *) &filesClient, 2, 0};
	*/
	localClient = create_and_setClient("local");
	serverClient = create_and_setClient("server");

}

void insert_files(){

	FILE_INFO* newFile = malloc(sizeof(FILE_INFO));
	FILE_INFO* File1 = malloc(sizeof(FILE_INFO));
	FILE_INFO* File2 = malloc(sizeof(FILE_INFO));
	// newFile = {'\0', '\0', '\0', -1};

	memset(newFile->name, 0, MAXNAME);
	memset(newFile->extension, 0, EXT);
	memset(newFile->last_modified, 0, DATE);
	newFile->size = -1;

	char * name = "teste";
	char * name2 = "teste2";
	char * ext = "txt";
	char * data = "15-05";

	// filesClient[0] = {"teste", "txt", "14-05", 8};
	strncpy(File1->name, name, sizeof(name));
	strncpy(File1->extension, ext, sizeof(ext));
	strncpy(File1->last_modified, data, sizeof(data));
	File1->size = 8;

	// filesClient[1] = {"teste2", "txt", "15-05", 8};
	strncpy(File2->name, name2, sizeof(name));
	strncpy(File2->extension, ext, sizeof(ext));
	strncpy(File2->last_modified, data, sizeof(data));
	File2->size = 8;


	int i;
	for (int i = 0; i < MAXFILES; ++i)
	{
		localClient->file_info[i] = *newFile;
		serverClient->file_info[i] = *newFile;
	}

	localClient->file_info[0] = *File1;
	serverClient->file_info[0] = *File1;

	localClient->file_info[1] = *File2;
	serverClient->file_info[1] = *File2;

}

void delete_file(char *file){

	//path = MAXNAME + "./tmp/sync_dir_" => 273
	char path[MAXNAME + sizeof(homeDir)];
	strcat(path, homeDir);
	strcat(path, localClient->userid);
	strcat(path, "/");
	strcat(path, file);

	int status = remove(path);	
	if(DEBUG) printf("\n sizeof: %i \t path: %s \t status: %i \n",(int) sizeof(path), path, status);

	if(status == 0){ //remove file cliente side
		delete_info_file(localClient, file);
		printf("File ( %s ) deleted sucessfully!\n", file);
	}
	else{
		printf("Error: unable to delete the %s file\n", file);
	}

	return;
}

void list_dir(int local){
	
	if(local == LIST_CLIENT){
		show_files(localClient, 0);		
	}
	else{
		show_files(serverClient, 1);		
	}
}

void delete_info_file(CLIENT* client, char* namefile){
  //atualiza quantidade de arquivos
  client->files_qty = client->files_qty - 1;

  char arquivo[MAXNAME+EXT+1];

  int i;
  //percorre array de arquivos para fazer match do nome
  for (i = 0; i < MAXFILES ; i++){

  	//concat name + . + ext => name.ext 
  	strcat(arquivo, client->file_info[i].name);
  	strcat(arquivo, ".");
  	strcat(arquivo, client->file_info[i].extension);


    if (strncmp(arquivo, namefile, sizeof(namefile)) == 0) {

      if(DEBUG) {
        fprintf(stderr,"CLIENT INDEX: %i \n",i);
      }      

      memset(client->file_info[i].name, 0, MAXNAME);
      memset(client->file_info[i].extension, 0, EXT);
      memset(client->file_info[i].last_modified, 0, DATE);
      client->file_info[i].size = -1;
    }
    memset(arquivo, 0, sizeof(arquivo)); //limpa string
  }
}

void show_files(CLIENT * client, int isServer){

  if(isServer){
    printf("Path: %s%s \n", serverDir, client->userid);
  }
  else{
    printf("Path: %s%s \n", homeDir, client->userid);
  }


  int i, j = 0;
  //percorre array de arquivos e printa
  for (i = 0; i < MAXFILES ; i++){

    if(client->file_info[i].size != 0 && client->file_info[i].size != -1){
      j++;
      printf("%i - \t name : %s \t extension: %s \t last_modified: %s \t size: %i \n", j, client->file_info[i].name, client->file_info[i].extension, client->file_info[i].last_modified, client->file_info[i].size);
    }
  }
  printf("files_qty: %i \n", client->files_qty);
}

int main(int argc, char const *argv[])
{
	init_client();
	insert_files();
	list_dir(LIST_CLIENT);
	printf(" \t ----- \n \n");
	list_dir(LIST_SERVER);
	printf(" \t ----- \n \n");

	delete_file("teste2.txt");
	// delete_info_file(localClient, "teste2");

	printf(" \t ----- \n \n");
	list_dir(LIST_CLIENT);

	return 0;
}