//
//  PlatformAbstractionLayer.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__PlatformAbstractionLayer_hpp__
#define __AGIResources__PlatformAbstractionLayer_hpp__

#include <stdint.h>
#include <stdlib.h>

#include <string>

#ifndef __printflike
#define __printflike(f,e)
#endif

namespace AGI { namespace Resources {

    class PlatformAbstractionLayer {
    public:
        virtual ~PlatformAbstractionLayer() {}

    public:
        virtual bool fileExists(const char* fileName) const = 0;
        virtual size_t fileSize(const char* fileName) const = 0;
        virtual bool fileRead(const char* fileName, size_t offset, void* data, size_t length) const = 0;
        virtual std::vector<std::string> fileList() const = 0;
        virtual std::string fileMD5Hash(const char* fileName) const = 0;

    public:
        virtual void log(const char *, ...) const __printflike(2, 3) = 0;
    };

}}

#endif /* __AGIResources__PlatformAbstractionLayer_hpp__ */
