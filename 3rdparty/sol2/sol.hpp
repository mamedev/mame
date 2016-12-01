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

#ifndef SOL_HPP
#define SOL_HPP

#if defined(UE_BUILD_DEBUG) || defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_TEST) || defined(UE_BUILD_SHIPPING) || defined(UE_SERVER)
#define SOL_INSIDE_UNREAL
#endif // Unreal Engine 4 bullshit

#ifdef SOL_INSIDE_UNREAL
#ifdef check
#define SOL_INSIDE_UNREAL_REMOVED_CHECK
#undef check
#endif 
#endif // Unreal Engine 4 Bullshit

#include "sol/state.hpp"
#include "sol/object.hpp"
#include "sol/function.hpp"
#include "sol/protected_function.hpp"
#include "sol/state.hpp"
#include "sol/coroutine.hpp"
#include "sol/variadic_args.hpp"

#ifdef SOL_INSIDE_UNREAL
#ifdef SOL_INSIDE_UNREAL_REMOVED_CHECK
#define check(expr) { if(UNLIKELY(!(expr))) { FDebug::LogAssertFailedMessage( #expr, __FILE__, __LINE__ ); _DebugBreakAndPromptForRemote(); FDebug::AssertFailed( #expr, __FILE__, __LINE__ ); CA_ASSUME(false); } }}
#endif 
#endif // Unreal Engine 4 Bullshit

#endif // SOL_HPP
