CC := g++
ckript_bin := ckript/bin/
ckript_out := $(ckript_bin)ckript
flags := -O0 -g -std=c++17
ckript_src := ckript/src/
server_src := server/src/
utils_src := utils/
ckript_build := ckript/build/
input := ckript/examples/hash_table.ck

srv:
	$(CC) $(flags) -o mish server/main.cpp $(ckript_src)*.cpp $(server_src)*.cpp $(utils_src)*.cpp

compile:
	@mkdir -p $(ckript_build)
	$(CC) $(flags) -lm -o $(ckript_out) $(ckript_src)*.cpp ckript/main.cpp

install:
	sudo cp ./mish /usr/local/bin/mish

run:
	./$(ckript_out) $(input)

debug:
	lldb ./mish

mem:
	valgrind --track-origins=yes ./$(ckript_out) $(input)

clean:
	rm $(ckript_build)*.o
	rm $(ckript_out)

update:
	git stash
	git pull
	git stash apply