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

#ifndef ROCKETCONTROLSPYTHONELEMENTINTERFACE_H
#define ROCKETCONTROLSPYTHONELEMENTINTERFACE_H

#include "SelectOptionProxy.h"
#include "DataGridRowProxy.h"

namespace Rocket {
namespace Controls {

class ElementDataGridRow;
class ElementFormControlSelect;

namespace Python {

/**
	Python interface to the element classes
	defined in the Rocket Controls library

	@author Lloyd Weehuizen
 */

class ElementInterface
{
public:
	/// Initialises the python element interface
	static void InitialisePythonInterface();
	/// Initialise the rocket element interface
	static void InitialiseRocketInterface();

private:
	// Sets the expanded state of a data grid row.
	static void SetRowExpanded(ElementDataGridRow* element, bool row_expanded);
	// Returns the options proxy for a select element.
	static SelectOptionProxy GetOptions(ElementFormControlSelect* element);
	// Override for ElementFormControlSelect's Add() without the last parameter.
	static int Add(ElementFormControlSelect* element, const Rocket::Core::String& rml, const Rocket::Core::String& value);

	// Default parameter submit for forms
	static void Submit(ElementForm* element);

	// Methods for simplyfing access to input element attributes
	static bool GetChecked(ElementFormControlInput* element);
	static void SetChecked(ElementFormControlInput* element, bool checked);
	
	static int GetMaxLength(ElementFormControlInput* element);
	static void SetMaxLength(ElementFormControlInput* element, int max_length);
	
	static int GetSize(ElementFormControlInput* element);
	static void SetSize(ElementFormControlInput* element, int size);
	
	static int GetMax(ElementFormControlInput* element);
	static void SetMax(ElementFormControlInput* element, int max);
	
	static int GetMin(ElementFormControlInput* element);
	static void SetMin(ElementFormControlInput* element, int min);

	static int GetStep(ElementFormControlInput* element);
	static void SetStep(ElementFormControlInput* element, int step);

	static DataGridRowProxy GetRows(ElementDataGrid* element);
};

}
}
}

#endif
