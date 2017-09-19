#include "Rectifier.h"
#include "CvUtil.h"

Rectifier::Rectifier() {
}

Rectifier::~Rectifier() {
}

bool Rectifier::load(const char *remaps) {
    return loadYaml("remap.yml", [this](auto fs) -> auto {
        fs["remapLeft0"] >> remapLeft0;
        fs["remapLeft1"] >> remapLeft1;
        fs["remapRight0"] >> remapRight0;
        fs["remapRight1"] >> remapRight1;
    });
}

void Rectifier::remapStereo(const Mat& imgLeft, const Mat& imgRight, Mat &outRemappedLeft, Mat &outRemappedRight) {
    clock_t begin = clock();
    if (imgLeft.cols != remapLeft0.cols || imgLeft.rows != remapLeft0.rows ||
        imgRight.cols != remapRight0.cols || imgRight.rows != remapRight1.rows) {
        Logger::error("Wrong image resolution for remap matrices");
        outRemappedLeft = Mat(imgLeft.rows, imgLeft.cols, CV_8UC3, Scalar(0,0,0));
        outRemappedRight = Mat(imgRight.rows, imgRight.cols, CV_8UC3, Scalar(0,0,0));
        return;
    }
    remap(imgLeft, outRemappedLeft, remapLeft0, remapLeft1, CV_INTER_LINEAR);
    remap(imgRight, outRemappedRight, remapRight0, remapRight1, CV_INTER_LINEAR);
    clock_t end = clock();
    double elapsedSec = double(end - begin) / CLOCKS_PER_SEC;
    Logger::log("Remap time: %fs", elapsedSec);
} 