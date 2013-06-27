/***************************************************************************

    guiengine.c

    GUI engine  of the core MAME system.

****************************************************************************

    Copyright Miodrag Milanovic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY MIODRAG MILANOVIC ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL MIODRAG MILANOVIC BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/
#include <Rocket/Core.h>
#include <Rocket/Debugger.h>
#include <Rocket/Controls.h>

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"


class ShellSystemInterface : public Rocket::Core::SystemInterface
{
public:
	/// Get the number of seconds elapsed since the start of the application
	/// @returns Seconds elapsed
	virtual float GetElapsedTime() { return 0; }
};

class ShellRenderInterfaceOpenGL : public Rocket::Core::RenderInterface
{
public:
	ShellRenderInterfaceOpenGL()
	{
	}

	/// Called by Rocket when it wants to render geometry that it does not wish to optimise.
	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{
	}

	/// Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
	virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture)
	{
		return (Rocket::Core::CompiledGeometryHandle) NULL;
	}

	/// Called by Rocket when it wants to render application-compiled geometry.
	virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
	{
	}
	
	/// Called by Rocket when it wants to release application-compiled geometry.
	virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
	}

	/// Called by Rocket when it wants to enable or disable scissoring to clip content.
	virtual void EnableScissorRegion(bool enable)
	{
	}
	/// Called by Rocket when it wants to change the scissor region.
	virtual void SetScissorRegion(int x, int y, int width, int height)
	{
	}

	/// Called by Rocket when a texture is required by the library.
	virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
	{
		return true;
	}
	/// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
	{
		return true;
	}
	/// Called by Rocket when a loaded texture is no longer required.
	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
	{
	}
};

ShellRenderInterfaceOpenGL opengl_renderer;
ShellSystemInterface system_interface;
Rocket::Core::Context* context = NULL;

//**************************************************************************
//  GUI ENGINE
//**************************************************************************

//-------------------------------------------------
//  gui_engine - constructor
//-------------------------------------------------

gui_engine::gui_engine(running_machine &machine)
	: m_machine(machine)
{
	context = NULL;	
}

//-------------------------------------------------
//  ~gui_engine - destructor
//-------------------------------------------------

gui_engine::~gui_engine()
{
	shutdown();
}

//-------------------------------------------------
//  initialize - initialize lua hookup to emu engine
//-------------------------------------------------

void gui_engine::initialize()
{
	// Rocket initialisation.
	Rocket::Core::SetRenderInterface(&opengl_renderer); 
	
	Rocket::Core::SetSystemInterface(&system_interface);
	
	Rocket::Core::Initialise();
	// Initialise the Rocket Controls library.
	Rocket::Controls::Initialise();
	
	// Create the main Rocket context and set it on the shell's input layer.
	context = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(1920, 1080));
	if (context == NULL)
	{
		Rocket::Core::Shutdown();
		printf("Failed");
		return;
	}
	Rocket::Debugger::Initialise(context);

	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(gui_engine::shutdown), this));
}

//-------------------------------------------------
//  shutdown - close and cleanup of gui engine
//-------------------------------------------------

void gui_engine::shutdown()
{
	// Shutdown Rocket.
	if (context!=NULL)
	{
		context->RemoveReference();
		Rocket::Core::Shutdown();	
		context = NULL;
	}
}

void gui_engine::update()
{
	context->Update();
	context->Render();
}
