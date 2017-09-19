#ifndef SBMTUNER_H
#define SBMTUNER_H

#include "stereoheaders.h"
#include "SbmConfig.h"
#include "SbmProcessor.h"
#include "Reprojector3D.h"
#include <functional>

enum Parity {
    ODD,
    EVEN,
    ANY
};

class SbmTuner {
public:
    bool init(const SbmConfig &config, function<void(const Mat &disparity)> disparityUpdateListener);
    void processImages(Mat &leftImg, Mat &rightImg);
    const Mat& getDisparity();
    const SbmConfig& getSbmConfig() const;
    SbmTuner();    
    virtual ~SbmTuner();
private:    
    void onUpdate(const char *valName, int &currentVal, int newVal, int min, int max, Parity parity);
        
    void runProcessor(); 
    void rerunProcessor(); 
    bool initStereoProcessor(int mode);
    bool initEngine(const SbmConfig &config, int mode);
    
    void createCommonControls();
    void createSbmControls();
    void createSgbmControls();
         
    int newModeIndex;    
    
    SbmConfig config;
    SbmConfig uiConfig;
    Mat leftGrayImg;
    Mat rightGrayImg;    
    Mat initLeftGrayImg;
    Mat initRightGrayImg;    
    Ptr<StereoProcessor> stereoProcessor;
    function<void(const Mat &disparity)> disparityUpdateListener;    
};

#endif /* SBMTUNER_H */

