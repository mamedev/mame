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

#ifndef ASMJIT_CORE_TARGET_H_INCLUDED
#define ASMJIT_CORE_TARGET_H_INCLUDED

#include "../core/arch.h"
#include "../core/func.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::CodeInfo]
// ============================================================================

//! Basic information about a code (or target). It describes its architecture,
//! code generation mode (or optimization level), and base address.
class CodeInfo {
public:
  //!< Architecture information.
  ArchInfo _archInfo;
  //! Natural stack alignment (ARCH+OS).
  uint8_t _stackAlignment;
  //! Default CDECL calling convention.
  uint8_t _cdeclCallConv;
  //! Default STDCALL calling convention.
  uint8_t _stdCallConv;
  //! Default FASTCALL calling convention.
  uint8_t _fastCallConv;
  //! Base address.
  uint64_t _baseAddress;

  //! \name Construction & Destruction
  //! \{

  inline CodeInfo() noexcept
    : _archInfo(),
      _stackAlignment(0),
      _cdeclCallConv(CallConv::kIdNone),
      _stdCallConv(CallConv::kIdNone),
      _fastCallConv(CallConv::kIdNone),
      _baseAddress(Globals::kNoBaseAddress) {}

  inline explicit CodeInfo(uint32_t archId, uint32_t archMode = 0, uint64_t baseAddress = Globals::kNoBaseAddress) noexcept
    : _archInfo(archId, archMode),
      _stackAlignment(0),
      _cdeclCallConv(CallConv::kIdNone),
      _stdCallConv(CallConv::kIdNone),
      _fastCallConv(CallConv::kIdNone),
      _baseAddress(baseAddress) {}

  inline CodeInfo(const CodeInfo& other) noexcept { init(other); }

  inline bool isInitialized() const noexcept {
    return _archInfo.archId() != ArchInfo::kIdNone;
  }

  inline void init(const CodeInfo& other) noexcept {
    *this = other;
  }

  inline void init(uint32_t archId, uint32_t archMode = 0, uint64_t baseAddress = Globals::kNoBaseAddress) noexcept {
    _archInfo.init(archId, archMode);
    _stackAlignment = 0;
    _cdeclCallConv = CallConv::kIdNone;
    _stdCallConv = CallConv::kIdNone;
    _fastCallConv = CallConv::kIdNone;
    _baseAddress = baseAddress;
  }

  inline void reset() noexcept {
    _archInfo.reset();
    _stackAlignment = 0;
    _cdeclCallConv = CallConv::kIdNone;
    _stdCallConv = CallConv::kIdNone;
    _fastCallConv = CallConv::kIdNone;
    _baseAddress = Globals::kNoBaseAddress;
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  inline CodeInfo& operator=(const CodeInfo& other) noexcept = default;

  inline bool operator==(const CodeInfo& other) const noexcept { return ::memcmp(this, &other, sizeof(*this)) == 0; }
  inline bool operator!=(const CodeInfo& other) const noexcept { return ::memcmp(this, &other, sizeof(*this)) != 0; }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns the target architecture information, see `ArchInfo`.
  inline const ArchInfo& archInfo() const noexcept { return _archInfo; }

  //! Returns the target architecture id, see `ArchInfo::Id`.
  inline uint32_t archId() const noexcept { return _archInfo.archId(); }
  //! Returns the target architecture sub-type, see `ArchInfo::SubId`.
  inline uint32_t archSubId() const noexcept { return _archInfo.archSubId(); }
  //! Returns the native size of the target's architecture GP register.
  inline uint32_t gpSize() const noexcept { return _archInfo.gpSize(); }
  //! Returns the number of GP registers of the target's architecture.
  inline uint32_t gpCount() const noexcept { return _archInfo.gpCount(); }

  //! Returns a natural stack alignment that must be honored (or 0 if not known).
  inline uint32_t stackAlignment() const noexcept { return _stackAlignment; }
  //! Sets a natural stack alignment that must be honored.
  inline void setStackAlignment(uint32_t sa) noexcept { _stackAlignment = uint8_t(sa); }

  inline uint32_t cdeclCallConv() const noexcept { return _cdeclCallConv; }
  inline void setCdeclCallConv(uint32_t cc) noexcept { _cdeclCallConv = uint8_t(cc); }

  inline uint32_t stdCallConv() const noexcept { return _stdCallConv; }
  inline void setStdCallConv(uint32_t cc) noexcept { _stdCallConv = uint8_t(cc); }

  inline uint32_t fastCallConv() const noexcept { return _fastCallConv; }
  inline void setFastCallConv(uint32_t cc) noexcept { _fastCallConv = uint8_t(cc); }

  inline bool hasBaseAddress() const noexcept { return _baseAddress != Globals::kNoBaseAddress; }
  inline uint64_t baseAddress() const noexcept { return _baseAddress; }
  inline void setBaseAddress(uint64_t p) noexcept { _baseAddress = p; }
  inline void resetBaseAddress() noexcept { _baseAddress = Globals::kNoBaseAddress; }

  //! \}
};

// ============================================================================
// [asmjit::Target]
// ============================================================================

//! Target is an abstract class that describes a machine code target.
class ASMJIT_VIRTAPI Target {
public:
  ASMJIT_BASE_CLASS(Target)
  ASMJIT_NONCOPYABLE(Target)

  //! Tartget type, see `TargetType`.
  uint8_t _targetType;
  //! Reserved for future use.
  uint8_t _reserved[7];
  //! Basic information about the Runtime's code.
  CodeInfo _codeInfo;

  enum TargetType : uint32_t {
    //! Uninitialized target or unknown target type.
    kTargetNone = 0,
    //! JIT target type, see `JitRuntime`.
    kTargetJit = 1
  };

  //! \name Construction & Destruction
  //! \{

  //! Creates a `Target` instance.
  ASMJIT_API Target() noexcept;
  //! Destroys the `Target` instance.
  ASMJIT_API virtual ~Target() noexcept;

  //! \}

  //! \name Accessors
  //! \{

  //! Returns CodeInfo of this target.
  //!
  //! CodeInfo can be used to setup a CodeHolder in case you plan to generate a
  //! code compatible and executable by this Runtime.
  inline const CodeInfo& codeInfo() const noexcept { return _codeInfo; }

  //! Returns the target architecture id, see `ArchInfo::Id`.
  inline uint32_t archId() const noexcept { return _codeInfo.archId(); }
  //! Returns the target architecture sub-id, see `ArchInfo::SubId`.
  inline uint32_t archSubId() const noexcept { return _codeInfo.archSubId(); }

  //! Returns the target type, see `TargetType`.
  inline uint32_t targetType() const noexcept { return _targetType; }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_TARGET_H_INCLUDED
