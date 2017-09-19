#include "CvUtil.h"

static Mat gray(1,400,CV_8U);    
    
void cvWin(const char *name, int flags) {
    namedWindow(name, flags);
    imshow(name, gray);
}

bool saveYaml(const char *fileName, function<void(FileStorage&)> save) {
    FileStorage fs(fileName, CV_STORAGE_WRITE);
    if (fs.isOpened()) {
        save(fs);
        fs.release();
    } else {
        Logger::log("Can not save to file: %s", fileName);
        return false;
    }
    return true;
}

bool loadYaml(const char *fileName, function<void(FileStorage&)> load) {
    FileStorage fs(fileName, CV_STORAGE_READ);
    if (fs.isOpened()) {
        load(fs);            
        fs.release();
    } else {
        Logger::log("Can not read file: %s", fileName);
        return false;
    }
    return true;
}
