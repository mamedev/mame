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

#ifndef ASMJIT_CORE_CODEHOLDER_H_INCLUDED
#define ASMJIT_CORE_CODEHOLDER_H_INCLUDED

#include "../core/arch.h"
#include "../core/datatypes.h"
#include "../core/operand.h"
#include "../core/string.h"
#include "../core/support.h"
#include "../core/target.h"
#include "../core/zone.h"
#include "../core/zonehash.h"
#include "../core/zonestring.h"
#include "../core/zonetree.h"
#include "../core/zonevector.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [Forward Declarations]
// ============================================================================

class BaseEmitter;
class CodeHolder;
class LabelEntry;
class Logger;

// ============================================================================
// [asmjit::AlignMode]
// ============================================================================

//! Align mode.
enum AlignMode : uint32_t {
  kAlignCode  = 0,                       //!< Align executable code.
  kAlignData  = 1,                       //!< Align non-executable code.
  kAlignZero  = 2,                       //!< Align by a sequence of zeros.
  kAlignCount = 3                        //!< Count of alignment modes.
};

// ============================================================================
// [asmjit::ErrorHandler]
// ============================================================================

//! Error handler can be used to override the default behavior of error handling
//! available to all classes that inherit `BaseEmitter`.
//!
//! Override `ErrorHandler::handleError()` to implement your own error handler.
class ASMJIT_VIRTAPI ErrorHandler {
public:
  ASMJIT_BASE_CLASS(ErrorHandler)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! Creates a new `ErrorHandler` instance.
  ASMJIT_API ErrorHandler() noexcept;
  //! Destroys the `ErrorHandler` instance.
  ASMJIT_API virtual ~ErrorHandler() noexcept;

  // --------------------------------------------------------------------------
  // [Handle Error]
  // --------------------------------------------------------------------------

  //! Error handler (must be reimplemented).
  //!
  //! Error handler is called after an error happened and before it's propagated
  //! to the caller. There are multiple ways how the error handler can be used:
  //!
  //! 1. User-based error handling without throwing exception or using C's
  //!    `longjmp()`. This is for users that don't use exceptions and want
  //!    customized error handling.
  //!
  //! 2. Throwing an exception. AsmJit doesn't use exceptions and is completely
  //!    exception-safe, but you can throw exception from your error handler if
  //!    this way is the preferred way of handling errors in your project.
  //!
  //! 3. Using plain old C's `setjmp()` and `longjmp()`. Asmjit always puts
  //!    `BaseEmitter` to a consistent state before calling `handleError()`
  //!    so `longjmp()` can be used without any issues to cancel the code
  //!    generation if an error occurred. There is no difference between
  //!    exceptions and `longjmp()` from AsmJit's perspective, however,
  //!    never jump outside of `CodeHolder` and `BaseEmitter` scope as you
  //!    would leak memory.
  virtual void handleError(Error err, const char* message, BaseEmitter* origin) = 0;
};

// ============================================================================
// [asmjit::CodeBuffer]
// ============================================================================

//! Code or data buffer.
struct CodeBuffer {
  //! The content of the buffer (data).
  uint8_t* _data;
  //! Number of bytes of `data` used.
  size_t _size;
  //! Buffer capacity (in bytes).
  size_t _capacity;
  //! Buffer flags.
  uint32_t _flags;

  enum Flags : uint32_t {
    //! Buffer is external (not allocated by asmjit).
    kFlagIsExternal = 0x00000001u,
    //! Buffer is fixed (cannot be reallocated).
    kFlagIsFixed = 0x00000002u
  };

  //! \name Overloaded Operators
  //! \{

  inline uint8_t& operator[](size_t index) noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }

  inline const uint8_t& operator[](size_t index) const noexcept {
    ASMJIT_ASSERT(index < _size);
    return _data[index];
  }

  //! \}

  //! \name Accessors
  //! \{

  inline uint32_t flags() const noexcept { return _flags; }
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }

  inline bool isAllocated() const noexcept { return _data != nullptr; }
  inline bool isFixed() const noexcept { return hasFlag(kFlagIsFixed); }
  inline bool isExternal() const noexcept { return hasFlag(kFlagIsExternal); }

  inline uint8_t* data() noexcept { return _data; }
  inline const uint8_t* data() const noexcept { return _data; }

  inline bool empty() const noexcept { return !_size; }
  inline size_t size() const noexcept { return _size; }
  inline size_t capacity() const noexcept { return _capacity; }

  //! \}

  //! \name Iterators
  //! \{

  inline uint8_t* begin() noexcept { return _data; }
  inline const uint8_t* begin() const noexcept { return _data; }

  inline uint8_t* end() noexcept { return _data + _size; }
  inline const uint8_t* end() const noexcept { return _data + _size; }

  //! \}
};

