BAYFIELDGB_SRC = src/bayfield_main.cpp src/emu_thread.cpp src/rom_utils.cpp src/clock.cpp src/joypad.cpp
STATIC_LIBS = graphics/libgfx.a emucore/libemucore.a
CXXFLAGS += -std=c++11 -O2 -Wall #-DDEBUG

OUTPUT = bayfield_gb

ifeq ($(OS),Windows_NT)
	OUTPUT := $(OUTPUT).exe
	LDFLAGS = -lmingw32 -lSDL2main -lSDL2.dll -lpthread -luser32 -lgdi32 -ldxguid -mwindows
else
	LDFLAGS = -lSDL2 -lpthread
endif

ifeq ($(ASAN),1)
	CXXFLAGS += -fsanitize=address
endif

all: bayfield_gb

fake_emucore:
	$(MAKE) -C emucore/ libemucore.a

fake_gfx:
	$(MAKE) -C graphics/ libgfx.a

bayfield_gb: fake_emucore fake_gfx $(STATIC_LIBS)
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(BAYFIELDGB_SRC) $(STATIC_LIBS) -Iemucore/ -Igraphics/ $(LDFLAGS)

clean: 
	$(MAKE) -C emucore/ clean
	$(MAKE) -C graphics/ clean
