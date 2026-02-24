# Compiler
CC = gcc
CFLAGS = -Wall -std=c99 -Wno-missing-braces

# Tell the compiler where the "installed" raylib lives
# This covers Intel Macs (/usr/local) and Silicon M1/M2/M3 Macs (/opt/homebrew)
INCLUDES = -Iinc -I/opt/homebrew/include -I/usr/local/include
LDFLAGS = -L/opt/homebrew/lib -L/usr/local/lib

# macOS Frameworks + Raylib
LIBS = -lraylib -framework IOKit -framework Cocoa -framework OpenGL

# Files
SRC = src/menu.c src/main_menu.c src/pause_menu.c
TARGET = menu_test

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
