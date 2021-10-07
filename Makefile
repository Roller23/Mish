CC := g++
ckript_bin := ckript/bin/
ckript_out := $(ckript_bin)ckript
flags := -O3 -std=c++17
ckript_src := ckript/src/
ckript_build := ckript/build/
input := ckript/examples/hash_table.ck

srv:
	$(CC) $(flags) -o mish server/main.cpp

compile:
	@mkdir -p $(ckript_build)
	$(CC) $(flags) -lm -o $(ckript_out) $(ckript_src)*.cpp ckript/main.cpp

run:
	./$(ckript_out) $(input)

debug:
	gdb ./$(ckript_out)

mem:
	valgrind --track-origins=yes ./$(ckript_out) $(input)

clean:
	rm $(ckript_build)*.o
	rm $(ckript_out)

update:
	git stash
	git pull
	git stash apply