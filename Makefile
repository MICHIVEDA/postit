CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11
TARGET = build/postit
SRCS = src/main.c src/utils.c src/print.c
HDRS = src/utils.h src/print.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS) | build
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)


build:
	mkdir -p build

clean:
	rm -f $(TARGET)

.PHONY: all clean
