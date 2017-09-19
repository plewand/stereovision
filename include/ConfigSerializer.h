#ifndef CONFIGSERIALIZER_H
#define CONFIGSERIALIZER_H

#include "Config.h"

bool loadConfig(const char *path, int argc, const char **argv, Config &config);
bool saveConfig(const char *path, const Config &config);


#endif /* CONFIGPARSER_H */

