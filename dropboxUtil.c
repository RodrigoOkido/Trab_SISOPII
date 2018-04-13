/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"


void showMenu(){
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
