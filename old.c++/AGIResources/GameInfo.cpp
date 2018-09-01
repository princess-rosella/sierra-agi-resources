//
//  GameInfo.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "GameInfo.hpp"

#include "PlatformAbstractionLayer.hpp"

#include <algorithm>
#include <memory>
#include <regex>

using namespace AGI::Resources;

GameInfo::GameInfo(const std::string& code, const std::string& description, uint32_t version, uint32_t flags, const std::unordered_map<GameFile, std::string>& files) : _code(code), _description(description), _version(version), _flags(flags), _files(files) {
}

static std::unordered_map<GameFile, std::string> buildV2(const std::vector<std::string>& files)
{
    std::unordered_map<GameFile, std::string> map;

    std::regex volumeMatch("^vol.(\\d)$", std::regex::icase);

    for (const auto& file : files) {
        if (strcasecmp(file.c_str(), "logdir") == 0) {
            map.emplace(std::make_pair(GameFile::Logic, file));
            continue;
        }
        else if (strcasecmp(file.c_str(), "picdir") == 0) {
            map.emplace(std::make_pair(GameFile::Picture, file));
            continue;
        }
        else if (strcasecmp(file.c_str(), "snddir") == 0) {
            map.emplace(std::make_pair(GameFile::Sound, file));
            continue;
        }
        else if (strcasecmp(file.c_str(), "viewdir") == 0) {
            map.emplace(std::make_pair(GameFile::View, file));
            continue;
        }
        else if (strcasecmp(file.c_str(), "words.tok") == 0) {
            map.emplace(std::make_pair(GameFile::Words, file));
            continue;
        }
        else if (strcasecmp(file.c_str(), "object") == 0) {
            map.emplace(std::make_pair(GameFile::Objects, file));
            continue;
        }

        std::smatch m;

        if (regex_search(file, m, volumeMatch)) {
            uint8_t volumeID = (uint8_t)std::stoi(m[1]);
            map.emplace(std::make_pair((GameFile)((uint8_t)GameFile::Volume_0 + volumeID), file));
        }
    }

    return map;
}

static std::unordered_map<GameFile, std::string> buildV3(const std::vector<std::string>& files)
{
    std::unordered_map<GameFile, std::string> map;

    std::regex volumeMatch("^(\\w+)vol.(\\d)$", std::regex::icase);
    std::regex dirMatch("^(\\w+)dir$", std::regex::icase);

    for (const auto& file : files) {
        if (strcasecmp(file.c_str(), "words.tok") == 0) {
            map.emplace(std::make_pair(GameFile::Words, file));
            continue;
        }
        else if (strcasecmp(file.c_str(), "object") == 0) {
            map.emplace(std::make_pair(GameFile::Objects, file));
            continue;
        }
        else if (regex_match(file, dirMatch)) {
            map.emplace(std::make_pair(GameFile::Directory, file));
            continue;
        }

        std::smatch m;

        if (regex_search(file, m, volumeMatch)) {
            uint8_t volumeID = (uint8_t)std::stoi(m[2]);
            map.emplace(std::make_pair((GameFile)((uint8_t)GameFile::Volume_0 + volumeID), file));
        }
    }

    return map;
}

static GameInfo detectV2(PlatformAbstractionLayer& platform) {
    auto map(buildV2(platform.fileList()));
    if (map.count(GameFile::Logic) == 0)
        throw std::runtime_error("Not an AGI v2 game");

    std::string logdirHash(platform.fileMD5Hash(map[GameFile::Logic].c_str()));

#define GAME_LD(code, desc, md5, version, flags) \
    if (logdirHash == md5) \
        return GameInfo(code, desc, version, flags, map)

    GAME_LD("kq1", "2.0F 1987-05-05 5.25\"/3.5\"", "10ad66e2ecbd66951534a50aedcd0128", 0x2917, 0);
    GAME_LD("kq2", "2.1 1987-04-10", "759e39f891a0e1d86dd29d7de485c6ac", 0x2440, 0);
    GAME_LD("kq3", "2.14 1988-03-15 3.5\"", "d3d17b77b3b3cd13246749231d9473cd", 0x2936, 0);

    return GameInfo("agiv2", std::string("Unknown AGI v2 game ") + logdirHash, 0x2917, 0, map);
}

static GameInfo detectV3(PlatformAbstractionLayer& platform) {
    auto map(buildV3(platform.fileList()));
    if (map.count(GameFile::Directory) == 0)
        throw std::runtime_error("Not an AGI v3 game");

    std::string logdirHash(platform.fileMD5Hash(map[GameFile::Directory].c_str()));

    GAME_LD("kq4", "2.0 1988-07-27 3.5\"", "fe44655c42f16c6f81046fdf169b6337", 0x3086, 0);

    return GameInfo("agiv3", std::string("Unknown AGI v3 game ") + logdirHash, 0x3086, 0, map);
}

GameInfo GameInfo::detect(PlatformAbstractionLayer& platform) {
    if (platform.fileExists("logdir") &&
        platform.fileExists("object") &&
        platform.fileExists("picdir") &&
        platform.fileExists("snddir") &&
        platform.fileExists("viewdir") &&
        platform.fileExists("words.tok") &&
        platform.fileExists("vol.0")) {
        // Looks like a version 2.
        return detectV2(platform);
    }

    return detectV3(platform);
}
