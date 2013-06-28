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
#include "png.h"


class ShellSystemInterface : public Rocket::Core::SystemInterface
{
private:
	// internal state
	running_machine &   m_machine;                          // reference to our machine

public:
	ShellSystemInterface(running_machine &machine)
		: m_machine(machine)
	{
	}
	
	// getters
	running_machine &machine() const { return m_machine; }	
	
	/// Get the number of seconds elapsed since the start of the application
	/// @returns Seconds elapsed
	virtual float GetElapsedTime() { return machine().time().as_double(); }
};

#define UI_BORDER_COLOR         MAKE_ARGB(0xff,0xff,0xff,0xff)

class ShellRenderInterfaceSystem : public Rocket::Core::RenderInterface
{
private:
	// internal state
	running_machine &   m_machine;                          // reference to our machine	
	
public:
	ShellRenderInterfaceSystem(running_machine &machine)
		: m_machine(machine)
	{
	}
	
	// getters
	running_machine &machine() const { return m_machine; }	

	/// Called by Rocket when it wants to render geometry that it does not wish to optimise.
	virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{

		for (int i = 0; i < num_indices/6; i++)
		{			
			render_texture *hilight_texture = machine().render().texture_alloc();
			rectangle bit = ((bitmap_rgb32 *)texture)->cliprect();
			rectangle myrect = ((bitmap_rgb32 *)texture)->cliprect();
			int p1 = indices[i*6+0];
			int p2 = indices[i*6+5];

			myrect.min_x = bit.max_x * vertices[p1].tex_coord.x;
			myrect.min_y = bit.max_y * vertices[p1].tex_coord.y;
			myrect.max_x = bit.max_x * vertices[p2].tex_coord.x;
			myrect.max_y = bit.max_y * vertices[p2].tex_coord.y;
			if (myrect.min_x > myrect.max_x) { int t = myrect.max_x; myrect.max_x = myrect.min_x; myrect.min_x = t; } // this should be flipping
			if (myrect.min_y > myrect.max_y) { int t = myrect.max_y; myrect.max_y = myrect.min_y; myrect.min_y = t; }

			hilight_texture->set_bitmap(*((bitmap_rgb32 *)texture), myrect, TEXFORMAT_ARGB32);		
			
			machine().render().ui_container().add_quad((vertices[p1].position.x+translation.x)/1280,(vertices[p1].position.y+translation.y)/960, (vertices[p2].position.x+translation.x)/1280,(vertices[p2].position.y+translation.y)/960, UI_BORDER_COLOR, hilight_texture,PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			//machine().render().texture_free(hilight_texture);
		}						
		
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

	
	// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
	#pragma pack(1) 
	struct TGAHeader 
	{
		char  idLength;
		char  colourMapType;
		char  dataType;
		short int colourMapOrigin;
		short int colourMapLength;
		char  colourMapDepth;
		short int xOrigin;
		short int yOrigin;
		short int width;
		short int height;
		char  bitsPerPixel;
		char  imageDescriptor;
	};
	// Restore packing
	#pragma pack()
	
	/// Called by Rocket when a texture is required by the library.
	virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
	{
		
		
		emu_file file("", OPEN_FLAG_READ);
		file_error filerr = file.open(source.CString());	
		if(filerr != FILERR_NONE)
		{
			return false;
		}		
		
		astring fname(source.CString());
		if (fname.find(0, ".png") != -1)
		{
			bitmap_argb32 bitmap;
			png_read_bitmap(file, bitmap);				
			bitmap_rgb32 *hilight_bitmap = auto_bitmap_rgb32_alloc(machine(), bitmap.width(), bitmap.height());
			for (int y = 0; y < bitmap.height(); ++y)
			{
				for (int x = 0; x < bitmap.width(); ++x)
				{
					hilight_bitmap->pix32(y, x) = bitmap.pix32(y,x);
				}
			}
			
			texture_handle = (Rocket::Core::TextureHandle)hilight_bitmap;			
			texture_dimensions.x = bitmap.width();
			texture_dimensions.y = bitmap.height();
			return true;
		}
		else 
		{
		
			file.seek(0, SEEK_END);
			size_t buffer_size = file.tell();
			file.seek(0, SEEK_SET);
			
			char* buffer = new char[buffer_size];
			file.read(buffer, buffer_size);
			file.close();

			TGAHeader header;
			memcpy(&header, buffer, sizeof(TGAHeader));
			
			int color_mode = header.bitsPerPixel / 8;
			int image_size = header.width * header.height * 4; // We always make 32bit textures 
			
			if (header.dataType != 2)
			{
				Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
				printf("Only 24/32bit uncompressed TGAs are supported.\n");
				return false;
			}
			
			// Ensure we have at least 3 colors
			if (color_mode < 3)
			{
				Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "Only 24 and 32bit textures are supported");
				printf("Only 24 and 32bit textures are supported\n");
				return false;
			}
			
			const char* image_src = buffer + sizeof(TGAHeader);
			unsigned char* image_dest = new unsigned char[image_size];
			
			// Targa is BGR, swap to RGB and flip Y axis
			for (long y = 0; y < header.height; y++)
			{
				long read_index = y * header.width * color_mode;
				long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * color_mode;
				for (long x = 0; x < header.width; x++)
				{
					image_dest[write_index] = image_src[read_index+2];
					image_dest[write_index+1] = image_src[read_index+1];
					image_dest[write_index+2] = image_src[read_index];
					if (color_mode == 4)
						image_dest[write_index+3] = image_src[read_index+3];
					else
						image_dest[write_index+3] = 255;
					
					write_index += 4;
					read_index += color_mode;
				}
			}

			texture_dimensions.x = header.width;
			texture_dimensions.y = header.height;
			
			bool success = GenerateTexture(texture_handle, image_dest, texture_dimensions);
			
			delete [] image_dest;
			delete [] buffer;
			return success;
		}
		
	}
	/// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
	virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
	{
		/* create a texture for hilighting items */
		bitmap_rgb32 *hilight_bitmap = auto_bitmap_rgb32_alloc(machine(), source_dimensions.x, source_dimensions.y);
		for (int y = 0; y < source_dimensions.y; ++y)
		{
			for (int x = 0; x < source_dimensions.x; ++x)
			{
				const Rocket::Core::byte* source_pixel = source + (source_dimensions.x * 4 * y) + (x * 4);				
				hilight_bitmap->pix32(y, x) = MAKE_ARGB(source_pixel[3],source_pixel[0],source_pixel[1],source_pixel[2]);
			}
		}
		//render_texture *hilight_texture = machine().render().texture_alloc();
		//hilight_texture->set_bitmap(*hilight_bitmap, hilight_bitmap->cliprect(), TEXFORMAT_ARGB32);		
		//texture_handle = (Rocket::Core::TextureHandle)hilight_texture;
		texture_handle = (Rocket::Core::TextureHandle)hilight_bitmap;
		return true;
	}
	/// Called by Rocket when a loaded texture is no longer required.
	virtual void ReleaseTexture(Rocket::Core::TextureHandle texture_handle)
	{
		//machine().render().texture_free((render_texture *)texture_handle);
	}
};

ShellRenderInterfaceSystem *system_renderer = NULL;
ShellSystemInterface *system_interface = NULL;
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
	system_renderer = new ShellRenderInterfaceSystem(machine());
	Rocket::Core::SetRenderInterface(system_renderer); 
	
