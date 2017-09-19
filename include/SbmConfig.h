#ifndef SMBCONFIG_H
#define SMBCONFIG_H

#include <opencv2/calib3d.hpp>

enum StereoAlgorithm {
    SBM = 0,
    SGBM
};

#define PP(a) #a << "=" << a << endl

struct SbmConfig {
    int algorithm;
    int nDispMult16;
    int blockSize;
    int uniquenesRatio;
    int preFilterCap;
    int disp12MaxDiff;
    int minDisparity;
    int speckleWindowSize;
    int speckleRangeMult16;
    int sbmPreFilterSize;
    int sbmFilterType;
    int sbmSmallerBlockSize;
    int sbmTextureThreshold;
    int sgbmP1;
    int sgbmP2;
    int sgbmMode;
    int disparityFiltering;
    int disparityFilteringLambda;
    int disparityFilteringSigma;
    int smoothKernelSize;

    SbmConfig() {
        algorithm = StereoAlgorithm::SBM;
        nDispMult16 = 4;
        blockSize = 19;
        uniquenesRatio = 15;
        preFilterCap = 17;
        minDisparity = 1;
        disp12MaxDiff = 1;
        speckleWindowSize = 400;
        speckleRangeMult16 = 200;

        sbmPreFilterSize = 9;
        sbmFilterType = StereoBM::PREFILTER_NORMALIZED_RESPONSE; // PREFILTER_NORMALIZED_RESPONSE = 0, PREFILTER_XSOBEL = 1 
        sbmSmallerBlockSize = 1;
        sbmTextureThreshold = 5;

        sgbmP1 = 100;
        sgbmP2 = 1000;
        sgbmMode = StereoSGBM::MODE_SGBM; //  StereoSGBM::MODE_HH
        
        disparityFiltering = 0;
        disparityFilteringLambda = 8000;
        disparityFilteringSigma = 1000;
        smoothKernelSize = 21;
    }

    string toString() {
        stringstream s;
        s << PP(algorithm)
                << PP(nDispMult16)
                << PP(blockSize)
                << PP(uniquenesRatio)
                << PP(preFilterCap)
                << PP(disp12MaxDiff)
                << PP(minDisparity)
                << PP(speckleWindowSize)
                << PP(speckleRangeMult16)
                << PP(sbmPreFilterSize)
                << PP(sbmFilterType)
                << PP(sbmSmallerBlockSize)
                << PP(sbmTextureThreshold)
                << PP(sgbmP1)
                << PP(sgbmP2)
                << PP(sgbmMode)
                << PP(disparityFiltering)
                << PP(disparityFilteringLambda)
                << PP(disparityFilteringSigma)
                << PP(smoothKernelSize);
        return s.str();
    }

};

#endif /* SMBCONFIG_H */

