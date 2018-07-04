/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

#include "dropboxUtil.h"
#include "election.h"
#include <pthread.h>
#include <sys/time.h>

struct timeval timeout={2,0};
struct timeval no_timeout={0,0};

int isPrimary, electionStart = FALSE;

int aux, count, server_count = 0;	

SERVER myServerInfo;

SERVER server_list[MAXSERVERS];

pthread_t threads_r[2];

char* endereco = "143.54.6.26";

void initConfigOfElection(int type, char* add){
	if(DEBUG) printf("[Controle] ELECTION PROGRAM");
	if(DEBUG) printf("type = %d \n", type);
	if(type == BACKUP){
		initMyServerInfo(type, add);
		int res = connectToPrimary(add);

		pthread_create(&threads_r[0], NULL, receiveCommands, NULL);

		pthread_create(&threads_r[1], NULL, alive_test, NULL);
	}
	if(type == PRIMARY){
		isPrimary = 1;
		initMyServerInfo(type, add);
		insertPrimaryServerOnList();

		pthread_create(&threads_r[0], NULL, receiveCommands, NULL);
		
		pthread_create(&threads_r[1], NULL, alive_test, NULL);
	}
}

//-----------------------| Função do servidor primario |-------------------------

void insertPrimaryServerOnList(){
	//server_list[0] = (SERVER*)malloc(sizeof(SERVER));
	server_list[0] = myServerInfo;
	server_count++;
	for(int i = server_count; i < MAXSERVERS; i++){
		server_list[i].type = -1;
	}
}

