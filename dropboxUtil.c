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
  printf("[3] List my files stored (cmd: list_server) \n");
  printf("[4] List files stored in directory (cmd: sync_dir_<username>) \n");
  printf("[5] Create sync_dir_<username> folder in the user /home (cmd: get_sync_dir) \n");
  printf("[6] (TEMPORARIO) Teste de mensagem cliente/servidor (cmd: test) <= SO FUNCIONA ESSE POR ENQUANTO!\n");
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


int createUserDir(char* userId){
    //aux = MAXNAME + "./home/sync_dir_" => 272
    char aux[272];

    strcpy(aux,homeDir);
    strcat(aux, userId);

    //cria o diretório com permissão de leitura/escrita/execução
    int i = mkdir(aux, 0700);

    if(i == -1) return 0; //Pasta já existe
    if(i != 0) { printf("[ERROR] createUserDir() -> Ao criar a pasta %s (CODE -2)\n", aux); return -2; // ERRO no mkdir

    return 1; //Pasta foi criada tudo OK
}