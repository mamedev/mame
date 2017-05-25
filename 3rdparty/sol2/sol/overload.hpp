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

#ifndef SOL_OVERLOAD_HPP
#define SOL_OVERLOAD_HPP

#include <utility>

namespace sol {
    template <typename... Functions>
    struct overload_set {
        std::tuple<Functions...> functions;
        template <typename Arg, typename... Args, meta::disable<std::is_same<overload_set, meta::unqualified_t<Arg>>> = meta::enabler>
        overload_set (Arg&& arg, Args&&... args) : functions(std::forward<Arg>(arg), std::forward<Args>(args)...) {}
        overload_set(const overload_set&) = default;
        overload_set(overload_set&&) = default;
        overload_set& operator=(const overload_set&) = default;
        overload_set& operator=(overload_set&&) = default;
    };

    template <typename... Args>
    decltype(auto) overload(Args&&... args) {
        return overload_set<std::decay_t<Args>...>(std::forward<Args>(args)...);
    }
}

#endif // SOL_OVERLOAD_HPP