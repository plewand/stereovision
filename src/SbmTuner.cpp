#include <opencv2/core/mat.hpp>

#include "SbmTuner.h"
#include "Config.h"
#include "Logger.h"
#include "SgbmProcessor.h"
#include "CvUtil.h"
#include "ConfigSerializer.h"
#include "Util3D.h"

static const char *DISPARITY_WIN_NAME = "disparity";
static const char *FILTERED_DISPARITY_WIN_NAME = "filtered disparity";
static const char *CONTROL_WIN_NAME = "controls";

static const int MIN_ALGORITHM_INDEX = 1;
static const int MAX_ALGORITHM_INDEX = 2;

//Values 0 - disabled

static const int MIN_DISP_NUM_MULTIPLIER = 1;
static const int MAX_DISP_NUM_MULTIPLIER = 20;
static const int MIN_BLOCK_SIZE = 3;
static const int MAX_BLOCK_SIZE = 79;
static const int MIN_PRE_FILTER_CAP = 0;
static const int MAX_PRE_FILTER_CAP = 63;
static const int MIN_UNIQENESS_RATIO = 0;
static const int MAX_UNIQENESS_RATIO = 45;

static const int MIN_PRE_FILTER_SIZE = 5;
static const int MAX_PRE_FILTER_SIZE = 255;
static const int MIN_FILTER_TYPE = 0;
static const int MAX_FILTER_TYPE = 1;
static const int MIN_SMALLER_BLOCK_SIZE = 0;
static const int MAX_SMALLER_BLOCK_SIZE = 151;
static const int MIN_TEXTURE_TRESHOLD = 0;
static const int MAX_TEXTURE_TRESHOLD = 151;

static const int MIN_MIN_DISPARITY = 0;
static const int MAX_MIN_DISPARITY = 151;
static const int MIN_P1 = 0; //8*number_of_image_channels*SADWindowSize*SADWindowSize
static const int MAX_P1 = 3000;
static const int MIN_P2 = 0; //32*number_of_image_channels*SADWindowSize*SADWindowSize
static const int MAX_P2 = 3000;
static const int MIN_DISP12_MAX_DIFF = 0; //0, disabled, maximum allowed difference (in integer pixel units) in the left-right disparity check
static const int MAX_DISP12_MAX_DIFF = 131;
static const int MIN_SPECLE_WINDOW_SIZE = 0; /// 50-200, 0 disabled
static const int MAX_SPECLE_WINDOW_SIZE = 600;
static const int MIN_SPECLE_RANGE_MULT_16 = 1; // 1,2 - only, if speckle size set
static const int MAX_SPECLE_RANGE_MULT_16 = 10;
static const int MIN_SGBM_MODE = 0; //MODE_SGBM = 0, MODE_HH   = 1, MODE_SGBM_3WAY = 2
static const int MAX_SGBM_MODE = 2;
static const int MIN_DISPARITY_FILTERING = 0; //0,1 - off,on
static const int MAX_DISPARITY_FILTERING = 1;
static const int MIN_DISPARITY_FILTERING_LAMBDA = 300; //0,1 - off,on
static const int MAX_DISPARITY_FILTERING_LAMBDA = 15000;
static const int MIN_DISPARITY_FILTERING_SIGMA = 200; //0,1 - off,on
static const int MAX_DISPARITY_FILTERING_SIGMA = 5000;
static const int MIN_SMOOTH_KERNEL_SIZE = 1;
static const int MAX_SMOOTH_KERNEL_SIZE = 55;

SbmTuner::SbmTuner() {
    newModeIndex = 1;
    disparityUpdateListener = nullptr;
}

SbmTuner::~SbmTuner() {
}

bool SbmTuner::init(const SbmConfig &config, function<void(const Mat &disparity) > disparityUpdateListener) {
    if (!initEngine(config, config.algorithm)) {
        return false;
    }
    this->disparityUpdateListener = disparityUpdateListener;
    createCommonControls();
    createSbmControls();
    createSgbmControls();

    return true;
}

bool SbmTuner::initEngine(const SbmConfig &config, int mode) {
    this->config = config;
    this->uiConfig = config;
    if (mode == StereoAlgorithm::SBM)
        newModeIndex = 1;
    else
        newModeIndex = 2;

    return initStereoProcessor(mode);
}

