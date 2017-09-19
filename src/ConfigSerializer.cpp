#include <functional>

#include "ConfigSerializer.h"
#include "Logger.h"

string trim(const string& str);
bool parseArg(const string &arg, pair<string, string> &parsed);
bool addParam(const string &arg, map<string, string> &params);
bool fillConfig(const map<string, string> &params, Config &config);

struct ParamInfo {
    const char *name;
    const char *comment;

    ParamInfo(const char *name, const char *comment) {
        this->name = name;
        this->comment = comment;
    }
};

const ParamInfo VALID_PARAMS[] = {
    ParamInfo("logLevel", "logging verbosity: error, info, debug"),
    ParamInfo("boardWidth", "horizontal number of chessboard fields - 1"),
    ParamInfo("boardHeight", "vertical number of chessboard fields - 1"),
    ParamInfo("fieldSize", "chessboard field size (any unit, may be a cm)"),
    ParamInfo("leftCamId", "index of left camera of stereo pair"),
    ParamInfo("rightCamId", "index of right camera of stereo pair"),
    ParamInfo("captureWidth", "for chessboard detection, 0 - using current resolution, remember to update remapWidth and remapHeight properly"),
    ParamInfo("captureHeight", "for chessboard detection"),
    ParamInfo("chessboardImagesDir", "directory where are saved latest detected chessboards"),
    ParamInfo("overwriteImages", "if true every checboard detection stage will overwrite previous run (it will be backuped)"),
    ParamInfo("runMode", "bit OR of run modes: 1-chessboard detect, 2-calibration, 4-test with static image pair, 8-test with camera"),
    ParamInfo("chessboardSleep", "seconds between taking chessboard photo"),
    ParamInfo("remapWidth", "width of remap matrices - the same as image width to be used with"),
    ParamInfo("remapHeight", "remap height of final resolution"),
    ParamInfo("show3d", "1 to show remapped disparity map to 3d"),
    ParamInfo("smoothKernelSize", "Gauss filter camera image, odd number"),
    ParamInfo("leftTestImage", "Left test image to create diosparity map (run mode 4)"),
    ParamInfo("rightTestImage", "Right test image to create diosparity map (run mode 4)"),
    ParamInfo("disparityFiltering", "[0-off, 1-on], needs Contrib modules"),
    ParamInfo("disparityFilteringLambda", "[1, 15000], 8000 typically"),
    ParamInfo("disparityFilteringSigma", "[1, 5000], 800-2000 typically"),

    ParamInfo("algorithm", "0-SBM, 1-SGBM"),
    ParamInfo("nDispMult16", "nDispMult16*16 = num disparity [1,20]"),
    ParamInfo("blockSize", "SADWIndowSize/block size [3, 79]"),
    ParamInfo("uniquenesRatio", "sbm/sgbm [0, 45]"),
    ParamInfo("preFilterCap", "sbm/sgbm [0, 63]"),
    ParamInfo("disp12MaxDiff", "sbm/sgbm [0, 45]"),
    ParamInfo("minDisparity", "sbm/sgbm [0, 151]"),
    ParamInfo("speckleWindowSize", "sbm/sgbm [0, 600]"),
    ParamInfo("speckleRangeMult16", "sbm/sgbm speckleRangeMult16*16=specleRange [0, 10]"),
    ParamInfo("sbmPreFilterSize", "[5,255]"),
    ParamInfo("sbmFilterType", "0-PREFILTER_NORMALIZED_RESPONSE, 1-PREFILTER_XSOBEL"),
    ParamInfo("sbmSmallerBlockSize", "[0,151]"),
    ParamInfo("sbmTextureThreshold", "[0,151]"),
    ParamInfo("sgbmP1", "[0, 3000], good approx.: 8*number_of_image_channels*SADWindowSize*SADWindowSize"),
    ParamInfo("sgbmP2", "[0, 3000], good approx.: 32*number_of_image_channels*SADWindowSize*SADWindowSize"),
    ParamInfo("sgbmMode", "0-MODE_SGBM, 1-MODE_HH, 2-MODE_SGBM_3WAY")    
};
const int VALID_PARAM_NUM = sizeof (VALID_PARAMS) / sizeof (VALID_PARAMS[0]);

