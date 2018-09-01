//
//  LogicDecoder.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "LogicDecoder.hpp"

#include "Endian.hpp"

using namespace AGI::Resources;

LogicInstruction::LogicInstruction() :
    _ip         (0),
    _destination(0) {
}

LogicInstruction::LogicInstruction(uint16_t ip, const std::string& opcode, const std::vector<LogicOperand>& operands, uint16_t destination) :
    _ip         (ip),
    _opcode     (opcode),
    _operands   (operands),
    _destination(destination) {
}

LogicInstructionBuffer::LogicInstructionBuffer() :
    _instructionStart  (nullptr),
    _instructionCurrent(nullptr),
    _instructionEnd    (nullptr) {
}

LogicDecoder::LogicDecoder(std::vector<uint8_t>&& buffer) : _buffer(std::move(buffer)) {
}

LogicDecoder::LogicDecoder(const std::vector<uint8_t>& buffer) : _buffer(buffer) {
}

void LogicDecoder::decode(const LogicInstructionSet& instructionSet, LogicCallback& callback) {
    uint8_t* data   = _buffer.data();
    uint8_t* m0     = data;
    uint16_t mstart = readUINT16LE(m0) + 2;
    uint8_t  mc     = m0[mstart];
    uint16_t mend   = readUINT16LE(m0 + mstart + 1);

    m0 += mstart + 3;

    for (size_t i = 0; i < mc; i++) {
        mend = readUINT16LE(m0 + i * 2);
        callback.message(i, mend ? std::string((const char*)m0 + mend - 2) : std::string());
    }

    uint8_t* istart = data + 2;
    uint8_t* ip     = istart;
    uint8_t* iend   = data + mstart;

    bool isInCondition = false;
    LogicInstructionBuffer ibuffer;

    ibuffer._instructionStart = istart;

    while (ip != iend) {
        ibuffer._instructionCurrent = ip;
        ibuffer._ip = (uint16_t)(ip - istart);
        ibuffer._destination = ibuffer._ip;
        ibuffer._opcode.clear();
        ibuffer._operands.clear();

        uint8_t instructionID = *ip++;

        if (instructionID == 0xff) {
            isInCondition = !isInCondition;

            if (isInCondition) {
                ibuffer._instructionStart = ibuffer._instructionCurrent;
                ibuffer._instructionEnd = ip;
                callback.beginCondition(ibuffer);
                callback.beginAnd(ibuffer);
            }
            else {
                size_t disp = readUINT16LE(ip);

                ibuffer._destination = ibuffer._ip + disp;
                ibuffer._instructionEnd = ip + 2;
                callback.endAnd(ibuffer);
                callback.endCondition(ibuffer);
                ibuffer._instructionStart = nullptr;
            }
        }
        else  if (instructionID == 0xfe) {
            size_t disp = readUINT16LE(ip);

            ibuffer._opcode = "goto";
            ibuffer._destination = ibuffer._ip + disp;
            ibuffer._instructionEnd = ip + 2;
            callback.instruction(ibuffer);
        }
        else if (isInCondition) {
            const auto&   condition = instructionSet.condition(instructionID);
            size_t        count     = condition.types().size();

            for (size_t index = 0; index < count; index++) {
                ibuffer._operands.push_back(LogicOperand(condition.types()[index], ip[index]));
            }

            ibuffer._instructionEnd = ip + count;
            callback.condition(ibuffer);
        }
        else {
            const auto&   instruction = instructionSet.instruction(instructionID);
            size_t        count       = instruction.types().size();

            for (size_t index = 0; index < count; index++) {
                ibuffer._operands.push_back(LogicOperand(instruction.types()[index], ip[index]));
            }

            ibuffer._instructionEnd = ip + count;
            callback.instruction(ibuffer);
        }
    }
}
