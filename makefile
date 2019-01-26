debug_flags = -ggdb -O0 
flags = -Wall -Wno-unused-but-set-variable -Wno-unused-variable -std=c++14 -Iinc -fPIC $(debug_flags)
lib_path = ./lib/linux 
libs = -lSDL2 -lSDL2main -ldl -lpthread
program_name = bin/game
source_file = src/linux_main.cpp
source_files = $(shell find src/ -name '*.cpp')
headers = $(shell find src/ -name '*.h')

.PHONY: default run debug valgrind

default: $(program_name)

$(program_name): $(source_files) $(headers)
	mkdir -p bin
	g++ $(flags) $(source_file) -o $(program_name) -L $(lib_path) $(libs)

run: $(program_name)
	./$(program_name)

debug: $(program_name)
	gdb ./$(program_name)

valgrind: $(program_name)
	kitty gdb $(program_name) &
	valgrind --vgdb-error=0 --suppressions=useable.supp $(program_name)
