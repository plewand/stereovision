#include "RunMode.h"
#include <chrono>
#include <GL/freeglut_std.h>
#include "CalibEngine.h"
#include "Config.h"
#include "ChessboardDetectEngine.h"
#include "ConfigSerializer.h"
#include "RunMode.h"
#include "Rectifier.h"
#include "Logger.h"
#include "SbmTuner.h"
#include "CvUtil.h"

const char *WIN_LEFT_RECTIFIED = "left rectified";
const char *WIN_RIGHT_RECTIFIED = "right rectified";

void setConfigSaveCallback(const Config &config, const SbmTuner &tuner);
void drawCorrespondenceLines(Mat &remappedLeft, Mat &remmapperRight);
void syncBlur(Mat &left, Mat &right, SbmTuner &tuner);

ReturnCode runChessboardDetection(const Config &config) {
    ChessboardDetectEngine chessboardDetectEngine;
    if (!chessboardDetectEngine.run(config)) {
        return ReturnCode::CHESSBOARD_DETECT_ERROR;
    }
    return ReturnCode::OK;
}

ReturnCode runCalibration(const Config &config) {
    CalibEngine calibEngine;
    if (!calibEngine.calib(config)) {
        return ReturnCode::CALIBRATION_ERROR;
    }
    return ReturnCode::OK;
}

ReturnCode runImageTest(const Config &config) {
    SbmTuner sbmTuner;

    Rectifier rectifier;
    if (!rectifier.load("mappings.yml")) {
        return ReturnCode::IMAGE_TEST_ERROR;
    }

    Mat inputLeft = imread(config.leftTestImage);
    Mat inputRight = imread(config.rightTestImage);

    if (!inputLeft.data || !inputRight.data) {
        Logger::log("No images captred to rectify");
        return ReturnCode::IMAGE_TEST_ERROR;
    }

    Mat remappedLeft;
    Mat remappedRight;
    rectifier.remapStereo(inputLeft, inputRight, remappedLeft, remappedRight);

    Mat Q;
    if (!loadYaml("Q.yml", [&Q](auto fs) -> auto {
            fs["Q"] >> Q;
        })) {
        return ReturnCode::IMAGE_TEST_ERROR;
    }

    if (!sbmTuner.init(config.sbm, [Q, &remappedLeft, &remappedRight, &rectifier, &inputLeft, &inputRight, &sbmTuner](auto disparity) -> void {            
            Reprojector3D::getReprojector()->reproject(disparity, Q, remappedLeft);
            syncBlur(remappedLeft, remappedRight, sbmTuner);
            drawCorrespondenceLines(remappedLeft, remappedRight);
            imshow(WIN_LEFT_RECTIFIED, remappedLeft);
            imshow(WIN_RIGHT_RECTIFIED, remappedRight);
            //To clear gaussian blur
            rectifier.remapStereo(inputLeft, inputRight, remappedLeft, remappedRight);
        })) {
        return ReturnCode::IMAGE_TEST_ERROR;
    }

    setConfigSaveCallback(config, sbmTuner);

    sbmTuner.processImages(remappedLeft, remappedRight);
    
    syncBlur(remappedLeft, remappedRight, sbmTuner);
    drawCorrespondenceLines(remappedLeft, remappedRight);
    namedWindow(WIN_LEFT_RECTIFIED);
    namedWindow(WIN_RIGHT_RECTIFIED);
    imshow(WIN_LEFT_RECTIFIED, remappedLeft);
    imshow(WIN_RIGHT_RECTIFIED, remappedRight);

    Reprojector3D::getReprojector()->loop();
    return ReturnCode::OK;
}

void drawCorrespondenceLines(Mat &remappedLeft, Mat &remappedRight) {
    int dy = 30;
    for (int i = dy; i < remappedRight.rows; i += dy) {
        line(remappedLeft, Point(0, i), Point(remappedRight.cols, i), Scalar(0, 255, 0));
        line(remappedRight, Point(0, i), Point(remappedRight.cols, i), Scalar(0, 255, 0));
    }
}

ReturnCode runCameraTest(const Config &config) {
    SbmTuner sbmTuner;
    ImgSourceCamera camera;

    Rectifier rectifier;
    if (!rectifier.load("mappings.yml")) {
        return ReturnCode::CAMERA_TEST_ERROR;
    }

    if (!camera.init(config)) {
        return ReturnCode::CAMERA_TEST_ERROR;
    }

    Mat Q;

    if (!loadYaml("Q.yml", [&Q](auto fs) -> auto {
            fs["Q"] >> Q;
        })) {
        return ReturnCode::CAMERA_TEST_ERROR;
    }

    if (!sbmTuner.init(config.sbm, [Q, &camera](auto disparity) -> void {      
        })) {
        return ReturnCode::CAMERA_TEST_ERROR;
    }

    namedWindow(WIN_LEFT_RECTIFIED);
    namedWindow(WIN_RIGHT_RECTIFIED);

    setConfigSaveCallback(config, sbmTuner);

    Reprojector3D::getReprojector()->setIdleFunc([&camera, &sbmTuner, &Q, &rectifier]() {
        static chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
        chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

        auto difference = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        if (difference.count() < 100) {
            return;
        }

        start = std::chrono::system_clock::now();

        camera.capture();
        Mat remappedLeft;
        Mat remappedRight;
        rectifier.remapStereo(camera.getLeft(), camera.getRight(), remappedLeft, remappedRight);
        sbmTuner.processImages(remappedLeft, remappedRight);
        Reprojector3D::getReprojector()->reproject(sbmTuner.getDisparity(), Q, camera.getLeft());
        syncBlur(remappedLeft, remappedRight, sbmTuner);
        drawCorrespondenceLines(remappedLeft, remappedRight);
        imshow(WIN_LEFT_RECTIFIED, remappedLeft);
        imshow(WIN_RIGHT_RECTIFIED, remappedRight);
    }
    );
    Reprojector3D::getReprojector()->loop();
    return ReturnCode::OK;
}

//To be better visible, what initial image is processed
void syncBlur(Mat &left, Mat &right, SbmTuner &tuner) {
    int ks = tuner.getSbmConfig().smoothKernelSize;
    if (ks > 1) {
        GaussianBlur(left, left, Size(ks, ks), 0, 0);
        GaussianBlur(right, right, Size(ks, ks), 0, 0);
    }
}

void setConfigSaveCallback(const Config &config, const SbmTuner &sbmTuner) {
    auto func = [&config, &sbmTuner](int key) -> auto {
        if (key != 10 && key != 13) {
            return;
        }
        //TODO: Change it. make static calls
        Logger::log("Saving config");
        Config curConfig = config;
        curConfig.sbm = sbmTuner.getSbmConfig();
        if (!saveConfig(CONFIG_PATH, curConfig)) {
            Logger::error("Config update failed");
        }
    };

    Reprojector3D::getReprojector()->setKeyboardFunc(func);
    Reprojector3D::getReprojector()->setKeyboardOpenCvFunc(func);
}