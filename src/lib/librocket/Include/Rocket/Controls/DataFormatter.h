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

#ifndef ROCKETCONTROLSDATAFORMATTER_H
#define ROCKETCONTROLSDATAFORMATTER_H

#include <Rocket/Core/ScriptInterface.h>
#include <Rocket/Core/String.h>
#include <Rocket/Controls/Header.h>

namespace Rocket {
namespace Controls {

class Element;

/**
	Abstract base class for a data formatter. A data formatter takes raw data
	and processes it into a final string. They are usually used in conjunction
	with a data source and a datagrid.

	@author Robert Curry
 */

class ROCKETCONTROLS_API DataFormatter
{
public:
	DataFormatter(const Rocket::Core::String& name = "");
	virtual ~DataFormatter();

	/// Returns the name by which this data formatter is referenced by.
	/// @return The name of this data formatter.
	const Rocket::Core::String& GetDataFormatterName();
	/// Returns a data formatter with the given name.
	/// @parameter [in] data_formatter_name The name of the data formatter to
	/// be returned.
	/// @return If the data formatter with the specified name has been
	/// constructed, a pointer to it will be returned. Otherwise, NULL.
	static DataFormatter* GetDataFormatter(const Rocket::Core::String& data_formatter_name);

	/// Formats the raw results of a data source request into RML.
	/// @param[out] formatted_data The formatted RML.
	/// @param[in] raw_data A list of the raw data fields.
	virtual void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data) = 0;

	virtual void* GetScriptObject() const;

private:
	Rocket::Core::String name;
};

}
}

#endif
