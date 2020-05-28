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

#ifndef ASMJIT_CORE_H_INCLUDED
#define ASMJIT_CORE_H_INCLUDED

//! \defgroup asmjit_core Core
//! \brief Core API.
//!
//! API that provides classes and functions not specific to any architecture.

//! \defgroup asmjit_builder Builder
//! \brief Builder API.
//!
//! Both Builder and Compiler are emitters that emit everything to a representation
//! that allows further processing. The code stored in such representation is
//! completely safe to be patched, simplified, reordered, obfuscated, removed,
//! injected, analyzed, or processed some other way. Each instruction, label,
//! directive, or other building block is stored as \ref BaseNode (or derived
//! class like \ref InstNode or \ref LabelNode) and contains all the information
//! necessary to pass that node later to the Assembler.

//! \defgroup asmjit_compiler Compiler
//! \brief Compiler API.
//!
//! Compiler tool is built on top of a \ref asmjit_builder API and adds register
//! allocation and support for defining and calling functions into it. At the
//! moment it's the easiest way to generate some code as most architecture and
//! OS specific stuff is properly abstracted, however, abstractions also mean
//! that not everything is possible with the Compiler.

//! \defgroup asmjit_func Function
//! \brief Function API.

//! \defgroup asmjit_jit JIT
//! \brief JIT API and Virtual Memory Management.

//! \defgroup asmjit_zone Zone
//! \brief Zone allocator and zone allocated containers.

//! \defgroup asmjit_support Support
//! \brief Support API.

//! \cond INTERNAL
//! \defgroup asmjit_ra RA
//! \brief Register allocator internals.
//! \endcond

#include "./core/globals.h"

#include "./core/arch.h"
#include "./core/assembler.h"
#include "./core/builder.h"
#include "./core/callconv.h"
#include "./core/codeholder.h"
#include "./core/compiler.h"
#include "./core/constpool.h"
#include "./core/cpuinfo.h"
#include "./core/datatypes.h"
#include "./core/emitter.h"
#include "./core/features.h"
#include "./core/func.h"
#include "./core/inst.h"
#include "./core/jitallocator.h"
#include "./core/jitruntime.h"
#include "./core/logging.h"
#include "./core/operand.h"
#include "./core/osutils.h"
#include "./core/string.h"
#include "./core/support.h"
#include "./core/target.h"
#include "./core/type.h"
#include "./core/virtmem.h"
#include "./core/zone.h"
#include "./core/zonehash.h"
#include "./core/zonelist.h"
#include "./core/zonetree.h"
#include "./core/zonestack.h"
#include "./core/zonestring.h"
#include "./core/zonevector.h"

#endif // ASMJIT_CORE_H_INCLUDED
