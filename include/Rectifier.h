#ifndef RECTIFIER_H
#define RECTIFIER_H

#include "stereoheaders.h"

class Rectifier {
public:
    bool load(const char *remaps);
    void remapStereo(const Mat& imgLeft, const Mat& imgRight, Mat &outRemappedLeft, Mat &outRemappedRight);
    Rectifier();    
    virtual ~Rectifier();
private:
    Mat remapLeft0;
    Mat remapLeft1;
    Mat remapRight0;
    Mat remapRight1;
};

#endif /* RECTIFIER_H */

