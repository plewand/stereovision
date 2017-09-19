#include "ChessboardDetectEngine.h"
#include "Logger.h"
#include <chrono>
#include <ctime>
#include <experimental/filesystem>
#include "SysUtil.h"

namespace fs = std::experimental::filesystem;

ChessboardDetectEngine::ChessboardDetectEngine() {
}

ChessboardDetectEngine::~ChessboardDetectEngine() {
 
}

bool ChessboardDetectEngine::run(const Config &config) {
    if(!camera.init(config)) {
        return false;
    }

    bool useTimer = config.chessboardSleep > 0;
    
    Mat counterImg;

    if (useTimer) {
        counterImg.create(140, 140, CV_8UC3);
        namedWindow("counter", WINDOW_NORMAL);
    }

    if (config.overwriteImages && fs::exists(config.chessboardImagesDir)) {
        TimeInfo timeInfo;
        getCurrentTime(timeInfo);        
        char suffix[64];
        sprintf(suffix, "%02d:%02d:%02d.%.3d", timeInfo.h, timeInfo.m, timeInfo.s, timeInfo.ms);
        fs::copy(config.chessboardImagesDir, config.chessboardImagesDir + "-" + suffix, fs::copy_options::recursive);
        fs::remove_all(config.chessboardImagesDir);
    }

    if (!fs::exists(config.chessboardImagesDir)) {
        if (!fs::create_directories(config.chessboardImagesDir)) {
            Logger::log("Problem with creating output dir");
            return false;
        }
    }

    namedWindow("left");
    namedWindow("right");
    int validNum = getInitalImageNum(config.chessboardImagesDir);
    int capturedNum = 0;
    vector < Point2f> corners;

    if (useTimer) {
        counterImg = cv::Scalar(0);
        imshow("counter", counterImg);
    }

    chrono::time_point<chrono::system_clock> lastCounterIncTime = chrono::system_clock::now();
    int counter = config.chessboardSleep;
    bool lastOk = true;

    for (;;) {
        camera.capture();

        imshow("left", camera.getLeft());
        imshow("right", camera.getRight());

        bool getImg = false;
        if (useTimer) {
            chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
            chrono::duration<double> duration = now - lastCounterIncTime;
            if (duration.count() > 1) {
                counter--;
                lastCounterIncTime = now;
                if (counter == 0) {
                    counter = config.chessboardSleep;
                    getImg = true;
                }
                drawCounter(counterImg, counter, lastOk);
            }
        }
        char key = (char) (waitKey(5) & 0xff);
        if (key == 27) {
            break;
        } else if (key != -1) {
            getImg = true;
            counter = config.chessboardSleep;
            imwrite("outimages/last_left.ppm", camera.getLeft());
            imwrite("outimages/last_right.ppm", camera.getRight());
        }

        if (!getImg) {
            continue;
        }

        corners.clear();
        bool foundLeft = findChessboardCorners(camera.getLeft(), config.boardSize, corners,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

        corners.clear();
        bool foundRight = findChessboardCorners(camera.getRight(), config.boardSize, corners,
                CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

        if (foundLeft && foundRight) {
            Logger::log("Image pair ok");
            validNum++;
            char fname[64];
            sprintf(fname, "/left_%03d.ppm", validNum);
            imwrite(config.chessboardImagesDir + fname, camera.getLeft());
            sprintf(fname, "/right_%03d.ppm", validNum);
            imwrite(config.chessboardImagesDir + fname, camera.getRight());
            lastOk = true;
        } else {
            lastOk = false;
            Logger::log("Some image not found left: %d, right %d", foundLeft, foundRight);
        }
        drawCounter(counterImg, counter, lastOk);
        capturedNum++;
    }
    Logger::log("Valid pairs %d of captured %d", validNum, capturedNum);
    return validNum > 0;
}

int ChessboardDetectEngine::getInitalImageNum(const string &imageDir) {
    vector<cv::String> fn;
    glob(imageDir + "/*", fn, false);
    return fn.size() / 2;
}

void ChessboardDetectEngine::drawCounter(Mat &counterImg, int counter, bool lastOk) {
    int font = FONT_HERSHEY_SIMPLEX;
    char tx[16];
    sprintf(tx, "%d", counter);
    counterImg = cv::Scalar(0);
    putText(counterImg, tx, Point(counterImg.cols / 2 - 40, counterImg.rows / 2 + 40), font, 4, Scalar(0, 255, 0), 5);
    Scalar col;
    if (lastOk) {
        col = Scalar(0, 255, 0);
    } else {
        col = Scalar(0, 0, 255);
    }
    circle(counterImg, Point(10, 10), 7, col, CV_FILLED);
    imshow("counter", counterImg);
}