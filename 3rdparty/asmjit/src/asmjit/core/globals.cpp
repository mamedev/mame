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

#include "../core/api-build_p.h"
#include "../core/globals.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

// ============================================================================
// [asmjit::DebugUtils]
// ============================================================================

ASMJIT_FAVOR_SIZE const char* DebugUtils::errorAsString(Error err) noexcept {
#ifndef ASMJIT_NO_TEXT
  static const char errorMessages[] =
    "Ok\0"
    "Out of memory\0"
    "Invalid argument\0"
    "Invalid state\0"
    "Invalid architecture\0"
    "Not initialized\0"
    "Already initialized\0"
    "Feature not enabled\0"
    "Too many handles or file descriptors\0"
    "Too large (code or memory request)\0"
    "No code generated\0"
    "Invalid directive\0"
    "Invalid label\0"
    "Too many labels\0"
    "Label already bound\0"
    "Label already defined\0"
    "Label name too long\0"
    "Invalid label name\0"
    "Invalid parent label\0"
    "Non-local label can't have parent\0"
    "Invalid section\0"
    "Too many sections\0"
    "Invalid section name\0"
    "Too many relocations\0"
    "Invalid relocation entry\0"
    "Relocation offset out of range\0"
    "Invalid assignment\0"
    "Invalid instruction\0"
    "Invalid register type\0"
    "Invalid register group\0"
    "Invalid register physical id\0"
    "Invalid register virtual id\0"
    "Invalid prefix combination\0"
    "Invalid lock prefix\0"
    "Invalid xacquire prefix\0"
    "Invalid xrelease prefix\0"
    "Invalid rep prefix\0"
    "Invalid rex prefix\0"
    "Invalid {...} register \0"
    "Invalid use of {k}\0"
    "Invalid use of {k}{z}\0"
    "Invalid broadcast {1tox}\0"
    "Invalid {er} or {sae} option\0"
    "Invalid address\0"
    "Invalid address index\0"
    "Invalid address scale\0"
    "Invalid use of 64-bit address or offset\0"
    "Invalid use of 64-bit address or offset that requires 32-bit zero-extension\0"
    "Invalid displacement\0"
    "Invalid segment\0"
    "Invalid immediate value\0"
    "Invalid operand size\0"
    "Ambiguous operand size\0"
    "Operand size mismatch\0"
    "Invalid option\0"
    "Option already defined\0"
    "Invalid type-info\0"
    "Invalid use of a low 8-bit GPB register\0"
    "Invalid use of a 64-bit GPQ register in 32-bit mode\0"
    "Invalid use of an 80-bit float\0"
    "Not consecutive registers\0"
    "No more physical registers\0"
    "Overlapped registers\0"
    "Overlapping register and arguments base-address register\0"
    "Unbound label cannot be evaluated by expression\0"
    "Arithmetic overflow during expression evaluation\0"
    "Unknown error\0";
  return Support::findPackedString(errorMessages, Support::min<Error>(err, kErrorCount));
#else
  DebugUtils::unused(err);
  static const char noMessage[] = "";
  return noMessage;
#endif
}

ASMJIT_FAVOR_SIZE void DebugUtils::debugOutput(const char* str) noexcept {
#if defined(_WIN32)
  ::OutputDebugStringA(str);
#else
  ::fputs(str, stderr);
#endif
}

ASMJIT_FAVOR_SIZE void DebugUtils::assertionFailed(const char* file, int line, const char* msg) noexcept {
  char str[1024];

  snprintf(str, 1024,
    "[asmjit] Assertion failed at %s (line %d):\n"
    "[asmjit] %s\n", file, line, msg);

  debugOutput(str);
  ::abort();
}

ASMJIT_END_NAMESPACE
