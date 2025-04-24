all: build clean

build: mangen.c Makefile
	gcc mangen.c -o mangen -lssl -lcrypto -Wall -Werror -fsanitize=address

clean:
	rm mangen
