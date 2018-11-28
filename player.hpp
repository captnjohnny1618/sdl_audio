#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

#include <boost/filesystem.hpp>

#include "sdl_wrapper.h"

std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

int count = 0;

enum player_mode{
    MODE_NORMAL,
    MODE_REPEAT_ONE,
    MODE_REPEAT_ALL,
    MODE_SHUFFLE
};

struct song{
    uint8_t * song;
    uint32_t total_bytes;
    uint32_t bytes_played;
};

void audio_callback(void * udata, uint8_t * stream, int len){
    song * curr_song = (song *)udata;
    
    // Copy new audio into the stream
    memcpy(stream, &curr_song->song[curr_song->bytes_played],len);

    // Update metadata
    curr_song->bytes_played += len;
}

int visualizer_thread(bool * exit_flag){
    
    while (!(*exit_flag)){
        std::cout << "vis thread" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}

class player{
public:

    player();
    ~player();

    void play();
    void pause();
    void set_playlist(char * s);
    void set_track(int track);
    void event_loop();
    void cout_playlist();
    void next_song();
    void prev_song();

private:
    // Visualizer data
    bool exiting = false;
    std::thread v_thread;
    int visualizer_width        = 512;
    int visualizer_height       = 512;
    uint32_t * visualizer_array = NULL;

    // Playlist/player management
    int curr_track   = 0;
    std::vector<std::string> playlist;
    bool is_playing  = false;
    player_mode mode = MODE_NORMAL;

    // Audio buffer management
    SDL_AudioDeviceID dev;
    song curr_song;
    size_t buffer_size = 4096;    
};

player::player(){
    imshow_initialize(visualizer_width,visualizer_height,"color");
    visualizer_array = new uint32_t[visualizer_width*visualizer_height];
    memset(visualizer_array,0,visualizer_width*visualizer_height);
    imshow_update(visualizer_array);
    std::cout << "launching thread"<< std::endl;
    v_thread = std::thread(visualizer_thread,&exiting);
}
player::~player(){
    exiting = true;
    delete[] visualizer_array;
    SDL_FreeWAV(curr_song.song);
    imshow_destroy();
}

void player::event_loop(){

    bool quit = false;

    int count = 0;
    while (!quit){
        
        SDL_Event e;
        SDL_Keycode key;
        
        // Check if we're ready for the next song
        if (curr_song.bytes_played >= curr_song.total_bytes-buffer_size)
            next_song();
        
        SDL_PollEvent(&e);

        // User clicks quit
        if (e.type == SDL_QUIT)
            quit = true;
        // Keyboard input
        else if (e.type == SDL_KEYDOWN){
            count = 0;            
            key = e.key.keysym.sym;
            if (key == SDLK_SPACE){
                if (is_playing)
                    pause();
                else
                    play();
            }
            else if (key == SDLK_RIGHT)
                next_song();
            else if (key == SDLK_LEFT)
                prev_song();            
            else if (key == SDLK_q)
                quit =true;
            else if (key == SDLK_m){
                std::vector<player_mode> modes = {MODE_NORMAL,MODE_REPEAT_ONE,MODE_REPEAT_ALL,MODE_SHUFFLE};
                auto num = mode;
                mode = modes[(num+1)%modes.size()];

                std::cout << "Mode: " << mode  << std::endl;
            }
            else if (key == SDLK_e){
                exiting = true;
            }
                
            else{
            }
        }
        
        else{
            count++;

            // Three modes, no delay, short delay, long delay
            int state = 1;
            if (count >= 10000)
                state = 2;            
            if (count >= 10500)
                state = 3;

            switch (state){
            case 1:{
                break;
            }
            case 2:{
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                break;
            }
            case 3:{
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
            }
            }
                
            continue;
        }
    }
}

void player::play(){
    is_playing = true;
    std::cout << "Playing: " << is_playing << std::endl;
    SDL_PauseAudioDevice(dev,1-is_playing); /* start audio playing. */
}

void player::pause(){
    is_playing = false;
    std::cout << "Playing: " << is_playing << std::endl;
    SDL_PauseAudioDevice(dev,1-is_playing); /* start audio playing. */
}

void player::next_song(){
    if (mode == MODE_NORMAL)
        if (curr_track < playlist.size()-1)
            curr_track += 1;
        else{
            curr_track = 0;
            pause();
        }
    else if (mode == MODE_REPEAT_ALL)
        curr_track = (curr_track + 1) % playlist.size();
    else if (mode == MODE_REPEAT_ONE)
        {}//curr_track = curr_track;
    else if (mode == MODE_SHUFFLE){
        std::uniform_int_distribution<int> uni(0,playlist.size()-1); // guaranteed unbiased            
        curr_track = uni(rng);
    }
    set_track(curr_track);
}

void player::prev_song(){
    if (mode == MODE_NORMAL)
        curr_track = std::max(curr_track-1,0);
    else if (mode == MODE_REPEAT_ALL)
        curr_track = (curr_track - 1)%playlist.size();
    else if (mode == MODE_REPEAT_ONE)
        {}//curr_track = curr_track;
    else if (mode == MODE_SHUFFLE){
        std::uniform_int_distribution<int> uni(0,playlist.size()-1); // guaranteed unbiased
        curr_track = uni(rng);
    }
    set_track(curr_track);
}

void player::set_track(int idx){
    bool play_state = is_playing;
    if (play_state ==true){
        pause();
    }

    // Loop to handle getting valid audio tracks
    bool is_valid = false;        
    bool cleanup_flag = false;
    std::string curr_path;    
    std::string tmp_path;
    
    while (!is_valid){
        
        curr_track = idx;
        cout_playlist();
        
        curr_path = playlist[curr_track];        
        
        // Handle filetypes.  We're only reading WAVs.
        std::string extension = boost::filesystem::extension(curr_path);
        if (extension == ".wav" || extension == ".WAV"){
            // great, we're done
            is_valid = true;
        }
        else if (extension == ".mp3" || extension == ".MP3"){
            // If we've gotten MP3s we want to convert to a temporary WAV
            // and use that instead.
            boost::filesystem::path fp = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();    
            tmp_path = fp.native() + ".wav";
            std::string cmd = "ffmpeg -loglevel quiet -i \"" + curr_path + "\" " + tmp_path + "";

            std::cout << "Converting file... "; 
            int ret = system(cmd.c_str());
            if (ret){
                std::cout << "File could not be converted correctly. Removing." << std::endl;
                playlist.erase(playlist.begin() + curr_track);
                continue;
            }
            else                    
                std::cout << " done." << std::endl;

            // Set final loading data
            curr_path = tmp_path;
            cleanup_flag = true;
            is_valid = true;
        }
        else{
            std::cout << "Current track ("  << playlist[curr_track] << ") is in an unsupported filetype. Removing." << std::endl;
            playlist.erase(playlist.begin() + curr_track);
        }
    }

    SDL_AudioSpec wav;
    SDL_AudioSpec want,have;

    curr_song.bytes_played = 0;
    SDL_LoadWAV(curr_path.c_str(), &wav, &curr_song.song, &curr_song.total_bytes);

    SDL_memcpy(&want,&wav,sizeof(wav));
    want.samples  = buffer_size; 
    want.callback = audio_callback;
    want.userdata = &curr_song;

    if (dev>0)
        SDL_CloseAudioDevice(dev);

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    }

    // Handle any "cleanup tasks" (resume playing or delete the temporary wav file)
    if (play_state == true)
        play();

    if (cleanup_flag)
        remove(tmp_path.c_str());
}

void player::set_playlist(char * s){

    // (char * s) is the path to a text file specifying the paths to music files
    std::ifstream plist(s);
    std::string tmp;
    while (std::getline(plist,tmp)){
        playlist.push_back(tmp);
    }

    // Set the first track and launch the event loop
    curr_track = 0;
    set_track(curr_track);
    event_loop();
}

void player::cout_playlist(){
    std::cout << "Current tracklist:" << std::endl;
    for(int i=0;i<playlist.size();i++){
        if (i == curr_track)
            std::cout << "*** ";
        else
            std::cout << "    ";
        std::cout <<  i << ": " <<  playlist[i] << std::endl;
    }
}
