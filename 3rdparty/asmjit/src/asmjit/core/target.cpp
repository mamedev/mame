// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include "../core/api-build_p.h"
#include "../core/target.h"

ASMJIT_BEGIN_NAMESPACE

Target::Target() noexcept
  : _environment{},
    _cpu_features{},
    _cpu_hints{} {}
Target::~Target() noexcept {}

ASMJIT_END_NAMESPACE
