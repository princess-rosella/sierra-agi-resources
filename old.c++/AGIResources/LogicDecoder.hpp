//
//  LogicDecoder.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__LogicDecoder_hpp__
#define __AGIResources__LogicDecoder_hpp__

#include "LogicInstructionSet.hpp"

namespace AGI { namespace Resources {

    class LogicDecoder;

    class LogicInstruction {
    protected:
        uint16_t                  _ip;
        std::string               _opcode;
        std::vector<LogicOperand> _operands;
        uint16_t                  _destination;

    public:
        LogicInstruction(uint16_t ip, const std::string& opcode, const std::vector<LogicOperand>& operands, uint16_t destination);

    protected:
        LogicInstruction();

    public:
        uint16_t                          ip()          const { return _ip; }
        const std::string&                opcode()      const { return _opcode; }
        const std::vector<LogicOperand>&  operands()    const { return _operands; }
        uint16_t                          destination() const { return _destination; }
    };

    class LogicInstructionBuffer : public LogicInstruction {
    public:
        friend class LogicDecoder;

    private:
        const uint8_t* _instructionStart;
        const uint8_t* _instructionCurrent;
        const uint8_t* _instructionEnd;

    private:
        LogicInstructionBuffer();

    public:
        const uint8_t* instructionStart()   const { return _instructionStart; }
        const uint8_t* instructionCurrent() const { return _instructionCurrent; }
        const uint8_t* instructionEnd()     const { return _instructionEnd; }

    private:
        LogicInstructionBuffer(const LogicInstructionBuffer&) = delete;
        LogicInstructionBuffer(LogicInstructionBuffer&&) = delete;

        LogicInstructionBuffer& operator = (const LogicInstructionBuffer&) = delete;
        LogicInstructionBuffer& operator = (LogicInstructionBuffer&&) = delete;
    };

    class LogicCallback {
    public:
        virtual ~LogicCallback() {}

    public:
        virtual void message(size_t index, const std::string& message) = 0;

        virtual void beginCondition(const LogicInstructionBuffer&) = 0;
        virtual void condition     (const LogicInstructionBuffer&) = 0;
        virtual void endCondition  (const LogicInstructionBuffer&) = 0;

        virtual void beginAnd(const LogicInstructionBuffer&) = 0;
        virtual void endAnd  (const LogicInstructionBuffer&) = 0;
        virtual void beginOr (const LogicInstructionBuffer&) = 0;
        virtual void endOf   (const LogicInstructionBuffer&) = 0;
        virtual void beginNot(const LogicInstructionBuffer&) = 0;
        virtual void endNot  (const LogicInstructionBuffer&) = 0;

        virtual void instruction(const LogicInstructionBuffer&) = 0;
    };

    class LogicDecoder {
    private:
        std::vector<uint8_t> _buffer;

    public:
        LogicDecoder(std::vector<uint8_t>&& buffer);
        LogicDecoder(const std::vector<uint8_t>& buffer);

    public:
        void decode(const LogicInstructionSet& instructionSet, LogicCallback& callback);

    private:
        void buildTables();
    };

}}

#endif /* __AGIResources__LogicDecoder_hpp__ */
