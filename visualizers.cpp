#include "visualizers.h"
#include <math.h>
//void render_frame_old(int16_t * song, size_t samples, uint32_t * vis_array, int w, int h){

void *  memset32(void * s, uint32_t c, size_t n_elements){
    // Almost the exact same implementation as memset, but takes n_elements (number of 32 bit words to write)
    // rather than number of bytes to write.

    // Useful for setting the background color of the visualizer array
    uint32_t * p = (uint32_t*)s;
    while (n_elements--)
        *p++ = c;
    return s;
}

void rect(int x, int y, int w, int h, int w_w, int w_h, uint32_t * vis_array,uint32_t C){
    // Draw left/right rectangle showing max channel values
    for (int i=x;i<(x+w);i++){
        for (int j=y;j<(y+h);j++){
            int idx = i + w_h*(w_h - j);
            vis_array[idx] = C;            
        }
    }    
}

void simple(struct vis_data * v){

    int16_t * song = v->song;
    size_t samples = v->samples;
    uint32_t * vis_array = v->vis_array;
    int w = v->w;
    int h = v->h;
    
    memset(vis_array,0,w*h*sizeof(uint32_t));

    // Step through each column and set one pixel on
    for (int i=0;i<w;i++){
        
        int sample_idx = (size_t)(i*((float)samples/(float)w));
        sample_idx = ((sample_idx%2 == 0) ? sample_idx : sample_idx - 1);

        // Extract the sound signal data
        float span = 140000;
        float di  = span/h;
        float i_cent = (h-1.0f)/2.0f;

        // Left channel (blue)
        int vertical_idx = song[sample_idx]/di+i_cent;        
        vis_array[i + vertical_idx*h] = 0xFFFF0000;
        
        // Right Channel (red)
        vertical_idx = song[sample_idx+1]/di + i_cent;
        vis_array[i + vertical_idx*h] = 0xFF0000FF;
    }    
}

void simple_bw(struct vis_data *v){

    int16_t * song = v->song;
    size_t samples = v->samples;
    uint32_t * vis_array = v->vis_array;
    int w = v->w;
    int h = v->h;
    
    //memset(vis_array,0xFF,w*h*sizeof(uint32_t));
    memset32(vis_array,0xFFFFFFFF,w*h);    

    // Step through each column and set one pixel on
    for (int i=0;i<w;i++){
        
        int sample_idx = (size_t)(i*((float)samples/(float)w));
        sample_idx = ((sample_idx%2 == 0) ? sample_idx : sample_idx - 1);

        // Extract the sound signal data
        float span = 140000;
        float di  = span/h;
        float i_cent = (h-1.0f)/2.0f;

        // Left channel (blue)
        int vertical_idx = song[sample_idx]/di+i_cent;        
        vis_array[i + vertical_idx*h] = 0xFF000000;
        
        // Right Channel (red)
        vertical_idx = song[sample_idx+1]/di + i_cent;
        vis_array[i + vertical_idx*h] = 0xFF000000;
    }
}


void hacker(struct vis_data *v){

    int16_t * song = v->song;
    size_t samples = v->samples;
    uint32_t * vis_array = v->vis_array;
    int w = v->w;
    int h = v->h;
    
    //memset(vis_array,0xFF,w*h*sizeof(uint32_t));
    memset32(vis_array,0xFF000000,w*h);    

    // Step through each column and set one pixel on
    for (int i=0;i<w;i++){
        
        int sample_idx = (size_t)(i*((float)samples/(float)w));
        sample_idx = ((sample_idx%2 == 0) ? sample_idx : sample_idx - 1);

        // Extract the sound signal data
        float span = 140000;
        float di  = span/h;
        float i_cent = (h-1.0f)/2.0f;

        // Left channel (blue)
        int vertical_idx = song[sample_idx]/di+i_cent;        
        vis_array[i + vertical_idx*h] = 0xFF00FF00;
        
        // Right Channel (red)
        vertical_idx = song[sample_idx+1]/di + i_cent;
        vis_array[i + vertical_idx*h] = 0xFF00FF00;
    }
}

int16_t max_l, max_r = 0;

void experimental(struct vis_data * v){
    int16_t * song = v->song;
    size_t samples = v->samples;
    uint32_t * vis_array = v->vis_array;
    int w = v->w;
    int h = v->h;

    memset32(vis_array,0xFF000000,w*h);

    int tmp_max_l, tmp_max_r = 0;
    
    // Find the maximum of each channel
    for (int i=0;i<samples;i++){
        tmp_max_l = std::max(song[2*i]  ,(int16_t)(tmp_max_l));
        tmp_max_r = std::max(song[2*i+1],(int16_t)(tmp_max_r));        
    }
    max_l = std::max(tmp_max_l,(int)(.95*max_l));
    max_r = std::max(tmp_max_r,(int)(.95*max_r));

    // Scale into pixels
    max_l = std::max((int)(((float)max_l/(float)32000) * 3*h/4),1);
    max_r = std::max((int)(((float)max_r/(float)32000) * 3*h/4),1);

    // Draw left/right rectangle showing max channel values
    rect(w*(1.0/4.0 - 1.0/8.0)  , h*(1.0/8.0) , w*(1.0/4.0) , max_l  ,w ,h ,vis_array , 0xFF00FF00);
    rect(w*(3.0/4.0 - 1.0/8.0 ) , h*(1.0/8.0) , w*(1.0/4.0) , max_r ,w ,h ,vis_array , 0xFF00FF00);
}
