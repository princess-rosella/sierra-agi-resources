//
//  PlatformAbstractionLayer_macOS.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__PlatformAbstractionLayer_macOS_hpp__
#define __AGIResources__PlatformAbstractionLayer_macOS_hpp__

#include "PlatformAbstractionLayer_POSIX.hpp"

#include <os/log.h>

namespace AGI { namespace Resources {

class PlatformAbstractionLayer_macOS : public PlatformAbstractionLayer_POSIX
{
protected:
    os_log_t _log;

public:
    PlatformAbstractionLayer_macOS(const std::string& folder);

public:
    virtual std::string fileMD5Hash(const char* fileName) const override;

public:
    virtual void log(const char *, ...) const override;
};

}}

#endif /* __AGIResources__PlatformAbstractionLayer_macOS_hpp__ */
