#ifndef STEREOPROCESSOR_H
#define STEREOPROCESSOR_H

#include "stereoheaders.h"
#include "SbmConfig.h"
#include "opencv2/ximgproc/disparity_filter.hpp"

using namespace ximgproc;

class StereoProcessor {
public:    
    virtual ~StereoProcessor(){};
    bool init(SbmConfig &config);
    void processImages(Mat &leftImg, Mat &rightImg);
    
    Mat& getDisparity() {
        return disparity;
    }
    
    const Mat& getFilteredDisparity() const {
        return filteredDisparity;
    }
    
protected:    
    virtual Ptr<StereoMatcher> createMatcher(const SbmConfig &config) = 0;
private:
    bool initRightMatcher(Ptr<StereoMatcher> leftMatcher, const SbmConfig &config);
    Ptr<StereoMatcher> sbm;
    Mat disparity;
    SbmConfig sbmParams;
    Ptr<StereoMatcher> rightMatcher;
    Ptr<DisparityWLSFilter> filter;
    Mat rightDisparity;
    Mat filteredDisparity;    
};

#endif /* STEREOPROCESSOR_H */

