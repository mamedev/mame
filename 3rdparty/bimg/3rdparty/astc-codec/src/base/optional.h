// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ASTC_CODEC_BASE_OPTIONAL_H_
#define ASTC_CODEC_BASE_OPTIONAL_H_

#include "src/base/type_traits.h"

#include <cassert>
#include <initializer_list>
#include <type_traits>
#include <utility>

#include <cstddef>

// Optional<T> - a template class to store an optional value of type T.
//
// Usage examples:
//
// Initialization and construction:
//   Optional<Foo> foo;            // |foo| doesn't contain a value.
//   Optional<Foo> foo(Foo(10));   // |foo| contains a copy-constructed value.
//   Optional<Foo> foo2(foo);      // |foo2| contains a copy of |foo|'s value.
//   Optional<Foo> foo3(std::move(foo2));  // Guess what?
//
// Assignment:
//   Foo foo_value(0);
//   Optional<Foo> foo;   // |foo| is empty.
//   Optional<Foo> foo2;  // |foo2| is empty.
//   foo2 = foo;          // |foo2| is still empty.
//   foo = foo_value;     // set value of |foo| to a copy of |foo_value|
//   foo = std::move(foo_value);  // move |foo_value| into |foo|.
//   foo2 = foo;          // now |foo2| has a copy of |foo|'s value.
//   foo = kNullopt;      // unset |foo|, it has no value.
//
// Checking and accessing value:
//   if (foo) {
//      // |foo| has a value.
//      doStuff(*foo);      // |*foo| is the value inside |foo|.
//      foo->callMethod();  // Same as (*foo).callMethod().
//   } else {
//      // |foo| is empty.
//   }
//
//   foo.value()              // Same as *foo
//   foo.valueOr(<default>)   // Return <default> is |foo| has no value.
//
// In-place construction:
//
//   Optional<Foo> foo;   // |foo| is empty.
//   foo.emplace(20);     // |foo| now contains a value constructed as Foo(20)
//
//   Optional<Foo> foo(kInplace, 20);  // |foo| is initialized with a value
//                                     // that is constructed in-place as
//                                     // Foo(20).
//
//   return makeOptional<Foo>(20);     // Takes Foo constructor arguments
//                                     // directly.
//
// Returning values:
//
//  Optional<Foo> myFunc(...) {
//      if (someCondition) {
//          return Foo(10);      // call Optional<Foo>(Foo&) constructor.
//      } else {
//          return {};           // call Optional<Foo>() constructor, which
//                               // builds an empty value.
//      }
//  }
//
// Memory layout:
//   Optional<Foo> is equivalent to:
//
//       struct {
//           bool flag;
//           Foo value;
//       };
//
//  in terms of memory layout. This means it *doubles* the size of integral
//  types. Also:
//
//  - Optional<Foo> can be constructed from anything that constructs a Foo.
//
//  - Same with Optional<Foo>(kInplace, Args...) where Args... matches any
//    arguments that can be passed to a Foo constructor.
//
//  - Comparison operators are provided. Beware: an empty Optional<Foo>
//    is always smaller than any Foo value.

namespace astc_codec {
namespace base {

namespace details {

// Base classes to reduce the number of instantiations of the Optional's
// internal members.
class OptionalFlagBase {
 public:
  void setConstructed(bool constructed) { mConstructed = constructed; }
  constexpr bool constructed() const { return mConstructed; }
  constexpr operator bool() const { return constructed(); }
  bool hasValue() const { return constructed(); }

  constexpr OptionalFlagBase(bool constructed = false)
      : mConstructed(constructed) { }

