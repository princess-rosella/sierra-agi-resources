//
//  main.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include <iostream>

#include "AGIResources/AGIResources.hpp"

#include "AGIResources/GameVolume.hpp"
#include "AGIResources/LogicDecoder.hpp"
#include "AGIResources/LogicDisassembler.hpp"
#include "AGIResources/LogicDumper.hpp"
#include "AGIResources/LogicInstructionSet.hpp"
#include "AGIResources/PictureRasterizer.hpp"
#include "AGIResources/PlatformAbstractionLayer_macOS.hpp"

using namespace AGI::Resources;

int main(int argc, const char * argv[]) {
    try {
        GameVolume game(new PlatformAbstractionLayer_macOS(argv[1]));

        std::cout << "Detected: " << game.info().code() << " " << game.info().description() << std::endl;

        game.enumerate([](GameVolume& volume, GameFile file, uint8_t id, size_t length) {
            if (file == GameFile::Picture) {
                uint8_t screen[160 * 168];
                uint8_t priority[160 * 168];
                PictureRasterizer rasterizer(volume.info(), screen, priority);
                PictureDecoder encoded(volume.load(file, id));

                encoded.decode(rasterizer);
                return;
            }
            else if (file == GameFile::Logic) {
                LogicDecoder decoder(volume.load(file, id));
                LogicInstructionSet instructionSet;
                LogicDumper dumper(std::cout);

                decoder.decode(instructionSet, dumper);
                return;
            }

            volume.load(file, id);
        });
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
