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

#include "XMLNodeHandlerDataGrid.h"
#include <Rocket/Core/StreamMemory.h>
#include <Rocket/Core/Log.h>
#include <Rocket/Core/Factory.h>
#include <Rocket/Core/XMLParser.h>
#include <Rocket/Controls/ElementDataGrid.h>

namespace Rocket {
namespace Controls {

XMLNodeHandlerDataGrid::XMLNodeHandlerDataGrid()
{
}

XMLNodeHandlerDataGrid::~XMLNodeHandlerDataGrid()
{
}

Core::Element* XMLNodeHandlerDataGrid::ElementStart(Core::XMLParser* parser, const Rocket::Core::String& name, const Rocket::Core::XMLAttributes& attributes)
{
	Core::Element* element = NULL;
	Core::Element* parent = parser->GetParseFrame()->element;

	ROCKET_ASSERT(name == "datagrid" ||
			   name == "col");

	if (name == "datagrid")
	{
		// Attempt to instance the grid.
		element = Core::Factory::InstanceElement(parent, name, name, attributes);
		ElementDataGrid* grid = dynamic_cast< ElementDataGrid* >(element);
		if (grid == NULL)
		{
			if (element != NULL)
				element->RemoveReference();

			Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Instancer failed to create data grid for tag %s.", name.CString());
			return NULL;
		}

		// Set the data source and table on the data grid.
		Rocket::Core::String data_source = attributes.Get< Rocket::Core::String >("source", "");
		grid->SetDataSource(data_source);

		parent->AppendChild(grid);
		grid->RemoveReference();

		// Switch to this handler for all columns.
		parser->PushHandler("datagrid");
	}
	else if (name == "col")
	{
		// Make a new node handler to handle the header elements.		
		element = Core::Factory::InstanceElement(parent, "datagridcolumn", "datagridcolumn", attributes);
		if (element == NULL)
			return NULL;

		ElementDataGrid* grid = dynamic_cast< ElementDataGrid* >(parent);
		if (grid != NULL)
		{
			grid->AddColumn(attributes.Get< Rocket::Core::String >("fields", ""), attributes.Get< Rocket::Core::String >("formatter", ""), attributes.Get< float >("width", 0), element);
			element->RemoveReference();
		}

		// Switch to element handler for all children.
		parser->PushDefaultHandler();
	}
	else
	{
		ROCKET_ERROR;
	}

	return element;
}

bool XMLNodeHandlerDataGrid::ElementEnd(Core::XMLParser* ROCKET_UNUSED(parser), const Rocket::Core::String& ROCKET_UNUSED(name))
{
	return true;
}

bool XMLNodeHandlerDataGrid::ElementData(Core::XMLParser* parser, const Rocket::Core::String& data)
{
	Core::Element* parent = parser->GetParseFrame()->element;

	// Parse the text into the parent element.
	return Core::Factory::InstanceElementText(parent, data);
}

void XMLNodeHandlerDataGrid::Release()
{
	delete this;
}

}
}
