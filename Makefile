# ==============================================================================
# Cross-Platform Makefile for Custom Text Editor
# Supports OS detection (macOS / Linux), clean, re, and install routines.
# ==============================================================================

# Name of your final compiled text editor binary executable
NAME     = immy

# Compiler setup
CXX      = g++
CXXFLAGS = -Wall -Werror -std=c++17

# ------------------------------------------------------------------------------
# OS Detection & Library Linking Config
# ------------------------------------------------------------------------------
# Mac and Linux require different compilation flag targets for ncurses.
# Linux natively relies on pkg-config or standard headers, while macOS
# occasionally requires linking explicitly to the ncurses framework or tinfo.

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    # macOS Specific Configuration
    LIBS = -lncurses
else
    # Linux (Ubuntu/Xubuntu/Debian) Specific Configuration
    LIBS = -lncursesw -ltinfo
endif

# Source and Object files
SRCS = buffer.cpp immy.cpp
OBJS = $(SRCS:.cpp=.o)

# Installation Paths (Standard POSIX paths)
PREFIX ?= /usr/local
BINDIR  = $(PREFIX)/bin

# ------------------------------------------------------------------------------
# Compilation Rules & Targets
# ------------------------------------------------------------------------------

# Default Target (Builds the binary executable)
all: $(NAME)

# Link the object files together with terminal screen libraries
$(NAME): $(OBJS)
	@echo "Linking binary: $(NAME)..."
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LIBS)
	@echo "Build successful!"

# Compile source files to structural intermediate object files
%.o: %.cpp
	@echo "Compiling: $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target: removes intermediate object build flags
clean:
	@echo "Cleaning intermediate compilation objects..."
	rm -f $(OBJS)

# Full Clean target: removes objects AND the final built executable binary
fclean: clean
	@echo "Removing executable binary file..."
	rm -f $(NAME)

# Rebuild target: performs a clean wipe and recompiles everything fresh
re: fclean all

# Install target: moves the binary smoothly to your global path system binary tier
install: all
	@echo "Installing $(NAME) to $(BINDIR)..."
	mkdir -p $(BINDIR)
	cp -f $(NAME) $(BINDIR)
	chmod 755 $(BINDIR)/$(NAME)
	@echo "Installation complete! You can now run '$(NAME)' globally from any directory."

# Uninstall target: cleanly drops the global installation binary mapping out of path
uninstall:
	@echo "Removing $(NAME) from $(BINDIR)..."
	rm -f $(BINDIR)/$(NAME)
	@echo "Uninstallation complete."

# Safety guard: declare targets as phonies to protect against matching filenames
.PHONY: all clean fclean re install uninstall
