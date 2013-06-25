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

#ifndef ROCKETCONTROLSDATAQUERY_H
#define ROCKETCONTROLSDATAQUERY_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Core/TypeConverter.h>
#include <Rocket/Core/Log.h>

namespace Rocket {
namespace Controls {

class DataSource;

/**
	DataQuery
	@author Robert Curry

	Represents a request for information from an DataSource, encapsulates the result and offers
	mechanisms to iterate through the returned rows.
*/

class ROCKETCONTROLS_API DataQuery
{
public:
	DataQuery();
	DataQuery(DataSource* data_source, const Rocket::Core::String& table, const Rocket::Core::String& fields, int offset = 0, int limit = -1, const Rocket::Core::String& order = "");
	virtual ~DataQuery();

	void ExecuteQuery(DataSource* data_source, const Rocket::Core::String& table, const Rocket::Core::String& fields, int offset = 0, int limit = -1, const Rocket::Core::String& order = "");
	bool NextRow();

	bool IsFieldSet(const Rocket::Core::String& field) const;

	template< typename T >
	T Get(const Rocket::Core::String& field_name, const T& default_value) const
	{
		FieldIndices::const_iterator itr = field_indices.find(field_name);
		if (itr == field_indices.end())
		{
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Field %s not found in query", field_name.CString());
			return default_value;
		}		
		
		T return_value = default_value;

		GetInto((*itr).second, return_value);			

		return return_value;
	}

	template< typename T >
	bool GetInto(const Rocket::Core::String& field_name, T& value) const
	{
		FieldIndices::const_iterator itr = field_indices.find(field_name);
		if (itr == field_indices.end())
		{
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Field %s not found in query", field_name.CString());
			return false;
		}		

		return GetInto((*itr).second, value);		
	}	

	template< typename T >
	T Get(const size_t field_index, const T& default_value) const
	{
		T return_value = default_value;

		GetInto(field_index, return_value);

		return return_value;
	}

	template< typename T >
	bool GetInto(const size_t field_index, T& value) const
	{
		if (field_index < rows[current_row].size())
		{
			return Rocket::Core::TypeConverter< Rocket::Core::String, T >::Convert(rows[current_row][field_index], value);
		}

		return false;
	}

	size_t GetNumFields()
	{
		return rows[current_row].size();
	}

private:
	Rocket::Core::StringList fields;

	DataSource* data_source;
	Rocket::Core::String table;
	int current_row;
	int offset;
	int limit;

	typedef std::vector< Rocket::Core::StringList > Rows;
	Rows rows;
	typedef std::map< Rocket::Core::String, size_t > FieldIndices;
	FieldIndices field_indices;
	
	void LoadRow();
};

}
}

#endif // ROCKETCONTROLSDATAQUERY_H
