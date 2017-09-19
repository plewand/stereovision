#include "CalibEngine.h"
#include "Logger.h"
#include "CvUtil.h"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

static const string DETECTED_CHESSBOARD_PATH = "./outimages/calib/chessboard";

CalibEngine::CalibEngine() {
}

CalibEngine::CalibEngine(const CalibEngine& orig) {
}

CalibEngine::~CalibEngine() {
}

bool CalibEngine::calib(const Config &config) {
    vector<cv::String> fn;
    glob(config.chessboardImagesDir + "/*.ppm", fn, false);
    if (fn.size() == 0 || fn.size() % 2 != 0) {
        return FAIL("Invalid file number");
    }
    int pairNum = fn.size() / 2;
    String firstElem = fn[0];
    String ext = firstElem.substr(firstElem.find_last_of('.'), 4);

    if (!fs::exists(DETECTED_CHESSBOARD_PATH) && !fs::create_directories(DETECTED_CHESSBOARD_PATH)) {
        return FAIL("Cannot create chessboard dir");
    }

    vector<vector < Point2f>> allLeftCorners;
    allLeftCorners.resize(pairNum);
    vector<vector < Point2f>> allRightCorners;
    allRightCorners.resize(pairNum);

    vector<bool> chessboardFoundLeft;
    chessboardFoundLeft.resize(pairNum);
    vector<bool> chessboardFoundRight;
    chessboardFoundRight.resize(pairNum);
    Size imgSize;

    for (int i = 0; i < pairNum; i++) {
        if (!buildCorners(i, config.chessboardImagesDir, "left_", ext, config.boardSize, allLeftCorners, imgSize, chessboardFoundLeft)) {
            return false;
        }
        if (!buildCorners(i, config.chessboardImagesDir, "right_", ext, config.boardSize, allRightCorners, imgSize, chessboardFoundRight)) {
            return false;
        }
    }

    vector<vector < Point2f>> correctLeftCorners;
    vector<vector < Point2f>> correctRightCorners;
    for (int i = 0; i < pairNum; i++) {
        if (!chessboardFoundLeft[i] || !chessboardFoundRight[i]) {
            printf("Removing stereo pair %d\n", i + 1);
            continue;
        }
        correctLeftCorners.push_back(allLeftCorners[i]);
        correctRightCorners.push_back(allRightCorners[i]);
    }
    Logger::log("Correct pairs pairs %d / %d", (int) correctLeftCorners.size(), pairNum);
    Logger::log("Start tuning, it can take a few minutes", (int) correctLeftCorners.size(), pairNum);
    if (correctLeftCorners.size() == 0) {
        printf("No chessboards found\n");
        return false;
    }
    Size remapSize;
    if (config.remapWidth > 0 && config.remapHeight > 0) {
        remapSize = Size(config.remapWidth, config.remapHeight);
    } else {
        remapSize = imgSize;
    }
    if (!stereoCalib(correctLeftCorners, correctRightCorners, config.boardSize, config.fieldSize, imgSize, remapSize)) {
        return false;
    }

    return true;
}

bool CalibEngine::buildCorners(int index, const string &folderStr, const char *side, const string &ext, Size boardSize, vector<vector < Point2f>> &allCorners, Size &outImgSize, vector<bool> &chessboardFound) {
    int numSize = 3;

    stringstream numPrefix;
    numPrefix << std::setfill('0') << std::setw(numSize) << index + 1;

    const string coreName = side + numPrefix.str();
    const string filename = folderStr + "/" + coreName + ext;
    Mat img = imread(filename, 0);
    if (img.empty()) {
        return FAIL("Img read failed: " + filename);
    }

    outImgSize = img.size();

    vector < Point2f> &corners = allCorners[index];
    bool found = findChessboardCorners(img, boardSize, corners,
            CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_NORMALIZE_IMAGE);

    if (!found) {
        printf("No corners found for: %s\n", filename.c_str());
        chessboardFound[index] = false;
        return true;
    }
    chessboardFound[index] = true;
    cornerSubPix(img, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 30, 0.01));
    drawChessboardCorners(img, boardSize, corners, found);
    string outPath = DETECTED_CHESSBOARD_PATH + "/corners_" + coreName + ext;
    imwrite(outPath, img);

    return true;
}

