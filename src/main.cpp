#include <chrono>
#include "CalibEngine.h"
#include "Config.h"
#include "ChessboardDetectEngine.h"
#include "ConfigSerializer.h"
#include "RunMode.h"
#include "Rectifier.h"
#include "Logger.h"
#include "SbmTuner.h"
#include "CvUtil.h"

int main(int argc, const char** argv) {
    Config config;        
    if (!loadConfig(CONFIG_PATH, argc, argv, config)) {
        return ReturnCode::IO_ERROR;
    }

    if (config.runMode & RunMode::CHESSBOARD) {
        auto ret = runChessboardDetection(config);
        if(ret != ReturnCode::OK) {
            return ret;
        }
    }

    if (config.runMode & RunMode::CALIB) {
        auto ret = runCalibration(config);        
        if(ret != ReturnCode::OK) {
            return ret;
        }
    }   

    if (config.runMode & RunMode::TEST_IMAGES) {
        auto ret = runImageTest(config);
        if(ret != ReturnCode::OK) {
            return ret;
        }        
    }   

    if (config.runMode & RunMode::TEST_CAMERA) {
       auto ret = runCameraTest(config);
        if(ret != ReturnCode::OK) {
            return ret;
        }        
    }

    return ReturnCode::OK;
}
