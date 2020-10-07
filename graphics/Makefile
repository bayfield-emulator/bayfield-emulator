CXXFLAGS += -std=c++11 -O2 -Wall

OUTPUT = demo

ifeq ($(OS),Windows_NT)
	OUTPUT := $(OUTPUT).exe
endif

ifeq ($(ASAN),1)
	CXXFLAGS += -fsanitize=address
endif

Demo: Demo.cpp
	$(CXX) -Wall Demo.cpp Window.cpp GPU.cpp -o $(OUTPUT) -w -lSDL2 -g -std=c++11

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $^

libgfx.a: Window.o GPU.o
	ar -rcs $@ $^

clean:
	rm -f libgfx.a $(OUTPUT) *.o
