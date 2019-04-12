#include "visualizers.h"
#include <math.h>
//void render_frame_old(int16_t * song, size_t samples, uint32_t * vis_array, int w, int h){

void line(int x0,int y0,int x1,int y1,uint32_t * vis_array, int h);

typedef struct RgbColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RgbColor;

typedef struct HsvColor
{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

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

void oscilloscope(struct vis_data *v){

    int16_t * song = v->song;
    size_t samples = v->samples;
    uint32_t * vis_array = v->vis_array;
    int w = v->w;
    int h = v->h;
    
    //memset(vis_array,0xFF,w*h*sizeof(uint32_t));
    memset32(vis_array,0xFF000000,w*h);    

    // Step through each column and set one pixel on
    for (int i=0;i<samples;i+=2){
        
        // Extract the sound signal data
        float span = 110000;
        float di  = span/h;
        float i_cent = (h-1.0f)/2.0f;

        // Right channel is X axis, Left channel is Y
        int x = song[i]/di + i_cent;
        int y = song[i+1]/di + i_cent;        

        vis_array[x + y*h] = 0xFF00FF00;
    }
}

void line(int x0,int y0,int x1,int y1,uint32_t * vis_array, int h){
    float delta_x = x1 - x0;
    float delta_y = y1 - y0;
    float delta_err = fabs(delta_y/delta_x);
    float err = 0.0f;
    int sign_y = (delta_y>0.0f) ? 1 : -1;

    // Bresenham
    if (delta_x!=0){
        int y = y0;        
        for (int x = x0; x < x1; x++){
            vis_array[x + y*h] = 0xFF00FF00;
            err = err + delta_err;
            if (err>=0.5f){
                y = y + sign_y;
                err = err - 1.0f;
            }
        }
    }
    // Vertical line
    else{
        for (int y=y0; y!=y1; y+=sign_y)
            vis_array[x0 + y*h] = 0xFF00FF00;                        
    }
}

void oscilloscope_fancy(struct vis_data *v){
    uint32_t color = 0x005DDBDD;    
    //uint32_t color = 0xFFDDDB5D;
    
    int16_t * song = v->song;
    size_t samples = v->samples;
    uint32_t * vis_array = v->vis_array;
    int w = v->w;
    int h = v->h;

    // Extract the sound signal data
    float span = 110000;
    float di  = span/h;
    float i_cent = (h-1.0f)/2.0f;
    
    //memset32(vis_array,0xFF000000,w*h);

    // Instead of zeroing, multiply green channel by some factor
    for (int i=0;i<h;i++){
        for (int j=0;j<w;j++){
            // Extract the green channel
            uint8_t g = (vis_array[j + i*h] & 0x0000FF00) >> 8;
            g *= 0.7f ;
            uint32_t new_val = g;
            new_val = (new_val << 8) + 0xFF000000;
            vis_array[j + i*h] = new_val;

            /////// Extract the color channels
            /////uint32_t curr_color = vis_array[j + i*h];
            /////uint8_t r = (curr_color & 0x00FF0000) >> 24;
            /////uint8_t g = (curr_color & 0x0000FF00) >> 16;
            /////uint8_t b = (curr_color & 0x000000FF) >> 8;
            /////
            /////RgbColor rgb;
            /////rgb.r = r;
            /////rgb.g = g;
            /////rgb.b = b;
            /////
            /////HsvColor hsv = RgbToHsv(rgb);
            /////
            /////a *= 0.75f;
            /////uint32_t new_val = a;
            /////new_val = (new_val << 24) + vis_array[j+i*h];
            /////vis_array[j + i*h] = new_val;            
        }
    }

    int x,y;
    int x_prev,y_prev;
    for (int i=0;i<samples;i+=2){
        
        x = song[i]/di + i_cent;
        y = song[i+1]/di + i_cent;        
        //vis_array[x + y*h] = 0xFF00FF00;
        vis_array[x + (h-y)*h] = color;        
        
        // Right channel is X axis, Left channel is Y
        //x = song[i]/di + i_cent;
        //y = song[i+1]/di + i_cent;
        //
        //if (i!=0)
        //    line(x_prev,y_prev,x,y,vis_array,h);
        //
        //x_prev = x;
        //y_prev = y;
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
