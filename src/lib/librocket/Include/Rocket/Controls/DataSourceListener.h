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

#ifndef ROCKETCONTROLSDATASOURCELISTENER_H
#define ROCKETCONTROLSDATASOURCELISTENER_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Core/String.h>

namespace Rocket {
namespace Controls {

class DataSource;

/**
	Interface for objects wishing to listen to data source events. Listeners should use the
	AttachListener() on DataSource to begin observing a data source.

	@author Robert Curry
 */

class ROCKETCONTROLS_API DataSourceListener
{
public:
	DataSourceListener();
	virtual ~DataSourceListener();

	/// Notification of the destruction of an observed data source.
	/// @param[in] data_source Data source being destroyed.
	virtual void OnDataSourceDestroy(DataSource* data_source);
	/// Notification of the addition of one or more rows to an observed data source's table.
	/// @param[in] data_source Data source being changed.
	/// @param[in] table The name of the changing table within the data source.
	/// @param[in] first_row_added Index of the first new row.
	/// @param[in] num_rows_added Number of new sequential rows being added.
	virtual void OnRowAdd(DataSource* data_source, const Rocket::Core::String& table, int first_row_added, int num_rows_added);
	/// Notification of the removal of one or more rows from an observed data source's table.
	/// @param[in] data_source Data source being changed.
	/// @param[in] table The name of the changing table within the data source.
	/// @param[in] first_row_removed Index of the first removed row.
	/// @param[in] num_rows_removed Number of new sequential rows being removed.
	virtual void OnRowRemove(DataSource* data_source, const Rocket::Core::String& table, int first_row_removed, int num_rows_removed);
	/// Notification of the changing of one or more rows from an observed data source's table.
	/// @param[in] data_source Data source being changed.
	/// @param[in] table The name of the changing table within the data source.
	/// @param[in] first_row_removed Index of the first changed row.
	/// @param[in] num_rows_removed Number of new sequential rows being changed.
	virtual void OnRowChange(DataSource* data_source, const Rocket::Core::String& table, int first_row_changed, int num_rows_changed);
	/// Notification of the change of all of the data of an observed data source's table.
	/// @param[in] data_source Data source being changed.
	/// @param[in] table The name of the changing table within the data source.
	virtual void OnRowChange(DataSource* data_source, const Rocket::Core::String& table);

protected:
	/// Sets up data source and table from a given Rocket::Core::String.
	/// @param[out] data_source A pointer to a data_source that gets loaded with the specified data source.
	/// @param[out] table_name A reference to an Rocket::Core::String that gets loaded with the specified data table.
	/// @param[in] data_source_name The data source and table in SOURCE.TABLE format.
	/// @return True if the data source name was in the correct format, and the data source was found.
	bool ParseDataSource(DataSource*& data_source, Rocket::Core::String& table_name, const Rocket::Core::String& data_source_name);
};

}
}

#endif // ROCKETCONTROLSDATASOURCELISTENER_H

