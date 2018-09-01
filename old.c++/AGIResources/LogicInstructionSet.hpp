//
//  LogicInstructionSet.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__LogicInstructionSet_hpp__
#define __AGIResources__LogicInstructionSet_hpp__

#include <stdint.h>
#include <stdlib.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace AGI { namespace Resources {

    class LogicOperand {
    public:
        enum class Type {
            Logic,
            Inventory,
            Picture,
            Sound,
            View,
            Vocabulary,
            Controller,
            Object,

            Constant,
            Message,
            Variable,
            VariableReference, // Variable containing a Variable ID
            Flag,
            FlagReference,     // Variable containing a Flag ID
            String,
        };

    private:
        Type    _type;
        uint8_t _data;

    public:
        LogicOperand(Type type, uint8_t data);

    public:
        inline Type        type()   const { return _type; }
        inline uint8_t     data()   const { return _data; }
    };

    class LogicInstructionInfo {
    private:
        std::string _name;
        std::vector<LogicOperand::Type> _types;

    public:
        LogicInstructionInfo(const char* name, size_t count, ...);

    public:
        inline const std::string&                     name()  const { return _name; };
        inline const std::vector<LogicOperand::Type>& types() const { return _types; }
    };

    class LogicInstructionSet {
    private:
        std::unordered_map<uint8_t, LogicInstructionInfo> _instructions;
        std::unordered_map<uint8_t, LogicInstructionInfo> _conditions;

    public:
        LogicInstructionSet();

    public:
        const LogicInstructionInfo& condition(uint8_t conditionID) const;
        const LogicInstructionInfo& instruction(uint8_t conditionID) const;
    };

}}

#endif /* __AGIResources__LogicInstructionSet_hpp__ */
