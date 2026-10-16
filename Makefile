
NAME := xshadowbind

CC := gcc
CFLAGS := -Wall -lX11 -lXi
SOURCES := src/*.h src/*.c

.PHONY: build run clean

build:
	$(CC) $(CFLAGS) $(SOURCES) -o $(NAME)

run: build
	./$(NAME)

clean:
	rm $(NAME)
