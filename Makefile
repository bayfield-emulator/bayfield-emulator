BAYFIELDGB_SRC = src/bayfield_main.cpp src/emu_thread.cpp src/rom_utils.cpp
STATIC_LIBS = graphics/libgfx.a emucore/libemucore.a
LDFLAGS += -lSDL2
CXXFLAGS += -std=c++11 -g -Wall -DDEBUG -fsanitize=address

all: bayfield_gb

fake_emucore:
	$(MAKE) -C emucore/ libemucore.a

fake_gfx:
	$(MAKE) -C graphics/ libgfx.a

bayfield_gb: fake_emucore fake_gfx $(STATIC_LIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(BAYFIELDGB_SRC) $(STATIC_LIBS) -Iemucore/ -Igraphics/

clean: 
	$(MAKE) -C emucore/ clean
	$(MAKE) -C graphics/ clean