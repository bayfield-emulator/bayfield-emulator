# Makefile for Bayfield Emulator

BAYFIELDGB_SRC = src/bayfield_main.cpp src/emu_thread.cpp src/rom_utils.cpp src/clock.cpp src/joypad.cpp
WIN_REQUIRED_DLLS = libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll SDL2.dll
STATIC_LIBS = graphics/libgfx.a emucore/libemucore.a sound/libsound.a
CXXFLAGS += -std=c++11 -O2 -Wall

OUTPUT = Bayfield

ARCH = $(shell uname -m | tr a-z A-Z)

ifeq ($(OS),Windows_NT)
	MINGW_BIN_DIR = /mingw64/bin
	OUTPUT := $(OUTPUT).exe
	LDFLAGS += -lmingw32 -lSDL2main -lSDL2.dll -lpthread -luser32 -lgdi32 -ldxguid -mwindows
	BAYFIELDGB_SRC += src/file_picker_windows.cpp
else
	UNAME_S := $(shell uname -s)
	LDFLAGS += -lSDL2 -lpthread
	ifeq ($(UNAME_S),Darwin)
		BAYFIELDGB_SRC += src/file_picker_cocoa.mm
		LDFLAGS += -framework Cocoa

		ifeq ($(BUILD_STYLE),static)
			LDFLAGS = $(shell sdl2-config --static-libs)
		endif
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

fake_sound:
	$(MAKE) -C sound/ libsound.a

bayfield_gb: fake_emucore fake_gfx fake_sound $(STATIC_LIBS)
ifeq ($(OS),Windows_NT)
	windres.exe -J rc -O coff -i $(CURDIR)\\meta\\meta.rc -o $(CURDIR)\\meta\\meta.res
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(BAYFIELDGB_SRC) meta/meta.res $(STATIC_LIBS) -Iemucore/ -Igraphics/ -Isound/ $(LDFLAGS)
else
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(BAYFIELDGB_SRC) $(STATIC_LIBS) -Iemucore/ -Igraphics/ -Isound/ $(LDFLAGS)
endif

clean:
	rm meta/meta.res ||:
	rm $(OUTPUT) ||:
	rm -rf Bayfield.app ||:
	$(MAKE) -C emucore/ clean
	$(MAKE) -C graphics/ clean
	$(MAKE) -C sound/ clean

pack:
ifeq ($(OS),Windows_NT)
	@printf "$(shell /usr/bin/bash -c "FILE_LIST=\"$(WIN_REQUIRED_DLLS)\"; for file in \$$FILE_LIST; do if [[ -f \"$(MINGW_BIN_DIR)/\$$file\" ]]; then cp $(MINGW_BIN_DIR)/\$$file \$$file; else printf \"COULD NOT LOCATE '\$$file', ENSURE DEPENDANCIES ARE SATISFIED.\\n\"; fi; done")\n"
	@zip -r9y "WINDOWS $(ARCH).zip" $(OUTPUT) $(WIN_REQUIRED_DLLS) assets\\eframe.bmp
else
ifeq ($(UNAME_S),Linux)
	@zip -r9y "LINUX $(ARCH).zip" $(OUTPUT) assets/eframe.bmp
endif
ifeq ($(UNAME_S),Darwin)
	mkdir -p Bayfield.app/Contents/MacOS Bayfield.app/Contents/Resources/assets
	cp Bayfield Bayfield.app/Contents/MacOS/Bayfield
	cp meta/icon.icns Bayfield.app/Contents/Resources/Bayfield.icns
	cp assets/eframe.bmp Bayfield.app/Contents/Resources/assets/eframe.bmp
	ln -s ../Resources/assets Bayfield.app/Contents/MacOS/assets
	cp meta/Info.plist Bayfield.app/Contents
	codesign -s '-' Bayfield.app
	zip -r9y "MACOS $(ARCH).zip" Bayfield.app
endif
endif