// ============================================================================
// [asmjit::Section]
// ============================================================================

//! Section entry.
class Section {
public:
  //! Section id.
  uint32_t _id;
  //! Section flags.
  uint32_t _flags;
  //! Section alignment requirements (0 if no requirements).
  uint32_t _alignment;
  //! Reserved for future use (padding).
  uint32_t _reserved;
  //! Offset of this section from base-address.
  uint64_t _offset;
  //! Virtual size of the section (zero initialized sections).
  uint64_t _virtualSize;
  //! Section name (max 35 characters, PE allows max 8).
  FixedString<Globals::kMaxSectionNameSize + 1> _name;
  //! Code or data buffer.
  CodeBuffer _buffer;

  //! Section flags.
  enum Flags : uint32_t {
    kFlagExec        = 0x00000001u,      //!< Executable (.text sections).
    kFlagConst       = 0x00000002u,      //!< Read-only (.text and .data sections).
    kFlagZero        = 0x00000004u,      //!< Zero initialized by the loader (BSS).
    kFlagInfo        = 0x00000008u,      //!< Info / comment flag.
    kFlagImplicit    = 0x80000000u       //!< Section created implicitly and can be deleted by `Target`.
  };

  //! \name Accessors
  //! \{

  inline uint32_t id() const noexcept { return _id; }
  inline const char* name() const noexcept { return _name.str; }

  inline uint8_t* data() noexcept { return _buffer.data(); }
  inline const uint8_t* data() const noexcept { return _buffer.data(); }

  inline uint32_t flags() const noexcept { return _flags; }
  inline bool hasFlag(uint32_t flag) const noexcept { return (_flags & flag) != 0; }
  inline void addFlags(uint32_t flags) noexcept { _flags |= flags; }
  inline void clearFlags(uint32_t flags) noexcept { _flags &= ~flags; }

  inline uint32_t alignment() const noexcept { return _alignment; }
  inline void setAlignment(uint32_t alignment) noexcept { _alignment = alignment; }

  inline uint64_t offset() const noexcept { return _offset; }
  inline void setOffset(uint64_t offset) noexcept { _offset = offset; }

  //! Returns the virtual size of the section.
  //!
  //! Virtual size is initially zero and is never changed by AsmJit. It's normal
  //! if virtual size is smaller than size returned by `bufferSize()` as the buffer
  //! stores real data emitted by assemblers or appended by users.
  //!
  //! Use `realSize()` to get the real and final size of this section.
  inline uint64_t virtualSize() const noexcept { return _virtualSize; }
  //! Sets the virtual size of the section.
  inline void setVirtualSize(uint64_t virtualSize) noexcept { _virtualSize = virtualSize; }

  //! Returns the buffer size of the section.
  inline size_t bufferSize() const noexcept { return _buffer.size(); }
  //! Returns the real size of the section calculated from virtual and buffer sizes.
  inline uint64_t realSize() const noexcept { return Support::max<uint64_t>(virtualSize(), bufferSize()); }

  //! Returns the `CodeBuffer` used by this section.
  inline CodeBuffer& buffer() noexcept { return _buffer; }
  //! Returns the `CodeBuffer` used by this section (const).
  inline const CodeBuffer& buffer() const noexcept { return _buffer; }

  //! \}
};

// ============================================================================
// [asmjit::LabelLink]
// ============================================================================

//! Data structure used to link either unbound labels or cross-section links.
struct LabelLink {
  //! Next link (single-linked list).
  LabelLink* next;
  //! Section id where the label is bound.
  uint32_t sectionId;
  //! Relocation id or Globals::kInvalidId.
  uint32_t relocId;
  //! Label offset relative to the start of the section.
  size_t offset;
  //! Inlined rel8/rel32.
  intptr_t rel;
};

