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

#ifndef ROCKETCONTROLSELEMENTDATAGRIDROW_H
#define ROCKETCONTROLSELEMENTDATAGRIDROW_H

#include <Rocket/Controls/Header.h>
#include <Rocket/Controls/DataSourceListener.h>
#include <Rocket/Controls/DataQuery.h>
#include <Rocket/Core/Element.h>
#include <queue>

namespace Rocket {
namespace Controls {

class ElementDataGrid;

/**
	Class for rows inside a data table. Used for both the header and the individual rows.

	@author Robert Curry
 */

class ROCKETCONTROLS_API ElementDataGridRow : public Core::Element, public DataSourceListener
{
friend class ElementDataGrid;

public:
	ElementDataGridRow(const Rocket::Core::String& tag);
	virtual ~ElementDataGridRow();

	void Initialise(ElementDataGrid* parent_grid, ElementDataGridRow* parent_row = NULL, int child_index = -1, ElementDataGridRow* header_row = NULL, int depth = -1);
	void SetChildIndex(int child_index);
	int GetDepth();

	void SetDataSource(const Rocket::Core::String& data_source_name);

	/// Checks dirty children and cells, and loads them if necessary.
	/// @return True if any children were updated.
	bool UpdateChildren();

	/// Returns the number of children that aren't dirty (have been loaded)
	int GetNumLoadedChildren();

	// Removes all the child cells and fetches them again from the data
	// source.
	void RefreshRows();

	/// Returns whether this row is expanded or not.
	bool IsRowExpanded();
	/// Shows all of this row's descendants.
	void ExpandRow();
	/// Hides all of this row's descendants.
	void CollapseRow();
	/// Expands the row if collapsed, or collapses the row if expanded.
	void ToggleRow();

	/// Returns the index of this row, relative to its parent.
	int GetParentRelativeIndex();
	/// Returns the index of this row, relative to the table rather than its parent.
	int GetTableRelativeIndex();
	/// Returns the parent row of this row.
	ElementDataGridRow* GetParentRow();
	/// Returns the grid that this row belongs to.
	ElementDataGrid* GetParentGrid();

protected:
	virtual void OnDataSourceDestroy(DataSource* data_source);
	virtual void OnRowAdd(DataSource* data_source, const Rocket::Core::String& table, int first_row_added, int num_rows_added);
	virtual void OnRowRemove(DataSource* data_source, const Rocket::Core::String& table, int first_row_removed, int num_rows_removed);
	virtual void OnRowChange(DataSource* data_source, const Rocket::Core::String& table, int first_row_changed, int num_rows_changed);
	virtual void OnRowChange(DataSource* data_source, const Rocket::Core::String& table);

private:
	typedef std::queue< ElementDataGridRow* > RowQueue;
	typedef std::vector< ElementDataGridRow* > RowList;

	// Called when a row change (addition or removal) occurs in one of our
	// children. Causes the table row index to be dirtied on all following
	// children.
	void ChildChanged(int child_index);
	// Checks if any columns are dependent on the number of children
	// present, and refreshes them from the data source if they are.
	void RefreshChildDependentCells();

	// Forces the row to recalculate its relative table index the next time
	// it is requested.
	void DirtyTableRelativeIndex();
	// Works out what the table relative index is for a given child.
	int GetChildTableRelativeIndex(int child_index);

	// Adds children underneath this row, and fetches their contents (and
	// possible children) from the row's data source. If first_row is left
	// as the default -1, the rows are appended at the end of the list.
	void AddChildren(int first_row_added = -1, int num_rows_added = 1);
	// Removes this rows children, and their children, etc, from the table.
	// If the num_rows_removed parameter is left as the -1 default, it'll
	// default to the rest of the children after the first row.
	void RemoveChildren(int first_row_removed = 0, int num_rows_removed = -1);
	// Returns the number of rows under this row (children, grandchildren, etc)
	int GetNumDescendants();

	// Adds or refreshes the cell contents, and undirties the row's cells.
	void Load(const Rocket::Controls::DataQuery& row_information);
	// Finds all children that have cell information missing (either though being
	// refreshed or not being loaded yet) and reloads them.
	void LoadChildren(float time_slice);
	// Loads a specific set of children. Called by the above function.
	void LoadChildren(int first_row_to_load, int num_rows_to_load, Rocket::Core::Time time_slice);

	// If the cells need reloading, this takes care of it. If any children
	// need updating, they are added to the queue.
	void UpdateCellsAndChildren(RowQueue& dirty_rows);

	// Sets the dirty_cells flag on this row, and lets our ancestors know.
	void DirtyCells();
	// Sets the dirty children flag on this row and the row's ancestors.
	void DirtyRow();
	// This row has one or more cells that need loading.
	bool dirty_cells;
	// This row has one or more children that have either dirty flag set.
	bool dirty_children;

	// Shows this row, and, if this was was expanded before it was hidden, its children as well.
	void Show();
	// Hides this row and all descendants.
	void Hide();
	bool row_expanded;

	int table_relative_index;
	bool table_relative_index_dirty;

	ElementDataGrid* parent_grid;

	ElementDataGridRow* parent_row;
	int child_index;
	int depth;

	RowList children;

	// The data source and table that the children are fetched from.
	DataSource* data_source;
	Rocket::Core::String data_table;
};

}
}

#endif
