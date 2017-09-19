#include "Reprojector3D.h"
#include <GL/freeglut.h>
#include <GL/gl.h>
#include "Util3D.h"

static const char *WIN_NAME = "Points3D";
static const char *DEPTH_MAP_WIN_NAME = "Depth";
static const float WIN_WIDTH = 640;
static const float WIN_HEIGHT = 480;
static const float MOVE_VEL = 5;
static const float ROT_VEL = 5;
static const float MAX_ANGLE_Y = 60;

Reprojector3D Reprojector3D::instance;

Reprojector3D::Reprojector3D() {
    initialized = false;
    initOnFirstDisparity = false;
    cameraTexture = 0;
    idleFunc = nullptr;
    keyBoardFunc = nullptr;
    keyBoardOpenCvFunc = nullptr;
}

Reprojector3D::~Reprojector3D() {
    glDeleteTextures(1, &cameraTexture);
}

bool Reprojector3D::init() {
    int argc = 0;
    glutInit(&argc, nullptr);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow(WIN_NAME);
    glutDisplayFunc(displayFunction);
    glutIdleFunc(waitFunction);
    glutKeyboardFunc(keyboardFunction);
    glutSpecialFunc(keyboardSpecialFunction);
    setupGL();
    createCameraTexture();
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        Logger::log("GL error: %x", error);
        return false;
    }
    initialized = true;
    return true;
}

void Reprojector3D::setupGL() {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, WIN_HEIGHT / WIN_WIDTH, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_TEXTURE_2D);
}

