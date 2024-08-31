# Compiler and flags
CC = gcc
CFLAGS = -rdynamic `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

# Directories
SRCDIR = src
BUILDDIR = build
INCDIR = include

# Files
SOURCES = $(SRCDIR)/main.c $(SRCDIR)/chess.c
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)
EXEC = $(BUILDDIR)/main

# Default rule to build the program
all: $(EXEC)

# Rule to build the executable
$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

# Rule to compile each source file into an object file
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(BUILDDIR)/*.o $(EXEC)

# Run the program
run: $(EXEC)
	./$(EXEC)