void CalibEngine::printMat(const char *name, const Mat &m) {
    cout << name << " = " << endl << " " << m << endl << endl;
}

bool CalibEngine::stereoCalib(const vector<vector < Point2f>> &allLeftCorners, const vector<vector < Point2f>> &allRightCorners, Size boardSize, float fieldSize, const Size &imgSize, const Size &remapSize) {
    vector<vector < Point3f>> objectPoints;
    objectPoints.resize(allLeftCorners.size());
    buildObjectPoints(fieldSize, boardSize, objectPoints);

    Mat cameraMatrixLeft = Mat::eye(3, 3, CV_64F);
    Mat cameraMatrixRight = Mat::eye(3, 3, CV_64F);
    Mat distCoeffsLeft;
    Mat distCoeffsRight;
    Mat R, T, E, F;

    resetCameraIntrinsic(cameraMatrixLeft, imgSize);
    resetCameraIntrinsic(cameraMatrixRight, imgSize);

    findCameraIntrinsic(objectPoints, allLeftCorners, imgSize, cameraMatrixLeft, distCoeffsLeft);
    findCameraIntrinsic(objectPoints, allRightCorners, imgSize, cameraMatrixRight, distCoeffsRight);

    resetCameraIntrinsic(cameraMatrixLeft, imgSize);
    resetCameraIntrinsic(cameraMatrixRight, imgSize);

    int flags = CALIB_FIX_INTRINSIC;
    TermCriteria criteria = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, 1e-6);

    //R – Output rotation matrix between the 1st and the 2nd camera coordinate systems.
    //T – Output translation vector between the coordinate systems of the cameras.
    //E – Output essential matrix.
    //F – Output fundamental matrix.
    double rms = stereoCalibrate(objectPoints, allLeftCorners, allRightCorners,
            cameraMatrixLeft, distCoeffsLeft,
            cameraMatrixRight, distCoeffsRight,
            imgSize, R, T, E, F,
            flags,
            criteria);


    printMat("cameraMatrixLeft", cameraMatrixLeft);
    printMat("cameraMatrixRight", cameraMatrixRight);
    printMat("distCoeffsLeft", distCoeffsLeft);
    printMat("distCoeffsRight", distCoeffsRight);
    printMat("rotation", R);
    printMat("translation", T);
    printMat("essential", E);
    printMat("fundamental", F);

    Mat R1, R2, P1, P2, Q;
    Mat remapLeft0, remapLeft1, remapRight0, remapRight1;
    Rect validRoiLeft, validRoiRight;

    if(!rectify(cameraMatrixLeft, cameraMatrixRight, distCoeffsLeft, distCoeffsRight,
            imgSize, R, T, R1, R2, P1, P2, &validRoiLeft, &validRoiRight)) {
        return false;
    }

    createRemapMatrices(cameraMatrixLeft, cameraMatrixRight, distCoeffsLeft, distCoeffsRight,
            remapSize, R1, P1, R2, P2, remapLeft0, remapLeft1, remapRight0, remapRight1);        

    if(!saveYaml("remap.yml", [remapLeft0, remapLeft1, remapRight0, remapRight1](auto fs) -> auto {
        fs << "remapLeft0" << remapLeft0 << "remapLeft1" << remapLeft1 <<
                "remapRight0" << remapRight0 << "remapRight1" << remapRight1;
    })){
        return false;
    }

    Logger::log("RMS error: %f", rms);
    return calcEpilines(allLeftCorners, allRightCorners, cameraMatrixLeft, cameraMatrixRight, distCoeffsLeft, distCoeffsRight, F);
}

void CalibEngine::resetCameraIntrinsic(Mat &mat, const Size &imgSize) {
    double *camPtrRight = (double*) mat.data;
    camPtrRight[0] = imgSize.width;
    camPtrRight[4] = imgSize.width;
    camPtrRight[2] = imgSize.width / 2;
    camPtrRight[5] = imgSize.height / 2;
}

void CalibEngine::findCameraIntrinsic(const vector<vector < Point3f>> &objPoints, const vector<vector < Point2f>> &imgPoints, const Size& imgSize, const Mat& cameraMatrix, Mat& distCoefs) {
    Mat rvecs;
    Mat tvecs;
    calibrateCamera(objPoints, imgPoints, imgSize, cameraMatrix, distCoefs, rvecs, tvecs);
}