void Reprojector3D::createCameraTexture() {
    glGenTextures(1, &cameraTexture);
    glBindTexture(GL_TEXTURE_2D, cameraTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Reprojector3D::initMatrices() {
    initLookPt[0] = (maxPt[0] + minPt[0]) / 2.0f;
    initLookPt[1] = (maxPt[1] + minPt[1]) / 2.0f;
    initLookPt[2] = maxPt[2];

    initEyePt[0] = initLookPt[0];
    initEyePt[1] = initLookPt[1];
    initEyePt[2] = initLookPt[2] + (maxPt[1] - minPt[1]);

    rotAngleX = 0;
    rotAngleY = 0;

    lookPt[0] = initLookPt[0];
    lookPt[1] = initLookPt[1];
    lookPt[2] = initLookPt[2];

    updateLookAt();
}

void Reprojector3D::createTexCoordArray(const Mat &points3d, float minX, float minY, float maxX, float maxY, Mat &tc2) {
    const float *ptPtr = (const float*) points3d.data;
    float *tcPtr = (float*) tc2.data;
    while (ptPtr < (const float*) points3d.dataend) {
        tcPtr[0] = (ptPtr[0] - minX) / (maxX - minX);
        tcPtr[1] = 1.0f - (ptPtr[1] - minY) / (maxY - minY);
        ptPtr += 3;
        tcPtr += 2;
    }
}

void Reprojector3D::findPoints3DBounds(const Mat& points3D, float *ptMin, float *ptMax) {
    int ptNum = points3D.rows * points3D.cols;
    if (ptNum <= 0) {
        ptMin[0] = ptMin[1] = ptMin[2] = 0.0f;
        ptMax[0] = ptMax[1] = ptMax[2] = 0.0f;
        return;
    }

    float *ptr = (float*) points3D.data;
    for (int i = 0; i < 3; i++) {
        ptMin[i] = ptr[i];
        ptMax[i] = ptr[i];
    }

    for (int i = 1; i < ptNum * 3; i += 3) {
        for (int i2 = 0; i2 < 3; i2++) {
            if (ptr[i2] < ptMin[i2]) {
                ptMin[i2] = ptr[i2];
            } else if (ptr[i2] > ptMax[i2]) {
                ptMax[i2] = ptr[i2];
            }
        }
        ptr += 3;
    }
    Logger::log("3d min pt: %f, %f %f", ptMin[0], ptMin[1], ptMin[2]);
    Logger::log("3d max pt: %f, %f %f", ptMax[0], ptMax[1], ptMax[2]);
}

void Reprojector3D::reproject(const Mat &disparity, const Mat &Q, const Mat &cameraImage) {
    if (points3D.empty()) {
        points3D.create(disparity.size(), CV_32FC3);
    }
    Mat disparity32F;
    if (disparity.type() == CV_16S) {
        disparity.convertTo(disparity32F, CV_32F, 1.0 / 16.0);
    } else {
        disparity32F = disparity;
    }
    reprojectImageTo3D(disparity32F, points3D, Q, false, CV_32F);
    if (!initOnFirstDisparity) {
        findPoints3DBounds(points3D, minPt, maxPt);
        initOnFirstDisparity = true;
        texCoord.create(points3D.rows, points3D.cols, CV_32FC2);
        initMatrices();
    }
    float distance = getCenterMinDistance(points3D, -1.0f, 3, 2);    

    if (!points3D.empty()) {        
        createDepthMat(points3D, depthMap);
        Mat depthMapCol;
        cvtColor(depthMap, depthMapCol, cv::COLOR_GRAY2RGB);
        int cx = depthMapCol.cols / 2 - DEPTH_CALC_SQUARE_SIZE / 2;
        int cy = depthMapCol.rows / 2 - DEPTH_CALC_SQUARE_SIZE / 2;
        rectangle(depthMapCol, Point(cx, cy), Point(cx + DEPTH_CALC_SQUARE_SIZE, cy + DEPTH_CALC_SQUARE_SIZE), Scalar(0, 255, 0));
        
        string text = to_string(distance);        
        int fontFace = CV_FONT_HERSHEY_PLAIN;
        double fontScale = 2;
        int thickness = 3;
        int baseline=0;
        Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
        Point textOrg(cx + DEPTH_CALC_SQUARE_SIZE/2, cy + DEPTH_CALC_SQUARE_SIZE + 10 + textSize.height);
        putText(depthMapCol, text, textOrg, fontFace, fontScale, Scalar(0, 255, 0), thickness, 8);
        
        namedWindow(DEPTH_MAP_WIN_NAME);
        imshow(DEPTH_MAP_WIN_NAME, depthMapCol);
    }

    createTexCoordArray(points3D, minPt[0], minPt[1], maxPt[0], maxPt[1], texCoord);
    cameraImage.copyTo(this->cameraImage);
    updateCameraTexture(cameraImage);
    glutPostRedisplay();
}

void Reprojector3D::createDepthMat(const Mat &points3D, Mat &depthMap) {    
    if(points3D.empty()) {
        return;
    }
    
    if (depthMap.empty()) {
        depthMap.create(points3D.rows, points3D.cols, CV_8UC1);
    }

    const float *points = (const float*) points3D.data;
    uchar *depth = depthMap.data;

    memset(depth, 0, depthMap.cols * depthMap.rows);
    float minZ;
    float maxZ;
    findPosZRange(points3D, minZ, maxZ);

    float scale = 255.0f / (maxZ - minZ);
    while (depth < depthMap.dataend) {
        *depth = (uchar) ((points[2] - maxZ) * scale);
        depth++;
        points += 3;
    }
}

void Reprojector3D::findPosZRange(const Mat &points3D, float &minZ, float &maxZ) {
    const float *points = (const float*) points3D.data;
    minZ = points[2];
    maxZ = points[2];
    points += 3;
    while (points < (const float*) points3D.dataend) {
        if (points[2] < minZ) {
            minZ = points[2];
        } else if (points[2] > maxZ) {
            maxZ = points[2];
        }
        points += 3;
    }
}

void Reprojector3D::renderScene(void) {
    glClearColor(0.3, 1.0, 0.11, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(eyePt[0], eyePt[1], eyePt[2], lookPt[0], lookPt[1], lookPt[2], 0, 1, 0);

    glBindTexture(GL_TEXTURE_2D, cameraTexture);
    glEnable(GL_TEXTURE_2D);

    drawVertexArray(points3D);

    glutSwapBuffers();
}

void Reprojector3D::drawVertexArray(const Mat &points3D) {
    if (points3D.empty()) {
        return;
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points3D.data);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoord.data);

    glDrawArrays(GL_POINTS, 0, points3D.cols * points3D.rows);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Reprojector3D::updateLookAt() {
    float initEyeToLookPtVec[3];
    sub(initEyePt, initLookPt, initEyeToLookPtVec);
    rotateAroundX(rotAngleX, initEyeToLookPtVec);
    rotateAroundY(rotAngleY, initEyeToLookPtVec);

    eyePt[0] = lookPt[0] + initEyeToLookPtVec[0];
    eyePt[1] = lookPt[1] + initEyeToLookPtVec[1];
    eyePt[2] = lookPt[2] + initEyeToLookPtVec[2];
}

Reprojector3D* Reprojector3D::getReprojector() {
    if (!instance.initialized)
        instance.init();
    return &instance;
}

void Reprojector3D::updateCameraTexture(const Mat &mat) {
    if (mat.type() != CV_8UC3) {
        Logger::log("Camera texture have to be CV_8SC3");
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, mat.data);
}

void Reprojector3D::keyPressed(int key) {
    switch (key) {
        case 'w':
            rotAngleX -= ROT_VEL;
            if (rotAngleX < -MAX_ANGLE_Y)
                rotAngleX = -MAX_ANGLE_Y;
            break;
        case 's':
            rotAngleX += ROT_VEL;
            if (rotAngleX > MAX_ANGLE_Y)
                rotAngleX = MAX_ANGLE_Y;
            break;
        case 'a':
            rotAngleY -= ROT_VEL;
            break;
        case 'd':
            rotAngleY += ROT_VEL;
            break;
        case 27:
            glutLeaveMainLoop();
            break;
        default:
            if (keyBoardFunc != nullptr) {
                keyBoardFunc(key);
            }
    }
    updateLookAt();
    glutPostRedisplay();
}

void Reprojector3D::specialKeyPressed(int key) {
    switch (key) {
        case GLUT_KEY_UP:
            moveAlong(MOVE_VEL);
            break;
        case GLUT_KEY_DOWN:
            moveAlong(-MOVE_VEL);
            break;
        case GLUT_KEY_LEFT:
            moveAside(-MOVE_VEL);
            break;
        case GLUT_KEY_RIGHT:
            moveAside(MOVE_VEL);
            break;
    }
    updateLookAt();
    glutPostRedisplay();
}

void Reprojector3D::moveAlong(float vel) {
    float lookVec[3];
    sub(lookPt, eyePt, lookVec);
    normalize(lookVec);
    mul(lookVec, vel, lookVec);
    add(lookPt, lookVec, lookPt);
}

void Reprojector3D::moveAside(float vel) {
    float lookVec[3];
    float upVec[3] = {0, 1, 0};
    float perp[3];
    sub(lookPt, eyePt, lookVec);
    normalize(lookVec);
    cross(lookVec, upVec, perp);
    normalize(perp);
    mul(perp, vel, perp);
    add(lookPt, perp, lookPt);
}

void Reprojector3D::waitFunction() {
    char key = waitKey(1) & 0xFF;
    if (key == 27) {
        glutLeaveMainLoop();
    }

    if (key > 0) {
        auto keyBoardOpenCvFunc = Reprojector3D::getReprojector()->keyBoardOpenCvFunc;
        if (keyBoardOpenCvFunc != nullptr) {
            keyBoardOpenCvFunc(key);
        }
    }

    auto idleFunc = Reprojector3D::getReprojector()->idleFunc;
    if (idleFunc != nullptr) {
        idleFunc();
    }
}

void Reprojector3D::keyboardFunction(unsigned char key, int x, int y) {
    Reprojector3D::getReprojector()->keyPressed(key);
}

void Reprojector3D::keyboardSpecialFunction(int key, int x, int y) {
    Reprojector3D::getReprojector()->specialKeyPressed(key);
}

void Reprojector3D::displayFunction() {
    Reprojector3D::getReprojector()->renderScene();
}

void Reprojector3D::setIdleFunc(function<void(void) > idleFunc) {
    this->idleFunc = idleFunc;
}

void Reprojector3D::setKeyboardFunc(function<void(int) > keyBoardFunc) {
    this->keyBoardFunc = keyBoardFunc;
}

void Reprojector3D::setKeyboardOpenCvFunc(function<void(int) > keyBoardOpenCvFunc) {
    this->keyBoardOpenCvFunc = keyBoardOpenCvFunc;
}

void Reprojector3D::loop() {
    glutMainLoop();
}