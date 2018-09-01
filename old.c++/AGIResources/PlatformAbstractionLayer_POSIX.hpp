//
//  PlatformAbstractionLayer_POSIX.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__PlatformAbstractionLayer_POSIX_hpp__
#define __AGIResources__PlatformAbstractionLayer_POSIX_hpp__

#include "PlatformAbstractionLayer.hpp"

namespace AGI { namespace Resources {

class PlatformAbstractionLayer_POSIX_FileDescriptor {
public:
    int _desc;

public:
    PlatformAbstractionLayer_POSIX_FileDescriptor(int desc);
    ~PlatformAbstractionLayer_POSIX_FileDescriptor();

    inline int desc() const { return _desc; }

public:
    ssize_t readall(void* data, size_t length);
};

class PlatformAbstractionLayer_POSIX : public PlatformAbstractionLayer
{
protected:
    std::string _folder;

protected:
    std::string fullPathName(const char* fileName) const;

public:
    PlatformAbstractionLayer_POSIX(const std::string& folder);

public:
    virtual bool fileExists(const char* fileName) const override;
    virtual size_t fileSize(const char* fileName) const override;
    virtual bool fileRead(const char* fileName, size_t offset, void* data, size_t length) const override;
    virtual std::vector<std::string> fileList() const override;
};

}}

#endif /* __AGIResources__PlatformAbstractionLayer_POSIX_hpp__ */
