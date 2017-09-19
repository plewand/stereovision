#ifndef CALIBENGINE_H
#define CALIBENGINE_H

#include <functional>

#include "stereoheaders.h"
#include "Config.h"

class CalibEngine {
public:
    CalibEngine();
    CalibEngine(const CalibEngine& orig);
    bool calib(const Config &config);
    virtual ~CalibEngine();
private:
    bool buildCorners(int index, const string &folderStr, const char *side, const string &ext, Size boardSize, vector<vector < Point2f>> &allCorners, Size &outImgSize, vector<bool> &chessboardFound);
    bool stereoCalib(const vector<vector < Point2f>> &allLeftCorners, const vector<vector < Point2f>> &allRightCorners, Size boardSize, float fieldSize, const Size &imgSize, const Size &remapSize);
    void findCameraIntrinsic(const vector<vector < Point3f>> &objPoints, const vector<vector < Point2f>> &imgPoints, const Size& imgSize, const Mat& cameraMatrix, Mat& distCoefs);
    void buildObjectPoints(float fieldSize, Size boardSize, vector<vector<Point3f>> &objectPoints);
    bool calcEpilines(const vector<vector < Point2f>> &allLeftCorners,
            const vector<vector < Point2f>> &allRightCorners,
            const Mat &cameraMatrixLeft,
            const Mat &cameraMatrixRight,
            const Mat &distCoeffsLeft,
            const Mat &distCoeffsRight,
            const Mat &F);
    bool rectify(const Mat& cameraMatrixLeft,
            const Mat& cameraMatrixRight,
            const Mat& distCoeffsLeft,
            const Mat& distCoeffsRight,
            Size imageSize,
            const Mat& R,
            const Mat& T,
            Mat& outR1,
            Mat& outR2,
            Mat& outP1,
            Mat& outP2,
            Rect *validRoiLeft,
            Rect *validRoiRight);
    void createRemapMatrices(const Mat& cameraMatrixLeft,
            const Mat& cameraMatrixRight,
            const Mat& distCoeffsLeft,
            const Mat& distCoeffsRight,
            Size imageSize,
            const Mat& R1,
            const Mat& P1,
            const Mat& R2,
            const Mat& P2,
            Mat& outRemapLeft0,
            Mat& outRemapLeft1,
            Mat& outRemapRight0,
            Mat& outRemapRight1);
    void printMat(const char *name, const Mat &m);    
    void resetCameraIntrinsic(Mat &mat, const Size &imgSize);
};

#endif /* CALIBENGINE_H */

