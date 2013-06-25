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
#include "TemplateCache.h"
#include "StreamFile.h"
#include "Template.h"
#include <Rocket/Core/Log.h>

namespace Rocket {
namespace Core {

static TemplateCache* instance = NULL;

TemplateCache::TemplateCache()
{
	ROCKET_ASSERT(instance == NULL);
	instance = this;
}

TemplateCache::~TemplateCache()
{
	for (Templates::iterator itr = instance->templates.begin(); itr != instance->templates.end(); ++itr)
	{
		delete (*itr).second;
	}

	instance = NULL;
}

bool TemplateCache::Initialise()
{
	new TemplateCache();

	return true;
}

void TemplateCache::Shutdown()
{
	delete instance;
}

Template* TemplateCache::LoadTemplate(const String& name)
{
	// Check if the template is already loaded
	Templates::iterator itr = instance->templates.find(name);
	if (itr != instance->templates.end())
		return (*itr).second;

	// Nope, we better load it
	Template* new_template = NULL;
	StreamFile* stream = new StreamFile();
	if (stream->Open(name))
	{
		new_template = new Template();
		if (!new_template->Load(stream))
		{
			Log::Message(Log::LT_ERROR, "Failed to load template %s.", name.CString());
			delete new_template;
			new_template = NULL;
		}
		else if (new_template->GetName().Empty())
		{
			Log::Message(Log::LT_ERROR, "Failed to load template %s, template is missing its name.", name.CString());
			delete new_template;
			new_template = NULL;
		}
		else
		{
			instance->templates[name] = new_template;
			instance->template_ids[new_template->GetName()] = new_template;
		}
	}
	else
	{
		Log::Message(Log::LT_ERROR, "Failed to open template file %s.", name.CString());		
	}
	stream->RemoveReference();

	return new_template;
}

Template* TemplateCache::GetTemplate(const String& name)
{
	// Check if the template is already loaded
	Templates::iterator itr = instance->template_ids.find(name);
	if (itr != instance->template_ids.end())
		return (*itr).second;

	return NULL;
}

}
}
