all: build clean

build:
	gcc mangen.c -o mangen -lssl -lcrypto

clean:
	rm mangen
