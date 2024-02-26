# Compiler
CC = gcc

# Compiler flags
CFLAGS = `pkg-config --cflags gtk+-3.0`
LIBS = `pkg-config --libs gtk+-3.0`

# Target executable
TARGET = gui

# Source files
SRCS = gui.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