void CalibEngine::buildObjectPoints(float fieldSize, Size boardSize, vector<vector<Point3f>> &objectPoints) {
    for (int i = 0; i < (int)objectPoints.size(); i++) {
        for (int j = 0; j < boardSize.height; j++)
            for (int k = 0; k < boardSize.width; k++)
                objectPoints[i].push_back(Point3f(j * fieldSize, k * fieldSize, 0));
    }
}

bool CalibEngine::calcEpilines(const vector<vector < Point2f>> &allLeftCorners,
        const vector<vector < Point2f>> &allRightCorners,
        const Mat &cameraMatrixLeft,
        const Mat &cameraMatrixRight,
        const Mat &distCoeffsLeft,
        const Mat &distCoeffsRight,
        const Mat &F) {
    double err = 0;
    int npoints = 0;
    vector<Vec3f> linesLeft;
    vector<Vec3f> linesRight;
    int nimages = allLeftCorners.size();
    for (int i = 0; i < nimages; i++) {
        int npt = (int) allLeftCorners[i].size();

        Mat imgptLeft(allLeftCorners[i]);
        undistortPoints(imgptLeft, imgptLeft, cameraMatrixLeft, distCoeffsLeft, Mat(), cameraMatrixLeft);
        computeCorrespondEpilines(imgptLeft, 1, F, linesLeft);

        Mat imgptRight(allRightCorners[i]);
        undistortPoints(imgptRight, imgptRight, cameraMatrixRight, distCoeffsRight, Mat(), cameraMatrixRight);
        computeCorrespondEpilines(imgptRight, 2, F, linesRight);

        for (int j = 0; j < npt; j++) {
            double errij = fabs(allLeftCorners[i][j].x * linesRight[j][0] +
                    allLeftCorners[i][j].y * linesRight[j][1] + linesRight[j][2]) +
                    fabs(allRightCorners[i][j].x * linesLeft[j][0] +
                    allRightCorners[i][j].y * linesLeft[j][1] + linesLeft[j][2]);
            err += errij;
        }
        npoints += npt;
    }

    Logger::log("Average reprojection err: %f", err / npoints);

    return saveYaml("intristics.yml", [cameraMatrixLeft, distCoeffsLeft, cameraMatrixRight, distCoeffsRight](auto fs) -> auto {
        fs << "M1" << cameraMatrixLeft << "D1" << distCoeffsLeft << "M2" << cameraMatrixRight << "D2" << distCoeffsRight;
    });
}

bool CalibEngine::rectify(const Mat& cameraMatrixLeft,
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
        Rect *validRoiRight) {

    Mat Q;

    stereoRectify(cameraMatrixLeft, distCoeffsLeft,
            cameraMatrixRight, distCoeffsRight,
            imageSize, R, T, outR1, outR2, outP1, outP2, Q,
            //CALIB_ZERO_DISPARITY, 0, imageSize, validRoiLeft, validRoiRight);
            0, 0, imageSize, validRoiLeft, validRoiRight);

    printMat("outR1", outR1);
    printMat("outR2", outR2);
    printMat("outP1", outP1);
    printMat("outP2", outP2);
    printMat("Q", Q);
    cout << "imgSize" << imageSize << endl;
    cout << "roiLeft" << *validRoiLeft << endl;
    cout << "roiRight" << *validRoiRight << endl;
   
    if(!saveYaml("extrinsics.yml", [R, T, outR1, outR2, outP1, outP2, Q](auto fs) -> auto {
        fs << "R" << R << "T" << T << "R1" << outR1 << "R2" << outR2 << "P1" << outP1 << "P2" << outP2 << "Q" << Q;
    })){
        return false;
    }
    if(!saveYaml("Q.yml", [Q](auto fs) -> auto {
        fs << "Q" << Q;
    })) {
        return false;
    }
    return true;
}

void CalibEngine::createRemapMatrices(const Mat& cameraMatrixLeft,
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
        Mat& outRemapRight1) {
    initUndistortRectifyMap(cameraMatrixLeft, distCoeffsLeft, R1, cameraMatrixLeft, imageSize, CV_16SC2, outRemapLeft0, outRemapLeft1);
    initUndistortRectifyMap(cameraMatrixRight, distCoeffsRight, R2, cameraMatrixRight, imageSize, CV_16SC2, outRemapRight0, outRemapRight1);
}