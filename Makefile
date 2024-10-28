LIBS_FLAGS = `pkg-config --cflags ncurses`
LIBS = `pkg-config --libs ncurses`
CFLAGS = -Wall -Wextra -pedantic -O3

SRC = gol.c
TARGET = gol
CC = clang
all:gol

gol:
	$(CC) $(CFLAGS) $(LIBS_FLAGS) $(LIBS) -o $(TARGET) $(SRC)

clean:
	rm -vf $(TARGET)
