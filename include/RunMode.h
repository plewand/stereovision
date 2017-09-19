#ifndef RUNMODE_H
#define RUNMODE_H

#include "Config.h"

#define CONFIG_PATH "config.properties"

enum RunMode {
    CHESSBOARD = 1,
    CALIB = 2,
    TEST_IMAGES = 4,
    TEST_CAMERA = 8
};

#define RETURN_CODE_ERROR_START (-100)

enum ReturnCode {
    OK = 0,
    CHESSBOARD_DETECT_ERROR = RETURN_CODE_ERROR_START,
    CALIBRATION_ERROR,
    IMAGE_TEST_ERROR,
    CAMERA_TEST_ERROR,
    IO_ERROR
};


ReturnCode runChessboardDetection(const Config &config);
ReturnCode runCalibration(const Config &config);
ReturnCode runImageTest(const Config &config);
ReturnCode runCameraTest(const Config &config);

#endif /* RUNMODE_H */

