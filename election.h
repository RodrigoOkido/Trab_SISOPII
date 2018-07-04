/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

/************************************************************************************
-----------------------| Interface de eleição e controle |---------------------------
	
	A interface que será utilizada pelos server fornce:
		- uma forma de reconhecer quem é o server primario
		- verificar se existe comunicação com o server primário
		- iniciar eleição caso o primario não responda
		- receber e enviar as atualizações do backups

************************************************************************************/

#define MAXSERVERS 5

#define PRIMARY 1
#define BACKUP 2

#define NEWUP	1
#define OK		0

#define TRUE	1
#define FALSE	-1

#define FSERVER		10
#define FCLIENT		11
#define FFILES		12
#define ALIVE 		13

#define S_CONNECT		51
#define S_ANSWER		52
#define START_ELECTION		53
#define NEW_SERVER_LIST		54

typedef struct {
	int type;
	char *address;
} THREAD_ARG, *p_THREAD_ARG;

typedef struct server {
	int type;
	int id;
	struct sockaddr_in serv_address; //Ja possuiu porta e address?
	int sockfd;
	//int port;
	int flag_update_list_server;
	int flag_update_list_clients;
	int flag_update_files;
} SERVER;

typedef struct request_server{
	int cmd;
	int id_from;
	struct sockaddr_in address;
} REQUEST_SERVER;

typedef struct election{
	int elected;
	int id_from;
	int id_max;
} ELECTION;


void initConfigOfElection(int type, char* add);

void insertPrimaryServerOnList();

void initMyServerInfo(int type, char* add);

void init_election_state(ELECTION *election_req);

/**
  Função para resolver os comandos recebidos dos outros SERVERs da rede
*/
void *receiveCommands();

void *alive_test();

void setNewPrimary(int id);

/** --- Primario ---
  Realiza o registro da existencia de um novo server
*/
int connectNewServer();

void sendListOfServers(int to_pos);

void sendListOfClients(int to_server);

void sendToAllServerList(int menos_esse);

/**
  Envia para o server primario as informações
*/
int connectToPrimary(char *add);;

/**
  Envia informações do novo servers nos backups
*/
int registerNewServerInBackups();

void sendUpdatesToServer(int to);

void setFlag(int flag);

void debugPrintListServers();

void debugPrintListClients();
