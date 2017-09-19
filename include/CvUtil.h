#ifndef CVUTIL_H
#define CVUTIL_H

#include <functional>
#include "stereoheaders.h"

void cvWin(const char *name, int flags = WINDOW_AUTOSIZE);
bool saveYaml(const char *fileName, function<void(FileStorage&)> save);
bool loadYaml(const char *fileName, function<void(FileStorage&)> load);

#endif /* CVUTIL_H */

