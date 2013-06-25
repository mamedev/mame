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
#include "StyleSheetFactory.h"
#include <Rocket/Core/StyleSheet.h>
#include "StreamFile.h"
#include "StyleSheetNodeSelectorNthChild.h"
#include "StyleSheetNodeSelectorNthLastChild.h"
#include "StyleSheetNodeSelectorNthOfType.h"
#include "StyleSheetNodeSelectorNthLastOfType.h"
#include "StyleSheetNodeSelectorFirstChild.h"
#include "StyleSheetNodeSelectorLastChild.h"
#include "StyleSheetNodeSelectorFirstOfType.h"
#include "StyleSheetNodeSelectorLastOfType.h"
#include "StyleSheetNodeSelectorOnlyChild.h"
#include "StyleSheetNodeSelectorOnlyOfType.h"
#include "StyleSheetNodeSelectorEmpty.h"
#include <Rocket/Core/Log.h>

namespace Rocket {
namespace Core {

static StyleSheetFactory* instance = NULL;

StyleSheetFactory::StyleSheetFactory()
{
	ROCKET_ASSERT(instance == NULL);
	instance = this;
}

StyleSheetFactory::~StyleSheetFactory()
{
	instance = NULL;
}

bool StyleSheetFactory::Initialise()
{
	new StyleSheetFactory();

	instance->selectors["nth-child"] = new StyleSheetNodeSelectorNthChild();
	instance->selectors["nth-last-child"] = new StyleSheetNodeSelectorNthLastChild();
	instance->selectors["nth-of-type"] = new StyleSheetNodeSelectorNthOfType();
	instance->selectors["nth-last-of-type"] = new StyleSheetNodeSelectorNthLastOfType();
	instance->selectors["first-child"] = new StyleSheetNodeSelectorFirstChild();
	instance->selectors["last-child"] = new StyleSheetNodeSelectorLastChild();
	instance->selectors["first-of-type"] = new StyleSheetNodeSelectorFirstOfType();
	instance->selectors["last-of-type"] = new StyleSheetNodeSelectorLastOfType();
	instance->selectors["only-child"] = new StyleSheetNodeSelectorOnlyChild();
	instance->selectors["only-of-type"] = new StyleSheetNodeSelectorOnlyOfType();
	instance->selectors["empty"] = new StyleSheetNodeSelectorEmpty();

	return true;
}

void StyleSheetFactory::Shutdown()
{
	if (instance != NULL)
	{
		ClearStyleSheetCache();

		for (SelectorMap::iterator i = instance->selectors.begin(); i != instance->selectors.end(); ++i)
			delete (*i).second;

		delete instance;
	}
}

StyleSheet* StyleSheetFactory::GetStyleSheet(const String& sheet_name)
{
	// Look up the sheet definition in the cache
	StyleSheets::iterator itr = instance->stylesheets.find(sheet_name);
	if (itr != instance->stylesheets.end())
	{
		(*itr).second->AddReference();
		return (*itr).second;
	}

	// Don't currently have the sheet, attempt to load it
	StyleSheet* sheet = instance->LoadStyleSheet(sheet_name);
	if (sheet == NULL)
		return NULL;

	// Add it to the cache, and add a reference count so the cache will keep hold of it.
	instance->stylesheets[sheet_name] = sheet;
	sheet->AddReference();

	return sheet;
}

StyleSheet* StyleSheetFactory::GetStyleSheet(const StringList& sheets)
{
	// Generate a unique key for these sheets
	String combined_key;
	for (size_t i = 0; i < sheets.size(); i++)
	{		
		URL path(sheets[i]);
		combined_key += path.GetFileName();
	}

	// Look up the sheet definition in the cache.
	StyleSheets::iterator itr = instance->stylesheet_cache.find(combined_key);
	if (itr != instance->stylesheet_cache.end())
	{
		(*itr).second->AddReference();
		return (*itr).second;
	}

	// Load and combine the sheets.
	StyleSheet* sheet = NULL;
	for (size_t i = 0; i < sheets.size(); i++)
	{
		StyleSheet* sub_sheet = GetStyleSheet(sheets[i]);
		if (sub_sheet)
		{
			if (sheet)
			{
				StyleSheet* new_sheet = sheet->CombineStyleSheet(sub_sheet);
				sheet->RemoveReference();
				sub_sheet->RemoveReference();

				sheet = new_sheet;
			}
			else
				sheet = sub_sheet;
		}
		else
			Log::Message(Log::LT_ERROR, "Failed to load style sheet %s.", sheets[i].CString());
	}

	if (sheet == NULL)
		return NULL;

	// Add to cache, and a reference to the sheet to hold it in the cache.
	instance->stylesheet_cache[combined_key] = sheet;
	sheet->AddReference();
	return sheet;
}

// Clear the style sheet cache.
void StyleSheetFactory::ClearStyleSheetCache()
{
	for (StyleSheets::iterator i = instance->stylesheets.begin(); i != instance->stylesheets.end(); ++i)
		(*i).second->RemoveReference();

	for (StyleSheets::iterator i = instance->stylesheet_cache.begin(); i != instance->stylesheet_cache.end(); ++i)
		(*i).second->RemoveReference();

	instance->stylesheets.clear();
	instance->stylesheet_cache.clear();
}

// Returns one of the available node selectors.
StyleSheetNodeSelector* StyleSheetFactory::GetSelector(const String& name)
{
	size_t index = name.Find("(");
	SelectorMap::iterator i = instance->selectors.find(name.Substring(0, index));
	if (i == instance->selectors.end())
		return NULL;
	return (*i).second;
}

StyleSheet* StyleSheetFactory::LoadStyleSheet(const String& sheet)
{
	StyleSheet* new_style_sheet = NULL;

	// Open stream, construct new sheet and pass the stream into the sheet
	// TODO: Make this support ASYNC
	StreamFile* stream = new StreamFile();
	if (stream->Open(sheet))
	{
		new_style_sheet = new StyleSheet();
		if (!new_style_sheet->LoadStyleSheet(stream))
		{
			new_style_sheet->RemoveReference();
			new_style_sheet = NULL;
		}
	}

	stream->RemoveReference();
	return new_style_sheet;
}

}
}
