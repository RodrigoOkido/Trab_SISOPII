dropbox: dropboxServer dropboxClient

dropboxServer: dropboxServer.c
	gcc -o dropboxServer dropboxServer.c

dropboxClient: dropboxClient.c
	gcc -o dropboxClient dropboxClient.c

clean:
	rm dropboxServer dropboxClient