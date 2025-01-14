/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "AirValidate.h"

#if ENABLE(B3_JIT)

#include "AirCode.h"
#include "AirInstInlines.h"

namespace JSC { namespace B3 { namespace Air {

namespace {

class Validater {
public:
    Validater(Code& code, const char* dumpBefore)
        : m_code(code)
        , m_dumpBefore(dumpBefore)
    {
    }

#define VALIDATE(condition, message) do {                               \
        if (condition)                                                  \
            break;                                                      \
        fail(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, #condition, toCString message); \
    } while (false)
    
    void run()
    {
        HashSet<StackSlot*> validSlots;
        HashSet<BasicBlock*> validBlocks;
        HashSet<Special*> validSpecials;

        for (BasicBlock* block : m_code)
            validBlocks.add(block);
        for (StackSlot* slot : m_code.stackSlots())
            validSlots.add(slot);
        for (Special* special : m_code.specials())
            validSpecials.add(special);

        for (BasicBlock* block : m_code) {
            for (unsigned instIndex = 0; instIndex < block->size(); ++instIndex) {
                Inst& inst = block->at(instIndex);
                for (Arg& arg : inst.args) {
                    switch (arg.kind()) {
                    case Arg::Stack:
                        VALIDATE(validSlots.contains(arg.stackSlot()), ("At ", inst, " in ", *block));
                        break;
                    case Arg::Special:
                        VALIDATE(validSpecials.contains(arg.special()), ("At ", inst, " in ", *block));
                        break;
                    default:
                        break;
                    }
                }
                VALIDATE(inst.isValidForm(), ("At ", inst, " in ", *block));
                if (instIndex == block->size() - 1)
                    VALIDATE(isTerminal(inst.opcode), ("At ", inst, " in ", *block));
                else
                    VALIDATE(!isTerminal(inst.opcode), ("At ", inst, " in ", *block));
            }
            for (BasicBlock* successor : block->successorBlocks())
                VALIDATE(validBlocks.contains(successor), ("In ", *block));
        }
    }

private:
    NO_RETURN_DUE_TO_CRASH void fail(
        const char* filename, int lineNumber, const char* function, const char* condition,
        CString message)
    {
        CString failureMessage;
        {
            StringPrintStream out;
            out.print("AIR VALIDATION FAILURE\n");
            out.print("    ", condition, " (", filename, ":", lineNumber, ")\n");
            out.print("    ", message, "\n");
            out.print("    After ", m_code.lastPhaseName(), "\n");
            failureMessage = out.toCString();
        }

        dataLog(failureMessage);
        if (m_dumpBefore) {
            dataLog("Before ", m_code.lastPhaseName(), ":\n");
            dataLog(m_dumpBefore);
        }
        dataLog("At time of failure:\n");
        dataLog(m_code);

        dataLog(failureMessage);
        WTFReportAssertionFailure(filename, lineNumber, function, condition);
        CRASH();
    }
    
    Code& m_code;
    const char* m_dumpBefore;
};

} // anonymous namespace

void validate(Code& code, const char* dumpBefore)
{
    Validater validater(code, dumpBefore);
    validater.run();
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

