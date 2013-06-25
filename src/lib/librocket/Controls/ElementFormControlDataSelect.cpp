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

#include <Rocket/Controls/ElementFormControlDataSelect.h>
#include <Rocket/Controls/DataQuery.h>
#include <Rocket/Controls/DataSource.h>
#include <Rocket/Controls/DataFormatter.h>
#include "WidgetDropDown.h"

namespace Rocket {
namespace Controls {

// Constructs a new ElementFormControlDataSelect.
ElementFormControlDataSelect::ElementFormControlDataSelect(const Rocket::Core::String& tag) : ElementFormControlSelect(tag)
{
	data_source = NULL;
	initialised = false;
}

ElementFormControlDataSelect::~ElementFormControlDataSelect()
{
	if (data_source != NULL)
		data_source->DetachListener(this);
}

// Sets the data source the control's options are driven from.
void ElementFormControlDataSelect::SetDataSource(const Rocket::Core::String& _data_source)
{
	SetAttribute("source", _data_source);
}

// If a new data source has been set on the control, this will attach to it and build the initial
// options.
void ElementFormControlDataSelect::OnUpdate()
{
	if (!initialised)
	{
		initialised = true;

		if (ParseDataSource(data_source, data_table, GetAttribute< Rocket::Core::String >("source", "")))
		{
			data_source->AttachListener(this);
			BuildOptions();
		}
	}
}

// Checks for changes to the data source or formatting attributes.
void ElementFormControlDataSelect::OnAttributeChange(const Core::AttributeNameList& changed_attributes)
{
	ElementFormControlSelect::OnAttributeChange(changed_attributes);

	if (changed_attributes.find("source") != changed_attributes.end())
	{
		if (data_source != NULL)
			data_source->DetachListener(this);

		initialised = false;
	}
	else if (changed_attributes.find("fields") != changed_attributes.end() ||
			 changed_attributes.find("valuefield") != changed_attributes.end() ||
			 changed_attributes.find("formatter") != changed_attributes.end())
	{
		BuildOptions();
	}
}

// Detaches from the data source and rebuilds the options.
void ElementFormControlDataSelect::OnDataSourceDestroy(DataSource* _data_source)
{
	if (data_source == _data_source)
	{
		data_source->DetachListener(this);
		data_source = NULL;
		data_table = "";

		BuildOptions();
	}
}

// Rebuilds the available options from the data source.
void ElementFormControlDataSelect::OnRowAdd(DataSource* ROCKET_UNUSED(data_source), const Rocket::Core::String& table, int ROCKET_UNUSED(first_row_added), int ROCKET_UNUSED(num_rows_added))
{
	if (table == data_table)
		BuildOptions();
}

// Rebuilds the available options from the data source.
void ElementFormControlDataSelect::OnRowRemove(DataSource* ROCKET_UNUSED(data_source), const Rocket::Core::String& table, int ROCKET_UNUSED(first_row_removed), int ROCKET_UNUSED(num_rows_removed))
{
	if (table == data_table)
		BuildOptions();
}

// Rebuilds the available options from the data source.
void ElementFormControlDataSelect::OnRowChange(DataSource* ROCKET_UNUSED(data_source), const Rocket::Core::String& table, int ROCKET_UNUSED(first_row_changed), int ROCKET_UNUSED(num_rows_changed))
{
	if (table == data_table)
		BuildOptions();
}

// Rebuilds the available options from the data source.
void ElementFormControlDataSelect::OnRowChange(DataSource* ROCKET_UNUSED(data_source), const Rocket::Core::String& table)
{
	if (table == data_table)
		BuildOptions();
}

// Builds the option list from the data source.
void ElementFormControlDataSelect::BuildOptions()
{
	widget->ClearOptions();

	if (data_source == NULL)
		return;

	// Store the old selection value and index. These will be used to restore the selection to the
	// most appropriate option after the options have been rebuilt.
	Rocket::Core::String old_value = GetValue();
	int old_selection = GetSelection();

	Rocket::Core::String fields_attribute = GetAttribute<Rocket::Core::String>("fields", "");
	Rocket::Core::String valuefield_attribute = GetAttribute<Rocket::Core::String>("valuefield", "");
	Rocket::Core::String data_formatter_attribute = GetAttribute<Rocket::Core::String>("formatter", "");
	DataFormatter* data_formatter = NULL;

	// Process the attributes.
	if (fields_attribute.Empty())
	{
		Core::Log::Message(Rocket::Core::Log::LT_ERROR, "DataQuery failed, no fields specified for %s.", GetTagName().CString());
		return;
	}

	if (valuefield_attribute.Empty())
	{
		valuefield_attribute = fields_attribute.Substring(0, fields_attribute.Find(","));
	}

	if (!data_formatter_attribute.Empty())
	{
		data_formatter = DataFormatter::GetDataFormatter(data_formatter_attribute);
		if (!data_formatter)
			Core::Log::Message(Rocket::Core::Log::LT_WARNING, "Unable to find data formatter named '%s', formatting skipped.", data_formatter_attribute.CString());
	}

	// Build a list of attributes
	Rocket::Core::String fields(valuefield_attribute);
	fields += ",";
	fields += fields_attribute;

	DataQuery query(data_source, data_table, fields);
	while (query.NextRow())
	{
		Rocket::Core::StringList fields;
		Rocket::Core::String value = query.Get<Rocket::Core::String>(0, "");

		for (size_t i = 1; i < query.GetNumFields(); ++i)
			fields.push_back(query.Get< Rocket::Core::String>(i, ""));

		Rocket::Core::String formatted("");
		if (fields.size() > 0)
			formatted = fields[0];

		if (data_formatter)
			data_formatter->FormatData(formatted, fields);

		// Add the data as an option.
		widget->AddOption(formatted, value, -1, false);
	}

	// If an option was selected before, attempt to restore the selection to it.
	if (old_selection > -1)
	{
		// Try to find a selection with the same value as the previous one.
		for (int i = 0; i < GetNumOptions(); ++i)
		{
			SelectOption* option = GetOption(i);
			if (option->GetValue() == old_value)
			{
				widget->SetSelection(i, true);
				return;
			}
		}

		// Failed to find an option with the same value. Attempt to at least set the same index.
		int new_selection = Rocket::Core::Math::Clamp(old_selection, 0, GetNumOptions() - 1);
		if (GetNumOptions() == 0)
			new_selection = -1;

		widget->SetSelection(new_selection, true);
	}
}

}
}
