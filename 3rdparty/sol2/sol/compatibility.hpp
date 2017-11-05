// The MIT License (MIT) 

// Copyright (c) 2013-2016 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_COMPATIBILITY_HPP
#define SOL_COMPATIBILITY_HPP

// The various pieces of the compatibility layer
// comes from https://github.com/keplerproject/lua-compat-5.2
// but has been modified in many places for use with Sol and luajit,
// though the core abstractions remain the same

#include "compatibility/version.hpp"

#ifndef SOL_NO_COMPAT

#ifdef __cplusplus
extern "C" {
#endif
#include "compatibility/5.1.0.h"
#include "compatibility/5.0.0.h"
#include "compatibility/5.x.x.h"
#include "compatibility/5.x.x.inl"
#ifdef __cplusplus
}
#endif

#endif // SOL_NO_COMPAT

#endif // SOL_COMPATIBILITY_HPP
