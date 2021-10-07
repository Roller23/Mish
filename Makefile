CC := g++
ckript_bin := ckript/bin/
ckript_out := $(ckript_bin)ckript
flags := -O3 -std=c++17
ckript_src := ckript/src/
ckript_build := ckript/build/
input := ckript/examples/hash_table.ck

compile:
	@mkdir -p $(ckript_build)
	$(CC) $(flags) -c $(ckript_src)utils.cpp -o $(ckript_build)utils.o
	$(CC) $(flags) -c $(ckript_src)AST.cpp -o $(ckript_build)AST.o
	$(CC) $(flags) -c $(ckript_src)interpreter.cpp -o $(ckript_build)interpreter.o
	$(CC) $(flags) -lm -c $(ckript_src)CVM.cpp -o $(ckript_build)CVM.o
	$(CC) $(flags) -c $(ckript_src)lexer.cpp -o $(ckript_build)lexer.o
	$(CC) $(flags) -c $(ckript_src)evaluator.cpp -o $(ckript_build)evaluator.o
	$(CC) $(flags) -c $(ckript_src)error-handler.cpp -o $(ckript_build)error-handler.o
	$(CC) $(flags) -c $(ckript_src)parser.cpp -o $(ckript_build)parser.o
	$(CC) $(flags) -c $(ckript_src)token.cpp -o $(ckript_build)token.o
	$(CC) $(flags) -o $(ckript_out) $(ckript_build)utils.o $(ckript_build)AST.o $(ckript_build)interpreter.o $(ckript_build)CVM.o $(ckript_build)lexer.o $(ckript_build)evaluator.o $(ckript_build)error-handler.o $(ckript_build)parser.o $(ckript_build)token.o ckript/main.cpp

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