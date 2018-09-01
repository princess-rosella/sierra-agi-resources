//
//  PlatformAbstractionLayer_POSIX.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "AGIResources.hpp"
#include "PlatformAbstractionLayer_POSIX.hpp"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>

using namespace AGI::Resources;

PlatformAbstractionLayer_POSIX_FileDescriptor::PlatformAbstractionLayer_POSIX_FileDescriptor(int desc) : _desc(desc) {
}

PlatformAbstractionLayer_POSIX_FileDescriptor::~PlatformAbstractionLayer_POSIX_FileDescriptor() {
    if (_desc >= 0)
        close(_desc);
}

ssize_t PlatformAbstractionLayer_POSIX_FileDescriptor::readall(void* data, size_t length) {
    ssize_t readed = 0;

    while (length) {
        ssize_t thisRead = read(_desc, data, length);
        if (thisRead < 0)
            return thisRead;
        else if (thisRead == 0)
            break;

        data    = ((uint8_t*)data) + thisRead;
        length -= thisRead;
    }

    if (length)
        memset(data, 0, length);

    return readed;
}

PlatformAbstractionLayer_POSIX::PlatformAbstractionLayer_POSIX(const std::string& folder) : _folder(folder) {
    if (_folder.length() == 0) {
        _folder = "./";
        return;
    }

    if (_folder[_folder.length() - 1] == '/')
        return;

    _folder += '/';
}

std::string PlatformAbstractionLayer_POSIX::fullPathName(const char* fileName) const {
    return _folder + fileName;
}

bool PlatformAbstractionLayer_POSIX::fileExists(const char* fileName) const {
    if (access(fullPathName(fileName).c_str(), R_OK))
        return false;

    return true;
}

size_t PlatformAbstractionLayer_POSIX::fileSize(const char* fileName) const {
    struct stat st;

    if (stat(fullPathName(fileName).c_str(), &st))
        return (size_t)-1u;

    return (size_t)st.st_size;
}

bool PlatformAbstractionLayer_POSIX::fileRead(const char* fileName, size_t offset, void* data, size_t length) const {
    std::unique_ptr<PlatformAbstractionLayer_POSIX_FileDescriptor> fd(new PlatformAbstractionLayer_POSIX_FileDescriptor(open(fullPathName(fileName).c_str(), O_RDONLY)));

    if (offset) {
        if (lseek(fd->desc(), offset, SEEK_SET) == -1)
            throw std::runtime_error(strerror(errno));
    }

    if (fd->readall(data, length) < 0)
        throw std::runtime_error(strerror(errno));

    return true;
}

std::vector<std::string> PlatformAbstractionLayer_POSIX::fileList() const {
    DIR *dir = opendir(_folder.c_str());
    if (!dir)
        return std::vector<std::string>();

    std::vector<std::string> files;

    while (struct dirent *entry = readdir(dir)) {
        std::string name(entry->d_name, entry->d_namlen);
        if (name == "." || name == ".." || name[0] == '.')
            continue;

        files.push_back(name);
    }

    closedir(dir);
    return files;
}

std::string AGI::Resources::format(const char* format, ...) {
    va_list va;
    char buffer[2048];

    va_start(va, format);
    vsnprintf(buffer, sizeof(buffer), format, va);
    va_end(va);

    return buffer;
}
