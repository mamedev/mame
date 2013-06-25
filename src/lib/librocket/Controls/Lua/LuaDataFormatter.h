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
 
#ifndef ROCKETCONTROLSLUALUADATAFORMATTER_H
#define ROCKETCONTROLSLUALUADATAFORMATTER_H

#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Controls/DataFormatter.h>

namespace Rocket {
namespace Controls {
namespace Lua {

class LuaDataFormatter : public Rocket::Controls::DataFormatter
{
public:
    LuaDataFormatter(const Rocket::Core::String& name = "");
    ~LuaDataFormatter();

    virtual void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data);

    //Helper function used to push on to the stack the table where the function ref should be stored
    static void PushDataFormatterFunctionTable(lua_State* L);

    int ref_FormatData; //the lua reference to the FormatData function
};

}
}
}
#endif
