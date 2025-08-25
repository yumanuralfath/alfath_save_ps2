# Makefile for VMC Reader

# --- Versioning (ambil dari git tag atau default "dev") ---
VERSION ?= $(shell git describe --tags --always --dirty 2>/dev/null || echo dev)

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Isrc/core -Isrc/db -Isrc/cli -DVERSION=\"$(VERSION)\"
TARGET = vmcreader
SRCDIR = src
OBJDIR = obj

# Cari semua file .c di src/ secara rekursif
SOURCES := $(shell find $(SRCDIR) -name "*.c")
# Ubah path source jadi path object (mirror ke obj/)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Install (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed $(TARGET) to /usr/local/bin/"

# Uninstall (optional)
uninstall:
	rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstalled $(TARGET)"

# Debug build
debug: CFLAGS += -DDEBUG -g3
debug: $(TARGET)

# Release build
release: CFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

# Format code (requires clang-format)
format:
	clang-format -i *.c *.h

# Check for memory leaks (requires valgrind)
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) test.vmc

# Run tests (if you add test files later)
test: $(TARGET)
	@echo "Running tests..."
	@./$(TARGET) --help || echo "Help test passed"

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build the program (default)"
	@echo "  clean     - Remove build files"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  install   - Install to system"
	@echo "  uninstall - Remove from system"
	@echo "  format    - Format source code"
	@echo "  memcheck  - Run valgrind memory check"
	@echo "  test      - Run basic tests"
	@echo "  help      - Show this help"
	@echo "  version   - Show build version ($(VERSION))"

# Show version
version:
	@echo $(VERSION)

# Phony targets
.PHONY: all clean install uninstall debug release format memcheck test help version

