debug_flags = -ggdb -O0 
flags = -Wall -Wno-unused-but-set-variable -Wno-unused-variable -std=c++14 -Iinc -fPIC $(debug_flags)
lib_path = ./lib/linux 
libs = -lSDL2 -lSDL2main -ldl -lpthread
program_name = bin/game
source_file = src/linux_main.cpp

make:
	mkdir -p bin
	g++ $(flags) $(source_file) -o $(program_name) -L $(lib_path) $(libs)

run: make
	./$(program_name)

debug: make
	gdb ./$(program_name)

valgrind: make
	kitty gdb $(program_name) &
	valgrind --vgdb-error=0 --suppressions=useable.supp $(program_name)
