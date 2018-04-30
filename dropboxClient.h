/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


/**
  Estabelece uma sessão entre o cliente com o servidor.
  @param host endereço do servidor
  @param port porta do servidor
*/
int login_server(char *host, int port);


/**
  Sincroniza o diretório “sync_dir_<nomeusuário>” com
  o servidor.
*/
void sync_client();


/**
  Envia um arquivo file para o servidor. Deverá ser
  executada quando for realizar upload de um arquivo.
  @param file –filename.ext do arquivo a ser enviado
*/
void send_file(char *file);


/**
  Obtém um arquivo file do servidor.
  Deverá ser executada quando for realizar download
  de um arquivo.
  @param file – filename.ext
*/
void get_file(char *file);


/**
  Exclui um arquivo file de “sync_dir_<nomeusuário>”.
  @param file –filename.ext
*/
void delete_file(char *file);


/**
  Fecha a sessão com o servidor.
*/
void close_session();
