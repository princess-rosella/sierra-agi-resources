//
//  PlatformAbstractionLayer_macOS.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "PlatformAbstractionLayer_macOS.hpp"

#include <cstdarg>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <vector>

#include <CommonCrypto/CommonCrypto.h>

using namespace AGI::Resources;

PlatformAbstractionLayer_macOS::PlatformAbstractionLayer_macOS(const std::string& folder) : PlatformAbstractionLayer_POSIX(folder), _log(os_log_create("AGI", "Resources")) {
}

std::string PlatformAbstractionLayer_macOS::fileMD5Hash(const char* fileName) const {
    struct stat st;
    std::unique_ptr<PlatformAbstractionLayer_POSIX_FileDescriptor> fd(new PlatformAbstractionLayer_POSIX_FileDescriptor(open(fullPathName(fileName).c_str(), O_RDONLY)));
    if (fstat(fd->desc(), &st))
        throw std::runtime_error(strerror(errno));

    uint8_t hash[CC_MD5_DIGEST_LENGTH];
    std::vector<uint8_t> data;

    data.resize(st.st_size);
    if (fd->readall(data.data(), data.size()) < 0)
        throw std::runtime_error(strerror(errno));

    if (!CC_MD5(data.data(), (CC_LONG)data.size(), hash))
        throw std::runtime_error("Failed to MD5 hash");

    char hashString[(CC_MD5_DIGEST_LENGTH * 2) + 1];

    snprintf(hashString, sizeof(hashString), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9], hash[10], hash[11], hash[12], hash[13], hash[14], hash[15]);
    return hashString;
}

void PlatformAbstractionLayer_macOS::log(const char *format, ...) const {
    va_list va;
    char buffer[2048];

    va_start(va, format);
    vsnprintf(buffer, sizeof(buffer), format, va);
    va_end(va);

    os_log_info(_log, "%{public}s", buffer);
}
