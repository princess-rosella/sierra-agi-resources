//
//  AGIResources.h
//  AGIResources
//
//  Copyright © 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__h__
#define __AGIResources__h__

#include <stdint.h>
#include <stdlib.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef __printflike
#define __printflike(f,e)
#endif

namespace AGI { namespace Resources {

class PlatformAbstractionLayer;

class GameInfo;
class GameVolume;
class GameVolumeEntry;

class PictureCallback;
class PictureDecoder;
class PictureRasterizer;
class PictureTracer;

class LogicCallback;
class LogicDecoder;
class LogicDisassembler;
class LogicDumper;
class LogicOperand;
class LogicInstructionInfo;

enum class GameFile: uint8_t {
    Volume_0,
    Volume_1,
    Volume_2,
    Volume_3,
    Volume_4,
    Volume_5,
    Volume_6,
    Volume_7,
    Volume_8,
    Volume_9,
    Directory,
    Logic,
    Picture,
    Sound,
    View,
    Objects,
    Words,
};

enum {
    PictureWidth  = 160,
    PictureHeight = 168
};

std::string format(const char* format, ...) __printflike(1, 2);

}}

#endif /* __AGIResources__h__ */
