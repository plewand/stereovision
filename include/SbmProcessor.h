#ifndef SBMPROCESSOR_H
#define SBMPROCESSOR_H

#include "StereoProcessor.h"

class SbmProcessor : public StereoProcessor {
public:
    SbmProcessor();
    virtual ~SbmProcessor();
protected:
    Ptr<StereoMatcher> createMatcher(const SbmConfig &config);
};

#endif /* SBMPROCESSOR_H */

