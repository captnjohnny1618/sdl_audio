CXX=clang++
SDL= -framework SDL2
LDFLAGS=$(SDL) -lboost_system -lboost_filesystem -lavcodec -lavformat -lswscale
CXXFLAGS=-std=c++11 -c -stdlib=libc++ -D_GLIBCXX_USE_NANOSLEEP -g
#-Wno-deprecated-declarations
EXE = audio_vis

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
all: $(EXE)

$(EXE): main.o visualizers.o
	$(CXX) $(LDFLAGS) $^ -o $@

main.o: main.cpp sdl_wrapper.h player.hpp streamer.hpp
	$(CXX) $(CXXFLAGS) $< -o $@ 

visualizers.o: visualizers.cpp visualizers.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm *.o && rm $(EXE)