// ============================================================================
// [asmjit::Expression]
// ============================================================================

struct Expression {
  enum OpType : uint8_t {
    kOpAdd = 0,
    kOpSub = 1,
    kOpMul = 2,
    kOpSll = 3,
    kOpSrl = 4,
    kOpSra = 5
  };

  enum ValueType : uint8_t {
    kValueNone = 0,
    kValueConstant = 1,
    kValueLabel = 2,
    kValueExpression = 3
  };

  union Value {
    uint64_t constant;
    Expression* expression;
    LabelEntry* label;
  };

  uint8_t opType;
  uint8_t valueType[2];
  uint8_t reserved[5];
  Value value[2];

  inline void reset() noexcept { memset(this, 0, sizeof(*this)); }

  inline void setValueAsConstant(size_t index, uint64_t constant) noexcept {
    valueType[index] = kValueConstant;
    value[index].constant = constant;
  }

  inline void setValueAsLabel(size_t index, LabelEntry* label) noexcept {
    valueType[index] = kValueLabel;
    value[index].label = label;
  }

  inline void setValueAsExpression(size_t index, Expression* expression) noexcept {
    valueType[index] = kValueLabel;
    value[index].expression = expression;
  }
};

// ============================================================================
// [asmjit::LabelEntry]
// ============================================================================

//! Label entry.
//!
//! Contains the following properties:
//!   * Label id - This is the only thing that is set to the `Label` operand.
//!   * Label name - Optional, used mostly to create executables and libraries.
//!   * Label type - Type of the label, default `Label::kTypeAnonymous`.
//!   * Label parent id - Derived from many assemblers that allow to define a
//!       local label that falls under a global label. This allows to define
//!       many labels of the same name that have different parent (global) label.
//!   * Offset - offset of the label bound by `Assembler`.
//!   * Links - single-linked list that contains locations of code that has
//!       to be patched when the label gets bound. Every use of unbound label
//!       adds one link to `_links` list.
//!   * HVal - Hash value of label's name and optionally parentId.
//!   * HashNext - Hash-table implementation detail.
class LabelEntry : public ZoneHashNode {
public:
  // Let's round the size of `LabelEntry` to 64 bytes (as `ZoneAllocator` has
  // granularity of 32 bytes anyway). This gives `_name` the remaining space,
  // which is should be 16 bytes on 64-bit and 28 bytes on 32-bit architectures.
  static constexpr uint32_t kStaticNameSize =
    64 - (sizeof(ZoneHashNode) + 8 + sizeof(Section*) + sizeof(size_t) + sizeof(LabelLink*));

  //! Label type, see `Label::LabelType`.
  uint8_t _type;
  //! Must be zero.
  uint8_t _flags;
  //! Reserved.
  uint16_t _reserved16;
  //! Label parent id or zero.
  uint32_t _parentId;
  //! Label offset relative to the start of the `_section`.
  uint64_t _offset;
  //! Section where the label was bound.
  Section* _section;
  //! Label links.
  LabelLink* _links;
  //! Label name.
  ZoneString<kStaticNameSize> _name;

  //! \name Accessors
  //! \{

  // NOTE: Label id is stored in `_customData`, which is provided by ZoneHashNode
  // to fill a padding that a C++ compiler targeting 64-bit CPU will add to align
  // the structure to 64-bits.

  //! Returns label id.
  inline uint32_t id() const noexcept { return _customData; }
  //! Sets label id (internal, used only by `CodeHolder`).
  inline void _setId(uint32_t id) noexcept { _customData = id; }

  //! Returns label type, see `Label::LabelType`.
  inline uint32_t type() const noexcept { return _type; }
  //! Returns label flags, returns 0 at the moment.
  inline uint32_t flags() const noexcept { return _flags; }

  //! Tests whether the label has a parent label.
  inline bool hasParent() const noexcept { return _parentId != Globals::kInvalidId; }
  //! Returns label's parent id.
  inline uint32_t parentId() const noexcept { return _parentId; }

  //! Returns the section where the label was bound.
  //!
  //! If the label was not yet bound the return value is `nullptr`.
  inline Section* section() const noexcept { return _section; }

