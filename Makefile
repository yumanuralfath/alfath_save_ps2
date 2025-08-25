# Makefile for Alfath Save PS2

# --- Versioning (ambil dari git tag atau default "dev") ---
VERSION ?= $(shell git describe --tags --always --dirty 2>/dev/null || echo dev)

# --- Cross-Platform Compatibility ---
# Tentukan variabel berdasarkan Sistem Operasi
# OS_NAME akan berisi "Windows", "Linux", atau "Darwin" (untuk macOS)
OS_NAME := $(shell uname -s)

# Compiler settings - ?= agar bisa di-override dari environment
CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g -Isrc/core -Isrc/db -Isrc/cli -DVERSION=\"$(VERSION)\"

# Target executable name
TARGET_NAME = alfathsave
SRCDIR = src
OBJDIR = obj

# Tambahkan .exe untuk Windows
ifeq ($(findstring MINGW,$(OS_NAME)),MINGW)
    TARGET_EXT = .exe
    RM = del /Q /F
    # Ganti backslash untuk path di Windows
    fixpath = $(subst /,\,$1)
else
    TARGET_EXT =
    RM = rm -rf
    fixpath = $1
endif

EXECUTABLE = $(TARGET_NAME)$(TARGET_EXT)

# Cari semua file .c di src/ secara rekursif
SOURCES := $(shell find $(SRCDIR) -name "*.c")
# Ubah path source jadi path object (mirror ke obj/)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Default target
all: $(EXECUTABLE)

# Linking
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	-$(RM) $(call fixpath, $(OBJDIR))
	-$(RM) $(EXECUTABLE)

# Install (optional - Unix-like specific)
install: $(EXECUTABLE)
	@echo "Install target is for Unix-like systems (Linux/macOS)."
	cp $(EXECUTABLE) /usr/local/bin/
	@echo "Installed $(EXECUTABLE) to /usr/local/bin/"

# Uninstall (optional - Unix-like specific)
uninstall:
	@echo "Uninstall target is for Unix-like systems (Linux/macOS)."
	rm -f /usr/local/bin/$(TARGET_NAME)
	@echo "Uninstalled $(TARGET_NAME)"

# Debug build
debug: CFLAGS += -DDEBUG -g3
debug: $(EXECUTABLE)

# Release build
release: CFLAGS += -DNDEBUG -O3
release: clean $(EXECUTABLE)

# Format code (requires clang-format)
format:
	clang-format -i *.c *.h

# Check for memory leaks (requires valgrind - Unix-like specific)
memcheck: $(EXECUTABLE)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXECUTABLE) test.vmc

# Run tests (if you add test files later)
test: $(EXECUTABLE)
	@echo "Running tests..."
	@./$(EXECUTABLE) --help || echo "Help test passed"

# Show help
help:
	@echo "Available targets:"
	@echo "  all       - Build the program (default)"
	@echo "  clean     - Remove build files"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release version"
	@echo "  install   - Install to system (Linux/macOS)"
	@echo "  uninstall - Remove from system (Linux/macOS)"
	@echo "  format    - Format source code"
	@echo "  memcheck  - Run valgrind memory check (Linux/macOS)"
	@echo "  test      - Run basic tests"
	@echo "  help      - Show this help"
	@echo "  version   - Show build version ($(VERSION))"

# Show version
version:
	@echo $(VERSION)

# Phony targets
.PHONY: all clean install uninstall debug release format memcheck test help version
