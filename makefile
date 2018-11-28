CXX=clang++
SDL= -framework SDL2
LDFLAGS=$(SDL) -lboost_system -lboost_filesystem
CXXFLAGS=-std=c++11 -c -stdlib=libc++ -D_GLIBCXX_USE_NANOSLEEP
EXE = audio_vis

all: $(EXE)

$(EXE): main.o
	$(CXX) $(LDFLAGS) $< -o $@

main.o: main.cpp sdl_wrapper.h player.hpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm *.o && rm $(EXE)
