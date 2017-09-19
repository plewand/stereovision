#ifndef IMGSOURCECAMERA_H
#define IMGSOURCECAMERA_H

#include "Config.h"

class ImgSourceCamera {
public:    
    bool init(const Config &config);
    void capture();
    Mat& getLeft();
    Mat& getRight();
    
    ImgSourceCamera();
    virtual ~ImgSourceCamera();
private:
    void setResolution(VideoCapture *cap, int width, int height);
    VideoCapture *leftCap;
    VideoCapture *rightCap;
    Mat imgLeft;
    Mat imgRight;
};

#endif /* IMGSOURCECAMERA_H */