bool loadConfig(const char *path, int argc, const char **argv, Config &config) {
    ifstream fs(path);
    if (!fs.is_open()) {
        Logger::log("Cannot open file %s", path);
        return false;
    }

    Logger::log("Loading config:");
    string line;
    map<string, string> params;
    while (getline(fs, line)) {
        line = trim(line);
        if ((line.size() > 0 && line[0] == '#') || line.size() == 0) {
            continue;
        }
        if (!addParam(line, params)) {
            return false;
        }
    }

    Logger::log("Loading args:");
    for (int j = 1; j < argc; j++) {
        int i;
        for (i = 0; i < VALID_PARAM_NUM; i++) {
            if (!strncmp(argv[j], VALID_PARAMS[i].name, strlen(VALID_PARAMS[i].name))) {
                if (!addParam(argv[j], params)) {
                    return false;
                }
                break;
            }
        }
        if (i == VALID_PARAM_NUM) {
            Logger::log("Invalid argument %s", argv[j]);
            return false;
        }
    }
    Logger::log("Filling config:");
    if (!fillConfig(params, config)) {
        return false;
    }
    
    return true;
}

template<class T, class C>
bool initVal(const map<string, string> &params, const char *param, T &value, C conv) {
    if (params.find(param) == params.end()) {
        Logger::error("No param %s", param);
        return false;
    }
    value = conv(params.at(param));
    return true;
}

bool initInt(const map<string, string> &params, const char *param, int &value) {    
    return initVal(params, param, value, [](const string & s) -> int {
        return atoi(s.c_str());
    });
}

bool initFloat(const map<string, string> &params, const char *param, float &value) {
    return initVal(params, param, value, [](const string & s) -> float {
        return (float) atof(s.c_str());
    });
}

bool initString(const map<string, string> &params, const char *param, string &value) {
    return initVal(params, param, value, [](const string & s) -> string {
        return s;
    });
}

bool initBool(const map<string, string> &params, const char *param, bool &value) {
    return initVal(params, param, value, [](const string & s) -> bool {
        return atoi(s.c_str()) != 0;
    });
}

bool fillConfig(const map<string, string> &params, Config &config) {
    if (!initString(params, "logLevel", config.logLevel) ||
            !initInt(params, "boardWidth", config.boardSize.width) ||
            !initInt(params, "boardHeight", config.boardSize.height) ||
            !initFloat(params, "fieldSize", config.fieldSize) ||
            !initInt(params, "leftCamId", config.leftCamId) ||
            !initInt(params, "rightCamId", config.rightCamId) ||
            !initInt(params, "captureWidth", config.captureWidth) ||
            !initInt(params, "captureHeight", config.captureHeight) ||
            !initString(params, "chessboardImagesDir", config.chessboardImagesDir) ||
            !initBool(params, "overwriteImages", config.overwriteImages) ||
            !initInt(params, "runMode", config.runMode) ||
            !initInt(params, "chessboardSleep", config.chessboardSleep),
            !initInt(params, "remapWidth", config.remapWidth) ||
            !initInt(params, "remapHeight", config.remapHeight) ||
            !initInt(params, "show3d", config.show3d) ||            
            !initString(params, "leftTestImage", config.leftTestImage) ||
            !initString(params, "rightTestImage", config.rightTestImage) ||             
            !initInt(params, "disparityFiltering", config.sbm.disparityFiltering) || 
            !initInt(params, "disparityFilteringLambda", config.sbm.disparityFilteringLambda) || 
            !initInt(params, "disparityFilteringSigma", config.sbm.disparityFilteringSigma) || 
            !initInt(params, "smoothKernelSize", config.sbm.smoothKernelSize) ||            

            !initInt(params, "algorithm", config.sbm.algorithm) ||
            !initInt(params, "nDispMult16", config.sbm.nDispMult16) ||
            !initInt(params, "blockSize", config.sbm.blockSize) ||
            !initInt(params, "uniquenesRatio", config.sbm.uniquenesRatio) ||
            !initInt(params, "preFilterCap", config.sbm.preFilterCap) ||

            !initInt(params, "disp12MaxDiff", config.sbm.disp12MaxDiff) ||
            !initInt(params, "minDisparity", config.sbm.minDisparity) ||
            !initInt(params, "speckleWindowSize", config.sbm.speckleWindowSize) ||
            !initInt(params, "speckleRangeMult16", config.sbm.speckleRangeMult16) ||

            !initInt(params, "sbmPreFilterSize", config.sbm.sbmPreFilterSize) ||
            !initInt(params, "sbmFilterType", config.sbm.sbmFilterType) ||
            !initInt(params, "sbmSmallerBlockSize", config.sbm.sbmSmallerBlockSize) ||
            !initInt(params, "sbmTextureThreshold", config.sbm.sbmTextureThreshold) ||

            !initInt(params, "sgbmP1", config.sbm.sgbmP1) ||
            !initInt(params, "sgbmP2", config.sbm.sgbmP2) ||
            !initInt(params, "sgbmMode", config.sbm.sgbmMode)) {
        return false;
    }   
    Logger::log(config.toString());
    return true;
}

