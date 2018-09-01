//
//  GameVolume.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__GameVolume_hpp__
#define __AGIResources__GameVolume_hpp__

#include "AGIResources.hpp"
#include "GameInfo.hpp"

namespace AGI { namespace Resources {

    class GameVolumeEntry
    {
    private:
        GameFile _file;
        size_t   _offset;
        size_t   _length;

    public:
        inline GameVolumeEntry(GameFile file, size_t offset, size_t length) : _file(file), _offset(offset), _length(length) {
        }

    public:
        inline GameFile file()   const { return _file; }
        inline size_t   offset() const { return _offset; }
        inline size_t   length() const { return _length; }
    };

    /**
     * Game Volume is the class that is able to understand how to retreive a
     * particular part of the game, based on it's identifier, regardless where on the
     * file system it is stored.
     *
     * It abstracts:
     *   1. Compression
     *   2. Encryption
     */
    class GameVolume
    {
    private:
        std::unique_ptr<PlatformAbstractionLayer> _platform;
        GameInfo                                  _info;

        std::unordered_map<GameFile, std::unordered_map<int, GameVolumeEntry>> _entries;

    private:
        typedef std::unordered_map<GameFile, size_t> volume_sizes_t;

    public:
        GameVolume(PlatformAbstractionLayer* platform);
        GameVolume(const GameInfo& info, PlatformAbstractionLayer* platform);

        std::vector<uint8_t> load(GameFile file, uint8_t id);

        bool exists(GameFile file, uint8_t id) const;

        template<typename Lambda>
        void enumerate(const Lambda& lambda) {
            for (const auto& file : _entries) {
                for (const auto& entry : file.second)
                    lambda(*this, file.first, entry.first, entry.second.length());
            }
        }

    public:
        inline PlatformAbstractionLayer& platform() const { return *_platform.get(); }
        inline const GameInfo& info() const { return _info; }

    private:
        void parseDirectory();
        void parseDirectoryV2();
        void parseDirectoryV3();
        void parseObjectsAndWordsEntries();

        void loadDirectoryV2(GameFile file, const volume_sizes_t& volumeSizes);
        void loadDirectoryV3(GameFile file, uint8_t* offsets, size_t length, const volume_sizes_t& volumeSizes);

        std::vector<uint8_t> loadV2(GameFile file, uint8_t id);
        std::vector<uint8_t> loadV3(GameFile file, uint8_t id);
        std::vector<uint8_t> loadRaw(GameFile file, uint8_t id);

        volume_sizes_t gatherVolumeSizes();

        const GameVolumeEntry& entry(GameFile file, uint8_t id) const;
    };

}}

#endif /* __AGIResources__GameInfo_hpp__ */
