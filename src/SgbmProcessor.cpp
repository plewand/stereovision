#include "SgbmProcessor.h"
#include "Logger.h"

SgbmProcessor::SgbmProcessor() {
}

SgbmProcessor::~SgbmProcessor() {
}

Ptr<StereoMatcher> SgbmProcessor::createMatcher(const SbmConfig &config) {
    return StereoSGBM::create(
            config.minDisparity, 
            config.nDispMult16 * 16, 
            config.blockSize,
            config.sgbmP1,
            config.sgbmP2,
            config.disp12MaxDiff,            
            config.preFilterCap,
            config.uniquenesRatio,
            config.speckleWindowSize,
            config.speckleRangeMult16 * 16,
            config.sgbmMode);        
}		