void SbmTuner::createCommonControls() {
    cvWin(DISPARITY_WIN_NAME);
    cvWin(CONTROL_WIN_NAME, WINDOW_AUTOSIZE);

    createTrackbar("Alg. [SBM=1, SGBM=2]", CONTROL_WIN_NAME, &newModeIndex, MAX_ALGORITHM_INDEX, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        if (t->newModeIndex == 1) {
            t->initEngine(t->config, StereoAlgorithm::SBM);
        } else if (t->newModeIndex == 2) {
            t->initEngine(t->config, StereoAlgorithm::SGBM);
        }
        t->runProcessor();
    }, this);
    createTrackbar("nDispMult16", CONTROL_WIN_NAME, &uiConfig.nDispMult16, MAX_DISP_NUM_MULTIPLIER, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("nDispMult16", t->config.nDispMult16, t->uiConfig.nDispMult16, MIN_DISP_NUM_MULTIPLIER, MAX_DISP_NUM_MULTIPLIER, Parity::ANY);
    }, this);
    createTrackbar("blockSize", CONTROL_WIN_NAME, &uiConfig.blockSize, MAX_BLOCK_SIZE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("blockSize", t->config.blockSize, t->uiConfig.blockSize, MIN_BLOCK_SIZE, MAX_BLOCK_SIZE, Parity::ODD);
    }, this);
    createTrackbar("preFilterCap", CONTROL_WIN_NAME, &uiConfig.preFilterCap, MAX_PRE_FILTER_CAP, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("preFilterCap", t->config.preFilterCap, t->uiConfig.preFilterCap, MIN_PRE_FILTER_CAP, MAX_PRE_FILTER_CAP, Parity::ANY);
    }, this);
    createTrackbar("uniquenesRatio", CONTROL_WIN_NAME, &uiConfig.uniquenesRatio, MAX_UNIQENESS_RATIO, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("uniquenesRatio", t->config.uniquenesRatio, t->uiConfig.uniquenesRatio, MIN_UNIQENESS_RATIO, MAX_UNIQENESS_RATIO, Parity::ANY);
    }, this);
    createTrackbar("minDisparity", CONTROL_WIN_NAME, &uiConfig.minDisparity, MAX_MIN_DISPARITY, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("minDisparity", t->config.minDisparity, t->uiConfig.minDisparity, MIN_MIN_DISPARITY, MAX_MIN_DISPARITY, Parity::ANY);
    }, this);
    createTrackbar("disp12MaxDiff", CONTROL_WIN_NAME, &uiConfig.disp12MaxDiff, MAX_DISP12_MAX_DIFF, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("disp12MaxDiff", t->config.disp12MaxDiff, t->uiConfig.disp12MaxDiff, MIN_DISP12_MAX_DIFF, MAX_DISP12_MAX_DIFF, Parity::ANY);
    }, this);
    createTrackbar("speckleWindowSize", CONTROL_WIN_NAME, &uiConfig.speckleWindowSize, MAX_SPECLE_WINDOW_SIZE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("speckleWindowSize", t->config.speckleWindowSize, t->uiConfig.speckleWindowSize, MIN_SPECLE_WINDOW_SIZE, MAX_SPECLE_WINDOW_SIZE, Parity::ANY);
    }, this);
    createTrackbar("speckleRange", CONTROL_WIN_NAME, &uiConfig.speckleRangeMult16, MAX_SPECLE_RANGE_MULT_16, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("speckleRange", t->config.speckleRangeMult16, t->uiConfig.speckleRangeMult16, MIN_SPECLE_RANGE_MULT_16, MAX_SPECLE_RANGE_MULT_16, Parity::ANY);
    }, this);
    createTrackbar("disparityFiltering", CONTROL_WIN_NAME, &uiConfig.disparityFiltering, MAX_DISPARITY_FILTERING, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("disparityFiltering", t->config.disparityFiltering, t->uiConfig.disparityFiltering, MIN_DISPARITY_FILTERING, MAX_DISPARITY_FILTERING, Parity::ANY);
    }, this);
    createTrackbar("disparityFilteringLambda", CONTROL_WIN_NAME, &uiConfig.disparityFilteringLambda, MAX_DISPARITY_FILTERING_LAMBDA, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("disparityFilteringLambda", t->config.disparityFilteringLambda, t->uiConfig.disparityFilteringLambda, MIN_DISPARITY_FILTERING_LAMBDA, MAX_DISPARITY_FILTERING_LAMBDA, Parity::ANY);
    }, this);
    createTrackbar("disparityFilteringSigma", CONTROL_WIN_NAME, &uiConfig.disparityFilteringSigma, MAX_DISPARITY_FILTERING_SIGMA, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("disparityFilteringSigma", t->config.disparityFilteringSigma, t->uiConfig.disparityFilteringSigma, MIN_DISPARITY_FILTERING_SIGMA, MAX_DISPARITY_FILTERING_SIGMA, Parity::ANY);
    }, this);
    createTrackbar("smoothKernelSize", CONTROL_WIN_NAME, &uiConfig.smoothKernelSize, MAX_SMOOTH_KERNEL_SIZE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("smoothKernelSize", t->config.smoothKernelSize, t->uiConfig.smoothKernelSize, MIN_SMOOTH_KERNEL_SIZE, MAX_SMOOTH_KERNEL_SIZE, Parity::ODD);
    }, this);

    setTrackbarMin("Alg. [SBM=1, SGBM=2]", CONTROL_WIN_NAME, MIN_ALGORITHM_INDEX);
    setTrackbarMin("nDispMult16", CONTROL_WIN_NAME, MIN_DISP_NUM_MULTIPLIER);
    setTrackbarMin("blockSize", CONTROL_WIN_NAME, MIN_BLOCK_SIZE);
    setTrackbarMin("preFilterCap", CONTROL_WIN_NAME, MIN_PRE_FILTER_CAP);
    setTrackbarMin("uniquenesRatio", CONTROL_WIN_NAME, MIN_UNIQENESS_RATIO);
    setTrackbarMin("minDisparity", CONTROL_WIN_NAME, MIN_MIN_DISPARITY);
    setTrackbarMin("disp12MaxDiff", CONTROL_WIN_NAME, MIN_DISP12_MAX_DIFF);
    setTrackbarMin("speckleWindowSize", CONTROL_WIN_NAME, MIN_SPECLE_WINDOW_SIZE);
    setTrackbarMin("speckleRange", CONTROL_WIN_NAME, MIN_SPECLE_RANGE_MULT_16);
    setTrackbarMin("disparityFiltering", CONTROL_WIN_NAME, MIN_DISPARITY_FILTERING);
    setTrackbarMin("disparityFilteringLambda", CONTROL_WIN_NAME, MIN_DISPARITY_FILTERING_LAMBDA);
    setTrackbarMin("disparityFilteringSigma", CONTROL_WIN_NAME, MIN_DISPARITY_FILTERING_SIGMA);
    setTrackbarMin("smoothKernelSize", CONTROL_WIN_NAME, MIN_SMOOTH_KERNEL_SIZE);
    waitKey(1);
}

