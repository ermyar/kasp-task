CC = gcc
CFLAGS = -Wall -Werror -fsanitize=address -O3
CLIB = -lcrypto -lpcre -lssl
TARGET = mangen
OBJECTS = mangen.c utils.c calc.c
HEADERS = utils.h calc.h

all: $(TARGET)

mangen: $(OBJECTS) $(HEADERS)
	$(CC) $(OBJECTS) -o $(TARGET) $(CFLAGS) $(CLIB)

test: $(TARGET)
	python3 test_mangen.py ./mangen

clean:
	rm mangen
	rm -rf __pycache__
