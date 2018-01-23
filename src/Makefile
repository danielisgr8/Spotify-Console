OBJS = main.o server.o utils.o player.o
CFLAGS = -c -g -Wall

spotifyConsole: $(OBJS)
	gcc -o spotifyConsole $(OBJS) -lncurses -lcurl

main.o: main.c server.h player.h
	gcc $(CFLAGS) main.c

server.o: server.c server.h utils.h
	gcc $(CFLAGS) server.c

utils.o: utils.c utils.h
	gcc $(CFLAGS) utils.c

player.o: player.c player.h
	gcc $(CFLAGS) player.c

clean:
	rm -f spotifyConsole $(OBJS)