void SbmTuner::createSbmControls() {
    createTrackbar("SBM preFilterSize", CONTROL_WIN_NAME, &uiConfig.sbmPreFilterSize, MAX_PRE_FILTER_SIZE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("SBM preFilterSize", t->config.sbmPreFilterSize, t->uiConfig.sbmPreFilterSize, MIN_PRE_FILTER_SIZE, MAX_PRE_FILTER_SIZE, Parity::ODD);
    }, this);
    createTrackbar("SBM filterType", CONTROL_WIN_NAME, &uiConfig.sbmFilterType, MAX_FILTER_TYPE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("SBM filterType", t->config.sbmFilterType, t->uiConfig.sbmFilterType, MIN_FILTER_TYPE, MAX_FILTER_TYPE, Parity::ANY);
    }, this);
    createTrackbar("SBM smallerBlockSize", CONTROL_WIN_NAME, &uiConfig.sbmSmallerBlockSize, MAX_SMALLER_BLOCK_SIZE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("smallerBlockSize", t->config.sbmSmallerBlockSize, t->uiConfig.sbmSmallerBlockSize, MIN_SMALLER_BLOCK_SIZE, MAX_SMALLER_BLOCK_SIZE, Parity::ANY);
    }, this);
    createTrackbar("SBM textureThreshold", CONTROL_WIN_NAME, &uiConfig.sbmTextureThreshold, MAX_TEXTURE_TRESHOLD, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("textureThreshold", t->config.sbmTextureThreshold, t->uiConfig.sbmTextureThreshold, MIN_TEXTURE_TRESHOLD, MAX_TEXTURE_TRESHOLD, Parity::ANY);
    }, this);

    setTrackbarMin("SBM preFilterSize", CONTROL_WIN_NAME, MIN_PRE_FILTER_SIZE);
    setTrackbarMin("SBM filterType", CONTROL_WIN_NAME, MIN_FILTER_TYPE);
    setTrackbarMin("SBM smallerBlockSize", CONTROL_WIN_NAME, MIN_SMALLER_BLOCK_SIZE);
    setTrackbarMin("SBM textureThreshold", CONTROL_WIN_NAME, MIN_TEXTURE_TRESHOLD);
}

