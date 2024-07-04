# Makefile for building the project using build.sh

# Use bash as the shell
SHELL := /bin/bash

.PHONY: all clean build

# Default target
all: build

# Build the project using build.sh
build:
	@echo "Building the project using build.sh..."
	@chmod +x ./build.sh
	@./build.sh

# Clean the project
clean:
	@echo "Cleaning the project..."
	@./build.sh clean
	# $(MAKE) clean
