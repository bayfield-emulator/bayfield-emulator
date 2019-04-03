CXXFLAGS += -Wall -g -std=c++11

Demo: Demo.cpp
	# swap clang++ for g++ if you're 2cool4LLVM
	clang++ -Wall Demo.cpp Window.cpp GPU.cpp -o demo -w -lSDL2 -g -std=c++11

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^

libgfx.a: Window.o GPU.o
	ar -rc $@ $^
	ranlib -s $@

clean:
	rm -f libgfx.a demo *.o