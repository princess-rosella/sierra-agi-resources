//
//  LogicInstructionSet.cpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#include "LogicInstructionSet.hpp"

using namespace AGI::Resources;

LogicOperand::LogicOperand(Type type, uint8_t data) : _type(type), _data(data) {
}

LogicInstructionInfo::LogicInstructionInfo(const char* name, size_t count, ...) : _name(name), _types() {
    va_list list;

    _types.resize(count);

    va_start(list, count);

    for (size_t index = 0; index < count; index++) {
        _types[index] = va_arg(list, LogicOperand::Type);
    }

    va_end(list);
}

LogicInstructionSet::LogicInstructionSet() {
    _conditions.emplace(std::make_pair(0x01, LogicInstructionInfo("==",  2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));
    _conditions.emplace(std::make_pair(0x02, LogicInstructionInfo("==",  2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _conditions.emplace(std::make_pair(0x03, LogicInstructionInfo("<",   2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));
    _conditions.emplace(std::make_pair(0x04, LogicInstructionInfo("<",   2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _conditions.emplace(std::make_pair(0x05, LogicInstructionInfo(">",   2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));
    _conditions.emplace(std::make_pair(0x06, LogicInstructionInfo(">",   2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _conditions.emplace(std::make_pair(0x07, LogicInstructionInfo("!!",  1, LogicOperand::Type::Flag)));
    _conditions.emplace(std::make_pair(0x08, LogicInstructionInfo("!!",  1, LogicOperand::Type::Variable)));
    _conditions.emplace(std::make_pair(0x09, LogicInstructionInfo("has", 1, LogicOperand::Type::Inventory)));

    _conditions.emplace(std::make_pair(0x0a, LogicInstructionInfo("obj.in.room", 2, LogicOperand::Type::Inventory, LogicOperand::Type::Variable)));

    _conditions.emplace(std::make_pair(0x0b, LogicInstructionInfo("pos",        5, LogicOperand::Type::Object, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant)));
    _conditions.emplace(std::make_pair(0x10, LogicInstructionInfo("obj.in.box", 5, LogicOperand::Type::Object, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant)));
    _conditions.emplace(std::make_pair(0x11, LogicInstructionInfo("center.pos", 5, LogicOperand::Type::Object, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant)));
    _conditions.emplace(std::make_pair(0x12, LogicInstructionInfo("right.pos",  5, LogicOperand::Type::Object, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant)));

    _conditions.emplace(std::make_pair(0x0c, LogicInstructionInfo("controller",      1, LogicOperand::Type::Controller)));
    _conditions.emplace(std::make_pair(0x0d, LogicInstructionInfo("have.key",        0)));
    _conditions.emplace(std::make_pair(0x0e, LogicInstructionInfo("said",            0)));
    _conditions.emplace(std::make_pair(0x0f, LogicInstructionInfo("compare.strings", 2, LogicOperand::Type::String, LogicOperand::Type::String)));

    _instructions.emplace(std::make_pair(0x00,  LogicInstructionInfo("return", 0)));

    _instructions.emplace(std::make_pair(0x01,  LogicInstructionInfo("++", 1, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x02,  LogicInstructionInfo("--", 1, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x03,  LogicInstructionInfo("=", 2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x04,  LogicInstructionInfo("=", 2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x05,  LogicInstructionInfo("+", 2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x06,  LogicInstructionInfo("+", 2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x07,  LogicInstructionInfo("-", 2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x08,  LogicInstructionInfo("-", 2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x09,  LogicInstructionInfo("lindirect", 2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x0a,  LogicInstructionInfo("rindirect", 2, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x0b,  LogicInstructionInfo("lindirect", 2, LogicOperand::Type::Variable, LogicOperand::Type::Constant)));

    _instructions.emplace(std::make_pair(0x0c,  LogicInstructionInfo("set",    1, LogicOperand::Type::Flag)));
    _instructions.emplace(std::make_pair(0x0d,  LogicInstructionInfo("reset",  1, LogicOperand::Type::Flag)));
    _instructions.emplace(std::make_pair(0x0e,  LogicInstructionInfo("toggle", 1, LogicOperand::Type::Flag)));

    _instructions.emplace(std::make_pair(0x0f,  LogicInstructionInfo("set",    1, LogicOperand::Type::FlagReference)));
    _instructions.emplace(std::make_pair(0x10,  LogicInstructionInfo("reset",  1, LogicOperand::Type::FlagReference)));
    _instructions.emplace(std::make_pair(0x11,  LogicInstructionInfo("toggle", 1, LogicOperand::Type::FlagReference)));

    _instructions.emplace(std::make_pair(0x12,  LogicInstructionInfo("new.room", 1, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x13,  LogicInstructionInfo("new.room", 1, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x14,  LogicInstructionInfo("load.logic", 1, LogicOperand::Type::Logic)));
    _instructions.emplace(std::make_pair(0x15,  LogicInstructionInfo("load.logic", 1, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x16,  LogicInstructionInfo("call.logic", 1, LogicOperand::Type::Logic)));
    _instructions.emplace(std::make_pair(0x17,  LogicInstructionInfo("call.logic", 1, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x18,  LogicInstructionInfo("load.pic", 1, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x19,  LogicInstructionInfo("draw.pic", 1, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x1a,  LogicInstructionInfo("show.pic", 0)));
    _instructions.emplace(std::make_pair(0x1b,  LogicInstructionInfo("discard.pic", 1, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x1c,  LogicInstructionInfo("overlay.pic", 1, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x1d,  LogicInstructionInfo("show.pri.screen", 0)));

    _instructions.emplace(std::make_pair(0x1e,  LogicInstructionInfo("load.view", 1, LogicOperand::Type::View)));
    _instructions.emplace(std::make_pair(0x1f,  LogicInstructionInfo("load.view", 1, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x20,  LogicInstructionInfo("discard.view", 1, LogicOperand::Type::View)));

    _instructions.emplace(std::make_pair(0x21,  LogicInstructionInfo("animate", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x22,  LogicInstructionInfo("unanimate.all", 0)));
    _instructions.emplace(std::make_pair(0x23,  LogicInstructionInfo("draw", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x24,  LogicInstructionInfo("erase", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x25,  LogicInstructionInfo("position", 3, LogicOperand::Type::Object, LogicOperand::Type::Constant, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x26,  LogicInstructionInfo("position", 3, LogicOperand::Type::Object, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x27,  LogicInstructionInfo("get.position", 3, LogicOperand::Type::Object, LogicOperand::Type::VariableReference, LogicOperand::Type::VariableReference)));
    _instructions.emplace(std::make_pair(0x28,  LogicInstructionInfo("reposition", 3, LogicOperand::Type::Object, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x29,  LogicInstructionInfo("set.view", 2, LogicOperand::Type::Object, LogicOperand::Type::View)));
    _instructions.emplace(std::make_pair(0x2a,  LogicInstructionInfo("set.view", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x2b,  LogicInstructionInfo("set.loop", 2, LogicOperand::Type::Object, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x2c,  LogicInstructionInfo("set.loop", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x2d,  LogicInstructionInfo("fix.loop", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x2e,  LogicInstructionInfo("release.loop", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x2f,  LogicInstructionInfo("set.cell", 2, LogicOperand::Type::Object, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x30,  LogicInstructionInfo("set.cell", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x31,  LogicInstructionInfo("last.cell", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x32,  LogicInstructionInfo("current.cell", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x33,  LogicInstructionInfo("current.loop", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x34,  LogicInstructionInfo("current.view", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x35,  LogicInstructionInfo("number.of.loops", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x36,  LogicInstructionInfo("set.priority", 2, LogicOperand::Type::Object, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x37,  LogicInstructionInfo("set.priority", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));
    _instructions.emplace(std::make_pair(0x38,  LogicInstructionInfo("release.priority", 2, LogicOperand::Type::Object, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x39,  LogicInstructionInfo("get.priority", 2, LogicOperand::Type::Object, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x3a,  LogicInstructionInfo("stop.update", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x3b,  LogicInstructionInfo("start.update", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x3c,  LogicInstructionInfo("force.update", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x3d,  LogicInstructionInfo("ignore.horizon", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x3e,  LogicInstructionInfo("observe.horizon", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x3f,  LogicInstructionInfo("set.horizon", 1, LogicOperand::Type::Constant)));

    _instructions.emplace(std::make_pair(0x40,  LogicInstructionInfo("object.on.water", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x41,  LogicInstructionInfo("object.on.land", 1, LogicOperand::Type::Object)));
    _instructions.emplace(std::make_pair(0x42,  LogicInstructionInfo("object.on.anything", 1, LogicOperand::Type::Object)));

    _instructions.emplace(std::make_pair(0x61,  LogicInstructionInfo("move.obj", 5, LogicOperand::Type::Object, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::Constant)));
    _instructions.emplace(std::make_pair(0x62,  LogicInstructionInfo("move.obj", 5, LogicOperand::Type::Object, LogicOperand::Type::Variable, LogicOperand::Type::Variable, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));

    _instructions.emplace(std::make_pair(0x77,  LogicInstructionInfo("display", 3, LogicOperand::Type::Constant, LogicOperand::Type::Constant, LogicOperand::Type::String)));
    _instructions.emplace(std::make_pair(0x78,  LogicInstructionInfo("display", 3, LogicOperand::Type::Variable, LogicOperand::Type::Variable, LogicOperand::Type::Variable)));
}

const LogicInstructionInfo& LogicInstructionSet::condition(uint8_t conditionID) const {
    assert(_conditions.find(conditionID) != _conditions.end());
    return _conditions.at(conditionID);
}

const LogicInstructionInfo& LogicInstructionSet::instruction(uint8_t instructionID) const {
    assert(_instructions.find(instructionID) != _instructions.end());
    return _instructions.at(instructionID);
}
