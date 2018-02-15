OBJS = main.o server.o utils.o player.o cJSON.o
CFLAGS = -c -g -Wall

spotifyConsole: $(OBJS)
	gcc -o spotifyConsole.exe $(OBJS) -lncurses -lcurl

main.o: main.c server.h player.h utils.h
	gcc $(CFLAGS) main.c

server.o: server.c server.h utils.h cJSON.h
	gcc $(CFLAGS) server.c

utils.o: utils.c utils.h player.h cJSON.h
	gcc $(CFLAGS) utils.c

player.o: player.c player.h server.h utils.h
	gcc $(CFLAGS) player.c

cJSON.o: cJSON.c cJSON.h
	gcc $(CFLAGS) cJSON.c

clean:
	rm -f spotifyConsole.exe $(OBJS)
