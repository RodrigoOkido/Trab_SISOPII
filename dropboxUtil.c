/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "dropboxServer.h"

const char homeDir[] = "/tmp/sync_dir_";
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
    printf("[4] List my files stored (cmd: list_server) \n");
    printf("[5] List files stored in directory (cmd: sync_dir_<username>) \n");
    printf("[6] Create sync_dir_<username> folder in the user /home (cmd: get_sync_dir) \n");
    printf("[7] Exit (cmd: exit)\n\n");

}



void iniciateList(){
    total_client = 0;
    client_list = malloc(MAXCLIENTS * sizeof(CLIENT));
}



CLIENT* create_and_setClient(char* user_id) {

    CLIENT* newClient = malloc(sizeof(CLIENT)); //Create new client

    //Inicialization of this new Client.
    newClient->devices[0]= 0;
    newClient->devices[1]= 0;
    newClient->devices[2]= 0;
    char id [sizeof(user_id)];
    memcpy( id, &user_id[0], sizeof(user_id));
    strncpy(newClient->userid , id , sizeof(id));
    newClient->userid[UNIQUE_ID] = '\0';
    newClient->files_qty = 0;
    newClient->logged_in = 0;

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



int parseFile(char* file){

    int lastSlashIndex; //Variable to store the index of the last slash ('/')
    int pointIndex; //Variable to store the point to indicate the extension ('.')

    int i;

    //Locate the index of the last '/' and '.'
    for (i = 0 ; i < strlen(file) ; i++){
        if (file[i] == '/'){
          lastSlashIndex = i;
        }
        if (file[i] == '.'){
          pointIndex = i;
        }
    }

    int file_name_size = pointIndex-lastSlashIndex; //Give the size of the filename
    int file_ext = strlen(file) - pointIndex; //Give the size of extension

    char name [file_name_size];
    if (lastSlashIndex == 0){ //If the file is located on current directory
      memcpy( name, &file[lastSlashIndex], file_name_size); //Pass the filename to name
    } else { //else increment lastSlashIndex to start taking the file without '/'
      memcpy( name, &file[lastSlashIndex+1], file_name_size);
    }
    name[file_name_size] = '\0'; //Guarantee the end of the word

    char ext [file_ext];
    memcpy( ext, &file[pointIndex], file_ext); //Pass the extension name to ext
    ext[file_ext] = '\0'; //Guarantee the end of the word


    //Copies the name and extension to 'filename' and 'extension'
    strncpy(filename , name , sizeof(name));
    filename[MAXNAME-1] = '\0';
    strncpy(extension , ext , sizeof(ext));
    extension[EXT-1] = '\0';

    memset(name, 0, sizeof name);
    memset(ext, 0, sizeof ext);
}



int createNewFile(CLIENT* actualClient, int filesize){

    //Creating and preparing the new file
    FILE_INFO* newFile = malloc(sizeof(FILE_INFO));
    strncpy(newFile->name, filename, MAXNAME-1);
    newFile->name[MAXNAME-1] = '\0';
    strncpy(newFile->extension, extension, EXT);
    newFile->extension[EXT] = '\0';

    //Saving the actual time for the file recently created
    time_t rawtime;
    struct tm *info;

    time( &rawtime );
    info = localtime( &rawtime );
    strftime(newFile->last_modified, DATE,"%x - %I:%M%p", info);

    newFile->size = filesize;


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

        //Clears the array for the next file
        memset(filename, 0, sizeof filename);
        //Clears the array for the next file extension
        memset(extension, 0, sizeof extension);

        return 0;
    } else {
        //Clears the array for the next file
        memset(filename, 0, sizeof filename);
        //Clears the array for the next file extension
        memset(extension, 0, sizeof extension);

        return -1;
    }
}


int get_sync_dir(char* userId){
    //aux = MAXNAME + "./home/sync_dir_" => 272
    char aux[272];

    strcpy(aux,homeDir);
    strcat(aux, userId);

    //cria o diretório com permissão de leitura/escrita/execução
    int i = mkdir(aux, 0777);

    if(i == -1) return 0; //Pasta já existe
    if(i != 0) { printf("[ERROR] get_sync_dir() -> Ao criar a pasta %s (CODE -2)\n", aux); return -2; } // ERRO no mkdir

    return 1; //Pasta foi criada tudo OK

}