  //! Tests whether the label has name.
  inline bool hasName() const noexcept { return !_name.empty(); }

  //! Returns the label's name.
  //!
  //! \note Local labels will return their local name without their parent
  //! part, for example ".L1".
  inline const char* name() const noexcept { return _name.data(); }

  //! Returns size of label's name.
  //!
  //! \note Label name is always null terminated, so you can use `strlen()` to
  //! get it, however, it's also cached in `LabelEntry` itself, so if you want
  //! to know the size the fastest way is to call `LabelEntry::nameSize()`.
  inline uint32_t nameSize() const noexcept { return _name.size(); }

  //! Returns links associated with this label.
  inline LabelLink* links() const noexcept { return _links; }

  //! Tests whether the label is bound.
  inline bool isBound() const noexcept { return _section != nullptr; }
  //! Tests whether the label is bound to a the given `sectionId`.
  inline bool isBoundTo(Section* section) const noexcept { return _section == section; }

  //! Returns the label offset (only useful if the label is bound).
  inline uint64_t offset() const noexcept { return _offset; }

  //! Returns the hash-value of label's name and its parent label (if any).
  //!
  //! Label hash is calculated as `HASH(Name) ^ ParentId`. The hash function
  //! is implemented in `Support::hashString()` and `Support::hashRound()`.
  inline uint32_t hashCode() const noexcept { return _hashCode; }

  //! \}
};

// ============================================================================
// [asmjit::RelocEntry]
// ============================================================================

//! Relocation entry.
//!
//! We describe relocation data in the following way:
//!
//! ```
//! +- Start of the buffer                              +- End of the data
//! |                               |*PATCHED*|         |  or instruction
//! |xxxxxxxxxxxxxxxxxxxxxx|LeadSize|ValueSize|TrailSize|xxxxxxxxxxxxxxxxxxxx->
//!                        |
//!                        +- Source offset
//! ```
struct RelocEntry {
  //! Relocation id.
  uint32_t _id;
  //! Type of the relocation.
  uint8_t _relocType;
  //! Size of the relocation data/value (1, 2, 4 or 8 bytes).
  uint8_t _valueSize;
  //! Number of bytes after `_sourceOffset` to reach the value to be patched.
  uint8_t _leadingSize;
  //! Number of bytes after `_sourceOffset + _valueSize` to reach end of the
  //! instruction.
  uint8_t _trailingSize;
  //! Source section id.
  uint32_t _sourceSectionId;
  //! Target section id.
  uint32_t _targetSectionId;
  //! Source offset (relative to start of the section).
  uint64_t _sourceOffset;
  //! Payload (target offset, target address, expression, etc).
  uint64_t _payload;

  //! Relocation type.
  enum RelocType : uint32_t {
    //! None/deleted (no relocation).
    kTypeNone = 0,
    //! Expression evaluation, `_payload` is pointer to `Expression`.
    kTypeExpression = 1,
    //! Relocate absolute to absolute.
    kTypeAbsToAbs = 2,
    //! Relocate relative to absolute.
    kTypeRelToAbs = 3,
    //! Relocate absolute to relative.
    kTypeAbsToRel = 4,
    //! Relocate absolute to relative or use trampoline.
    kTypeX64AddressEntry = 5
  };

  //! \name Accessors
  //! \{

  inline uint32_t id() const noexcept { return _id; }

  inline uint32_t relocType() const noexcept { return _relocType; }
  inline uint32_t valueSize() const noexcept { return _valueSize; }

  inline uint32_t leadingSize() const noexcept { return _leadingSize; }
  inline uint32_t trailingSize() const noexcept { return _trailingSize; }

  inline uint32_t sourceSectionId() const noexcept { return _sourceSectionId; }
  inline uint32_t targetSectionId() const noexcept { return _targetSectionId; }

  inline uint64_t sourceOffset() const noexcept { return _sourceOffset; }
  inline uint64_t payload() const noexcept { return _payload; }

  Expression* payloadAsExpression() const noexcept {
    return reinterpret_cast<Expression*>(uintptr_t(_payload));
  }

  //! \}
};

// ============================================================================
// [asmjit::AddressTableEntry]
// ============================================================================

