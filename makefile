# Bradley Juma

server:simpleServer.c
	gcc -Wall -w  simpleServer.c connectionQueue.c solvedSpellList.c logQueue.c spellQueue.c open_listenfd.c -o server -lpthread



