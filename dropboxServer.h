/**
  INF01151 - Sistemas Operacionais II N - Semestre 2018/1
  TRABALHO PRÁTICO PARTE 1: THREADS, SINCRONIZAÇÃO E COMUNICAÇÃO
  Prof(a): Alberto Egon Schaeffer Filho
  Integrantes: Douglas Lazaro, Henrique la Porta, Lisiane Aguiar , Rodrigo Okido
*/

/**
  Sincroniza o servidor com o diretório “sync_dir_<nomeusuário>” do
  cliente.
*/
void sync_server();



/**
  Recebe um arquivo file do cliente.
  Deverá ser executada quando for realizar upload de um arquivo.
  @param file – path/filename.ext do arquivo a ser recebido
*/
void receive_file(char *file);



/**
  Envia o arquivo file para o usuário.
  Deverá ser executada quando for realizar download de um arquivo.
  @param file – filename.ext
*/
void send_file(char *file);

int receiveCommandFromClient();
void establishConnectionWithClient();

void *handle_request();