class AddressTableEntry : public ZoneTreeNodeT<AddressTableEntry> {
public:
  ASMJIT_NONCOPYABLE(AddressTableEntry)

  uint64_t _address;
  uint32_t _slot;

  //! \name Construction & Destruction
  //! \{

  inline explicit AddressTableEntry(uint64_t address) noexcept
    : _address(address),
      _slot(0xFFFFFFFFu) {}

  //! \}

  //! \name Accessors
  //! \{

  inline uint64_t address() const noexcept { return _address; }
  inline uint32_t slot() const noexcept { return _slot; }

  inline bool hasAssignedSlot() const noexcept { return _slot != 0xFFFFFFFFu; }

  inline bool operator<(const AddressTableEntry& other) const noexcept { return _address < other._address; }
  inline bool operator>(const AddressTableEntry& other) const noexcept { return _address > other._address; }

  inline bool operator<(uint64_t queryAddress) const noexcept { return _address < queryAddress; }
  inline bool operator>(uint64_t queryAddress) const noexcept { return _address > queryAddress; }

  //! \}
};

// ============================================================================
// [asmjit::CodeHolder]
// ============================================================================

//! Contains basic information about the target architecture plus its settings,
//! and holds code & data (including sections, labels, and relocation information).
//! CodeHolder can store both binary and intermediate representation of assembly,
//! which can be generated by `BaseAssembler` and/or `BaseBuilder`.
//!
//! \note `CodeHolder` has ability to attach an `ErrorHandler`, however, the
//! error handler is not triggered by `CodeHolder` itself, it's only used by
//! emitters attached to `CodeHolder`.
class CodeHolder {
public:
  ASMJIT_NONCOPYABLE(CodeHolder)

  //! Basic information about the code (architecture and other info).
  CodeInfo _codeInfo;
  //! Emitter options, propagated to all emitters when changed.
  uint32_t _emitterOptions;

  //! Attached `Logger`, used by all consumers.
  Logger* _logger;
  //! Attached `ErrorHandler`.
  ErrorHandler* _errorHandler;

  //! Code zone (used to allocate core structures).
  Zone _zone;
  //! Zone allocator, used to manage internal containers.
  ZoneAllocator _allocator;

  //! Attached code emitters.
  ZoneVector<BaseEmitter*> _emitters;
  //! Section entries.
  ZoneVector<Section*> _sections;
  //! Label entries.
  ZoneVector<LabelEntry*> _labelEntries;
  //! Relocation entries.
  ZoneVector<RelocEntry*> _relocations;
  //! Label name -> LabelEntry (only named labels).
  ZoneHash<LabelEntry> _namedLabels;

  //! Count of label links, which are not resolved.
  size_t _unresolvedLinkCount;
  //! Pointer to an address table section (or null if this section doesn't exist).
  Section* _addressTableSection;
  //! Address table entries.
  ZoneTree<AddressTableEntry> _addressTableEntries;

  //! \name Construction & Destruction
  //! \{

  //! Creates an uninitialized CodeHolder (you must init() it before it can be used).
  ASMJIT_API CodeHolder() noexcept;
  //! Destroys the CodeHolder.
  ASMJIT_API ~CodeHolder() noexcept;

  inline bool isInitialized() const noexcept { return _codeInfo.isInitialized(); }

  //! Initializes CodeHolder to hold code described by `codeInfo`.
  ASMJIT_API Error init(const CodeInfo& info) noexcept;
  //! Detaches all code-generators attached and resets the `CodeHolder`.
  ASMJIT_API void reset(uint32_t resetPolicy = Globals::kResetSoft) noexcept;

  //! \}

  //! \name Attach & Detach
  //! \{

  //! Attaches an emitter to this `CodeHolder`.
  ASMJIT_API Error attach(BaseEmitter* emitter) noexcept;
  //! Detaches an emitter from this `CodeHolder`.
  ASMJIT_API Error detach(BaseEmitter* emitter) noexcept;

  //! \}

  //! \name Allocators
  //! \{

  inline ZoneAllocator* allocator() const noexcept { return const_cast<ZoneAllocator*>(&_allocator); }

  //! \}

  //! \name Code Emitter
  //! \{

