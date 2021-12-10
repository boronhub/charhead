CXX=clang++
INCLUDES=-Iincludes/ 
CXXFLAGS=-std=c++20 -g -fstandalone-debug -Wall -Wextra -Werror -pedantic $(INCLUDES)

charhead: bin/charhead

bin/charhead: src/compiler_run.cc src/compiler.cc 
	${CXX} ${CXX_FLAGS} $(CXXEXTRAS) $(INCLUDES) $^ `llvm-config --cxxflags --ldflags --libs --libfiles --system-libs` -o $@

.DEFAULT_GOAL := charhead
.PHONY: clean charhead 

clean:
	rm -rf bin/*