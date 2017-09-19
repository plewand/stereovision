#ifndef REPROJECTOR3D_H
#define REPROJECTOR3D_H

#include "stereoheaders.h"
#include <GL/gl.h>
#include <functional>

class Reprojector3D {
public:
    Reprojector3D();    
    virtual ~Reprojector3D();    
    void loop();
    void reproject(const Mat &disparity, const Mat &Q, const Mat &cameraImage);    
    void setIdleFunc(function<void(void)> idleFunc);
    void setKeyboardFunc(function<void(int)> keyBoardFunc);
    void setKeyboardOpenCvFunc(function<void(int)> keyBoardOpenCvFunc);
    static Reprojector3D* getReprojector();
private:
    void renderScene(void);
    bool init();
    void drawVertexArray(const Mat &points3D);
    void setupGL();
    void findPoints3DBounds(const Mat& points3D, float *ptMin, float *ptMax);
    void keyPressed(int key);
    void specialKeyPressed(int key);
    void initMatrices();
    void updateLookAt();
    void createCameraTexture();
    void updateCameraTexture(const Mat &mat);
    void createTexCoordArray(const Mat &points3d, float minX, float minY, float maxX, float maxY, Mat &tc2);
    void moveAlong(float vel);
    void moveAside(float vel);    
    void createDepthMat(const Mat &points3D, Mat &depthMap);
    void findPosZRange(const Mat &points3D, float &minZ, float &maxZ);
    
    bool initialized;
    Mat points3D;
    float minPt[3];
    float maxPt[3];
    bool initOnFirstDisparity;
    float initLookPt[3];
    float initEyePt[3];
    float lookPt[3];
    float eyePt[3];
    float rotAngleX;
    float rotAngleY;
    GLuint cameraTexture;
    Mat cameraImage;
    Mat texCoord;
    function<void(void)> idleFunc;
    function<void(int)> keyBoardFunc;
    function<void(int)> keyBoardOpenCvFunc;
    Mat depthMap;
    
    static void displayFunction();
    static void waitFunction();
    static void keyboardFunction(unsigned char key, int x, int y);
    static void keyboardSpecialFunction(int key, int x, int y);
    
    static Reprojector3D instance;
};

#endif /* REPROJECTOR3D_H */

