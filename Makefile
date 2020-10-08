BAYFIELDGB_SRC = src/bayfield_main.cpp src/emu_thread.cpp src/rom_utils.cpp src/clock.cpp src/joypad.cpp
STATIC_LIBS = graphics/libgfx.a emucore/libemucore.a
CXXFLAGS += -std=c++11 -O2 -Wall #-DDEBUG

OUTPUT = Bayfield

ifeq ($(OS),Windows_NT)
	OUTPUT := $(OUTPUT).exe
	LDFLAGS += -lmingw32 -lSDL2main -lSDL2.dll -lpthread -luser32 -lgdi32 -ldxguid -mwindows
	BAYFIELDGB_SRC += src/file_picker_windows.cpp
else
	UNAME_S := $(shell uname -s)
	LDFLAGS += -lSDL2 -lpthread
	ifeq ($(UNAME_S),Darwin)
		BAYFIELDGB_SRC += src/file_picker_cocoa.mm
		LDFLAGS += -framework Cocoa
	else
		BAYFIELDGB_SRC += src/file_picker_none.cpp
	endif
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
ifeq ($(OS),Windows_NT)
	windres.exe -J rc -O coff -i $(CURDIR)\\meta\\meta.rc -o $(CURDIR)\\meta\\meta.res
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(BAYFIELDGB_SRC) meta/meta.res $(STATIC_LIBS) -Iemucore/ -Igraphics/ $(LDFLAGS)
else
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(BAYFIELDGB_SRC) $(STATIC_LIBS) -Iemucore/ -Igraphics/ $(LDFLAGS)
endif

clean: 
	rm meta/meta.res ||:
	rm $(OUTPUT) ||:
	$(MAKE) -C emucore/ clean
	$(MAKE) -C graphics/ clean

pack:
ifeq ($(OS),Windows_NT)
	zip -r9 "WINDOWS x86-64.zip" Bayfield.exe libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll SDL2.dll assets\\eframe.bmp
else
ifeq ($(UNAME_S),Linux)
	zip -r9 "LINUX x86-64.zip" Bayfield assets/eframe.bmp
endif
ifeq ($(UNAME_S),Darwin)
	zip -r9 "MACOS x86-64.zip" Bayfield assets/eframe.bmp
endif
endif
	