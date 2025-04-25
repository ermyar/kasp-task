all: build test clean

build: mangen.c Makefile
	gcc mangen.c -o mangen -Wall -Werror -fsanitize=address -lpcre -lssl -lcrypto


test: build
	python3 test_mangen.py ./mangen

clean:
	rm mangen
	rm -rf __pycache__
