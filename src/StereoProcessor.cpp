#include <opencv2/core/mat.hpp>
#include "StereoProcessor.h"

bool StereoProcessor::init(SbmConfig &config) {
    sbm = createMatcher(config);
    if(!sbm) {
        return false;
    }
    if(config.disparityFiltering) {
        return initRightMatcher(sbm, config);
    }
    return true;
}

bool StereoProcessor::initRightMatcher(Ptr<StereoMatcher> leftMatcher, const SbmConfig &config) {
    rightMatcher = createRightMatcher(leftMatcher);
    filter = createDisparityWLSFilter(leftMatcher);
    filter->setLambda((float)config.disparityFilteringLambda);
    filter->setSigmaColor(config.disparityFilteringSigma / 1000);    
    return true;
}

void StereoProcessor::processImages(Mat &leftImg, Mat &rightImg) {
    if (!disparity.data) {
        disparity = Mat(leftImg.rows, leftImg.cols, CV_16S);
    }
    sbm->compute(leftImg, rightImg, disparity);
    
    if(rightMatcher) {
        rightMatcher->compute(rightImg, leftImg, rightDisparity);
        filter->filter(disparity, leftImg, filteredDisparity, rightDisparity);
    }
}
