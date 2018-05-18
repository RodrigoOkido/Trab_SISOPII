all: dropbox
	# TO DO run dropboxServer
	# TO DO run dropboxClient

dropbox: dropboxServer.o dropboxClient.o dropboxUtil.o

dropboxServer.o: dropboxServer.c dropboxUtil.c
	gcc -pthread -o dropboxServer dropboxServer.c dropboxUtil.c

dropboxClient.o: dropboxClient.c dropboxUtil.c
	gcc -pthread -o dropboxClient dropboxClient.c dropboxUtil.c

dropboxUtil.o: dropboxUtil.c
	gcc -c dropboxUtil.c

teste_delete: teste_delete.c
	gcc -o teste_delete teste_delete.c

clean:
	rm dropboxClient dropboxServer teste_delete *.o