	system_interface = new ShellSystemInterface(machine());
	Rocket::Core::SetSystemInterface(system_interface);
	
	Rocket::Core::Initialise();
	// Initialise the Rocket Controls library.
	Rocket::Controls::Initialise();
	
	// Create the main Rocket context and set it on the shell's input layer.
	context = Rocket::Core::CreateContext("main", Rocket::Core::Vector2i(1280, 960));
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
	/* always start clean */
	machine().render().ui_container().empty();
	
	context->Update();
	context->Render();
}


void gui_engine::loadFonts(const char* directory)
{
	Rocket::Core::String font_names[4];
	font_names[0] = "Delicious-Roman.otf";
	font_names[1] = "Delicious-Italic.otf";
	font_names[2] = "Delicious-Bold.otf";
	font_names[3] = "Delicious-BoldItalic.otf";

	for (int i = 0; i < sizeof(font_names) / sizeof(Rocket::Core::String); i++)
	{
		Rocket::Core::FontDatabase::LoadFontFace(Rocket::Core::String(directory) + font_names[i]);
	}
}

void gui_engine::loadDocument(const char *filename)
{
	Rocket::Core::ElementDocument* document = context->LoadDocument(filename);
	if (document != NULL)
	{
		document->Show();
		document->RemoveReference();
	}
}
