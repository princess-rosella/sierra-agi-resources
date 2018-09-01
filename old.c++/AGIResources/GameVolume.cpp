//
//  GameVolume.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "GameVolume.hpp"

#include "Endian.hpp"
#include "LZWExpand.hpp"
#include "PlatformAbstractionLayer.hpp"

using namespace AGI::Resources;

GameVolume::GameVolume(PlatformAbstractionLayer* platform) : _info(GameInfo::detect(*platform)), _platform(platform) {
    parseDirectory();
}

GameVolume::GameVolume(const GameInfo& info, PlatformAbstractionLayer* platform) : _info(info), _platform(platform) {
    parseDirectory();
}

void GameVolume::parseDirectory() {
    if (_info.version() >= 0x3000)
        parseDirectoryV3();
    else
        parseDirectoryV2();
}

static inline GameFile volumeToGameFile(uint8_t volume) {
    return (GameFile)((uint8_t)GameFile::Volume_0 + volume);
}

GameVolume::volume_sizes_t GameVolume::gatherVolumeSizes() {
    uint8_t volume = 0;
    GameVolume::volume_sizes_t results;

    auto& files = _info.files();

    for (;; volume++) {
        auto it(files.find(volumeToGameFile(volume)));
        if (it == files.end())
            break;

        size_t size = _platform->fileSize(it->second.c_str());
        if (size == (size_t)-1)
            break;

        results.emplace(volumeToGameFile(volume), size);
    }

    return results;
}

void GameVolume::parseObjectsAndWordsEntries() {
    size_t objectFileSize = _platform->fileSize(_info.files().at(GameFile::Objects).c_str());
    size_t wordsFileSize = _platform->fileSize(_info.files().at(GameFile::Words).c_str());
    if (objectFileSize == (size_t)-1 || wordsFileSize == (size_t)-1)
        throw std::runtime_error("Failed to determine size of words/objects resources");

    _entries.emplace(GameFile::Objects, std::unordered_map<int, GameVolumeEntry>({ std::make_pair(0, GameVolumeEntry(GameFile::Objects, 0, objectFileSize)) }));
    _entries.emplace(GameFile::Words, std::unordered_map<int, GameVolumeEntry>({ std::make_pair(0, GameVolumeEntry(GameFile::Words, 0, wordsFileSize)) }));
}

void GameVolume::parseDirectoryV2() {
    parseObjectsAndWordsEntries();

    auto volumeSizes(gatherVolumeSizes());

    loadDirectoryV2(GameFile::Picture, volumeSizes);
    loadDirectoryV2(GameFile::View, volumeSizes);
    loadDirectoryV2(GameFile::Sound, volumeSizes);
    loadDirectoryV2(GameFile::Logic, volumeSizes);
}

void GameVolume::parseDirectoryV3() {
    parseObjectsAndWordsEntries();

    std::string dirFile = _info.files().at(GameFile::Directory);
    size_t fileSize = _platform->fileSize(dirFile.c_str());
    if (fileSize == (size_t)-1)
        throw std::runtime_error("Failed to determine size of resource directory");

    std::vector<uint8_t> dirData;

    dirData.resize(fileSize);
    if (!_platform->fileRead(dirFile.c_str(), 0, dirData.data(), fileSize))
        throw std::runtime_error("Failed to read resource directory");

    const uint16_t* dirOffsets = (const uint16_t*)dirData.data();
    uint16_t dirLengths[4] = {0, 0, 0, 0};

    for (int i = 0; i < 4; i++) {
        if (i == 3)
            dirLengths[i] = fileSize - dirOffsets[i];
        else
            dirLengths[i] = dirOffsets[i + 1] - dirOffsets[i];
    }

    auto volumeSizes(gatherVolumeSizes());

    loadDirectoryV3(GameFile::Logic,   (dirData.data() + dirOffsets[0]), dirLengths[0], volumeSizes);
    loadDirectoryV3(GameFile::Picture, (dirData.data() + dirOffsets[1]), dirLengths[1], volumeSizes);
    loadDirectoryV3(GameFile::View,    (dirData.data() + dirOffsets[2]), dirLengths[2], volumeSizes);
    loadDirectoryV3(GameFile::Sound,   (dirData.data() + dirOffsets[3]), dirLengths[3], volumeSizes);
}

