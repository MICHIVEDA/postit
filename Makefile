CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = build/postit
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC) | build
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)


build:
	mkdir -p build

clean:
	rm -f $(TARGET)

.PHONY: all clean
