//
//  LogicDisassembler.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "LogicDisassembler.hpp"

using namespace AGI::Resources;

void LogicDisassembler::message(size_t index, const std::string& message) {
    _messages.push_back(message);
}

void LogicDisassembler::beginCondition(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::condition(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::endCondition(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::beginAnd(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::endAnd(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::beginOr(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::endOf(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::beginNot(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::endNot(const LogicInstructionBuffer& instruction) {
}

void LogicDisassembler::instruction(const LogicInstructionBuffer& instruction) {
}
