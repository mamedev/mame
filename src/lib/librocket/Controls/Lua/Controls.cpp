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

#include "precompiled.h"
#include <Rocket/Controls/Lua/Controls.h>
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Lua/Interpreter.h>
#include <Rocket/Core/Log.h>
#include "SelectOptionsProxy.h"
#include "DataFormatter.h"
#include "DataSource.h"
#include "ElementForm.h"
#include "ElementFormControl.h"
#include "ElementFormControlSelect.h"
#include "ElementFormControlDataSelect.h"
#include "ElementFormControlInput.h"
#include "ElementFormControlTextArea.h"
#include "ElementDataGrid.h"
#include "ElementDataGridRow.h"
#include "ElementTabSet.h"

using Rocket::Core::Lua::LuaType;
namespace Rocket {
namespace Controls {
namespace Lua {

//This will define all of the types from RocketControls for Lua. There is not a
//corresponding function for types of RocketCore, because they are defined automatically
//when the Interpreter starts.
void RegisterTypes(lua_State* L)
{
    if(Rocket::Core::Lua::Interpreter::GetLuaState() == NULL)
    {
        Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR,
            "In Rocket::Controls::Lua::RegisterTypes: Tried to register the \'Controls\' types for Lua without first initializing the Interpreter.");
        return;
    }
    LuaType<ElementForm>::Register(L);
    LuaType<ElementFormControl>::Register(L);
        //Inherits from ElementFormControl
        LuaType<ElementFormControlSelect>::Register(L);
            LuaType<ElementFormControlDataSelect>::Register(L);
        LuaType<ElementFormControlInput>::Register(L);
        LuaType<ElementFormControlTextArea>::Register(L);
    LuaType<DataFormatter>::Register(L);
    LuaType<DataSource>::Register(L);
    LuaType<ElementDataGrid>::Register(L);
    LuaType<ElementDataGridRow>::Register(L);
    LuaType<ElementTabSet>::Register(L);
    //proxy tables
    LuaType<SelectOptionsProxy>::Register(L);
}

}
}
}
