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

#ifndef ASMJIT_CORE_LOGGING_H_INCLUDED
#define ASMJIT_CORE_LOGGING_H_INCLUDED

#include "../core/inst.h"
#include "../core/string.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

#ifndef ASMJIT_NO_LOGGING

// ============================================================================
// [Forward Declarations]
// ============================================================================

class BaseEmitter;
class BaseReg;
class Logger;
struct Operand_;

#ifndef ASMJIT_NO_BUILDER
class BaseBuilder;
class BaseNode;
#endif

// ============================================================================
// [asmjit::FormatOptions]
// ============================================================================

class FormatOptions {
public:
  uint32_t _flags;
  uint8_t _indentation[4];

  enum Flags : uint32_t {
    //! Show also binary form of each logged instruction (assembler).
    kFlagMachineCode = 0x00000001u,
    //! Show a text explanation of some immediate values.
    kFlagExplainImms = 0x00000002u,
    //! Use hexadecimal notation of immediate values.
    kFlagHexImms = 0x00000004u,
    //! Use hexadecimal notation of address offsets.
    kFlagHexOffsets = 0x00000008u,
    //! Show casts between virtual register types (compiler).
    kFlagRegCasts = 0x00000010u,
    //! Show positions associated with nodes (compiler).
    kFlagPositions = 0x00000020u,
    //! Annotate nodes that are lowered by passes.
    kFlagAnnotations = 0x00000040u,

    // TODO: These must go, keep this only for formatting.
    //! Show an additional output from passes.
    kFlagDebugPasses = 0x00000080u,
    //! Show an additional output from RA.
    kFlagDebugRA = 0x00000100u
  };

  enum IndentationType : uint32_t {
    //! Indentation used for instructions and directives.
    kIndentationCode = 0u,
    //! Indentation used for labels and function nodes.
    kIndentationLabel = 1u,
    //! Indentation used for comments (not inline comments).
    kIndentationComment = 2u,
    kIndentationReserved = 3u
  };

  //! \name Construction & Destruction
  //! \{

  constexpr FormatOptions() noexcept
    : _flags(0),
      _indentation { 0, 0, 0, 0 } {}

  constexpr FormatOptions(const FormatOptions& other) noexcept = default;
  inline FormatOptions& operator=(const FormatOptions& other) noexcept = default;

  inline void reset() noexcept {
    _flags = 0;
    _indentation[0] = 0;
    _indentation[1] = 0;
    _indentation[2] = 0;
    _indentation[3] = 0;
  }

  //! \}

  //! \name Accessors
  //! \{

  constexpr uint32_t flags() const noexcept { return _flags; }
  constexpr bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline void setFlags(uint32_t flags) noexcept { _flags = flags; }
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
  inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

  constexpr uint8_t indentation(uint32_t type) const noexcept { return _indentation[type]; }
  inline void setIndentation(uint32_t type, uint32_t n) noexcept { _indentation[type] = uint8_t(n); }
  inline void resetIndentation(uint32_t type) noexcept { _indentation[type] = uint8_t(0); }

  //! \}
};

// ============================================================================
// [asmjit::Logger]
// ============================================================================

//! Abstract logging interface and helpers.
//!
//! This class can be inherited and reimplemented to fit into your logging
//! subsystem. When reimplementing use `Logger::_log()` method to log into
//! a custom stream.
//!
//! There are two `Logger` implementations offered by AsmJit:
//!   - `FileLogger` - allows to log into `FILE*`.
//!   - `StringLogger` - logs into a `String`.
class ASMJIT_VIRTAPI Logger {
public:
  ASMJIT_BASE_CLASS(Logger)
  ASMJIT_NONCOPYABLE(Logger)

  //! Format options.
  FormatOptions _options;

  //! \name Construction & Destruction
  //! \{

  //! Creates a `Logger` instance.
  ASMJIT_API Logger() noexcept;
  //! Destroys the `Logger` instance.
  ASMJIT_API virtual ~Logger() noexcept;

  //! \}

  //! \name Format Options
  //! \{

  inline FormatOptions& options() noexcept { return _options; }
  inline const FormatOptions& options() const noexcept { return _options; }

  inline uint32_t flags() const noexcept { return _options.flags(); }
  inline bool hasFlag(uint32_t flag) const noexcept { return _options.hasFlag(flag); }
  inline void setFlags(uint32_t flags) noexcept { _options.setFlags(flags); }
  inline void addFlags(uint32_t flags) noexcept { _options.addFlags(flags); }
  inline void clearFlags(uint32_t flags) noexcept { _options.clearFlags(flags); }

  inline uint32_t indentation(uint32_t type) const noexcept { return _options.indentation(type); }
  inline void setIndentation(uint32_t type, uint32_t n) noexcept { _options.setIndentation(type, n); }
  inline void resetIndentation(uint32_t type) noexcept { _options.resetIndentation(type); }

