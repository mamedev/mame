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
 
#ifndef ROCKETCONTROLSLUALUADATASOURCE_H
#define ROCKETCONTROLSLUALUADATASOURCE_H

#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Controls/DataSource.h>
#include <Rocket/Core/String.h>

namespace Rocket {
namespace Controls {
namespace Lua {

class LuaDataSource : public Rocket::Controls::DataSource
{
public:
    //default initilialize the lua func references to -1
    LuaDataSource(const Rocket::Core::String& name = "");

	/// Fetches the contents of one row of a table within the data source.
	/// @param[out] row The list of values in the table.
	/// @param[in] table The name of the table to query.
	/// @param[in] row_index The index of the desired row.
	/// @param[in] columns The list of desired columns within the row.
	virtual void GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns);
	/// Fetches the number of rows within one of this data source's tables.
	/// @param[in] table The name of the table to query.
	/// @return The number of rows within the specified table. Returns -1 in case of an incorrect Lua function.
	virtual int GetNumRows(const Rocket::Core::String& table);

    //make the protected members of DataSource public
    using DataSource::NotifyRowAdd;
    using DataSource::NotifyRowRemove;
    using DataSource::NotifyRowChange;

    //lua reference to DataSource.GetRow
    int getRowRef;
    //lua reference to DataSource.GetNumRows
    int getNumRowsRef;
};

}
}
}
#endif
