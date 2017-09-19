#ifndef CHESSBOARDDETECTENGINE_H
#define CHESSBOARDDETECTENGINE_H

#include "stereoheaders.h"
#include "Config.h"
#include "ImgSourceCamera.h"

class ChessboardDetectEngine {
public:
    ChessboardDetectEngine();    
    virtual ~ChessboardDetectEngine();
    bool run(const Config &config);
private:
    int getInitalImageNum(const string &imageDir);
    void drawCounter(Mat &counterImg, int counter, bool lastOk);
    ImgSourceCamera camera;
};

#endif /* CAPTUREENGINE_H */

