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

#ifndef ROCKETCONTROLSELEMENTDATAGRID_H
#define ROCKETCONTROLSELEMENTDATAGRID_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Controls/DataSourceListener.h>
#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Controls {

class DataFormatter;
class ElementDataGridRow;

/**
	A table driven from a data source.

	@author Robert Curry
 */

class ROCKETCONTROLS_API ElementDataGrid : public Core::Element, public DataSourceListener
{
public:
	ElementDataGrid(const Rocket::Core::String& tag);
	virtual ~ElementDataGrid();

	/// Sets a new data source for the contents of the data grid.
	/// @param[in] data_source_name The name of the new data source.
	void SetDataSource(const Rocket::Core::String& data_source_name);

	/**
		A column inside a table.

		@author Robert Curry
	 */
	struct Column
	{
		/// The list of fields that this column reads from the data source for
		/// each row.
		Rocket::Core::StringList fields;

		/// The data formatter this is used to process the field information
		/// into what is finally displayed in the data grid.
		DataFormatter* formatter;
		/// The header that is displayed at the top of the column, in the
		/// header row.
		Core::Element* header;

		/// The width of this column.
		float current_width;

		/// Whether this column has a forced refresh when a child node changes.
		/// This is to allow the expand/collapse buttons to be added or removed
		/// when a child node is added.
		bool refresh_on_child_change;
	};

	/// Adds a column to the table.
	/// @param[in] fields A comma-separated list of fields that this column reads from the data source.
	/// @param[in] formatter The name of the data formatter to be used to format the raw column data into RML.
	/// @param[in] initial_width The initial width, in pixels, of the column.
	/// @param[in] header_rml The RML to use as the column header.
	/// @return True if the column was added successfully, false if not.
	bool AddColumn(const Rocket::Core::String& fields, const Rocket::Core::String& formatter, float initial_width, const Rocket::Core::String& header_rml);
	/// Adds a column to the table.
	/// @param[in] fields A comma-separated list of fields that this column reads from the data source.
	/// @param[in] formatter The name of the data formatter to be used to format the raw column data into RML.
	/// @param[in] initial_width The initial width, in pixels, of the column.
	/// @param[in] header_element The element hierarchy to use as the column header.
	void AddColumn(const Rocket::Core::String& fields, const Rocket::Core::String& formatter, float initial_width, Core::Element* header_element);
	/// Returns the number of columns in this table
	int GetNumColumns();
	/// Returns the column at the specified index.
	const Column* GetColumn(int column_index);
	/// Returns a CSV string containing all the fields that each column requires, in order.
	const Rocket::Core::String& GetAllColumnFields();

	/// Adds a new row to the table. This is only called from child rows.
	/// @param[in] parent The parent row that the row is being added under.
	/// @param[in] index The index of the child, relative to its parent.
	/// @return A pointer to the newly created row.
	ElementDataGridRow* AddRow(ElementDataGridRow* parent, int index);
	/// Removes a series of rows from the table.
	/// @param[in] index The index of the first row, relative to the table.
	/// @param[in] num_rows The number of rows to remove. Defaults to one.
	void RemoveRows(int index, int num_rows = 1);

	/// Returns the number of rows in the table
	int GetNumRows() const;
	/// Returns the row at the given index in the table.
	/// @param[in] index The index of the row, relative to the table.
	ElementDataGridRow* GetRow(int index) const;

protected:
	virtual void OnUpdate();

	virtual void ProcessEvent(Core::Event& event);

	/// Gets the markup and content of the element.
	/// @param content[out] The content of the element.
	virtual void GetInnerRML(Rocket::Core::String& content) const;

private:
	typedef std::vector< Column > ColumnList;
	typedef std::vector< ElementDataGridRow* > RowList;

	ColumnList columns;
	Rocket::Core::String column_fields;

	// The row that contains the header elements of the table.
	ElementDataGridRow* header;

	// The root row, all the top level rows are children under this. Not
	// actually rendered, has "display: none".
	ElementDataGridRow* root;
	// If this is non-empty, then in the previous update the data source was set
	// and we must set it this update.
	Rocket::Core::String new_data_source;

	// The block element that contains all our rows. Only used for applying styles.
	Core::Element* body;
	// Stores if the body has already been made visible by having enough rows added.
	bool body_visible;
};

}
}

#endif
