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
#include "ElementInterface.h"
#include <Rocket/Core/Python/ConverterScriptObject.h>
#include <Rocket/Core/Factory.h>
#include <Rocket/Core/Python/ElementInstancer.h>
#include <Rocket/Core/Python/ElementWrapper.h>
#include <Rocket/Controls/ElementDataGrid.h>
#include <Rocket/Controls/ElementDataGridCell.h>
#include <Rocket/Controls/ElementDataGridRow.h>
#include <Rocket/Controls/ElementDataGridExpandButton.h>
#include <Rocket/Controls/ElementForm.h>
#include <Rocket/Controls/ElementFormControlDataSelect.h>
#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementFormControlSelect.h>
#include <Rocket/Controls/ElementFormControlTextArea.h>
#include <Rocket/Controls/ElementTabSet.h>
#include "SelectOptionProxy.h"
#include "DataGridRowProxy.h"

namespace Rocket {
namespace Controls {
namespace Python {

typedef std::map< Rocket::Core::String, PyObject* > ClassDefinitions;
ClassDefinitions class_definitions;

void ElementInterface::InitialisePythonInterface()
{
	// ElementDataGrid.
	bool (ElementDataGrid::*AddColumn)(const Rocket::Core::String&, const Rocket::Core::String&, float, const Rocket::Core::String&) = &ElementDataGrid::AddColumn;
	class_definitions["DataGrid"] = python::class_< ElementDataGrid, Core::Python::ElementWrapper< ElementDataGrid >, boost::noncopyable, python::bases< Core::Element > >("ElementDataGrid", python::init< const char* >())
		.def("AddColumn", AddColumn)
		.def("SetDataSource", &ElementDataGrid::SetDataSource)
		.add_property("rows", &ElementInterface::GetRows)
		.ptr();

	Rocket::Core::Python::ConverterScriptObject< ElementDataGrid > datagrid_converter;

	// ElementDataGridRow.
	class_definitions["DataGridRow"] = python::class_< ElementDataGridRow, Core::Python::ElementWrapper< ElementDataGridRow >, boost::noncopyable, python::bases< Core::Element > >("ElementDataGridRow", python::init< const char* >())
		.add_property("row_expanded", &ElementDataGridRow::IsRowExpanded, &ElementInterface::SetRowExpanded)
		.add_property("parent_grid", python::make_function(&ElementDataGridRow::GetParentGrid, python::return_value_policy< python::return_by_value >()))
		.add_property("parent_row", python::make_function(&ElementDataGridRow::GetParentRow, python::return_value_policy< python::return_by_value >()))
		.add_property("parent_relative_index", &ElementDataGridRow::GetParentRelativeIndex)
		.add_property("table_relative_index", &ElementDataGridRow::GetTableRelativeIndex)
		.ptr();

	DataGridRowProxy::InitialisePythonInterface();
	Rocket::Core::Python::ConverterScriptObject< ElementDataGridRow > datagridrow_converter;

	// ElementDataGridCell.
	class_definitions["DataGridCell"] = python::class_< ElementDataGridCell, Core::Python::ElementWrapper< ElementDataGridCell >, boost::noncopyable, python::bases< Core::Element > >("ElementDataGridCell", python::init< const char* >())
		.ptr();

	// ElementDataGridCell.
	class_definitions["DataGridExpand"] = python::class_< ElementDataGridExpandButton, Core::Python::ElementWrapper< ElementDataGridExpandButton >, boost::noncopyable, python::bases< Core::Element > >("ElementDataGridExpand", python::init< const char* >())
		.ptr();

	// ElementForm.
	class_definitions["Form"] = python::class_< ElementForm,Core::Python::ElementWrapper< ElementForm >, boost::noncopyable, python::bases< Core::Element > >("Form", python::init< const char* >())
		.def("Submit", &ElementForm::Submit)
		.def("Submit", &ElementInterface::Submit)
		.ptr();

	// ElementFormControl.
	python::class_< ElementFormControl, Core::Python::ElementWrapper< ElementFormControl >, boost::noncopyable, python::bases< Core::Element > >("IElementFormControl", python::no_init)
		.add_property("name", &ElementFormControl::GetName, &ElementFormControl::SetName)
		.add_property("value", &ElementFormControl::GetValue, &ElementFormControl::SetValue)
		.add_property("disabled", &ElementFormControl::IsDisabled, &ElementFormControl::SetDisabled)
		.ptr();

	// ElementFormControlInput.
	class_definitions["FormControlInput"] = python::class_< ElementFormControlInput, Core::Python::ElementWrapper< ElementFormControlInput >, boost::noncopyable, python::bases< ElementFormControl > >("ElementFormControlInput", python::init< const char* >())
		.add_property("checked", &ElementInterface::GetChecked, &ElementInterface::SetChecked)
		.add_property("maxlength", &ElementInterface::GetMaxLength, &ElementInterface::SetMaxLength)
		.add_property("size", &ElementInterface::GetSize, &ElementInterface::SetSize)
		.add_property("max", &ElementInterface::GetMax, &ElementInterface::SetMax)
		.add_property("min", &ElementInterface::GetMin, &ElementInterface::SetMin)
		.add_property("step", &ElementInterface::GetStep, &ElementInterface::SetStep)
		.ptr();

	// ElementFormControlTextArea.
	class_definitions["FormControlTextArea"] = python::class_< ElementFormControlTextArea,Core::Python::ElementWrapper< ElementFormControlTextArea >, boost::noncopyable, python::bases< ElementFormControl > >("ElementFormControlTextArea", python::init< const char* >())
		.add_property("cols", &ElementFormControlTextArea::GetNumColumns, &ElementFormControlTextArea::SetNumColumns)
		.add_property("rows", &ElementFormControlTextArea::GetNumRows, &ElementFormControlTextArea::SetNumRows)
		.add_property("wordwrap", &ElementFormControlTextArea::GetWordWrap, &ElementFormControlTextArea::SetWordWrap)
		.add_property("maxlength", &ElementFormControlTextArea::GetMaxLength, &ElementFormControlTextArea::SetMaxLength)
		.ptr();
	
	// ElementFormControlSelect.
	SelectOptionProxy::InitialisePythonInterface();
	class_definitions["FormControlSelect"] = python::class_< ElementFormControlSelect, Core::Python::ElementWrapper< ElementFormControlSelect >, boost::noncopyable, python::bases< ElementFormControl > >("ElementFormControlSelect", python::init< const char* >())
		.def("Add", &ElementFormControlSelect::Add)
		.def("Add", &ElementInterface::Add)
		.def("Remove", &ElementFormControlSelect::Remove)
		.add_property("options", &ElementInterface::GetOptions)
		.add_property("selection", &ElementFormControlSelect::GetSelection, &ElementFormControlSelect::SetSelection)
		.ptr();

	// ElementFormControlDataSelect.
	class_definitions["FormControlDataSelect"] = python::class_< ElementFormControlDataSelect, Core::Python::ElementWrapper< ElementFormControlDataSelect >, boost::noncopyable, python::bases< ElementFormControlSelect > >("ElementFormControlDataSelect", python::init< const char* >())
		.def("SetDataSource", &ElementFormControlDataSelect::SetDataSource)
		.ptr();

	// ElementTabSet.
	void (ElementTabSet::*SetTab)(int, const Rocket::Core::String&) = &ElementTabSet::SetTab;
	void (ElementTabSet::*SetPanel)(int, const Rocket::Core::String&) = &ElementTabSet::SetPanel;
	class_definitions["TabSet"] = python::class_< ElementTabSet, Core::Python::ElementWrapper< ElementTabSet >, boost::noncopyable, python::bases< Core::Element > >("ElementTabSet", python::init< const char* >())
		.add_property("num_tabs", &ElementTabSet::GetNumTabs)
		.def("SetTab", SetTab)
		.def("SetPanel", SetPanel)
		.add_property("active_tab", &ElementTabSet::GetActiveTab, &ElementTabSet::SetActiveTab)
		.ptr();
}

void ElementInterface::InitialiseRocketInterface()
{	
	Core::Factory::RegisterElementInstancer("datagrid", new Core::Python::ElementInstancer( (*class_definitions.find("DataGrid")).second))->RemoveReference();
	Core::Factory::RegisterElementInstancer("datagridexpand", new Core::Python::ElementInstancer( (*class_definitions.find("DataGridExpand")).second))->RemoveReference();
	Core::Factory::RegisterElementInstancer("#rktctl_datagridrow", new Core::Python::ElementInstancer( (*class_definitions.find("DataGridRow")).second))->RemoveReference();
	Core::Factory::RegisterElementInstancer("#rktctl_datagridcell", new Core::Python::ElementInstancer( (*class_definitions.find("DataGridCell")).second))->RemoveReference();	
	
	Core::Factory::RegisterElementInstancer("form", new Core::Python::ElementInstancer((*class_definitions.find("Form")).second ))->RemoveReference();	
	Core::Factory::RegisterElementInstancer("input", new Core::Python::ElementInstancer((*class_definitions.find("FormControlInput")).second ))->RemoveReference();	
	Core::Factory::RegisterElementInstancer("textarea", new Core::Python::ElementInstancer((*class_definitions.find("FormControlTextArea")).second ))->RemoveReference();
	Core::Factory::RegisterElementInstancer("dataselect", new Core::Python::ElementInstancer((*class_definitions.find("FormControlDataSelect")).second ))->RemoveReference();
	Core::Factory::RegisterElementInstancer("select", new Core::Python::ElementInstancer((*class_definitions.find("FormControlSelect")).second ))->RemoveReference();
	Core::Factory::RegisterElementInstancer("tabset", new Core::Python::ElementInstancer((*class_definitions.find("TabSet")).second ))->RemoveReference();
}

// Sets the expanded state of a data grid row.
void ElementInterface::SetRowExpanded(ElementDataGridRow* element, bool row_expanded)
{
	if (row_expanded)
		element->ExpandRow();
	else
		element->CollapseRow();
}

// Returns the options proxy for a select element.
SelectOptionProxy ElementInterface::GetOptions(ElementFormControlSelect* element)
{
	return SelectOptionProxy(element);
}

// Override for ElementFormControlSelect's Add() without the last parameter.
int ElementInterface::Add(ElementFormControlSelect* element, const Rocket::Core::String& rml, const Rocket::Core::String& value)
{
	return element->Add(rml, value);
}

// Default parameter submit for forms
void ElementInterface::Submit(ElementForm* element)
{
	element->Submit();
}

bool ElementInterface::GetChecked(ElementFormControlInput* element)
{
	return element->HasAttribute("checked");
}

void ElementInterface::SetChecked(ElementFormControlInput* element, bool checked)
{
	if (checked)
		element->SetAttribute("checked", true);
	else
		element->RemoveAttribute("checked");
}

int ElementInterface::GetMaxLength(ElementFormControlInput* element)
{
	return element->GetAttribute<int>("maxlength", -1);
}

void ElementInterface::SetMaxLength(ElementFormControlInput* element, int max_length)
{
	element->SetAttribute("maxlength", max_length);
}

int ElementInterface::GetSize(ElementFormControlInput* element)
{
	return element->GetAttribute<int>("size", 20);
}

void ElementInterface::SetSize(ElementFormControlInput* element, int size)
{
	element->SetAttribute("size", size);
}

int ElementInterface::GetMin(ElementFormControlInput* element)
{
	return element->GetAttribute<int>("min", 0);
}

void ElementInterface::SetMin(ElementFormControlInput* element, int min)
{
	element->SetAttribute("min", min);
}

int ElementInterface::GetMax(ElementFormControlInput* element)
{
	return element->GetAttribute<int>("max", 100);
}

void ElementInterface::SetMax(ElementFormControlInput* element, int max)
{
	element->SetAttribute("max", max);
}

int ElementInterface::GetStep(ElementFormControlInput* element)
{
	return element->GetAttribute<int>("step", 1);
}

void ElementInterface::SetStep(ElementFormControlInput* element, int step)
{
	element->SetAttribute("step", step);
}

DataGridRowProxy ElementInterface::GetRows(ElementDataGrid* element)
{
	return DataGridRowProxy(element);
}

}
}
}
