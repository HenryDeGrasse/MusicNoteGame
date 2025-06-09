CC = gcc
CFLAGS = -Wall -O2
TARGET = note_game
SOURCES = play_note_game.c
OBJECTS = $(SOURCES:.c=.o)

# Platform-specific linker flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -framework AudioUnit
else ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lasound
endif

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