  //! \}

  //! \name Logging Interface
  //! \{

  //! Logs `str` - must be reimplemented.
  virtual Error _log(const char* data, size_t size) noexcept = 0;

  //! Logs string `str`, which is either null terminated or having size `size`.
  inline Error log(const char* data, size_t size = SIZE_MAX) noexcept { return _log(data, size); }
  //! Logs content of a string `str`.
  inline Error log(const String& str) noexcept { return _log(str.data(), str.size()); }

  //! Formats the message by using `snprintf()` and then sends the result
  //! to `log()`.
  ASMJIT_API Error logf(const char* fmt, ...) noexcept;

  //! Formats the message by using `vsnprintf()` and then sends the result
  //! to `log()`.
  ASMJIT_API Error logv(const char* fmt, va_list ap) noexcept;

  //! Logs binary data.
  ASMJIT_API Error logBinary(const void* data, size_t size) noexcept;

  //! \}
};

// ============================================================================
// [asmjit::FileLogger]
// ============================================================================

//! Logger that can log to a `FILE*`.
class ASMJIT_VIRTAPI FileLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(FileLogger)

  FILE* _file;

  //! \name Construction & Destruction
  //! \{

  //! Creates a new `FileLogger` that logs to `FILE*`.
  ASMJIT_API FileLogger(FILE* file = nullptr) noexcept;
  //! Destroys the `FileLogger`.
  ASMJIT_API virtual ~FileLogger() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the logging output stream or null if the logger has no output
  //! stream.
  inline FILE* file() const noexcept { return _file; }

  //! Sets the logging output stream to `stream` or null.
  //!
  //! \note If the `file` is null the logging will be disabled. When a logger
  //! is attached to `CodeHolder` or any emitter the logging API will always
  //! be called regardless of the output file. This means that if you really
  //! want to disable logging at emitter level you must not attach a logger
  //! to it.
  inline void setFile(FILE* file) noexcept { _file = file; }

  //! \}

  ASMJIT_API Error _log(const char* data, size_t size = SIZE_MAX) noexcept override;
};

// ============================================================================
// [asmjit::StringLogger]
// ============================================================================

//! Logger that stores everything in an internal string buffer.
class ASMJIT_VIRTAPI StringLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(StringLogger)

  //! Logger data as string.
  String _content;

  //! \name Construction & Destruction
  //! \{

  //! Create new `StringLogger`.
  ASMJIT_API StringLogger() noexcept;
  //! Destroys the `StringLogger`.
  ASMJIT_API virtual ~StringLogger() noexcept;

  //! \}

  //! \name Logger Data Accessors
  //! \{

  //! Returns aggregated logger data as `char*` pointer.
  //!
  //! The pointer is owned by `StringLogger`, it can't be modified or freed.
  inline const char* data() const noexcept { return _content.data(); }
  //! Returns size of the data returned by `data()`.
  inline size_t dataSize() const noexcept { return _content.size(); }

  //! \}

  //! \name Logger Data Manipulation
  //! \{

  //! Clears the accumulated logger data.
  inline void clear() noexcept { _content.clear(); }

  //! \}

  ASMJIT_API Error _log(const char* data, size_t size = SIZE_MAX) noexcept override;
};

// ============================================================================
// [asmjit::Logging]
// ============================================================================

struct Logging {
  ASMJIT_API static Error formatRegister(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    uint32_t regType,
    uint32_t regId) noexcept;

  ASMJIT_API static Error formatLabel(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t labelId) noexcept;

  ASMJIT_API static Error formatOperand(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const Operand_& op) noexcept;

  ASMJIT_API static Error formatInstruction(
    String& sb,
    uint32_t flags,
    const BaseEmitter* emitter,
    uint32_t archId,
    const BaseInst& inst, const Operand_* operands, uint32_t opCount) noexcept;

  ASMJIT_API static Error formatTypeId(
    String& sb,
    uint32_t typeId) noexcept;

#ifndef ASMJIT_NO_BUILDER
  ASMJIT_API static Error formatNode(
    String& sb,
    uint32_t flags,
    const BaseBuilder* cb,
    const BaseNode* node_) noexcept;
#endif

  // Only used by AsmJit internals, not available to users.
#ifdef ASMJIT_EXPORTS
  enum {
    // Has to be big to be able to hold all metadata compiler can assign to a
    // single instruction.
    kMaxInstLineSize = 44,
    kMaxBinarySize = 26
  };

  static Error formatLine(
    String& sb,
    const uint8_t* binData, size_t binSize, size_t dispSize, size_t immSize, const char* comment) noexcept;
#endif
};
#endif

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_LOGGER_H_INCLUDED
