/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/inotify.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>

#define DEBUG       1 //DEBUGGING PURPOSE
#define PORT        4000
#define BUFFER_TAM  256

#define MAXNAME     256
#define MAXFILES    20
#define MAXDEVICES  2

#define UNIQUE_ID   10
#define EXT         6
#define DATE        18
#define MAXCLIENTS  10

//parseCommands
#define UPLOAD        1
#define DOWNLOAD      2
#define DELETE        3
#define LIST_SERVER   4
#define LIST_CLIENT   5
#define GET_SYNC_DIR  6
#define EXIT          7
#define CONNECT       99
#define ERROR         -1

#define MAX_THREADS   100
#define ACK           100


extern int command_code;
extern int mkdir();

// Local & Remote Diretory
static const char homeDir[] = "/tmp/sync_dir_";
static const char serverDir[] = "/tmp/SERVER/sync_dir_";

/**
  name[MAXNAME] refere-se ao nome do arquivo.
  extension[MAXNAME] refere-se ao tipo de extensão do arquivo.
  last_modified [MAXNAME] refere-se a data da última
  mofidicação no arquivo.
  size indica o tamanho do arquivo, em bytes.
*/
typedef struct file_info  {
  char name[MAXNAME];
  char extension[EXT];
  char last_modified[DATE];
  int size;
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
  int devices[MAXDEVICES];
	struct sockaddr_in serv_request, serv_response;
	int sockreq;
	int sockaction;
  char userid[UNIQUE_ID];
  FILE_INFO file_info[MAXFILES];
  int files_qty;
  int logged_in;
} CLIENT;


typedef struct Request {
  int cmd;
  char user[MAXNAME];
  char buffer[BUFFER_TAM];
} REQUEST;

typedef struct File_package {
  char name[MAXNAME+EXT+1];
  char extension[MAXNAME];
  int size;
  int package;
  char buffer[BUFFER_TAM];
} FILE_PACKAGE;

#ifndef _DROPBOXUTIL_H_
#define _DROPBOXUTIL_H_
extern int file_length,total_client;
extern CLIENT client_list[MAXCLIENTS];

#endif



//const char homeDir[] = "./temp/sync_dir_"; //===> ALTERAR PARA /home/sync_dir_ <=======//

/**
  Show the menu for the client when connected.
*/
void showMenu();



/**
 * Iniciate the list of clients.
 */
void iniciateList();



/**
  Create a client and set his session configs. Called only if userID is new.
*/
CLIENT* create_and_setClient(char* user_id, int isServer);



/**
  Search if the client exists on the list clients. If 0,
  the user will be able to login with all your changes made in
  the session before (if changes was made).

  @param userId, actualClient
  @return >= 0 The client exist and is set (Returning the index of him in the list)
  @return < 0 The client dont exist.
*/
CLIENT* find_or_createClient(char* userid);



/**
  Check the input of the user on the client side. This is called to check what
  action the user wants to do.

  @param cmd[] The input command received
  @return Return the code of the command wanted
*/
int parseCommand(char cmd[]);



/**
  Parse the file which user wants to send to the server.

  @param file The file which will be send to server
  @return return 0 - OK
  @return return < 0 - SOMETHING WRONG
*/
int parseFile(struct File_package* file);



/**
	Delete the file.

	@param client The client which want to delete.
	@param nameFile The filename wanted to delete. 
*/
void delete_info_file(CLIENT* client, char* namefile);



/**
 * Create a new file. This function is called when the user wants to upload
 * some file.
 *
 * @return return 0 - new file created successfully
 * @return return < 0 - something wrong
 */
int createNewFile(CLIENT* actualClient, struct File_package* file);



/**
  Cria um diretório na máquina referente ao usuário
  @param userId
  @return >  0 - operação OK
  @return == 0 - Pasta já existe
  @return <  0 - ERROR
*/
int get_sync_dir(char* userId);

void show_files(CLIENT * client, int isServer);

int copy_file(char *old_filename, char  *new_filename);

int logged_device(CLIENT* client, int io);
