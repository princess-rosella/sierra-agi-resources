//
//  LogicDumper.hpp
//  AGI
//
//  Copyright (c) 2018 Princess Rosella. All rights reserved.
//

#ifndef __AGIResources__LogicDumper_hpp__
#define __AGIResources__LogicDumper_hpp__

#include "LogicDecoder.hpp"

#include <ostream>

namespace AGI { namespace Resources {

    class LogicDumper : public LogicCallback {
    private:
        std::vector<std::string> _messages;
        std::ostream& _out;

    public:
        inline LogicDumper(std::ostream& out) : _out(out) {
        }

    public:
        virtual void message(size_t index, const std::string& message) override;

        virtual void beginCondition(const LogicInstructionBuffer&) override;
        virtual void condition     (const LogicInstructionBuffer&) override;
        virtual void endCondition  (const LogicInstructionBuffer&) override;

        virtual void beginAnd(const LogicInstructionBuffer&) override;
        virtual void endAnd  (const LogicInstructionBuffer&) override;
        virtual void beginOr (const LogicInstructionBuffer&) override;
        virtual void endOf   (const LogicInstructionBuffer&) override;
        virtual void beginNot(const LogicInstructionBuffer&) override;
        virtual void endNot  (const LogicInstructionBuffer&) override;

        virtual void instruction(const LogicInstructionBuffer&) override;
    };

}}

#endif /* __AGIResources__LogicDumper_hpp__ */
