/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"


int total_client;
CLIENT* client_list;


void showMenu() {

    system("clear");
    printf(">>>>> Welcome to the Dropbox! <<<<<\n\n");
    printf("[1] Upload a file (cmd: upload <path/filename.ext)\n");
    printf("[2] Download a file (cmd: download <filename.ext>)\n");
    printf("[3] Delete a file (cmd: delete <filename.ext>)\n");
    printf("[4] List my files stored (cmd: list_server) \n");
    printf("[5] List files stored in directory (cmd: sync_dir_<username>) \n");
    printf("[6] Create sync_dir_<username> folder in the user /home (cmd: get_sync_dir) \n");
    printf("[7] Exit (cmd: exit)\n\n");

}



void iniciateList(){

    total_client = 0;
    client_list = malloc(MAXCLIENTS * sizeof(CLIENT));

}



void createClient(char* user_id) {

    CLIENT* newClient = malloc(sizeof(CLIENT));
    newClient->devices[0]= 0;
    newClient->devices[1]= 0;
    newClient->devices[2]= 0;
    newClient->userid[sizeof(newClient->userid)] = *user_id;
    newClient->logged_in = 0;

    client_list[total_client] = *newClient;
    total_client++;

}



int searchClient(char* userid) {

    int i;
    for (i = 0; i < total_client ; i++){
      if (strcmp(client_list[i].userid, userid) == 0) {
        return 1;
      }
    }

    return 0;
}



int parseCommand (char cmd[]){

  	if(strncmp(cmd,"upload",6) == 0){
  		//printf("UPLOAD CODE\n");
  		command_code = 1;
  	}
  	else if(strncmp(cmd,"download",8) == 0){
  		//printf("DOWNLOAD CODE\n");
  		command_code = 2;
    }
    else if(strncmp(cmd,"delete",6) == 0){
      //printf("DELETE CODE\n");
      command_code = 3;
    }
  	else if(strncmp(cmd,"list_server",11) == 0){
  		//printf("LIST_SERVER CODE\n");
  		command_code = 4;
  	}
  	else if(strncmp(cmd,"list_client",11) == 0){
  		//printf("LIST_CLIENT CODE\n");
  		command_code = 5;
  	}
  	else if(strncmp(cmd,"get_sync_dir",11) == 0){
  		//printf("GET_SYNC_DIR CODE\n");
  		command_code = 6;
  	}
  	else if(strncmp(cmd,"exit",4) == 0){
  		//printf("EXIT CODE\n");
  		command_code = 7;
  	}
  	else {
  		//printf("ERROR CODE\n");
  		command_code = -1;
  	}

  	return command_code;
}



int parseFile(char* file){

    int lastSlashIndex; //Variable to store the index of the last slash ('/')
    int pointIndex; //Variable to store the point to indicate the extension ('.')

    int i;
    for (i = 0 ; i < strlen(file) ; i++){
        if (file[i] == '/'){
          lastSlashIndex = i;
        }
        if (file[i] == '.'){
          pointIndex = i;
        }
    }

    int file_name_size = pointIndex-lastSlashIndex;
    int file_ext = sizeof(file) - pointIndex;

    char name [file_name_size];
    if (lastSlashIndex == 0){
      memcpy( name, &file[lastSlashIndex], file_name_size); //Pass the filename to name
    } else {
      memcpy( name, &file[lastSlashIndex+1], file_name_size);
    }
    name[file_name_size] = '\0'; //Garantee the end of the word

    char ext [file_ext];
    memcpy( ext, &file[pointIndex+1], file_ext); //Pass the extension name to ext
    ext[file_ext] = '\0'; //Garantee the end of the word

    strncpy(filename , name , sizeof(name));
    strncpy(extension , ext , sizeof(ext));

}



int createUserDir(char* userId){
    //aux = MAXNAME + "./home/sync_dir_" => 272
    char aux[272];

    //strcpy(aux,homeDir);
    //strcat(aux, userId);

    //cria o diretório com permissão de leitura/escrita/execução
    //int i = mkdir(aux, 0700);

    //if(i == -1) return 0; //Pasta já existe
    //if(i != 0) { printf("[ERROR] createUserDir() -> Ao criar a pasta %s (CODE -2)\n", aux); return -2; // ERRO no mkdir

    //return 1; //Pasta foi criada tudo OK

}
