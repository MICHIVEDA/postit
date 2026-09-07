CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11
TARGET = build/postit
SRCS = src/main.c src/utils.c

all: $(TARGET)

$(TARGET): $(SRC) | build
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)


build:
	mkdir -p build

clean:
	rm -f $(TARGET)

.PHONY: all clean
