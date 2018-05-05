all: dropbox
	# TO DO run dropboxServer
	# TO DO run dropboxClient

dropbox: dropboxServer.o dropboxClient.o dropboxUtil.o

dropboxServer.o: dropboxServer.c dropboxUtil.c
	gcc -o dropboxServer dropboxServer.c dropboxUtil.c

dropboxClient.o: dropboxClient.c dropboxUtil.c
	gcc -o dropboxClient dropboxClient.c dropboxUtil.c

dropboxUtil.o: dropboxUtil.c
	gcc -c dropboxUtil.c

clean:
	rm dropboxClient dropboxServer *.o
