CXX           := clang++
CXXFLAGS      := -Wall -Wextra -pedantic -gdwarf-4 -std=c++20 -g -fstandalone-debug -c -o
ASSIGNMENT    := a2

BUILDDIR      := build
SOURCES       := $(wildcard *.cpp)
SOURCES_SUBD  := $(shell find */ -name "*.cpp")
DIRS          := $(patsubst %,$(BUILDDIR)/%,${SOURCES_SUBD:.cpp=})
OBJECTS       := $(patsubst %,$(BUILDDIR)/%,${SOURCES:.cpp=.o})
OBJECTS_SUBD  := $(patsubst %,$(BUILDDIR)/%,${SOURCES_SUBD:.cpp=.o})


.DEFAULT_GOAL := default
.PHONY: default prepare clean bin all run test help

default: help

$(DIRS):
	mkdir -p $@

prepare: $(DIRS)
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.cpp
	@echo "[\033[36mINFO\033[0m] Compiling object:" $<
	$(CXX) $(CXXFLAGS) $@ $< -MMD -MF ./$@.d

$(ASSIGNMENT) : $(OBJECTS) $(OBJECTS_SUBD)
	@echo "[\033[36mINFO\033[0m] Linking objects:" $@
	$(CXX) -o $@ $^

clean:						## cleans up project folder
	@printf "[\e[0;36mINFO\e[0m] Cleaning up folder...\n"
	rm -f $(ASSIGNMENT)
	rm -rf ./$(BUILDDIR)
	rm -rf testreport.html
	rm -rf ./valgrind_logs

bin: prepare $(ASSIGNMENT)	## compiles project to executable binary
	@printf "[\e[0;36mINFO\e[0m] Compiling binary...\n"
	chmod +x $(ASSIGNMENT)

all: clean bin				## all of the above

run: all					## runs the project with default config
	@printf "[\e[0;36mINFO\e[0m] Executing binary...\n"
	./$(ASSIGNMENT) ./configs/game_config.txt ./configs/message_config.txt

test: all					## runs public testcases on the project
	@printf "[\e[0;36mINFO\e[0m] Executing testrunner...\n"
	chmod +x testrunner
	./testrunner -c test.toml

help:						## prints the help text
	@printf "Usage: make \e[0;36m<TARGET>\e[0m\n"
	@printf "Available targets:\n"
	@awk -F':.*?##' '/^[a-zA-Z_-]+:.*?##.*$$/{printf "  \033[36m%-10s\033[0m%s\n", $$1, $$2}' $(MAKEFILE_LIST)

-include $(wildcard *.d)
