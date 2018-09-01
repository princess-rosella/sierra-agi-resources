//
//  Endian.h
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__Endian_hpp__
#define __AGIResources__Endian_hpp__

namespace AGI { namespace Resources {

    inline uint16_t readUINT16LE(uint8_t* data) {
       uint16_t low  = data[0];
       uint16_t high = data[1];
       return (high << 8) | low;
    }

}}

#endif /* __AGIResources__Endian_hpp__ */
