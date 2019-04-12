#pragma once

#include <stdlib.h>
#include <cstdio>
#include <iostream>

struct vis_data{
    int16_t * song;
    size_t samples;
    uint32_t * vis_array;
    int w;
    int h;
};

void simple(struct vis_data * v);
void simple_bw(struct vis_data * v);
void hacker(struct vis_data * v);
void experimental(struct vis_data * v);
void oscilloscope(struct vis_data * v);
void oscilloscope_fancy(struct vis_data * v);