class GameVolumeEntryBuilder {
public:
    uint8_t index;
    uint8_t volume;
    size_t  offset;

public:
    GameVolumeEntryBuilder(uint8_t i, uint8_t v, size_t o) : index(i), volume(v), offset(o) {
    }

    bool operator < (const GameVolumeEntryBuilder& other) const {
        if (volume < other.volume)
            return true;
        else if (volume > other.volume)
            return false;

        return offset < other.offset;
    }
};

void GameVolume::loadDirectoryV2(GameFile file, const volume_sizes_t& volumeSizes) {
    std::string dirFile = _info.files().at(file);
    size_t fileSize = _platform->fileSize(dirFile.c_str());
    if (fileSize == (size_t)-1)
        throw std::runtime_error(format("Failed to determine size of resource %i directory", (int)file));

    std::vector<uint8_t> dirData;

    dirData.resize(fileSize);
    if (!_platform->fileRead(dirFile.c_str(), 0, dirData.data(), fileSize))
        throw std::runtime_error(format("Failed to read resource %i directory", (int)file));

    loadDirectoryV3(file, dirData.data(), fileSize, volumeSizes);
}

void GameVolume::loadDirectoryV3(GameFile file, uint8_t* offsets, size_t length, const volume_sizes_t& volumeSizes) {
    assert(length % 3 == 0);

    std::unordered_map<int, GameVolumeEntry> map;
    std::vector<GameVolumeEntryBuilder> entries;

    entries.reserve(length / 3);
    map.reserve(length / 3);

    for (uint8_t i = 0; length; i++, offsets += 3, length -= 3) {
        uint8_t volume = offsets[0] >> 4;
        size_t  offset = ((offsets[0] & 0xf) << 16) | (offsets[1] << 8) | offsets[2];

        if (offset == 0xfffff)
            // Does not exists
            continue;

        if (volumeSizes.count(volumeToGameFile(volume)) == 0) {
            _platform->log("Directory %i, Entry %i made reference to missing volume %i", (int)file, (int)i, (int)volume);
            continue;
        }

        entries.push_back(GameVolumeEntryBuilder(i, volume, offset));
    }

    std::sort(entries.begin(), entries.end());

    for (std::vector<GameVolumeEntryBuilder>::const_iterator it = entries.begin(); it != entries.end(); ++it) {
        std::vector<GameVolumeEntryBuilder>::const_iterator next(it); ++next;

        auto volume = volumeToGameFile((*it).volume);
        size_t offset = (*it).offset;
        size_t nextOffset = (*it).offset;

        if (next == entries.end())
            nextOffset = volumeSizes.at(volume);
        else if ((*next).volume != (*it).volume)
            nextOffset = volumeSizes.at(volume);
        else
            nextOffset = (*next).offset;

        map.emplace((*it).index, GameVolumeEntry(volume, offset, nextOffset - offset));
    }

    _entries.emplace(file, map);
}

bool GameVolume::exists(GameFile file, uint8_t id) const {
    if (_entries.count(file) == 0)
        return false;

    const auto& map = _entries.at(file);
    if (map.count(id) == 0)
        return false;

    return true;
}

std::vector<uint8_t> GameVolume::load(GameFile file, uint8_t id) {
    if (_info.version() >= 0x3000)
        return loadV3(file, id);
    else
        return loadV2(file, id);
}

#define CRYPT_KEY_SIERRA    (uint8_t*)("Avis Durgan")
#define CRYPT_KEY_AGDS      (uint8_t*)("Alex Simkin")
#define CRYPT_KEY_LENGTH    11

static void decrypt(uint8_t* data, size_t len, uint8_t* key, size_t keyLength) {
    for (size_t i = 0; i < len; i++)
        *(data + i) ^= *(key + (i % keyLength));
}

static void decryptLogic(uint8_t* data, size_t len, uint8_t* key, size_t keyLength) {
    uint8_t* m0     = data;
    uint16_t mstart = readUINT16LE(m0) + 2;
    uint8_t  mc     = m0[mstart];
    uint16_t mend   = readUINT16LE(m0 + mstart + 1);

    m0 += mstart + 3;
    mstart = mc << 1;

    if (mc > 0)
        decrypt(m0 + mstart, mend - mstart, key, keyLength);
}

