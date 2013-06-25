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
#include <Rocket/Core.h>
#include <algorithm>
#include "FileInterfaceDefault.h"
#include "GeometryDatabase.h"
#include "PluginRegistry.h"
#include "StyleSheetFactory.h"
#include "TemplateCache.h"
#include "TextureDatabase.h"

namespace Rocket {
namespace Core {

// Rocket's renderer interface.
static RenderInterface* render_interface = NULL;
/// Rocket's system interface.
static SystemInterface* system_interface = NULL;
// Rocket's file I/O interface.
FileInterface* file_interface =  NULL;
#ifndef ROCKET_NO_FILE_INTERFACE_DEFAULT
static FileInterfaceDefault file_interface_default;
#endif
static bool initialised = false;

typedef std::map< String, Context* > ContextMap;
static ContextMap contexts;

#ifndef ROCKET_VERSION
	#define ROCKET_VERSION "custom"
#endif

/**
	A 'plugin' for unobtrusively intercepting the destruction of contexts.
 */

class PluginContextRelease : public Plugin
{
	public:
		virtual void OnShutdown()
		{
			delete this;
		}

		virtual void OnContextDestroy(Context* context)
		{
			contexts.erase(context->GetName());
		}
};

bool Initialise()
{
	// Check for valid interfaces, or install default interfaces as appropriate.
	if (system_interface == NULL)
	{	
		Log::Message(Log::LT_ERROR, "No system interface set!");
		return false;
	}

	if (file_interface == NULL)
	{		
#ifndef ROCKET_NO_FILE_INTERFACE_DEFAULT
		file_interface = &file_interface_default;
		file_interface->AddReference();
#else
		Log::Message(Log::LT_ERROR, "No file interface set!");
		return false;
#endif
	}

	Log::Initialise();

	TextureDatabase::Initialise();

	FontDatabase::Initialise();

	StyleSheetSpecification::Initialise();
	StyleSheetFactory::Initialise();

	TemplateCache::Initialise();

	Factory::Initialise();

	// Notify all plugins we're starting up.
	PluginRegistry::RegisterPlugin(new PluginContextRelease());
	PluginRegistry::NotifyInitialise();

	initialised = true;

	return true;
}

void Shutdown()
{
	// Notify all plugins we're being shutdown.
	PluginRegistry::NotifyShutdown();

	// Release all remaining contexts.
	for (ContextMap::iterator itr = contexts.begin(); itr != contexts.end(); ++itr)
		Core::Log::Message(Log::LT_WARNING, "Context '%s' still active on shutdown.", (*itr).first.CString());
	contexts.clear();

	TemplateCache::Shutdown();
	StyleSheetFactory::Shutdown();
	StyleSheetSpecification::Shutdown();
	FontDatabase::Shutdown();
	TextureDatabase::Shutdown();
	Factory::Shutdown();

	Log::Shutdown();

	initialised = false;

	if (render_interface != NULL)
		render_interface->RemoveReference();

	if (file_interface != NULL)
		file_interface->RemoveReference();

	if (system_interface != NULL)
		system_interface->RemoveReference();
	
	render_interface = NULL;
	file_interface = NULL;
	system_interface = NULL;
}

// Returns the version of this Rocket library.
String GetVersion()
{
	return ROCKET_VERSION;
}

// Sets the interface through which all Rocket messages will be routed.
void SetSystemInterface(SystemInterface* _system_interface)
{
	if (system_interface == _system_interface)
		return;

	if (system_interface != NULL)
		system_interface->RemoveReference();

	system_interface = _system_interface;
	if (system_interface != NULL)
		system_interface->AddReference();
}

// Returns Rocket's system interface.
SystemInterface* GetSystemInterface()
{
	return system_interface;
}

// Sets the interface through which all rendering requests are made.
void SetRenderInterface(RenderInterface* _render_interface)
{
	if (render_interface == _render_interface)
		return;

	if (render_interface != NULL)
		render_interface->RemoveReference();

	render_interface = _render_interface;
	if (render_interface != NULL)
		render_interface->AddReference();
}

// Returns Rocket's render interface.
RenderInterface* GetRenderInterface()
{
	return render_interface;
}

// Sets the interface through which all file I/O requests are made.
void SetFileInterface(FileInterface* _file_interface)
{
	if (file_interface == _file_interface)
		return;

	if (file_interface != NULL)
		file_interface->RemoveReference();

	file_interface = _file_interface;
	if (file_interface != NULL)
		file_interface->AddReference();
}

// Returns Rocket's file interface.
FileInterface* GetFileInterface()
{
	return file_interface;
}

// Creates a new element context.
Context* CreateContext(const String& name, const Vector2i& dimensions, RenderInterface* custom_render_interface)
{
	if (!initialised)
		return NULL;

	if (custom_render_interface == NULL &&
		render_interface == NULL)
		Log::Message(Log::LT_WARNING, "Failed to create context '%s', no render interface specified and no default render interface exists.", name.CString());

	if (GetContext(name) != NULL)
	{
		Log::Message(Log::LT_WARNING, "Failed to create context '%s', context already exists.", name.CString());
		return NULL;
	}

	Context* new_context = Factory::InstanceContext(name);
	if (new_context == NULL)
	{
		Log::Message(Log::LT_WARNING, "Failed to instance context '%s', instancer returned NULL.", name.CString());
		return NULL;
	}

	// Set the render interface on the context, and add a reference onto it.
	if (custom_render_interface)
		new_context->render_interface = custom_render_interface;
	else
		new_context->render_interface = render_interface;
	new_context->render_interface->AddReference();

	new_context->SetDimensions(dimensions);
	contexts[name] = new_context;

	PluginRegistry::NotifyContextCreate(new_context);

	return new_context;
}

// Fetches a previously constructed context by name.
Context* GetContext(const String& name)
{
	ContextMap::iterator i = contexts.find(name);
	if (i == contexts.end())
		return NULL;

	return (*i).second;
}

// Fetches a context by index.
Context* GetContext(int index)
{
	ContextMap::iterator i = contexts.begin();
	int count = 0;

	if (index >= GetNumContexts())
		index = GetNumContexts() - 1;

	while (count < index)
	{
		++i;
		++count;
	}

	if (i == contexts.end())
		return NULL;

	return (*i).second;
}

// Returns the number of active contexts.
int GetNumContexts()
{
	return (int) contexts.size();
}

// Registers a generic rocket plugin
void RegisterPlugin(Plugin* plugin)
{
	if (initialised)
		plugin->OnInitialise();

	PluginRegistry::RegisterPlugin(plugin);
}

// Forces all compiled geometry handles generated by libRocket to be released.
void ReleaseCompiledGeometries()
{
	GeometryDatabase::ReleaseGeometries();
}

// Forces all texture handles loaded and generated by libRocket to be released.
void ReleaseTextures()
{
	TextureDatabase::ReleaseTextures();
}

}
}