  inline const ZoneVector<BaseEmitter*>& emitters() const noexcept { return _emitters; }

  //! Returns global emitter options, internally propagated to all attached emitters.
  inline uint32_t emitterOptions() const noexcept { return _emitterOptions; }

  //! Enables the given global emitter `options` and propagates the resulting
  //! options to all attached emitters.
  ASMJIT_API void addEmitterOptions(uint32_t options) noexcept;

  //! Disables the given global emitter `options` and propagates the resulting
  //! options to all attached emitters.
  ASMJIT_API void clearEmitterOptions(uint32_t options) noexcept;

  //! \}

  //! \name Code & Architecture
  //! \{

  //! Returns the target architecture information, see `ArchInfo`.
  inline const ArchInfo& archInfo() const noexcept { return _codeInfo.archInfo(); }
  //! Returns the target code information, see `CodeInfo`.
  inline const CodeInfo& codeInfo() const noexcept { return _codeInfo; }

  //! Returns the target architecture id.
  inline uint32_t archId() const noexcept { return archInfo().archId(); }
  //! Returns the target architecture sub-id.
  inline uint32_t archSubId() const noexcept { return archInfo().archSubId(); }

  //! Tests whether a static base-address is set.
  inline bool hasBaseAddress() const noexcept { return _codeInfo.hasBaseAddress(); }
  //! Returns a static base-address (uint64_t).
  inline uint64_t baseAddress() const noexcept { return _codeInfo.baseAddress(); }

  //! \}

  //! \name Logging & Error Handling
  //! \{

  //! Returns the attached logger.
  inline Logger* logger() const noexcept { return _logger; }
  //! Attaches a `logger` to CodeHolder and propagates it to all attached emitters.
  ASMJIT_API void setLogger(Logger* logger) noexcept;
  //! Resets the logger to none.
  inline void resetLogger() noexcept { setLogger(nullptr); }

  //! Tests whether the global error handler is attached.
  inline bool hasErrorHandler() const noexcept { return _errorHandler != nullptr; }
  //! Returns the global error handler.
  inline ErrorHandler* errorHandler() const noexcept { return _errorHandler; }
  //! Sets the global error handler.
  inline void setErrorHandler(ErrorHandler* handler) noexcept { _errorHandler = handler; }
  //! Resets the global error handler to none.
  inline void resetErrorHandler() noexcept { setErrorHandler(nullptr); }

  //! \}

  //! \name Code Buffer
  //! \{

  ASMJIT_API Error growBuffer(CodeBuffer* cb, size_t n) noexcept;
  ASMJIT_API Error reserveBuffer(CodeBuffer* cb, size_t n) noexcept;

  //! \}

  //! \name Sections
  //! \{

  //! Returns an array of `Section*` records.
  inline const ZoneVector<Section*>& sections() const noexcept { return _sections; }
  //! Returns the number of sections.
  inline uint32_t sectionCount() const noexcept { return _sections.size(); }

  //! Tests whether the given `sectionId` is valid.
  inline bool isSectionValid(uint32_t sectionId) const noexcept { return sectionId < _sections.size(); }

  //! Creates a new section and return its pointer in `sectionOut`.
  //!
  //! Returns `Error`, does not report a possible error to `ErrorHandler`.
  ASMJIT_API Error newSection(Section** sectionOut, const char* name, size_t nameSize = SIZE_MAX, uint32_t flags = 0, uint32_t alignment = 1) noexcept;

  //! Returns a section entry of the given index.
  inline Section* sectionById(uint32_t sectionId) const noexcept { return _sections[sectionId]; }

  //! Returns section-id that matches the given `name`.
  //!
  //! If there is no such section `Section::kInvalidId` is returned.
  ASMJIT_API Section* sectionByName(const char* name, size_t nameSize = SIZE_MAX) const noexcept;

  //! Returns '.text' section (section that commonly represents code).
  //!
  //! \note Text section is always the first section in `CodeHolder::sections()` array.
  inline Section* textSection() const noexcept { return _sections[0]; }

  //! Tests whether '.addrtab' section exists.
  inline bool hasAddressTable() const noexcept { return _addressTableSection != nullptr; }

