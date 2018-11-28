#define NDEBUG
#include "player.hpp"

int main(int argc, char ** argv){

    player p;
    p.set_playlist(argv[1]);
    
    return 0;
}