void SbmTuner::createSgbmControls() {
    createTrackbar("SGBM P1", CONTROL_WIN_NAME, &uiConfig.sgbmP1, MAX_P1, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("SGBM P1", t->config.sgbmP1, t->uiConfig.sgbmP1, MIN_P1, MAX_P1, Parity::ANY);
    }, this);
    createTrackbar("SGBM P2", CONTROL_WIN_NAME, &uiConfig.sgbmP2, MAX_P2, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("SGBM P2", t->config.sgbmP2, t->uiConfig.sgbmP2, MIN_P2, MAX_P2, Parity::ANY);
    }, this);
    createTrackbar("SGBM mode", CONTROL_WIN_NAME, &uiConfig.sgbmMode, MAX_SGBM_MODE, [](int, void* tuner) -> void {
        SbmTuner *t = (SbmTuner*) tuner;
        t->onUpdate("disp12MaxDiff", t->config.sgbmMode, t->uiConfig.sgbmMode, MIN_SGBM_MODE, MAX_SGBM_MODE, Parity::ANY);
    }, this);

    setTrackbarMin("SGBM P1", CONTROL_WIN_NAME, MIN_P1);
    setTrackbarMin("SGBM P2", CONTROL_WIN_NAME, MIN_P2);
    setTrackbarMin("SGBM mode", CONTROL_WIN_NAME, MIN_SGBM_MODE);
}

bool SbmTuner::initStereoProcessor(int mode) {
    if (mode == StereoAlgorithm::SBM) {
        config.algorithm = StereoAlgorithm::SBM;
        stereoProcessor = new SbmProcessor();
    } else if (mode == StereoAlgorithm::SGBM) {
        config.algorithm = StereoAlgorithm::SGBM;
        stereoProcessor = new SgbmProcessor();
    } else {
        return false;
    }
    if (!stereoProcessor->init(config))
        return false;

    return true;
}

void SbmTuner::onUpdate(const char *valName, int &currentVal, int newVal, int min, int max, Parity parity) {
    int requestedSize = newVal;

    bool isEven = newVal % 2 == 0;
    if ((parity == Parity::EVEN && !isEven) || (parity == Parity::ODD && isEven)) {
        newVal++;
    }

    if (newVal < min) {
        newVal = min;
    } else if (newVal > max) {
        newVal = max;
    }

    if (requestedSize != newVal) {
        setTrackbarPos(valName, CONTROL_WIN_NAME, newVal);
    }
    if (currentVal == newVal) {
        return;
    }

    currentVal = newVal;

    Logger::debug("%s %d", valName, currentVal);

    rerunProcessor();
}

void SbmTuner::processImages(Mat &leftImg, Mat &rightImg) {
    if (stereoProcessor) {
        if (leftImg.type() == CV_8UC3) {
            cv::cvtColor(leftImg, leftGrayImg, CV_BGR2GRAY);
            cv::cvtColor(rightImg, rightGrayImg, CV_BGR2GRAY);
        } else {
            leftGrayImg = leftImg;
            rightGrayImg = rightImg;            
        }
        leftGrayImg.copyTo(initLeftGrayImg);
        rightGrayImg.copyTo(initRightGrayImg);

        runProcessor();
    }
}

void SbmTuner::rerunProcessor() {
    if (initEngine(config, config.algorithm)) {
        runProcessor();
    } else {
        Logger::log("Error setting new SBM parameters");
    }
}

void SbmTuner::runProcessor() {
    if (leftGrayImg.empty() || rightGrayImg.empty()) {
        return;
    }

    int ks = config.smoothKernelSize;
    if (ks > 1) {
        GaussianBlur(initLeftGrayImg, leftGrayImg, Size(ks, ks), 0, 0);
        GaussianBlur(initRightGrayImg, rightGrayImg, Size(ks, ks), 0, 0);
    }

    clock_t begin = clock();

    stereoProcessor->processImages(leftGrayImg, rightGrayImg);

    clock_t end = clock();
    double elapsedSec = double(end - begin) / CLOCKS_PER_SEC;

    Logger::log("SBM Time: %fs", elapsedSec);

    double minVal;
    double maxVal;

    minMaxLoc(stereoProcessor->getDisparity(), &minVal, &maxVal);

    Mat imgDisparity8U;
    stereoProcessor->getDisparity().convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));

    if (disparityUpdateListener != nullptr) {
        disparityUpdateListener(stereoProcessor->getDisparity());
    }

    imshow(DISPARITY_WIN_NAME, imgDisparity8U);
    const Mat &filteredDepth = stereoProcessor->getFilteredDisparity();
    if (!filteredDepth.empty()) {
        namedWindow(FILTERED_DISPARITY_WIN_NAME);
        minMaxLoc(filteredDepth, &minVal, &maxVal);
        filteredDepth.convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));
        imshow(FILTERED_DISPARITY_WIN_NAME, imgDisparity8U);
    } else {
        destroyWindow(FILTERED_DISPARITY_WIN_NAME);
    }
}

const Mat& SbmTuner::getDisparity() {
    if (!stereoProcessor->getFilteredDisparity().empty()) {
        return stereoProcessor->getFilteredDisparity();
    }
    return stereoProcessor->getDisparity();
}

const SbmConfig& SbmTuner::getSbmConfig() const {
    return config;
}
