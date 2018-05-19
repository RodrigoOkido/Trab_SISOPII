/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxServer.h"

int total_client;

//USER INPUT COMMAND CODE
int command_code; //Store the command code of the action wanted by user.

//FILE VARIABLES CONTROLLER
char filename[MAXNAME]; //Char array to take the name of the file created (activated in PARSEFILE() function below).
char extension[EXT]; //Pointer to get the extension of the file created (activated in PARSEFILE() function below).
int file_length; //Stores the length of the new file.


CLIENT* client_list;
int listActivated = 0;


void showMenu(CLIENT* actualClient) {

    system("clear");
    printf(">>>>> Welcome to the Dropbox! [USER: %s] <<<<<\n\n", actualClient->userid);
    printf("[1] Upload a file (cmd: upload <path/filename.ext)\n");
    printf("[2] Download a file (cmd: download <filename.ext>)\n");
    printf("[3] Delete a file (cmd: delete <filename.ext>)\n");
    printf("[4] List my files stored in server (cmd: list_server) \n");
    printf("[5] List files stored in directory (cmd: list_client) \n");
    printf("[6] Create sync_dir_<username> folder in the user /home (cmd: get_sync_dir) \n");
    printf("[7] Exit (cmd: exit)\n\n");

}

void iniciateList(){
    total_client = 0;
    client_list = malloc(MAXCLIENTS * sizeof(CLIENT));
}

CLIENT* create_and_setClient(char* user_id) {

    CLIENT* newClient = malloc(sizeof(CLIENT)); //Create new client
    FILE_PACKAGE *fileReceive = (FILE_PACKAGE*)malloc(sizeof(FILE_PACKAGE));
    FILE *readFile;
    int count;

    //Inicialization of this new Client.
    newClient->devices[0]= 0;
    newClient->devices[1]= 0;
    newClient->devices[2]= 0;
    char id [sizeof(user_id)];
    memcpy( id, &user_id[0], sizeof(user_id));
    strncpy(newClient->userid , id , sizeof(id));
    newClient->userid[UNIQUE_ID] = '\0';
    newClient->logged_in = 0;


    char path[MAXNAME + sizeof(serverDir)]; //folder
    char aux[MAXNAME + sizeof(serverDir) + EXT + 1]; //file
    memset(path, 0, sizeof(path));

    strcat(path, serverDir);
    strcat(path,user_id);
    strcat(path, "/");

    DIR* clientDir = opendir(path);
    struct dirent *ent;

    if(clientDir){
      //existe diretório do cliente no servidor
      if(DEBUG) printf("exists directory  %s \n", path);

      /* print all the files and directories within directory */
      while ((ent = readdir (clientDir)) != NULL) {

        if ((strncmp(ent->d_name, ".", sizeof(".")) == 0) || (strncmp(ent->d_name, "..", sizeof("..")) == 0)){
          if(DEBUG) printf (" pula %s\n", ent->d_name);
        }
        else{
          if(count <= MAXFILES){ // Lê arquivos até o limite maximo de arquivos
            if(DEBUG) printf ("%s\n", ent->d_name);

            /* Leitura de arquivos da pasta */
            strcpy(fileReceive->name, ent->d_name); //concatena nome do arquivo
            parseFile(fileReceive); //parse para file_info

            //concat path + file para leitura
            memset(aux, 0, sizeof(aux));
            strcat(aux, path);
            strcat(aux, "/");
            strcat(aux, ent->d_name);

            readFile = fopen(aux,"r"); //abre o arquivo para leitura

            fseek(readFile, 0, SEEK_END); //Seek the pointer to the end of the file to check the last byte number.
            file_length = ftell(readFile); //Store the file length of the received file.
            fseek(readFile, 0, SEEK_SET); //Turn the pointer to the beginning.

            fclose(readFile); // fecha o arquivo

            fileReceive->size = file_length; //copy size to struct
            fileReceive->package = 0; //set package to zero

            createNewFile(newClient, fileReceive); //cria o file_info e adiciona na struct cliente

            //TODO sync OR get_file => local folder 
            count++;
          }

        }

      }
      closedir (clientDir);
    } 
    else{
      //não existe diretório do cliente no servidor

      //TODO permissão /home
      int i = mkdir(path, 0777);
      if(i == -1) printf("[ERROR] create directory  %s \n", path); //Pasta já existe
      if(i != 0) printf("[ERROR] create directory  %s \n", path); // ERRO no mkdir
      newClient->files_qty = 0;
    }


    //Put the new Client in the list of clients
    client_list[total_client] = *newClient;

    //Set actual Client for this one. The actual indicates the current
    //user logged in Dropbox.
    return &client_list[total_client];
    total_client++; //Increment the total of the Clients after created.
}