template<class T>
bool saveVal(const char *paramName, const T &value, ofstream &fs) {
    const ParamInfo *param = nullptr;
    for (const ParamInfo &p : VALID_PARAMS) {
        if (!strcmp(p.name, paramName)) {
            param = &p;
        }
    }

    if (param == nullptr) {
        Logger::error("No param %s", paramName);
        return false;
    }
    fs << "# " << param->comment << endl << param->name << " = " << value << endl << endl;
    return true;
}

bool saveConfig(const char *path, const Config &config) {
    ofstream fs(path);
    if (!fs.is_open()) {
        Logger::error("Cannot open file %s", path);
        return false;
    }

    Logger::log("Saving config");
    
    if (!saveVal("logLevel", config.logLevel, fs) ||
            !saveVal("boardWidth", config.boardSize.width, fs) ||
            !saveVal("boardHeight", config.boardSize.height, fs) ||
            !saveVal("fieldSize", config.fieldSize, fs) ||
            !saveVal("leftCamId", config.leftCamId, fs) ||
            !saveVal("rightCamId", config.rightCamId, fs) ||
            !saveVal("captureWidth", config.captureWidth, fs) ||
            !saveVal("captureHeight", config.captureHeight, fs) ||
            !saveVal("chessboardImagesDir", config.chessboardImagesDir, fs) ||
            !saveVal("overwriteImages", config.overwriteImages, fs) ||
            !saveVal("runMode", config.runMode, fs) ||
            !saveVal("chessboardSleep", config.chessboardSleep, fs) ||
            !saveVal("remapWidth", config.remapWidth, fs) ||
            !saveVal("remapHeight", config.remapHeight, fs) ||
            !saveVal("show3d", config.show3d, fs) ||            
            !saveVal("leftTestImage", config.leftTestImage, fs) ||
            !saveVal("rightTestImage", config.rightTestImage, fs) ||            

            !saveVal("algorithm", config.sbm.algorithm, fs) ||
            !saveVal("nDispMult16", config.sbm.nDispMult16, fs) ||
            !saveVal("blockSize", config.sbm.blockSize, fs) ||
            !saveVal("uniquenesRatio", config.sbm.uniquenesRatio, fs) ||
            !saveVal("preFilterCap", config.sbm.preFilterCap, fs) ||
            !saveVal("disparityFiltering", config.sbm.disparityFiltering, fs) ||
            !saveVal("disparityFilteringLambda", config.sbm.disparityFilteringLambda, fs) ||
            !saveVal("disparityFilteringSigma", config.sbm.disparityFilteringSigma, fs) ||
            !saveVal("smoothKernelSize", config.sbm.smoothKernelSize, fs) ||

            !saveVal("disp12MaxDiff", config.sbm.disp12MaxDiff, fs) ||
            !saveVal("minDisparity", config.sbm.minDisparity, fs) ||
            !saveVal("speckleWindowSize", config.sbm.speckleWindowSize, fs) ||
            !saveVal("speckleRangeMult16", config.sbm.speckleRangeMult16, fs) ||

            !saveVal("sbmPreFilterSize", config.sbm.sbmPreFilterSize, fs) ||
            !saveVal("sbmFilterType", config.sbm.sbmFilterType, fs) ||
            !saveVal("sbmSmallerBlockSize", config.sbm.sbmSmallerBlockSize, fs) ||
            !saveVal("sbmTextureThreshold", config.sbm.sbmTextureThreshold, fs) ||

            !saveVal("sgbmP1", config.sbm.sgbmP1, fs) ||
            !saveVal("sgbmP2", config.sbm.sgbmP2, fs) ||
            !saveVal("sgbmMode", config.sbm.sgbmMode, fs)) {
        return false;
    }    
    return true;
}

bool addParam(const string &arg, map<string, string> &params) {
    pair<string, string> param;
    if (!parseArg(arg, param)) {
        return false;
    }
    params.insert(param);
    return true;
}

bool parseArg(const string &arg, pair<string, string> &parsed) {
    int equal = arg.find("=");
    if (equal < 0) {
        Logger::log("Corrupted line %s", arg.c_str());
        return false;
    }
    string key = trim(arg.substr(0, equal));
    string value = trim(arg.substr(equal + 1, arg.length()));
    Logger::log(key + "=" + value);
    parsed = make_pair(key, value);
    return true;
}

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}
