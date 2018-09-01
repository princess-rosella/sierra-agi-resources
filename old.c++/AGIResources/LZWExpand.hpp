//
//  LZWExpand.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__LZWExpand_hpp__
#define __AGIResources__LZWExpand_hpp__

#include <stdint.h>
#include <stdlib.h>

namespace AGI { namespace Resources {

    bool LZWExpand(const uint8_t* input, size_t inputSize, uint8_t* output, size_t outputSize);

}}

#endif /* __AGIResources__LZWExpand_hpp__ */
