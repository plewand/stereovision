#include "SbmProcessor.h"
#include "Logger.h"

SbmProcessor::SbmProcessor() {
}

SbmProcessor::~SbmProcessor() {
}

Ptr<StereoMatcher> SbmProcessor::createMatcher(const SbmConfig &config) {
    int blockSize = config.blockSize >= 5 ? config.blockSize : 5;
    int preFilterCap = config.preFilterCap >= 1 ? config.preFilterCap : 1;
    
    Ptr<StereoBM> sbm = StereoBM::create(config.nDispMult16*16, blockSize);
    sbm->setUniquenessRatio(config.uniquenesRatio);    
    sbm->setPreFilterCap(preFilterCap);    
    
    sbm->setPreFilterSize(config.sbmPreFilterSize);     
    sbm->setPreFilterType(config.sbmFilterType);
    sbm->setSmallerBlockSize(config.sbmSmallerBlockSize);
    sbm->setTextureThreshold(config.sbmTextureThreshold);
    sbm->setMinDisparity(config.minDisparity);
    sbm->setSpeckleWindowSize(config.speckleWindowSize);
    sbm->setSpeckleRange(config.speckleRangeMult16 * 16);
    sbm->setDisp12MaxDiff(config.disp12MaxDiff);
    return sbm;
}