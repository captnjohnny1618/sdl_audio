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
#include "visualizers.h"

std::random_device rd;     // only used once to initialise (seed) engine
std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)

int count = 0;

enum player_mode{
    MODE_NORMAL,
    MODE_REPEAT_ONE,
    MODE_REPEAT_ALL,
    MODE_SHUFFLE
};

//enum vis_mode{
//    VIS_SIMPLE,
//    VIS_BW,
//    VIS_HACKER,
//    VIS_EXPERIMENTAL
//};

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

int visualizer_thread(void * udata);
//void render_frame_old(int16_t * song, size_t samples, uint32_t * vis_array, int w, int h);

typedef void (*callback)(struct vis_data * v);

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

    void set_visualization(callback ptr_reg_callback); // function that registers callback with render frame
    void (*render_frame)(struct vis_data * v);

    // Accessors
    bool is_exiting(){return exiting;};
    bool is_playing(){return playing;};
    bool is_energy_saver(){return energy_saver;};
    int get_frame_rate(){return frame_rate;};
    size_t get_buffer_size(){return buffer_size;};
    size_t get_sampling_rate(){return sampling_rate;};
    uint32_t * get_visualizer_array(){return visualizer_array;};
    song * get_song(){return &curr_song;};
    int get_height(){return visualizer_height;};
    int get_width(){return visualizer_width;};    

private:
    
    // Visualizer data
    bool exiting                = false;
    int curr_vis                = 3;
    std::vector<callback> visualizations = {simple,simple_bw,hacker,experimental,oscilloscope,oscilloscope_fancy};
    int frame_rate              = 24;
    int visualizer_width        = 512;
    int visualizer_height       = 512;
    uint32_t * visualizer_array = NULL;

    // Playlist/player management
    int curr_track   = 0;
    std::vector<std::string> playlist;
    bool playing  = false;
    player_mode mode = MODE_NORMAL;
    bool energy_saver = true;

    // Audio buffer management
    SDL_AudioDeviceID dev;    
    song curr_song;
    size_t buffer_size = 4096;
    size_t sampling_rate = 44100;
};

player::player(){
    imshow_initialize(visualizer_width,visualizer_height,"color");
    visualizer_array = new uint32_t[visualizer_width*visualizer_height];
    memset(visualizer_array,0,visualizer_width*visualizer_height);
    set_visualization(visualizations[curr_vis]);
    imshow_update(visualizer_array);
    
    SDL_CreateThread(visualizer_thread,"visualizer_thread",(void*)this);
}
player::~player(){    
    exiting = true;    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));     // Let the visualizer thread catch up just in case
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
                if (playing)
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
            else if (key == SDLK_e){
                energy_saver = true - energy_saver;
                std::cout << "Energy_Saver: " << (energy_saver ? "true":"false") << std::endl;
            }
            
            else if (key == SDLK_m){
                std::vector<player_mode> modes = {MODE_NORMAL,MODE_REPEAT_ONE,MODE_REPEAT_ALL,MODE_SHUFFLE};
                auto num = mode;
                mode = modes[(num+1)%modes.size()];
                std::cout << "Mode: " << mode  << std::endl;
            }
            else if (key == SDLK_v){
                auto num = curr_vis;
                curr_vis = (num+1)%visualizations.size();
                set_visualization(visualizations[curr_vis]);
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
    playing = true;
    std::cout << "Playing: " << playing << std::endl;
    SDL_PauseAudioDevice(dev,1-playing); /* start audio playing. */
}

void player::pause(){
    playing = false;
    std::cout << "Playing: " << playing << std::endl;
    SDL_PauseAudioDevice(dev,1-playing); /* start audio playing. */
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
    bool play_state = playing;
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
            idx =-1;
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
            std::cout << " -> ";
        else
            std::cout << "    ";
        std::cout <<  i << ": " <<  playlist[i] << std::endl;
    }
}

int visualizer_thread(void * udata){
    player * p = (player*)udata;
    song * s = p->get_song();

    // Get the delay between frames
    float millis = 1000.0f*1.0f/(float)p->get_frame_rate();

    // Compute samples per visualizer update
    // Total samples per audio buffer update
    int samples_per_chunk = p->get_buffer_size();
    float sec_per_chunk = (float)samples_per_chunk/(float)p->get_sampling_rate();
    size_t samples_per_frame = p->get_buffer_size()/(p->get_frame_rate()*sec_per_chunk);
    size_t bytes_per_frame = 4*samples_per_frame;

    int16_t * frame_buffer = new int16_t[bytes_per_frame];

    // Always displaying something until time to quit
    int frame_ticker = 0;
    int prev_byte_played = s->bytes_played;

    struct vis_data v;
    v.w         = p->get_width();
    v.h         = p->get_height();
    v.song      = frame_buffer;
    v.samples   = samples_per_frame;
    v.vis_array = p->get_visualizer_array();

    while (!(p->is_exiting())){
        auto start = std::chrono::high_resolution_clock::now();
        if (!p->is_playing()){
            // Render "silence" (generate some noise to display, but don't actually send to audio buffer)
            // Max val of int16: -32768 through 32767
            float noise_magnitude_percent = 2;
            int noise_limit = 32767/100*noise_magnitude_percent;
            for (int i=0;i<2*samples_per_frame;i++){ // factor of 2 since we have LR channels
                frame_buffer[i] = rand()%((noise_limit - (-noise_limit))+1) + (-noise_limit);
            }
        }
        else{
            if (s->bytes_played != prev_byte_played){
                prev_byte_played = s->bytes_played;
                frame_ticker = 0;
            }
            // Copy data into the frame audio buffer and render
            memcpy(frame_buffer,&s->song[prev_byte_played + frame_ticker * bytes_per_frame],bytes_per_frame);
            frame_ticker++;
        }
        //(p->render_frame)(frame_buffer,samples_per_frame,p->get_visualizer_array(),p->get_width(),p->get_height());
        (p->render_frame)(&v);
        imshow_update(p->get_visualizer_array());
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
        if (p->is_energy_saver())
            std::this_thread::sleep_for(std::chrono::milliseconds((int)millis)-duration);
    }

    std::cout << "Visualizer shutting down" << std::endl;
    delete[] frame_buffer;

    return 0;
}

void player::set_visualization(callback render_frame_callback){
    render_frame = render_frame_callback;
}