  //! Returns '.addrtab' section.
  //!
  //! This section is used exclusively by AsmJit to store absolute 64-bit
  //! addresses that cannot be encoded in instructions like 'jmp' or 'call'.
  inline Section* addressTableSection() const noexcept { return _addressTableSection; }

  //! Ensures that '.addrtab' section exists (creates it if it doesn't) and
  //! returns it. Can return `nullptr` on out of memory condition.
  ASMJIT_API Section* ensureAddressTableSection() noexcept;

  //! Used to add an address to an address table.
  //!
  //! This implicitly calls `ensureAddressTableSection()` and then creates
  //! `AddressTableEntry` that is inserted to `_addressTableEntries`. If the
  //! address already exists this operation does nothing as the same addresses
  //! use the same slot.
  //!
  //! This function should be considered internal as it's used by assemblers to
  //! insert an absolute address into the address table. Inserting address into
  //! address table without creating a particula relocation entry makes no sense.
  ASMJIT_API Error addAddressToAddressTable(uint64_t address) noexcept;

  //! \}

  //! \name Labels & Symbols
  //! \{

  //! Returns array of `LabelEntry*` records.
  inline const ZoneVector<LabelEntry*>& labelEntries() const noexcept { return _labelEntries; }

  //! Returns number of labels created.
  inline uint32_t labelCount() const noexcept { return _labelEntries.size(); }

  //! Tests whether the label having `id` is valid (i.e. created by `newLabelEntry()`).
  inline bool isLabelValid(uint32_t labelId) const noexcept {
    return labelId < _labelEntries.size();
  }

  //! Tests whether the `label` is valid (i.e. created by `newLabelEntry()`).
  inline bool isLabelValid(const Label& label) const noexcept {
    return label.id() < _labelEntries.size();
  }

  //! \overload
  inline bool isLabelBound(uint32_t labelId) const noexcept {
    return isLabelValid(labelId) && _labelEntries[labelId]->isBound();
  }

  //! Tests whether the `label` is already bound.
  //!
  //! Returns `false` if the `label` is not valid.
  inline bool isLabelBound(const Label& label) const noexcept {
    return isLabelBound(label.id());
  }

  //! Returns LabelEntry of the given label `id`.
  inline LabelEntry* labelEntry(uint32_t labelId) const noexcept {
    return isLabelValid(labelId) ? _labelEntries[labelId] : static_cast<LabelEntry*>(nullptr);
  }

  //! Returns LabelEntry of the given `label`.
  inline LabelEntry* labelEntry(const Label& label) const noexcept {
    return labelEntry(label.id());
  }

  //! Returns offset of a `Label` by its `labelId`.
  //!
  //! The offset returned is relative to the start of the section. Zero offset
  //! is returned for unbound labels, which is their initial offset value.
  inline uint64_t labelOffset(uint32_t labelId) const noexcept {
    ASMJIT_ASSERT(isLabelValid(labelId));
    return _labelEntries[labelId]->offset();
  }

  //! \overload
  inline uint64_t labelOffset(const Label& label) const noexcept {
    return labelOffset(label.id());
  }

  //! Returns offset of a label by it's `labelId` relative to the base offset.
  //!
  //! \remarks The offset of the section where the label is bound must be valid
  //! in order to use this function, otherwise the value returned will not be
  //! reliable.
  inline uint64_t labelOffsetFromBase(uint32_t labelId) const noexcept {
    ASMJIT_ASSERT(isLabelValid(labelId));
    const LabelEntry* le = _labelEntries[labelId];
    return (le->isBound() ? le->section()->offset() : uint64_t(0)) + le->offset();
  }

  //! \overload
  inline uint64_t labelOffsetFromBase(const Label& label) const noexcept {
    return labelOffsetFromBase(label.id());
  }

  //! Creates a new anonymous label and return its id in `idOut`.
  //!
  //! Returns `Error`, does not report error to `ErrorHandler`.
  ASMJIT_API Error newLabelEntry(LabelEntry** entryOut) noexcept;

  //! Creates a new named label label-type `type`.
  //!
  //! Returns `Error`, does not report a possible error to `ErrorHandler`.
  ASMJIT_API Error newNamedLabelEntry(LabelEntry** entryOut, const char* name, size_t nameSize, uint32_t type, uint32_t parentId = Globals::kInvalidId) noexcept;

