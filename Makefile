all: build test clean

build: mangen.c Makefile
	gcc mangen.c -o mangen -lssl -lcrypto -Wall -Werror -fsanitize=address


test: build
	python3 test_mangen.py ./mangen

clean:
	rm mangen
	rm -rf __pycache__
