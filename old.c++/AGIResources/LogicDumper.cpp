//
//  LogicDumper.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "LogicDumper.hpp"

using namespace AGI::Resources;

void LogicDumper::message(size_t index, const std::string& message) {
    _messages.push_back(message);

    _out << "// Message #" << index << ": \"" << message << "\"" << std::endl;
}

void LogicDumper::beginCondition(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::condition(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::endCondition(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::beginAnd(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::endAnd(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::beginOr(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::endOf(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::beginNot(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::endNot(const LogicInstructionBuffer& instruction) {
}

void LogicDumper::instruction(const LogicInstructionBuffer& instruction) {
}
