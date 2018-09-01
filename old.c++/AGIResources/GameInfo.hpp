//
//  GameInfo.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__GameInfo_hpp__
#define __AGIResources__GameInfo_hpp__

#include "AGIResources.hpp"

namespace AGI { namespace Resources {

    /**
     * Game Description is the class that basically does two things:
     * 1. It will try to detect the game's ID, description, versions and special features the AGI game has.
     * 2. It abstracts the file layout, special cases like Amiga games
     */
    class GameInfo
    {
    private:
        std::string _code;
        std::string _description;
        uint32_t    _version;
        uint32_t    _flags;

        std::unordered_map<GameFile, std::string> _files;

    public:
        GameInfo(const std::string& code, const std::string& description, uint32_t version, uint32_t flags, const std::unordered_map<GameFile, std::string>& files);

    public:
        inline const std::string& code() const { return _code; }
        inline const std::string& description() const { return _description; }
        inline uint32_t version() const { return _version; }
        inline uint32_t flags() const { return _flags; }
        inline const std::unordered_map<GameFile, std::string>& files() const { return _files; }

    public:
        static GameInfo detect(PlatformAbstractionLayer& platform);
    };

}}

#endif /* __AGIResources__GameInfo_hpp__ */