  //! Returns a label id by name.
  ASMJIT_API uint32_t labelIdByName(const char* name, size_t nameSize = SIZE_MAX, uint32_t parentId = Globals::kInvalidId) noexcept;

  inline Label labelByName(const char* name, size_t nameSize = SIZE_MAX, uint32_t parentId = Globals::kInvalidId) noexcept {
    return Label(labelIdByName(name, nameSize, parentId));
  }

  //! Tests whether there are any unresolved label links.
  inline bool hasUnresolvedLinks() const noexcept { return _unresolvedLinkCount != 0; }
  //! Returns the number of label links, which are unresolved.
  inline size_t unresolvedLinkCount() const noexcept { return _unresolvedLinkCount; }

  //! Creates a new label-link used to store information about yet unbound labels.
  //!
  //! Returns `null` if the allocation failed.
  ASMJIT_API LabelLink* newLabelLink(LabelEntry* le, uint32_t sectionId, size_t offset, intptr_t rel) noexcept;

  //! Resolves cross-section links (`LabelLink`) associated with each label that
  //! was used as a destination in code of a different section. It's only useful
  //! to people that use multiple sections as it will do nothing if the code only
  //! contains a single section in which cross-section links are not possible.
  ASMJIT_API Error resolveUnresolvedLinks() noexcept;

  //! Binds a label to a given `sectionId` and `offset` (relative to start of the section).
  //!
  //! This function is generally used by `BaseAssembler::bind()` to do the heavy lifting.
  ASMJIT_API Error bindLabel(const Label& label, uint32_t sectionId, uint64_t offset) noexcept;

  //! \}

  //! \name Relocations
  //! \{

  //! Tests whether the code contains relocation entries.
  inline bool hasRelocEntries() const noexcept { return !_relocations.empty(); }
  //! Returns array of `RelocEntry*` records.
  inline const ZoneVector<RelocEntry*>& relocEntries() const noexcept { return _relocations; }

  //! Returns a RelocEntry of the given `id`.
  inline RelocEntry* relocEntry(uint32_t id) const noexcept { return _relocations[id]; }

  //! Creates a new relocation entry of type `relocType` and size `valueSize`.
  //!
  //! Additional fields can be set after the relocation entry was created.
  ASMJIT_API Error newRelocEntry(RelocEntry** dst, uint32_t relocType, uint32_t valueSize) noexcept;

  //! \}

  //! \name Utilities
  //! \{

  //! Flattens all sections by recalculating their offsets, starting at 0.
  //!
  //! \note This should never be called more than once.
  ASMJIT_API Error flatten() noexcept;

  //! Returns computed the size of code & data of all sections.
  //!
  //! \note All sections will be iterated over and the code size returned
  //! would represent the minimum code size of all combined sections after
  //! applying minimum alignment. Code size may decrease after calling
  //! `flatten()` and `relocateToBase()`.
  ASMJIT_API size_t codeSize() const noexcept;

  //! Relocates the code to the given `baseAddress`.
  //!
  //! \param baseAddress Absolute base address where the code will be relocated
  //! to. Please note that nothing is copied to such base address, it's just an
  //! absolute value used by the relocator to resolve all stored relocations.
  //!
  //! \note This should never be called more than once.
  ASMJIT_API Error relocateToBase(uint64_t baseAddress) noexcept;

  //! Options that can be used with \ref copySectionData().
  enum CopyOptions : uint32_t {
    //! If virtual size of the section is larger than the size of its buffer
    //! then all bytes between buffer size and virtual size will be zeroed.
    kCopyWithPadding = 0x1
  };

  //! Copies a single section into `dst`.
  ASMJIT_API Error copySectionData(void* dst, size_t dstSize, uint32_t sectionId, uint32_t options = 0) noexcept;

  //! Copies all sections into `dst`.
  //!
  //! This should only be used if the data was flattened and there are no gaps
  //! between the sections. The `dstSize` is always checked and the copy will
  //! never write anything outside the provided buffer.
  ASMJIT_API Error copyFlattenedData(void* dst, size_t dstSize, uint32_t options = 0) noexcept;

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_CODEHOLDER_H_INCLUDED
