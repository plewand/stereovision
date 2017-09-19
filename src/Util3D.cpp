#include <cmath>
#include "Util3D.h"

float getCenterMinDistance(const Mat &depthMap, float depthMult, int channelNum, int depthChannelOffset) {
    Mat depth32F;
    if (depthMap.type() == CV_16S) {
        depthMap.convertTo(depth32F, CV_32F, 1.0 / 16.0);
    } else {
        depth32F = depthMap;
    }

    int cx = depth32F.cols / 2;
    int cy = depth32F.rows / 2;
    int sx = cx - DEPTH_CALC_SQUARE_SIZE / 2;
    int sy = cy - DEPTH_CALC_SQUARE_SIZE / 2;

    float minVal = 10000000;

    const float *ptr = (const float*) depth32F.data + sy * depth32F.cols * channelNum + sx*channelNum + depthChannelOffset;

    for (int j = 0; j < DEPTH_CALC_SQUARE_SIZE; j++) {
        for (int i = 0; i < DEPTH_CALC_SQUARE_SIZE; i++) {
            float depth = *ptr * depthMult;
            if (depth < minVal) {
                minVal = depth;
            }
            ptr += channelNum;
        }
        ptr += (depthMap.cols - DEPTH_CALC_SQUARE_SIZE) * channelNum;
    }
    return minVal;
}

void normalize(float *vec) {
    float len = length(vec);
    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

void add(const float *a, const float *b, float *c) {
    c[0] = a[0] + b[0];
    c[1] = a[1] + b[1];
    c[2] = a[2] + b[2];
}

void sub(const float *a, const float *b, float *c) {
    c[0] = a[0] - b[0];
    c[1] = a[1] - b[1];
    c[2] = a[2] - b[2];
}

void mul(const float *a, float s, float *b) {
    b[0] = a[0] * s;
    b[1] = a[1] * s;
    b[2] = a[2] * s;
}

void cross(const float *a, const float *b, float *c) {
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}

float length(const float *vec) {
    return sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

void rotateAroundX(float angle, float *pt) {
    float rad = deg2rad(angle);
    float vec[3] = {pt[0], pt[1], pt[2]};
    float cosa = cosf(rad);
    float sina = sinf(rad);
    pt[1] = cosa * vec[1] - sina * vec[2];
    pt[2] = sina * vec[1] + cosa * vec[2];
}

void rotateAroundY(float angle, float *pt) {
    float rad = deg2rad(angle);
    float vec[3] = {pt[0], pt[1], pt[2]};
    float cosa = cosf(rad);
    float sina = sinf(rad);
    pt[0] = cosa * vec[0] - sina * vec[2];
    pt[2] = sina * vec[0] + cosa * vec[2];
}

float deg2rad(float deg) {
    return 3.1415f * deg / 180.0f;
}