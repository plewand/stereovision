#include "ImgSourceCamera.h"

ImgSourceCamera::ImgSourceCamera() :
leftCap(NULL),
rightCap(NULL) {
}

ImgSourceCamera::~ImgSourceCamera() {
    delete leftCap;
    delete rightCap;
}

bool ImgSourceCamera::init(const Config &config) {
    leftCap = new VideoCapture(config.leftCamId);
    setResolution(leftCap, config.captureWidth, config.captureHeight);
    if (!leftCap->isOpened()) {
        Logger::error("left cam open error");
        return false;
    }
    rightCap = new VideoCapture(config.rightCamId);
    setResolution(rightCap, config.captureWidth, config.captureHeight);
    if (!rightCap->isOpened()) {
        Logger::error("right cam open error");
        return false;
    }
    return true;
}

void ImgSourceCamera::setResolution(VideoCapture *cap, int width, int height) {
    if(width == 0) {
        return;
    }
    bool set = cap->set(CAP_PROP_FRAME_WIDTH, width);
    if( !set) {
        Logger::log("Cound set video width");
    } else{
        bool set = cap->set(CAP_PROP_FRAME_HEIGHT, height);
        if( !set) {
            Logger::log("Cound set video height");
        }
    }
}

void ImgSourceCamera::capture() {        
    *leftCap >> imgLeft;
    *rightCap >> imgRight;    
}

Mat& ImgSourceCamera::getLeft() {
    return imgLeft;
}

Mat& ImgSourceCamera::getRight() {
    return imgRight;
}
