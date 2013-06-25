/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef ROCKETCOREPYTHONWRAPPER_H
#define ROCKETCOREPYTHONWRAPPER_H

#include <Rocket/Core/Debug.h>
#include <Rocket/Core/Python/Python.h>

#include <boost/none.hpp>

namespace Rocket {
namespace Core {
namespace Python {

/**
	Generic Python Wrapper, using boost preprocessor iteration for constructor params
	
	Defines a basic wrapper, then template overloads it for each variation of the
	number of arguments.

	@author Lloyd Weehuizen
 */

struct WrapperNone {};

#define WRAPPER_MAX_ARGS	6

#define WRAPPER_TEMPLATE_ARG(z, n, d) BOOST_PP_COMMA_IF(n) typename T##n = WrapperNone

template <typename T, BOOST_PP_REPEAT_1( BOOST_PP_INC(WRAPPER_MAX_ARGS), WRAPPER_TEMPLATE_ARG, N) >
class Wrapper {};

#undef WRAPPER_TEMPLATE_ARG

#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, WRAPPER_MAX_ARGS, <Rocket/Core/Python/WrapperIter.h>))
#include BOOST_PP_ITERATE()

}
}
}

#endif
