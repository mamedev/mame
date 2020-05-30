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

#ifndef ASMJIT_CORE_FEATURES_H_INCLUDED
#define ASMJIT_CORE_FEATURES_H_INCLUDED

#include "../core/globals.h"
#include "../core/support.h"

ASMJIT_BEGIN_NAMESPACE

//! \addtogroup asmjit_core
//! \{

// ============================================================================
// [asmjit::BaseFeatures]
// ============================================================================

class BaseFeatures {
public:
  typedef Support::BitWord BitWord;

  enum : uint32_t {
    kMaxFeatures = 128,
    kNumBitWords = kMaxFeatures / Support::kBitWordSizeInBits
  };

  BitWord _bits[kNumBitWords];

  //! \name Construction & Destruction
  //! \{

  inline BaseFeatures() noexcept { reset(); }
  inline BaseFeatures(const BaseFeatures& other) noexcept = default;
  inline explicit BaseFeatures(Globals::NoInit_) noexcept {}

  inline void reset() noexcept {
    for (size_t i = 0; i < kNumBitWords; i++)
      _bits[i] = 0;
  }

  //! \}

  //! \name Overloaded Operators
  //! \{

  inline BaseFeatures& operator=(const BaseFeatures& other) noexcept = default;

  inline bool operator==(const BaseFeatures& other) noexcept { return  eq(other); }
  inline bool operator!=(const BaseFeatures& other) noexcept { return !eq(other); }

  //! \}

  //! \name Cast
  //! \{

  template<typename T>
  inline T& as() noexcept { return static_cast<T&>(*this); }

  template<typename T>
  inline const T& as() const noexcept { return static_cast<const T&>(*this); }

  //! \}

  //! \name Accessors
  //! \{

  //! Returns all features as `BitWord` array.
  inline BitWord* bits() noexcept { return _bits; }
  //! Returns all features as `BitWord` array (const).
  inline const BitWord* bits() const noexcept { return _bits; }

  //! Tests whether the feature `featureId` is present.
  inline bool has(uint32_t featureId) const noexcept {
    ASMJIT_ASSERT(featureId < kMaxFeatures);

    uint32_t idx = featureId / Support::kBitWordSizeInBits;
    uint32_t bit = featureId % Support::kBitWordSizeInBits;

    return bool((_bits[idx] >> bit) & 0x1);
  }

  //! Tests whether all features as defined by `other` are present.
  inline bool hasAll(const BaseFeatures& other) const noexcept {
    for (uint32_t i = 0; i < kNumBitWords; i++)
      if ((_bits[i] & other._bits[i]) != other._bits[i])
        return false;
    return true;
  }

  //! \}

  //! \name Utilities
  //! \{

  //! Adds the given CPU `featureId` to the list of features.
  inline void add(uint32_t featureId) noexcept {
    ASMJIT_ASSERT(featureId < kMaxFeatures);

    uint32_t idx = featureId / Support::kBitWordSizeInBits;
    uint32_t bit = featureId % Support::kBitWordSizeInBits;

    _bits[idx] |= BitWord(1) << bit;
  }

  template<typename... Args>
  inline void add(uint32_t featureId, Args... otherIds) noexcept {
    add(featureId);
    add(otherIds...);
  }

  //! Removes the given CPU `featureId` from the list of features.
  inline void remove(uint32_t featureId) noexcept {
    ASMJIT_ASSERT(featureId < kMaxFeatures);

    uint32_t idx = featureId / Support::kBitWordSizeInBits;
    uint32_t bit = featureId % Support::kBitWordSizeInBits;

    _bits[idx] &= ~(BitWord(1) << bit);
  }

  template<typename... Args>
  inline void remove(uint32_t featureId, Args... otherIds) noexcept {
    remove(featureId);
    remove(otherIds...);
  }

  inline bool eq(const BaseFeatures& other) const noexcept {
    for (size_t i = 0; i < kNumBitWords; i++)
      if (_bits[i] != other._bits[i])
        return false;
    return true;
  }

  //! \}
};

//! \}

ASMJIT_END_NAMESPACE

#endif // ASMJIT_CORE_FEATURES_H_INCLUDED
