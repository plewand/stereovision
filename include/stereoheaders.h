#ifndef STEREOHEADERS_H
#define STEREOHEADERS_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include "Logger.h"

using namespace std;
using namespace cv;

#define FAIL(MSG) (Logger::log(MSG), false);

#endif /* STEREOHEADERS_H */