 private:
  bool mConstructed = false;
};

template<size_t Size, size_t Align>
class OptionalStorageBase {
 protected:
  using StoreT = typename std::aligned_storage<Size, Align>::type;
  StoreT mStorage = {};
};

}  // namespace details

// A tag type for empty optional construction
struct NulloptT {
  constexpr explicit NulloptT(int) { }
};

// A tag type for inplace value construction
struct InplaceT {
  constexpr explicit InplaceT(int) { }
};

// Tag values for null optional and inplace construction
constexpr NulloptT kNullopt{1};
constexpr InplaceT kInplace{1};

// Forward declaration for an early use
template<class T>
class Optional;

// A type trait for checking if a type is an optional instantiation
// Note: if you want to refer to the template name inside the template,
//  you need to declare this alias outside of it - because the
//  class name inside of the template stands for an instantiated template
//  E.g, for template <T> class Foo if you say 'Foo' inside the class, it
//  actually means Foo<T>;
template<class U>
using is_any_optional =
    is_template_instantiation_of<typename std::decay<U>::type, Optional>;

template<class T>
class Optional
    : private details::OptionalFlagBase,
      private details::OptionalStorageBase<sizeof(T),
                                           std::alignment_of<T>::value> {
  // make sure all optionals are buddies - this is needed to implement
  // conversion from optionals of other types
  template<class U>
  friend class Optional;

  template<class U>
  using self = Optional<U>;

  using base_flag = details::OptionalFlagBase;
  using base_storage =
      details::OptionalStorageBase<sizeof(T), std::alignment_of<T>::value>;

 public:
  // std::optional will have this, so let's provide it
  using value_type = T;

  // make sure we forbid some Optional instantiations where things may get
  // really messy
  static_assert(!std::is_same<typename std::decay<T>::type, NulloptT>::value,
                "Optional of NulloptT is not allowed");
  static_assert(!std::is_same<typename std::decay<T>::type, InplaceT>::value,
                "Optional of InplaceT is not allowed");
  static_assert(!std::is_reference<T>::value,
                "Optional references are not allowed: use a pointer instead");

  // constructors
  constexpr Optional() { }
  constexpr Optional(NulloptT) { }

  Optional(const Optional& other) : base_flag(other.constructed()) {
    if (this->constructed()) {
      new (&get()) T(other.get());
    }
  }
  Optional(Optional&& other) : base_flag(other.constructed()) {
    if (this->constructed()) {
      new (&get()) T(std::move(other.get()));
    }
  }

  // Conversion constructor from optional of similar type
  template<class U, class = enable_if_c<!is_any_optional<U>::value &&
                                        std::is_constructible<T, U>::value>>
  Optional(const Optional<U>& other) : base_flag(other.constructed()) {
    if (this->constructed()) {
      new (&get()) T(other.get());
    }
  }

  // Move-conversion constructor
  template<class U, class = enable_if_c<!is_any_optional<U>::value &&
                                        std::is_constructible<T, U>::value>>
  Optional(Optional<U>&& other) : base_flag(other.constructed()) {
    if (this->constructed()) {
      new (&get()) T(std::move(other.get()));
    }
  }

  // Construction from a raw value
  Optional(const T& value) : base_flag(true) { new (&get()) T(value); }
  // Move construction from a raw value
  Optional(T&& value) : base_flag(true) { new (&get()) T(std::move(value)); }

  // Inplace construction from a list of |T|'s ctor arguments
  template<class... Args>
  Optional(InplaceT, Args&&... args) : base_flag(true) {
    new (&get()) T(std::forward<Args>(args)...);
  }

  // Inplace construction from an initializer list passed into |T|'s ctor
  template<class U, class = enable_if<
                        std::is_constructible<T, std::initializer_list<U>>>>
  Optional(InplaceT, std::initializer_list<U> il) : base_flag(true) {
    new (&get()) T(il);
  }

  // direct assignment
  Optional& operator=(const Optional& other) {
    if (&other == this) {
      return *this;
    }

    if (this->constructed()) {
      if (other.constructed()) {
        get() = other.get();
      } else {
        destruct();
        this->setConstructed(false);
      }
    } else {
      if (other.constructed()) {
        new (&get()) T(other.get());
        this->setConstructed(true);
      } else {
        ;  // we're good
      }
    }
    return *this;
  }

  // move assignment
  Optional& operator=(Optional&& other) {
    if (this->constructed()) {
      if (other.constructed()) {
        get() = std::move(other.get());
      } else {
        destruct();
        this->setConstructed(false);
      }
    } else {
      if (other.constructed()) {
        new (&get()) T(std::move(other.get()));
        this->setConstructed(true);
      } else {
        ;  // we're good
      }
    }
    return *this;
  }

  // conversion assignment
  template<class U,
           class = enable_if_convertible<typename std::decay<U>::type, T>>
  Optional& operator=(const Optional<U>& other) {
    if (this->constructed()) {
      if (other.constructed()) {
        get() = other.get();
      } else {
        destruct();
        this->setConstructed(false);
      }
    } else {
      if (other.constructed()) {
        new (&get()) T(other.get());
        this->setConstructed(true);
      } else {
        ;  // we're good
      }
    }
    return *this;
  }

  // conversion move assignment
  template<class U,
           class = enable_if_convertible<typename std::decay<U>::type, T>>
  Optional& operator=(Optional<U>&& other) {
    if (this->constructed()) {
      if (other.constructed()) {
        get() = std::move(other.get());
      } else {
        destruct();
        this->setConstructed(false);
      }
    } else {
      if (other.constructed()) {
        new (&get()) T(std::move(other.get()));
        this->setConstructed(true);
      } else {
        ;  // we're good
      }
    }
    return *this;
  }

  // the most complicated one: forwarding constructor for anything convertible
  // to |T|, excluding the stuff implemented above explicitly
  template<class U,
           class = enable_if_c<
               !is_any_optional<typename std::decay<U>::type>::value &&
               std::is_convertible<typename std::decay<U>::type, T>::value>>
  Optional& operator=(U&& other) {
    if (this->constructed()) {
      get() = std::forward<U>(other);
    } else {
      new (&get()) T(std::forward<U>(other));
      this->setConstructed(true);
    }
    return *this;
  }

  // Adopt value checkers from the parent
  using base_flag::operator bool;
  using base_flag::hasValue;

  T& value() {
    assert(this->constructed());
    return get();
  }
  constexpr const T& value() const {
    assert(this->constructed());
    return get();
  }

  T* ptr() { return this->constructed() ? &get() : nullptr; }
  constexpr const T* ptr() const {
    return this->constructed() ? &get() : nullptr;
  }

  // Value getter with fallback
  template<class U = T,
           class = enable_if_convertible<typename std::decay<U>::type, T>>
  constexpr T valueOr(U&& defaultValue) const {
    return this->constructed() ? get() : std::move(defaultValue);
  }

  // Pointer-like operators
  T& operator*() {
    assert(this->constructed());
    return get();
  }
  constexpr const T& operator*() const {
    assert(this->constructed());
    return get();
  }

  T* operator->() {
    assert(this->constructed());
    return &get();
  }
  constexpr const T* operator->() const {
    assert(this->constructed());
    return &get();
  }

  ~Optional() {
    if (this->constructed()) {
      destruct();
    }
  }

  void clear() {
    if (this->constructed()) {
      destruct();
      this->setConstructed(false);
    }
  }

  template<class U,
           class = enable_if_convertible<typename std::decay<U>::type, T>>
  void reset(U&& u) {
    *this = std::forward<U>(u);
  }

  // In-place construction with possible destruction of the old value
  template<class... Args>
  void emplace(Args&&... args) {
    if (this->constructed()) {
      destruct();
    }
    new (&get()) T(std::forward<Args>(args)...);
    this->setConstructed(true);
  }

  // In-place construction with possible destruction of the old value
  // initializer-list version
  template<class U, class = enable_if<
                        std::is_constructible<T, std::initializer_list<U>>>>
  void emplace(std::initializer_list<U> il) {
    if (this->constructed()) {
      destruct();
    }
    new (&get()) T(il);
    this->setConstructed(true);
  }

 private:
  // A helper function to convert the internal raw storage to T&
  constexpr const T& get() const {
    return *reinterpret_cast<const T*>(
        reinterpret_cast<const char*>(&this->mStorage));
  }

  // Same thing, mutable
  T& get() { return const_cast<T&>(const_cast<const Optional*>(this)->get()); }

  // Shortcut for a destructor call for the stored object
  void destruct() { get().T::~T(); }
};

template<class T>
Optional<typename std::decay<T>::type> makeOptional(T&& t) {
  return Optional<typename std::decay<T>::type>(std::forward<T>(t));
}

template<class T, class... Args>
Optional<typename std::decay<T>::type> makeOptional(Args&&... args) {
  return Optional<typename std::decay<T>::type>(kInplace,
                                                std::forward<Args>(args)...);
}

template<class T>
bool operator==(const Optional<T>& l, const Optional<T>& r) {
  return l.hasValue() ? r.hasValue() && *l == *r : !r.hasValue();
}
template<class T>
bool operator==(const Optional<T>& l, NulloptT) {
  return !l;
}
template<class T>
bool operator==(NulloptT, const Optional<T>& r) {
  return !r;
}
template<class T>
bool operator==(const Optional<T>& l, const T& r) {
  return bool(l) && *l == r;
}
template<class T>
bool operator==(const T& l, const Optional<T>& r) {
  return bool(r) && l == *r;
}

template<class T>
bool operator!=(const Optional<T>& l, const Optional<T>& r) {
  return !(l == r);
}
template<class T>
bool operator!=(const Optional<T>& l, NulloptT) {
  return bool(l);
}
template<class T>
bool operator!=(NulloptT, const Optional<T>& r) {
  return bool(r);
}
template<class T>
bool operator!=(const Optional<T>& l, const T& r) {
  return !l || !(*l == r);
}
template<class T>
bool operator!=(const T& l, const Optional<T>& r) {
  return !r || !(l == *r);
}

template<class T>
bool operator<(const Optional<T>& l, const Optional<T>& r) {
  return !r ? false : (!l ? true : *l < *r);
}
template<class T>
bool operator<(const Optional<T>&, NulloptT) {
  return false;
}
template<class T>
bool operator<(NulloptT, const Optional<T>& r) {
  return bool(r);
}
template<class T>
bool operator<(const Optional<T>& l, const T& r) {
  return !l || *l < r;
}
template<class T>
bool operator<(const T& l, const Optional<T>& r) {
  return bool(r) && l < *r;
}

}  // namespace base
}  // namespace astc_codec

#endif  // ASTC_CODEC_BASE_OPTIONAL_H_
