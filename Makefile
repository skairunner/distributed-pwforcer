CFLAGS=-Wall -std=c11 -fdiagnostics-color=always -g -D_XOPEN_SOURCE=700 -Iinclude -Llib
LIBS=-lrt -lpthread -lpwchecker

simplesockets: src/simplesockets.c include/simplesockets.h
	gcc $(CFLAGS) -c -o obj/simplesockets.o src/simplesockets.c $(LIBS)

packets: src/packets.c include/packets.h
	gcc $(CFLAGS) -c -o obj/packets.o src/packets.c -D_GNU_SOURCE $(LIBS)

pwhelper: src/passwordhelper.c include/passwordhelper.h
	gcc $(CFLAGS) -c -o obj/passwordhelper.o src/passwordhelper.c  $(LIBS)

dependencies: packets simplesockets pwhelper

server: dependencies src/server.c
	gcc $(CFLAGS) -c -o obj/server.o src/server.c $(LIBS)
	gcc $(CFLAGS) -o bin/server obj/server.o obj/simplesockets.o obj/packets.o obj/passwordhelper.o $(LIBS)	

client: dependencies src/client.c
	gcc $(CFLAGS) -c -o obj/client.o src/client.c $(LIBS)
	gcc $(CFLAGS) -o bin/client obj/client.o obj/simplesockets.o obj/packets.o obj/passwordhelper.o $(LIBS)

testpwhelper: dependencies src/testpwhelper.c
	gcc $(CFLAGS) -c -o obj/testpwhelper.o src/testpwhelper.c $(LIBS)
	gcc $(CFLAGS) -o bin/testpwhelper obj/testpwhelper.o obj/passwordhelper.o $(LIBS)
	./bin/testpwhelper

testserver: server
	./bin/server input.txt output.txt

testclient: client
	./bin/client 127.0.0.1 2

valgrind_server: server
	valgrind --leak-check=yes ./bin/server input.txt output.txt

valgrind_client: client
	valgrind --leak-check=yes ./bin/client 127.0.0.1 2

crackerapps: server client

clean:
	rm -rf bin obj
	mkdir obj
	mkdir bin