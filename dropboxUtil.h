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

#define PORT 		4000
#define BUFFER_TAM 	256

#define MAXNAME		256
#define MAXFILES	20

#define UNIQUE_ID 10
#define EXT 5
#define DATE 18
#define MAXCLIENTS 150


int total_client;


//USER INPUT COMMAND CODE
int command_code; //Store the command code of the action wanted by user.


//FILE VARIABLES CONTROLLER
char filename[MAXNAME]; //Char array to take the name of the file created (activated in PARSEFILE() function below).
char extension[EXT]; //Pointer to get the extension of the file created (activated in PARSEFILE() function below).
int file_length; //Stores the length of the new file.

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
typedef struct client	{
  int devices[2];
  char userid[UNIQUE_ID];
  FILE_INFO file_info[MAXFILES];
  int logged_in;
} CLIENT;



/**
	Show the menu for the client when connected.
*/
void showMenu();



/**
 * Iniciate the list of clients.
 */
void iniciateList();



/**
	Create a client and set his session configs. Creates only if userID is new.
*/
void create_and_setClient();



/**
	Search if the client exists on the list clients. If 0,
	the user will be able to login with all your changes made in
	the session before (if changes was made).


	@param userId
	@return >= 0 The client exist and is set (Returning the index of him in the list)
	@return < 0 The client dont exist.
*/
int search_and_setClient(char* userid);



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
int parseFile(char* File);



/**
 * Create a new file.
 *
 * @return return 0 - new file created successfully
 * @return return < 0 - something wrong
 */
int createNewFile();



/**
  Cria um diretório na máquina referente ao usuário
  @param userId
  @return >  0 - operação OK
  @return == 0 - Pasta já existe
  @return <  0 - ERROR
*/
int get_sync_dir(char* userId);
