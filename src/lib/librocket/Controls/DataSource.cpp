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

#include <Rocket/Controls/DataSource.h>
#include <Rocket/Controls/DataSourceListener.h>
#include <Rocket/Core/StringUtilities.h>
#include <Rocket/Core/Log.h>
#include <algorithm>

namespace Rocket {
namespace Controls {

const Rocket::Core::String DataSource::CHILD_SOURCE("#child_data_source");
const Rocket::Core::String DataSource::DEPTH("#depth");
const Rocket::Core::String DataSource::NUM_CHILDREN("#num_children");

typedef std::map< Rocket::Core::String, DataSource* > DataSourceMap;
static DataSourceMap data_sources;

DataSource::DataSource(const Rocket::Core::String& _name)
{
	if (!_name.Empty())
	{
		name = _name;
	}
	else
	{
		name.FormatString(64, "%x", this);
	}
	data_sources[name] = this;
}

DataSource::~DataSource()
{
	ListenerList listeners_copy = listeners;
	for (ListenerList::iterator i = listeners_copy.begin(); i != listeners_copy.end(); ++i)
	{
		(*i)->OnDataSourceDestroy(this);
	}

	DataSourceMap::iterator iterator = data_sources.find(name);
	if (iterator != data_sources.end() &&
		iterator->second == this)
	{
		data_sources.erase(name);
	}
}

const Rocket::Core::String& DataSource::GetDataSourceName()
{
	return name;
}

DataSource* DataSource::GetDataSource(const Rocket::Core::String& data_source_name)
{
	DataSourceMap::iterator i = data_sources.find(data_source_name);
	if (i == data_sources.end())
	{
		return NULL;
	}

	return (*i).second;
}

void DataSource::AttachListener(DataSourceListener* listener)
{
	if (find(listeners.begin(), listeners.end(), listener) != listeners.end())
	{
		ROCKET_ERROR;
		return;
	}
	listeners.push_back(listener);
}

void DataSource::DetachListener(DataSourceListener* listener)
{
	ListenerList::iterator i = find(listeners.begin(), listeners.end(), listener);
	ROCKET_ASSERT(i != listeners.end());
	if (i != listeners.end())
	{
		listeners.erase(i);
	}
}

void* DataSource::GetScriptObject() const
{
	return NULL;
}

void DataSource::NotifyRowAdd(const Rocket::Core::String& table, int first_row_added, int num_rows_added)
{
	ListenerList listeners_copy = listeners;
	for (ListenerList::iterator i = listeners_copy.begin(); i != listeners_copy.end(); ++i)
	{
		(*i)->OnRowAdd(this, table, first_row_added, num_rows_added);
	}
}

void DataSource::NotifyRowRemove(const Rocket::Core::String& table, int first_row_removed, int num_rows_removed)
{
	ListenerList listeners_copy = listeners;
	for (ListenerList::iterator i = listeners_copy.begin(); i != listeners_copy.end(); ++i)
	{
		(*i)->OnRowRemove(this, table, first_row_removed, num_rows_removed);
	}
}

void DataSource::NotifyRowChange(const Rocket::Core::String& table, int first_row_changed, int num_rows_changed)
{
	ListenerList listeners_copy = listeners;
	for (ListenerList::iterator i = listeners_copy.begin(); i != listeners_copy.end(); ++i)
	{
		(*i)->OnRowChange(this, table, first_row_changed, num_rows_changed);
	}
}

void DataSource::NotifyRowChange(const Rocket::Core::String& table)
{
	ListenerList listeners_copy = listeners;
	for (ListenerList::iterator i = listeners_copy.begin(); i != listeners_copy.end(); ++i)
	{
		(*i)->OnRowChange(this, table);
	}
}

void DataSource::BuildRowEntries(Rocket::Core::StringList& row, const RowMap& row_map, const Rocket::Core::StringList& columns)
{
	// Reserve the number of entries.
	row.resize(columns.size());
	for (size_t i = 0; i < columns.size(); i++)
	{
		// Look through our row_map for each entry and add it to the result set
		RowMap::const_iterator itr = row_map.find(columns[i]);
		if (itr != row_map.end())
		{
			row[i] = (*itr).second;
		}
		else
		{
			row[i] = "";
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Failed to find required data source column %s", columns[i].CString());
		}
	}
}

}
}
