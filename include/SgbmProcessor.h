#ifndef SGBMPROCESSOR_H
#define SGBMPROCESSOR_H

#include "StereoProcessor.h"

class SgbmProcessor : public StereoProcessor {
public:
    SgbmProcessor();
    virtual ~SgbmProcessor();
protected:
    Ptr<StereoMatcher> createMatcher(const SbmConfig &config);
};

#endif /* SGBMPROCESSOR_H */