CLIENT* find_or_createClient(char* userid) {

    int i;

    if(listActivated == 0){
      iniciateList();
      listActivated = 1;
    }

    for (i = 0; i < total_client ; i++){
        if (strncmp(client_list[i].userid, userid, sizeof(userid)) == 0) {
            if(DEBUG) {
                fprintf(stderr,"CLIENT INDEX: %i",i);
            }
            return &client_list[i];
        }
    }

    if (MAXCLIENTS == total_client){
      fprintf(stderr, "Unable to create more clients\n");
      return NULL;
    } else {
        return create_and_setClient(userid);
    }
}

int parseCommand (char cmd[]){

  	if(strncmp(cmd,"upload",6) == 0){
  		//printf("UPLOAD CODE\n");
  		command_code = UPLOAD;
  	}
  	else if(strncmp(cmd,"download",8) == 0){
  		//printf("DOWNLOAD CODE\n");
  		command_code = DOWNLOAD;
    }
    else if(strncmp(cmd,"delete",6) == 0){
      //printf("DELETE CODE\n");
      command_code = DELETE;
    }
  	else if(strncmp(cmd,"list_server",11) == 0){
  		//printf("LIST_SERVER CODE\n");
  		command_code = LIST_SERVER;
  	}
  	else if(strncmp(cmd,"list_client",11) == 0){
  		//printf("LIST_CLIENT CODE\n");
  		command_code = LIST_CLIENT;
  	}
  	else if(strncmp(cmd,"get_sync_dir",11) == 0){
  		//printf("GET_SYNC_DIR CODE\n");
  		command_code = GET_SYNC_DIR;
  	}
  	else if(strncmp(cmd,"exit",4) == 0){
  		//printf("EXIT CODE\n");
  		command_code = EXIT;
  	}
  	else {
  		//printf("ERROR CODE\n");
  		command_code = ERROR;
  	}

  	return command_code;
}


int parseFile(struct File_package* file){

    char *word = strtok(file->name,"/");
    char auxFile[MAXNAME];
    while(word){
        strcpy(auxFile, word);
        word = strtok(NULL, "/");
    }

    char *token = strtok(auxFile, ".");
    strcpy(file->name, token);
    token = strtok(NULL, ".");
    strcpy(file->extension, token);

    return 0;
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



int createNewFile(CLIENT* actualClient, struct File_package* file){

    //Creating and preparing the new file
    FILE_INFO* newFile = malloc(sizeof(FILE_INFO));
    strncpy(newFile->name, file->name, MAXNAME-1);
    newFile->name[MAXNAME-1] = '\0';
    strncpy(newFile->extension, file->extension, EXT);
    newFile->extension[EXT] = '\0';

    //Saving the actual time for the file recently created
    time_t rawtime;
    struct tm *info;

    time( &rawtime );
    info = localtime( &rawtime );
    strftime(newFile->last_modified, DATE,"%x - %I:%M%p", info);

    newFile->size = file->size;


    //Checks if actualClient exists.
    if (actualClient != NULL){
        actualClient->file_info[actualClient->files_qty] = *newFile;
        actualClient->files_qty++;

        if(DEBUG){
            fprintf(stderr,"NAME : |%s|\n", actualClient->file_info->name );
            fprintf(stderr,"EXTENSION : |%s|\n", actualClient->file_info->extension );
            fprintf(stderr,"TIME : |%s|\n", actualClient->file_info->last_modified ); // MES/DIA/ANO HORA:MIN
            fprintf(stderr,"SIZE : |%i| bytes\n", actualClient->file_info->size );
            fprintf(stderr,"QTD FILES : |%i|\n", actualClient->files_qty );
        }

        return 0;
    } else {

        return -1;
    }
}


int get_sync_dir(char* userId){
    char aux[MAXNAME + sizeof(homeDir)];
    memset(aux, 0, sizeof(aux));

    strcat(aux,homeDir);
    strcat(aux, userId);

    //cria o diretório com permissão de leitura/escrita/execução
    int i = mkdir(aux, 0777);

    if(i == -1) return 0; //Pasta já existe
    if(i != 0) { printf("[ERROR] get_sync_dir() -> Ao criar a pasta %s (CODE -2)\n", aux); return -2; } // ERRO no mkdir

    return 1; //Pasta foi criada tudo OK

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

  if(j == 0) printf("\t no files in struct \n \n"); 
}