// Inicia informaçõe do server e cria um novo socket que será utilizado para comunicação entre servers
void initMyServerInfo(int type, char* add){
	if(DEBUG) printf("[Controle] initMyServerInfo\n");
	//myServerInfo = (SERVER*)malloc(sizeof(SERVER));
	myServerInfo.type = type;
	myServerInfo.id = 0;

	// crina novo socket
	if ((myServerInfo.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket");

	// inicializa a estrutura de endereço dele
	myServerInfo.serv_address.sin_family = AF_INET;
	myServerInfo.serv_address.sin_port = htons(PORT+type); //4001 ou 4002
	myServerInfo.serv_address.sin_addr.s_addr = inet_addr(endereco);
	bzero(&(myServerInfo.serv_address.sin_zero), 8);

	printf("\n\nReceived packet from %s:%d\n", inet_ntoa(myServerInfo.serv_address.sin_addr), ntohs(myServerInfo.serv_address.sin_port));

	if (bind(myServerInfo.sockfd, (struct sockaddr *) &myServerInfo.serv_address, sizeof(struct sockaddr)) < 0)
		printf("ERROR on binding");
}

//-----------------------| Função do servidor primario |-------------------------
void *receiveCommands()
{
	while(1)
	{	
		if(DEBUG) printf("[Controle] receiveCommands\n");
		socklen_t len;
		struct sockaddr_in addr_request;
	
		REQUEST_SERVER *req_from_server = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
	
		if(DEBUG) printf("[Controle] receiveCommands - wait receive\n");
		aux = recvfrom(myServerInfo.sockfd, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
		if(aux < 0) printf("ERROR receiveCommands");
	
		if(req_from_server->cmd == S_CONNECT)
		{
			if(DEBUG) printf("[Controle] receiveCommands - Connect new server \n");
			printf("\n\nconnect Received packet from %s:%d\n", inet_ntoa(req_from_server->address.sin_addr), ntohs(req_from_server->address.sin_port));
			int pos = connectNewServer(req_from_server->address);
			
			printf("\n\nconnect Received packet from %s:%d\n", inet_ntoa(server_list[pos].serv_address.sin_addr), ntohs(server_list[pos].serv_address.sin_port));
			if(DEBUG) printf("[Controle] receiveCommands - send new server id: %d\n",server_list[pos].id);
			aux = sendto(myServerInfo.sockfd, &server_list[pos].id, sizeof(server_list[pos].id), 0,(struct sockaddr *) &server_list[pos].serv_address, sizeof(struct sockaddr));
			if(aux<0) printf("[ERROR] in package\n");
	
			sendUpdatesToServer(pos);

			sendToAllServerList(pos);
		}
		else if(req_from_server->cmd == START_ELECTION){
			printf("[receiveCommands] - Election request receive\n");
			electionStart = TRUE;
			
			//envia ack
			req_from_server->cmd = S_ANSWER;
			printf("[receiveCommands] - envia ACK\n");
			aux = sendto(myServerInfo.sockfd, &req_from_server, sizeof(REQUEST_SERVER), 0,(struct sockaddr *) &server_list[req_from_server->id_from].serv_address, sizeof(struct sockaddr));
			if(aux<0) printf("[ERROR] in package\n");

			//recebe pacote de eleição
			printf("[receiveCommands] - Recebe Election packet\n");
			ELECTION *election_req = (ELECTION*)malloc(sizeof(ELECTION));
			aux = recvfrom(myServerInfo.sockfd, election_req, sizeof(ELECTION), 0, (struct sockaddr *) &addr_request, &len);
			if(aux < 0) printf("ERROR receiveCommands");

			election_req->id_from = myServerInfo.id;
			if(election_req->id_max < myServerInfo.id)
			{
				election_req->id_max = myServerInfo.id;
			}

			//envia ack
			req_from_server->cmd = S_ANSWER;
			printf("[receiveCommands] - envia ACK\n");
			aux = sendto(myServerInfo.sockfd, &req_from_server, sizeof(REQUEST_SERVER), 0,(struct sockaddr *) &server_list[req_from_server->id_from].serv_address, sizeof(struct sockaddr));
			if(aux<0) printf("[ERROR] in package\n");

			//enviar para o proximo
			//Quem é o proximo?
			int next;
			for(int i = myServerInfo.id+1; i < MAXSERVERS; i++){
				if(server_list[i].type == BACKUP){
					next = i;
					i = MAXSERVERS+1;
				}else if(server_list[i].type == PRIMARY){
					i++;
				}else if(server_list[i].type == -1){
					i = 0;
				}
			}
			printf("[receiveCommands] - Envio o ELECTION para o proximo\n");
			aux = sendto(myServerInfo.sockfd, &election_req, sizeof(ELECTION), 0,(struct sockaddr *) &server_list[req_from_server->id_from].serv_address, sizeof(struct sockaddr));
			if(aux<0) printf("[ERROR] in package\n");
			
			//RECEBER ACK
			printf("[receiveCommands] - recebe o ack\n");
			aux = recvfrom(myServerInfo.sockfd, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
			if(aux < 0){}

			//aguarda receber todo
			printf("[receiveCommands] - Election receive\n");
			aux = recvfrom(myServerInfo.sockfd, election_req, sizeof(ELECTION), 0, (struct sockaddr *) &addr_request, &len);
			if(aux < 0) printf("ERROR receiveCommands");
		}
		else if(req_from_server->cmd == NEW_SERVER_LIST){

			//envia ACK
			REQUEST_SERVER *answer = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
			answer->cmd = S_ANSWER;
			answer->id_from = myServerInfo.id;
			if(DEBUG) printf("[Controle] New server list - enviando ACk");
			aux = sendto(myServerInfo.sockfd, &answer, (sizeof(REQUEST_SERVER)), 0,(const struct sockaddr *) &server_list[req_from_server->id_from].serv_address, sizeof(struct sockaddr));
			if(aux < 0) printf("ERROR connectToPrimary");

			// recebe lista
			if(DEBUG) printf("[Controle] New server list - wait receive servers\n");
			aux = recvfrom(myServerInfo.sockfd, server_list, sizeof(SERVER)*MAXSERVERS, 0, (struct sockaddr *) &addr_request, &len);
			if(aux < 0) printf("ERROR connectToPrimary");

			//envia ACK
			answer->cmd = S_ANSWER;
			answer->id_from = myServerInfo.id;
			if(DEBUG) printf("[Controle] New server list - enviando ACk");
			aux = sendto(myServerInfo.sockfd, &answer, (sizeof(REQUEST_SERVER)), 0,(const struct sockaddr *) &server_list[req_from_server->id_from].serv_address, sizeof(struct sockaddr));
			if(aux < 0) printf("ERROR connectToPrimary");

			if(DEBUG) debugPrintListServers();
		}

		else{ printf("Deu me***! Para tudo! \n");exit(1);}
	}
}

void *alive_test()
{
	socklen_t len;
	int socket_msg, count;
	struct sockaddr_in my_address, server_ad, addr_request;
	struct hostent *server; 

	if(DEBUG) printf("[Controle] alive_test - criar socket de comunicacao\n");

	// crina novo socket
	if ((socket_msg = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("ERROR opening socket ALIVE");

	// inicializa a estrutura de endereço dele
	my_address.sin_family = AF_INET;
	my_address.sin_port = htons(PORT+myServerInfo.type+2); //4003 ou 4004
	my_address.sin_addr.s_addr = inet_addr(endereco);;
	bzero(&(my_address.sin_zero), 8);

	if (bind(socket_msg, (struct sockaddr *) &my_address, sizeof(struct sockaddr)) < 0)
		printf("ERROR on binding");

	server = gethostbyname(inet_ntoa(server_list[0].serv_address.sin_addr));

	server_ad.sin_family = AF_INET;
	server_ad.sin_port = htons(PORT+3);
	server_ad.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(server_ad.sin_zero), 8);
	
	REQUEST_SERVER *req_from_server = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
	while(1)
	{
		if(myServerInfo.type == PRIMARY)
		{
			printf("PRIMARY\n");
			setsockopt(socket_msg,SOL_SOCKET,SO_RCVTIMEO,(char*)&no_timeout,sizeof(struct timeval));

			aux = recvfrom(socket_msg, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
			
			printf("\n\n {{{ ALIVE }}}\n\n");
			req_from_server->cmd = S_ANSWER;
			req_from_server->id_from = myServerInfo.id;
			aux = sendto(socket_msg, &req_from_server, (sizeof(REQUEST_SERVER)), 0,(const struct sockaddr *) &addr_request, sizeof(struct sockaddr));
		}
		if(myServerInfo.type == BACKUP && electionStart == FALSE)
		{
			printf("BACKUP\n");
			setsockopt(socket_msg,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
			sleep(3);
			req_from_server->cmd = ALIVE;
			req_from_server->id_from = myServerInfo.id;
			aux = sendto(socket_msg, &req_from_server, (sizeof(REQUEST_SERVER)), 0,(const struct sockaddr *) &server_ad, sizeof(struct sockaddr));

			aux = recvfrom(socket_msg, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
			if(aux < 0){
				if(count != 0){
					printf("\n\n {{{ TIMEOUT }}}\n");
					printf("\n {{{ INIT ELECTION }}}\n");
					if(electionStart != 1){
						electionStart = TRUE;
	
						ELECTION *election_req = (ELECTION*)malloc(sizeof(ELECTION));
						election_req->elected = FALSE;
						election_req->id_from = myServerInfo.id;
						election_req->id_max = myServerInfo.id;
						//inicia eleição
						init_election_state(election_req);
					}
				}
				else count++;
			}
			else { printf("\n\n {{{ ALIVE }}}\n\n");}
			
		}
	}
}

void init_election_state(ELECTION *election_req)
{
	socklen_t len;
	struct sockaddr_in  addr_request;
	int next;
	//Quem é proximo?
	for(int i = myServerInfo.id+1; i < MAXSERVERS; i++){
		if(server_list[i].type == BACKUP){
			next = i;
			i = MAXSERVERS+1;
		}else if(server_list[i].type == PRIMARY){
			
		}else if(server_list[i].type == -1){
			i = 0;
		}
	}

	// se o proximo sou eu mesmo (não tem outro server disponivel)
	if(next = myServerInfo.id){
		setNewPrimary(myServerInfo.id);
		return; 
	}

	printf("[init_election_state] - enviando election request\n");
	// informa proximo server que vai iniciar uma eleição
	REQUEST_SERVER *req_from_server = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
	req_from_server->cmd = START_ELECTION;
	req_from_server->id_from = myServerInfo.id;
	aux = sendto(myServerInfo.sockfd, &req_from_server, (sizeof(REQUEST_SERVER)), 0,(const struct sockaddr *) &server_list[next].serv_address, sizeof(struct sockaddr));
	if(aux < 0) printf("ERROR connectToPrimary");

	//RECEBER ACK
	printf("[init_election_state] - Recebendo ACK\n");
	aux = recvfrom(myServerInfo.sockfd, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
	if(aux < 0){}
	
	// Envia info de eleição
	printf("[init_election_state] - enviando election packet\n");
	aux = sendto(myServerInfo.sockfd, &election_req, sizeof(ELECTION), 0,(struct sockaddr *) &server_list[next].serv_address, sizeof(struct sockaddr));
	if(aux<0) printf("[ERROR] in package\n");

	//RECEBER ACK
	printf("[init_election_state] - Recebendo ACK\n");
	aux = recvfrom(myServerInfo.sockfd, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
	if(aux < 0){}

	//Aguarda voltar a mensagem para finalizar a eleição
	printf("[init_election_state] - aguardando resultado\n");
	aux = recvfrom(myServerInfo.sockfd, election_req, sizeof(ELECTION), 0, (struct sockaddr *) &addr_request, &len);
	if(aux < 0){}

	if(election_req->id_max == myServerInfo.id)
	{
		setNewPrimary(myServerInfo.id);
		//send elected
	}

	//send max id
}

void setNewPrimary(int id)
{
	printf("\n {{{ NEW PRIMARY ID: %d }}}\n", id);
	if(id == myServerInfo.id)
	{
		isPrimary = TRUE;
		myServerInfo.type = PRIMARY;
	}
	server_list[id].type = PRIMARY;
	electionStart = FALSE;
}

int connectNewServer(struct sockaddr_in new_server)
{
	int pos = server_count;
	server_count++;
	printf("\n\nnew server Received packet from %s:%d\n", inet_ntoa(new_server.sin_addr), ntohs(new_server.sin_port));
	if(server_count < MAXSERVERS){
		server_list[pos].type = BACKUP;
		server_list[pos].id = pos;
		server_list[pos].serv_address = new_server;
		server_list[pos].sockfd = 0;
		server_list[pos].flag_update_list_server = NEWUP;
		server_list[pos].flag_update_list_clients = NEWUP;
		server_list[pos].flag_update_files = NEWUP;
	}
	printf("\n\nnew server Received packet from %s:%d\n", inet_ntoa(server_list[pos].serv_address.sin_addr), ntohs(server_list[pos].serv_address.sin_port));
	return server_list[pos].id;
}

void sendUpdatesToServer(int to)
{
	if(server_list[to].flag_update_list_server == NEWUP)
	{
		sendListOfServers(to);
		server_list[to].flag_update_list_server = OK;
	}

	if(server_list[to].flag_update_list_clients == NEWUP)
	{
		sendListOfClients(to);
		server_list[to].flag_update_list_clients = OK;
	}

	if(server_list[to].flag_update_files == NEWUP)
	{

	}
}

void sendToAllServerList(int menos_esse){
	printf("\n[ LISTA SERVER ] - INIT\n");

	socklen_t len;
	struct sockaddr_in addr_request;

	for(int i = 0; i < MAXSERVERS; i++)
	{
		if(server_list[i].type != PRIMARY && server_list[i].type != FALSE && server_list[i].id != menos_esse){

			// envia comando
			printf("\n[ LISTA SERVER ] - ENVIANDO PARA ID: %d\n", i);
			REQUEST_SERVER *req_from_server = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
			req_from_server->cmd = NEW_SERVER_LIST;
			req_from_server->id_from = myServerInfo.id;
			aux = sendto(myServerInfo.sockfd, &req_from_server, (sizeof(REQUEST_SERVER)), 0,(struct sockaddr *) &server_list[i].serv_address, sizeof(struct sockaddr));
			if(aux < 0) printf("ERROR connectToPrimary");

			// recebe ACK
			printf("[ LISTA SERVER ] - recebendo do ID: %d\n", i);
			aux = recvfrom(myServerInfo.sockfd, req_from_server, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &addr_request, &len);
	if(aux < 0){}
			// chama func sendListOfServers(int to_server)
			sendListOfServers(i);
		}
	}
}

//Envia a lista de serves apenas para o server do parametro
void sendListOfServers(int to_server)
{
	socklen_t r_len;
	struct sockaddr_in r_addr;

	if(DEBUG) printf("[Controle] sendListOfServers - enviando para id:  %d\n", to_server);
	int aux = sendto(myServerInfo.sockfd, &server_list, (sizeof(SERVER)*MAXSERVERS), 0,(struct sockaddr *) &server_list[to_server].serv_address, sizeof(struct sockaddr));
	if(aux < 0) printf("ERROR sendListOfServers");

	REQUEST_SERVER *answer = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
	answer->cmd = S_ANSWER;
	if(DEBUG) printf("[Controle] sendListOfServers - wait receive\n");
	aux = recvfrom(myServerInfo.sockfd, answer, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &r_addr, &r_len);
	if(aux < 0) printf("ERROR sendListOfServers");
}


//Envia a lista de clientes apenas para o server do parametro
void sendListOfClients(int to_server)
{
	socklen_t r_len;
	struct sockaddr_in r_addr;
	
	if(DEBUG) printf("[Controle] sendListOfClients - enviando para id:  %d\n", to_server);

	REQUEST_SERVER *answer = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
	answer->cmd = S_ANSWER;
	// para cada cliente envia parametro a parametro da struct

		//----| Send - int devices[MAXDEVICES] |----
		aux = sendto(myServerInfo.sockfd, &client_list, sizeof(CLIENT), 0,(struct sockaddr *) &server_list[to_server].serv_address, sizeof(struct sockaddr));

		aux = recvfrom(myServerInfo.sockfd, answer, sizeof(REQUEST_SERVER), 0, (struct sockaddr *) &r_addr, &r_len);

	if(DEBUG) printf("[Controle] sendListOfClients - list finish\n");
}

//-----------------------| Função do servidor backup |-------------------------
int connectToPrimary(char *add){
	if(DEBUG) printf("[Controle] connectToPrimary\n");

	char p_address[32];
	strcpy(p_address, add);
	socklen_t len;
	struct sockaddr_in serv_addr, response;
	struct hostent *server; 
	server = gethostbyname(p_address);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT+1);
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);

	//enviar infos para o primario
	REQUEST_SERVER *req_to_server = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));

	req_to_server->cmd = S_CONNECT;
	req_to_server->id_from = myServerInfo.id;
	req_to_server->address = myServerInfo.serv_address;

	printf("\n\nconnect to primary Received packet from %s:%d\n", inet_ntoa(myServerInfo.serv_address.sin_addr), ntohs(myServerInfo.serv_address.sin_port));
	printf("\n\nconnect to primary Received packet from %s:%d\n", inet_ntoa(req_to_server->address.sin_addr), ntohs(req_to_server->address.sin_port));

	if(DEBUG) printf("[Controle] connectToPrimary - send\n");
	aux = sendto(myServerInfo.sockfd, req_to_server, sizeof(REQUEST_SERVER), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	
	// A resposta será um numero que representa o id do Backup e tambem o ACK do server pelo pedido.
	if(DEBUG) printf("[Controle] connectToPrimary - aguardando id\n");
	aux = recvfrom(myServerInfo.sockfd, &myServerInfo.id, sizeof(myServerInfo.id), 0, (struct sockaddr *) &response, &len);

	if(DEBUG) printf("[Controle] connectToPrimary - my new id: %d\n", myServerInfo.id);

	//--------------| Recebendo lista de servers |-------------------
	if(DEBUG) printf("[Controle] connectToPrimary - wait receive servers\n");
	aux = recvfrom(myServerInfo.sockfd, server_list, sizeof(SERVER)*MAXSERVERS, 0, (struct sockaddr *) &response, &len);
	if(aux < 0) printf("ERROR connectToPrimary");

	REQUEST_SERVER *answer = (REQUEST_SERVER*)malloc(sizeof(REQUEST_SERVER));
	answer->cmd = S_ANSWER;
	answer->id_from = myServerInfo.id;
	if(DEBUG) printf("[Controle] connectToPrimary - enviando ACk");
	aux = sendto(myServerInfo.sockfd, &answer, (sizeof(REQUEST_SERVER)), 0,(const struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	if(aux < 0) printf("ERROR connectToPrimary");

	if(DEBUG) debugPrintListServers();

	//--------------| Recebendo lista de clients |-------------------
	if(DEBUG) printf("[Controle] connectToPrimary - wait receive clients\n");

		CLIENT *aux_cli = (CLIENT*)malloc(sizeof(CLIENT));
		//------| Receive - int devices[MAXDEVICES] |-------
		aux = recvfrom(myServerInfo.sockfd, client_list, sizeof(CLIENT), 0, (struct sockaddr *) &response, &len);

		aux = sendto(myServerInfo.sockfd, &answer, (sizeof(REQUEST_SERVER)), 0,(struct sockaddr *) &serv_addr, sizeof(struct sockaddr));

	if(DEBUG) printf("[Controle] connectToPrimary - finish list");
	if(DEBUG) debugPrintListClients();
	return 0;
}

int registerNewServerInBackups(){ return 0;}

void setFlag(int flag)
{
	for(int i = 0; i < MAXSERVERS; i++)
	{
		if(flag = FSERVER)
		{
			server_list[i].flag_update_list_server = NEWUP;
		}
		if(flag = FCLIENT)
		{
			server_list[i].flag_update_list_clients = NEWUP;
		}
		if(flag = FFILES)
		{
			server_list[i].flag_update_files = NEWUP;
		}
	}
}

void debugPrintListServers(){

	printf("\n\t----| SERVERS |----\n");
	for(int i = 0; i < MAXSERVERS; i++){
		printf("\tid: %d type: %d\n", server_list[i].id, server_list[i].type);
	}
}

void debugPrintListClients(){

	printf("\n\t----| CLIENTS |----\n");
	for(int i = 0; i < MAXCLIENTS; i++){
		printf("\tid: %s\n", client_list[i].userid);
	}
}