std::vector<uint8_t> GameVolume::loadRaw(GameFile file, uint8_t id) {
    auto e = entry(file, id);
    std::vector<uint8_t> buffer;

    buffer.resize(e.length());

    if (!_platform->fileRead(_info.files().at(e.file()).c_str(), e.offset(), buffer.data(), e.length()))
        throw std::runtime_error(format("Failed to read volume %i", (int)e.file()));

    if (file == GameFile::Objects) {
        uint16_t n = readUINT16LE(buffer.data());
        if (n > e.length())
            decrypt(buffer.data(), buffer.size(), CRYPT_KEY_SIERRA, CRYPT_KEY_LENGTH);
    }

    if (file == GameFile::Words || file == GameFile::Objects)
        return buffer;

    if (buffer[0] != 0x12 || buffer[1] != 0x34)
        throw std::runtime_error("Invalid resource signature");

    return buffer;
}

std::vector<uint8_t> GameVolume::loadV2(GameFile file, uint8_t id) {
    std::vector<uint8_t> buffer(loadRaw(file, id));
    uint16_t uncompressedLength = readUINT16LE(buffer.data() + 3);

    buffer.erase(buffer.begin(), buffer.begin() + 5);

    if (buffer.size() != uncompressedLength)
        buffer.resize(uncompressedLength);

    if (file == GameFile::Logic)
        decryptLogic(buffer.data(), buffer.size(), CRYPT_KEY_SIERRA, CRYPT_KEY_LENGTH);

    return buffer;
}

static void picExpand(const uint8_t* input, size_t inputLength, uint8_t* output, size_t outputLength) {
    bool   dataOffsetNibble = false;
    size_t dataOffset       = 0;

    auto readByte = [input, &dataOffsetNibble, &dataOffset]() {
        if (!dataOffsetNibble) {
            return input[dataOffset++];
        }
        else {
            uint8_t curByte = input[dataOffset++] << 4;
            return (uint8_t)((input[dataOffset] >> 4) | curByte);
        }
    };

    auto readNibble = [input, &dataOffsetNibble, &dataOffset]() {
        if (!dataOffsetNibble) {
            dataOffsetNibble = true;
            return input[dataOffset] >> 4;
        } else {
            dataOffsetNibble = false;
            return input[dataOffset++] & 0x0F;
        }
    };

    while (dataOffset < inputLength) {
        uint8_t curByte = readByte();

        *output++ = curByte;

        switch (curByte) {
        case 0xf0:
            *output++ = readNibble();
            break;
        case 0xf2:
            *output++ = readNibble();
            break;
        case 0xff:
            dataOffset = inputLength;
            break;
        }
    }
}

std::vector<uint8_t> GameVolume::loadV3(GameFile file, uint8_t id) {
    std::vector<uint8_t> buffer(loadRaw(file, id));

    uint16_t flags              = buffer[2];
    uint16_t uncompressedLength = readUINT16LE(buffer.data() + 3);
    uint16_t compressedLength   = readUINT16LE(buffer.data() + 5);

    if (compressedLength == uncompressedLength) {
        buffer.erase(buffer.begin(), buffer.begin() + 7);

        if (buffer.size() != compressedLength)
            buffer.resize(compressedLength);

        if (file == GameFile::Logic)
            decryptLogic(buffer.data(), buffer.size(), CRYPT_KEY_SIERRA, CRYPT_KEY_LENGTH);

        return buffer;
    }

    std::vector<uint8_t> uncompressedBuffer;

    uncompressedBuffer.reserve(uncompressedLength + 32);
    uncompressedBuffer.resize(uncompressedLength, 0);
    buffer.resize(buffer.size() + 32);

    if (file == GameFile::Picture && flags & 0x80)
        picExpand(buffer.data() + 7, compressedLength, uncompressedBuffer.data(), uncompressedLength);
    else
        LZWExpand(buffer.data() + 7, compressedLength, uncompressedBuffer.data(), uncompressedLength);

    return uncompressedBuffer;
}

const GameVolumeEntry& GameVolume::entry(GameFile file, uint8_t id) const {
    if (_entries.count(file) == 0)
        throw std::runtime_error(format("Directory %i does not exists", (int)file));

    const auto& map = _entries.at(file);
    if (map.count(id) == 0)
        throw std::runtime_error(format("Directory %i Entry %i does not exists", (int)file, (int)id));

    return map.at(id);
}
