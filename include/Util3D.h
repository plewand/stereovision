#ifndef UTIL3D_H
#define UTIL3D_H

#include "stereoheaders.h"

void normalize(float *vec);
void add(const float *a, const float *b, float *c);
void sub(const float *a, const float *b, float *c);
void mul(const float *a, float s, float *b);
void cross(const float *a, const float *b, float *c);
float length(const float *vec);
void rotateAroundX(float angle, float *pt);
void rotateAroundY(float angle, float *pt);
float deg2rad(float deg);

#define DEPTH_CALC_SQUARE_SIZE (50)

float getCenterMinDistance(const Mat &depthMap, float depthMult = 1.0f, int channelNum = 1, int depthChannelOffset = 1);

#endif /* UTIL3D_H */

