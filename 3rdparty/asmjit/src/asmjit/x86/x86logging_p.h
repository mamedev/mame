// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef ASMJIT_X86_X86LOGGING_P_H_INCLUDED
#define ASMJIT_X86_X86LOGGING_P_H_INCLUDED

#include "../core/api-config.h"
#ifndef ASMJIT_NO_LOGGING

#include "../core/logging.h"
#include "../core/string.h"
#include "../x86/x86globals.h"

ASMJIT_BEGIN_SUB_NAMESPACE(x86)

//! \addtogroup asmjit_x86
//! \{

// ============================================================================
// [asmjit::x86::LoggingInternal]
// ============================================================================

namespace LoggingInternal {
  Error formatRegister(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    uint32_t regType,
    uint32_t regId) noexcept;

  Error formatOperand(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const Operand_& op) noexcept;

  Error formatInstruction(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept;
};

//! \}

ASMJIT_END_SUB_NAMESPACE

#endif // !ASMJIT_NO_LOGGING
#endif // ASMJIT_X86_X86LOGGING_P_H_INCLUDED
