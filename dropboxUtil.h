/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 		4000
#define BUFFER_TAM 	256

#define MAXNAME		256
#define MAXFILES	20

#define UNIQUE_ID 10
#define EXT 4
#define DATE 20
#define MAXCLIENTS 150

//const char homeDir[] = "./temp/sync_dir_"; //===> ALTERAR PARA /home/sync_dir_ <=======//

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



void showMenu();
void createClient();
int searchClient();
int parseCommand(char cmd[]);

/**
  Cria um diretório na máquina referente ao usuário
  @param userId
  @return >  0 - operação OK
  @return == 0 - Pasta já existe
  @return <  0 - ERROR
*/
int createUserDir(char* userId);
