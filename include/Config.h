#ifndef CONFIG_H
#define CONFIG_H

#include "stereoheaders.h"
#include "SbmConfig.h"

struct Config {
    string logLevel;
    Size boardSize;
    float fieldSize;
    int leftCamId;
    int rightCamId;
    int captureWidth;
    int captureHeight;
    string chessboardImagesDir;
    bool overwriteImages;
    int runMode;
    int chessboardSleep;
    int remapWidth;
    int remapHeight;
    int show3d;
    string leftTestImage;
    string rightTestImage;
    SbmConfig sbm;

    string toString() {
        stringstream s;
        s << PP(logLevel)
                << PP(boardSize)
                << PP(fieldSize)
                << PP(leftCamId)
                << PP(rightCamId)
                << PP(captureWidth)
                << PP(captureHeight)
                << PP(chessboardImagesDir)
                << PP(overwriteImages)
                << PP(runMode)
                << PP(chessboardSleep)
                << PP(remapWidth)
                << PP(remapHeight)
                << PP(show3d)
                << PP(leftTestImage)
                << PP(rightTestImage)
                << sbm.toString();

        return s.str();
    }
};

#endif /* CONFIG_H */

