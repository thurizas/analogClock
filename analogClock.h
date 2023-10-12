#pragma once

#include "resource.h"

#define HAND_PNTS 5

typedef struct fpoint_tag
{
    double    x;
    double    y;
} fpoint, * lpfpoint;


fpoint   handTemplate[HAND_PNTS] = {{-2.0,0.0}, {-2.0,0.9}, {0.0,1.0}, {2.0, 0.9}, {2.0, 0.0}};