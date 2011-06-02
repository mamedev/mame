//============================================================
//
//  drawd3d.c - Win32 Direct3D HLSL implementation
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

// Useful info:
//  Windows XP/2003 shipped with DirectX 8.1
//  Windows 2000 shipped with DirectX 7a
//  Windows 98SE shipped with DirectX 6.1a
//  Windows 98 shipped with DirectX 5
//  Windows NT shipped with DirectX 3.0a
//  Windows 95 shipped with DirectX 2

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#undef interface

// MAME headers
#include "emu.h"
#include "render.h"
#include "ui.h"
#include "rendutil.h"
#include "options.h"
#include "emuopts.h"
#include "aviio.h"
#include "png.h"

// MAMEOS headers
#include "d3dintf.h"
#include "winmain.h"
#include "window.h"
#include "config.h"
#include "strconv.h"
#include "d3dcomm.h"
#include "drawd3d.h"



//============================================================
//  GLOBALS
//============================================================

static hlsl_options g_hlsl_presets[6] =
{
	{ 0.00f,   1,   1, 0.1875f, 0.1875f,
	  0.03f, 0.03f,
	  0.25f, 1.0f, 1.0f, 0.6f, 0.0f,
	  0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 0.0f, 0.0f,
	  1.1f, 1.1f, 1.1f,
	  0.9f, 0.9f, 0.9f,
	  0.05f, 0.05f, 0.05f,
	  0.45f, 0.45f, 0.45f,
	  1.4f },
	{ 0.15f, 512, 384, 0.1875f, 0.1875f,
	  0.03f, 0.03f,
	  0.25f, 1.0f, 1.0f, 0.6f, 0.0f,
	  0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 0.0f, 0.0f,
	  1.1f, 1.1f, 1.1f,
	  0.9f, 0.9f, 0.9f,
	  0.05f, 0.05f, 0.05f,
	  0.45f, 0.45f, 0.45f,
	  1.4f },
	{ 0.15f, 512, 384, 0.1875f, 0.1875f,
	  0.03f, 0.03f,
	  0.25f, 1.0f, 1.0f, 0.6f, 0.0f,
	  0.0f, 0.0f,
	  0.4f, 0.1f, 0.0, 0.0f,-0.25f,-0.3f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 0.0f, 0.0f,
	  1.1f, 1.1f, 1.1f,
	  0.9f, 0.9f, 0.9f,
	  0.05f, 0.05f, 0.05f,
	  0.45f, 0.45f, 0.45f,
	  1.4f },
	{ 0.15f, 512, 384, 0.1875f, 0.1875f,
	  0.03f, 0.03f,
	  0.25f, 1.0f, 1.0f, 0.6f, 0.0f,
	  0.0f, 0.0f,
	  0.4f, 0.1f, 0.0, 0.0f,-0.25f,-0.3f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 0.0f, 0.0f,
	  1.0f, 1.0f, 1.0f,
	  1.0f, 1.0f, 1.0f,
	  0.05f, 0.05f, 0.05f,
	  0.45f, 0.45f, 0.45f,
	  1.0f },
	{ 0.15f, 512, 384, 0.1875f, 0.1875f,
	  0.03f, 0.03f,
	  0.25f, 1.0f, 1.0f, 0.6f, 0.0f,
	  0.0f, 0.0f,
	  0.4f, 0.1f, 0.0, 0.0f,-0.25f,-0.3f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 0.0f, 0.0f,
	  1.0f, 1.0f, 1.0f,
	  1.0f, 1.0f, 1.0f,
	  0.05f, 0.05f, 0.05f,
	  0.45f, 0.45f, 0.45f,
	  0.8f },
	{ 0.15f, 512, 384, 0.1875f, 0.1875f,
	  0.03f, 0.03f,
	  0.25f, 1.0f, 1.0f, 0.6f, 0.0f,
	  0.0f, 0.0f,
	  0.4f, 0.1f, 0.0, 0.0f,-0.25f,-0.3f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  0.7f, 0.3f, 0.0f, 0.0f, 0.7f, 0.3f, 0.3f, 0.0f, 0.7f,
	  0.0f, 0.0f, 0.0f,
	  1.0f, 1.0f, 1.0f,
	  1.0f, 1.0f, 1.0f,
	  0.05f, 0.05f, 0.05f,
	  0.45f, 0.45f, 0.45f,
	  0.8f }
};

static slider_state *g_slider_list;



//============================================================
//  PROTOTYPES
//============================================================

static file_error open_next(d3d_info *d3d, emu_file &file, const char *extension, int idx);



//============================================================
//  hlsl_info constructor
//============================================================

hlsl_info::hlsl_info()
{
	master_enable = false;
	yiq_enable = false;
	prescale_size = 1;
	preset = -1;
	shadow_bitmap = NULL;
	shadow_texture = NULL;
	registered_targets = 0;
	options = NULL;

}



//============================================================
//  hlsl_info destructor
//============================================================

hlsl_info::~hlsl_info()
{
	master_enable = false;
	yiq_enable = false;
	prescale_size = 1;
	preset = -1;
	shadow_bitmap = NULL;
	shadow_texture = NULL;
	registered_targets = 0;
	options = NULL;

}



//============================================================
//  hlsl_info::window_save
//============================================================

void hlsl_info::window_save()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	HRESULT result = (*d3dintf->device.create_texture)(d3d->device, snap_width, snap_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &snap_copy_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init system-memory target for HLSL snapshot (%08x), bailing\n", (UINT32)result);
		return;
	}
	(*d3dintf->texture.get_surface_level)(snap_copy_texture, 0, &snap_copy_target);

	result = (*d3dintf->device.create_texture)(d3d->device, snap_width, snap_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &snap_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init video-memory target for HLSL snapshot (%08x), bailing\n", (UINT32)result);
		return;
	}
	(*d3dintf->texture.get_surface_level)(snap_texture, 0, &snap_target);

	render_snap = true;
	snap_rendered = false;
}



//============================================================
//  hlsl_info::window_record
//============================================================

void hlsl_info::window_record()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	windows_options &options = downcast<windows_options &>(window->machine().options());
	const char *filename = options.d3d_hlsl_write();

	if (avi_output_file != NULL)
		end_avi_recording();
	else if (filename[0] != 0)
		begin_avi_recording(filename);
}


//============================================================
//  hlsl_info::avi_update_snap
//============================================================

void hlsl_info::avi_update_snap(d3d_surface *surface)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	D3DLOCKED_RECT rect;

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (avi_snap == NULL || (int)snap_width != avi_snap->width || (int)snap_height != avi_snap->height)
	{
		if (avi_snap != NULL)
		{
			auto_free(window->machine(), avi_snap);
		}
		avi_snap = auto_alloc(window->machine(), bitmap_t((int)snap_width, (int)snap_height, BITMAP_FORMAT_RGB32));
	}

	// copy the texture
	HRESULT result = (*d3dintf->device.get_render_target_data)(d3d->device, surface, avi_copy_surface);
	if (result != D3D_OK)
	{
		printf("Couldn't copy (%08x)\n", (UINT32)result);
		return;
	}

	// lock the texture
	result = (*d3dintf->surface.lock_rect)(avi_copy_surface, &rect, NULL, D3DLOCK_DISCARD);
	if (result != D3D_OK)
	{
		printf("Couldn't lock (%08x)\n", (UINT32)result);
		return;
	}

	// loop over Y
	for (int srcy = 0; srcy < (int)snap_height; srcy++)
	{
		BYTE *src = (BYTE *)rect.pBits + srcy * rect.Pitch;
		BYTE *dst = (BYTE *)avi_snap->base + srcy * avi_snap->rowpixels * 4;

		for(int x = 0; x < snap_width; x++)
		{
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
			*dst++ = *src++;
		}
	}

	// unlock
	result = (*d3dintf->surface.unlock_rect)(avi_copy_surface);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during texture unlock_rect call\n", (int)result);
}



//============================================================
//  hlsl_render_snapshot
//============================================================

void hlsl_info::render_snapshot(d3d_surface *surface)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	D3DLOCKED_RECT rect;

	render_snap = false;

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (avi_snap == NULL || snap_width != (avi_snap->width / 2) || snap_height != (avi_snap->height / 2))
	{
		if (avi_snap != NULL)
		{
			auto_free(window->machine(), avi_snap);
		}
		avi_snap = auto_alloc(window->machine(), bitmap_t(snap_width / 2, snap_height / 2, BITMAP_FORMAT_RGB32));
	}

	// copy the texture
	HRESULT result = (*d3dintf->device.get_render_target_data)(d3d->device, surface, snap_copy_target);
	if (result != D3D_OK)
	{
		printf("Couldn't copy (%08x)\n", (UINT32)result);
		return;
	}

	// lock the texture
	result = (*d3dintf->surface.lock_rect)(snap_copy_target, &rect, NULL, D3DLOCK_DISCARD);
	if (result != D3D_OK)
	{
		printf("Couldn't lock (%08x)\n", (UINT32)result);
		return;
	}

	for(int cy = 0; cy < 2; cy++)
	{
		for(int cx = 0; cx < 2; cx++)
		{
			// loop over Y
			for (int srcy = 0; srcy < snap_height / 2; srcy++)
			{
				int toty = (srcy + cy * (snap_height / 2));
				int totx = cx * (snap_width / 2);
				BYTE *src = (BYTE *)rect.pBits + toty * rect.Pitch + totx * 4;
				BYTE *dst = (BYTE *)avi_snap->base + srcy * avi_snap->rowpixels * 4;

				for(int x = 0; x < snap_width / 2; x++)
				{
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = *src++;
				}
			}

			int idx = cy * 2 + cx;

			emu_file file(window->machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			file_error filerr = open_next(d3d, file, "png", idx);
			if (filerr != FILERR_NONE)
				return;

			// add two text entries describing the image
			astring text1(APPNAME, " ", build_version);
			astring text2(window->machine().system().manufacturer, " ", window->machine().system().description);
			png_info pnginfo = { 0 };
			png_add_text(&pnginfo, "Software", text1);
			png_add_text(&pnginfo, "System", text2);

			// now do the actual work
			png_error error = png_write_bitmap(file, &pnginfo, avi_snap, 1 << 24, NULL);
			if (error != PNGERR_NONE)
				mame_printf_error("Error generating PNG for HLSL snapshot: png_error = %d\n", error);

			// free any data allocated
			png_free(&pnginfo);
		}
	}

	// unlock
	result = (*d3dintf->surface.unlock_rect)(snap_copy_target);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during texture unlock_rect call\n", (int)result);

	if(snap_texture != NULL)
	{
		(*d3dintf->texture.release)(snap_texture);
		snap_texture = NULL;
	}

	if(snap_target != NULL)
	{
		(*d3dintf->surface.release)(snap_target);
		snap_target = NULL;
	}

	if(snap_copy_texture != NULL)
	{
		(*d3dintf->texture.release)(snap_copy_texture);
		snap_copy_texture = NULL;
	}

	if(snap_copy_target != NULL)
	{
		(*d3dintf->surface.release)(snap_copy_target);
		snap_copy_target = NULL;
	}
}


//============================================================
//  hlsl_info::record_texture
//============================================================

void hlsl_info::record_texture()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_surface *surface = avi_final_target;

	// ignore if nothing to do
	if (avi_output_file == NULL || surface == NULL)
		return;

	// get the current time
	attotime curtime = window->machine().time();

	avi_update_snap(surface);

	// loop until we hit the right time
	while (avi_next_frame_time <= curtime)
	{
		// handle an AVI recording
		// write the next frame
		avi_error avierr = avi_append_video_frame_rgb32(avi_output_file, avi_snap);
		if (avierr != AVIERR_NONE)
		{
			end_avi_recording();
			return;
		}

		// advance time
		avi_next_frame_time += avi_frame_period;
		avi_frame++;
	}
}


//============================================================
//  hlsl_info::frame_complete
//============================================================

void hlsl_info::frame_complete()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	if(render_snap && snap_rendered)
	{
		render_snapshot(snap_target);
	}
}


//============================================================
//  hlsl_info::end_hlsl_avi_recording
//============================================================

void hlsl_info::end_avi_recording()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	if (avi_output_file != NULL)
		avi_close(avi_output_file);

	avi_output_file = NULL;
	avi_frame = 0;
}


//============================================================
//  hlsl_info::begin_avi_recording
//============================================================

void hlsl_info::begin_avi_recording(const char *name)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	// stop any existing recording
	end_avi_recording();

	// reset the state
	avi_frame = 0;
	avi_next_frame_time = window->machine().time();

	// build up information about this new movie
	avi_movie_info info;
	info.video_format = 0;
	info.video_timescale = 1000 * ((window->machine().primary_screen != NULL) ? ATTOSECONDS_TO_HZ(window->machine().primary_screen->frame_period().attoseconds) : screen_device::DEFAULT_FRAME_RATE);
	info.video_sampletime = 1000;
	info.video_numsamples = 0;
	info.video_width = snap_width;
	info.video_height = snap_height;
	info.video_depth = 24;

	info.audio_format = 0;
	info.audio_timescale = window->machine().sample_rate();
	info.audio_sampletime = 1;
	info.audio_numsamples = 0;
	info.audio_channels = 2;
	info.audio_samplebits = 16;
	info.audio_samplerate = window->machine().sample_rate();

	// create a new temporary movie file
	file_error filerr;
	astring fullpath;
	{
		emu_file tempfile(window->machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (name != NULL)
		{
			filerr = tempfile.open(name);
		}
		else
		{
			filerr = open_next(d3d, tempfile, "avi", 0);
		}

		// compute the frame time
		{
			avi_frame_period = attotime::from_seconds(1000) / info.video_timescale;
		}

		// if we succeeded, make a copy of the name and create the real file over top
		if (filerr == FILERR_NONE)
		{
			fullpath = tempfile.fullpath();
		}
	}

	if (filerr == FILERR_NONE)
	{
		// create the file and free the string
		avi_error avierr = avi_create(fullpath, &info, &avi_output_file);
		if (avierr != AVIERR_NONE)
		{
			mame_printf_error("Error creating AVI: %s\n", avi_error_string(avierr));
		}
	}
}


//============================================================
//  hlsl_info::set_texture
//============================================================

void hlsl_info::set_texture(d3d_texture_info *texture)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	(*d3dintf->effect.set_texture)(effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
	if (yiq_enable)
		(*d3dintf->effect.set_texture)(yiq_encode_effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
	else
		(*d3dintf->effect.set_texture)(color_effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
	(*d3dintf->effect.set_texture)(pincushion_effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
}


//============================================================
//  hlsl_info::init
//============================================================

void hlsl_info::init(d3d *d3dintf, win_window_info *window)
{
	if (!d3dintf->post_fx_available)
		return;

	this->d3dintf = d3dintf;
	this->window = window;

	master_enable = downcast<windows_options &>(window->machine().options()).d3d_hlsl_enable();
	yiq_enable = downcast<windows_options &>(window->machine().options()).screen_yiq_enable();
	prescale_size = 1;
	preset = downcast<windows_options &>(window->machine().options()).d3d_hlsl_preset();

	snap_width = downcast<windows_options &>(window->machine().options()).d3d_snap_width();
	snap_height = downcast<windows_options &>(window->machine().options()).d3d_snap_height();
}


//============================================================
//  hlsl_info::init_fsfx_quad
//============================================================

void hlsl_info::init_fsfx_quad(void *vertbuf)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	// get a pointer to the vertex buffer
	fsfx_vertices = (d3d_vertex *)vertbuf;
	if (fsfx_vertices == NULL)
		return;

	// fill in the vertexes clockwise
	windows_options &options = downcast<windows_options &>(window->machine().options());
	float scale_top = options.screen_scale_top();
	float scale_bottom = options.screen_scale_bottom();

	fsfx_vertices[0].x = (d3d->width * (scale_top * 0.5f - 0.5f));
	fsfx_vertices[0].y = 0.0f;
	fsfx_vertices[1].x = d3d->width - (d3d->width * (scale_top * 0.5f - 0.5f));
	fsfx_vertices[1].y = 0.0f;
	fsfx_vertices[2].x = (d3d->width * (scale_bottom * 0.5f - 0.5f));
	fsfx_vertices[2].y = d3d->height;
	fsfx_vertices[3].x = d3d->width - (d3d->width * (scale_bottom * 0.5f - 0.5f));
	fsfx_vertices[3].y = d3d->height;

	fsfx_vertices[0].u0 = 0.0f;
	fsfx_vertices[0].v0 = 0.0f;
	fsfx_vertices[1].u0 = 1.0f;
	fsfx_vertices[1].v0 = 0.0f;
	fsfx_vertices[2].u0 = 0.0f;
	fsfx_vertices[2].v0 = 1.0f;
	fsfx_vertices[3].u0 = 1.0f;
	fsfx_vertices[3].v0 = 1.0f;

	// set the color, Z parameters to standard values
	for (int i = 0; i < 4; i++)
	{
		fsfx_vertices[i].z = 0.0f;
		fsfx_vertices[i].rhw = 1.0f;
		fsfx_vertices[i].color = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
}


//============================================================
//  hlsl_info::create_resources
//============================================================

int hlsl_info::create_resources()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return 0;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	HRESULT result = (*d3dintf->device.create_texture)(d3d->device, (int)snap_width, (int)snap_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &avi_copy_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init system-memory target for HLSL AVI dumping (%08x)\n", (UINT32)result);
		return 1;
	}
	(*d3dintf->texture.get_surface_level)(avi_copy_texture, 0, &avi_copy_surface);

	result = (*d3dintf->device.create_texture)(d3d->device, (int)snap_width, (int)snap_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &avi_final_texture);
	if (result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to init video-memory target for HLSL AVI dumping (%08x)\n", (UINT32)result);
		return 1;
	}
	(*d3dintf->texture.get_surface_level)(avi_final_texture, 0, &avi_final_target);

	windows_options &winoptions = downcast<windows_options &>(window->machine().options());

	// experimental: load a PNG to use for vector rendering; it is treated
	// as a brightness map
	emu_file file(window->machine().options().art_path(), OPEN_FLAG_READ);
	shadow_bitmap = render_load_png(file, NULL, winoptions.screen_shadow_mask_texture(), NULL, NULL);

	// experimental: if we have a shadow bitmap, create a texture for it
	if (shadow_bitmap != NULL)
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = shadow_bitmap->base;
		texture.rowpixels = shadow_bitmap->rowpixels;
		texture.width = shadow_bitmap->width;
		texture.height = shadow_bitmap->height;
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		shadow_texture = texture_create(d3d, &texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32));
	}

	options = (hlsl_options*)global_alloc_clear(hlsl_options);

	if(preset == -1)
	{
		options->shadow_mask_alpha = winoptions.screen_shadow_mask_alpha();
		options->shadow_mask_count_x = winoptions.screen_shadow_mask_count_x();
		options->shadow_mask_count_y = winoptions.screen_shadow_mask_count_y();
		options->shadow_mask_u_size = winoptions.screen_shadow_mask_u_size();
		options->shadow_mask_v_size = winoptions.screen_shadow_mask_v_size();
		options->curvature = winoptions.screen_curvature();
		options->pincushion = winoptions.screen_pincushion();
		options->scanline_alpha = winoptions.screen_scanline_amount();
		options->scanline_scale = winoptions.screen_scanline_scale();
		options->scanline_height = winoptions.screen_scanline_height();
		options->scanline_bright_scale = winoptions.screen_scanline_bright_scale();
		options->scanline_bright_offset = winoptions.screen_scanline_bright_offset();
		options->scanline_offset = winoptions.screen_scanline_offset();
		options->defocus_x = winoptions.screen_defocus_x();
		options->defocus_y = winoptions.screen_defocus_y();
		options->red_converge_x = winoptions.screen_red_converge_x();
		options->red_converge_y = winoptions.screen_red_converge_y();
		options->green_converge_x = winoptions.screen_green_converge_x();
		options->green_converge_y = winoptions.screen_green_converge_y();
		options->blue_converge_x = winoptions.screen_blue_converge_x();
		options->blue_converge_y = winoptions.screen_blue_converge_y();
		options->red_radial_converge_x = winoptions.screen_red_radial_converge_x();
		options->red_radial_converge_y = winoptions.screen_red_radial_converge_y();
		options->green_radial_converge_x = winoptions.screen_green_radial_converge_x();
		options->green_radial_converge_y = winoptions.screen_green_radial_converge_y();
		options->blue_radial_converge_x = winoptions.screen_blue_radial_converge_x();
		options->blue_radial_converge_y = winoptions.screen_blue_radial_converge_y();
		options->red_from_red = winoptions.screen_red_from_red();
		options->red_from_green = winoptions.screen_red_from_green();
		options->red_from_blue = winoptions.screen_red_from_blue();
		options->green_from_red = winoptions.screen_green_from_red();
		options->green_from_green = winoptions.screen_green_from_green();
		options->green_from_blue = winoptions.screen_green_from_blue();
		options->blue_from_red = winoptions.screen_blue_from_red();
		options->blue_from_green = winoptions.screen_blue_from_green();
		options->blue_from_blue = winoptions.screen_blue_from_blue();
		options->red_offset = winoptions.screen_red_offset();
		options->green_offset = winoptions.screen_green_offset();
		options->blue_offset = winoptions.screen_blue_offset();
		options->red_scale = winoptions.screen_red_scale();
		options->green_scale = winoptions.screen_green_scale();
		options->blue_scale = winoptions.screen_blue_scale();
		options->red_power = winoptions.screen_red_power();
		options->green_power = winoptions.screen_green_power();
		options->blue_power = winoptions.screen_blue_power();
		options->red_floor = winoptions.screen_red_floor();
		options->green_floor = winoptions.screen_green_floor();
		options->blue_floor = winoptions.screen_blue_floor();
		options->red_phosphor_life = winoptions.screen_red_phosphor();
		options->green_phosphor_life = winoptions.screen_green_phosphor();
		options->blue_phosphor_life = winoptions.screen_blue_phosphor();
		options->saturation = winoptions.screen_saturation();
	}
	else
	{
		options = &g_hlsl_presets[preset];
	}

	g_slider_list = init_slider_list();

	const char *fx_dir = downcast<windows_options &>(window->machine().options()).screen_post_fx_dir();
	char primary_name_cstr[1024];
	char post_name_cstr[1024];
	char prescale_name_cstr[1024];
	char pincushion_name_cstr[1024];
	char phosphor_name_cstr[1024];
	char focus_name_cstr[1024];
	char deconverge_name_cstr[1024];
	char color_name_cstr[1024];
	char yiq_encode_name_cstr[1024];
	char yiq_decode_name_cstr[1024];

	sprintf(primary_name_cstr, "%s\\primary.fx", fx_dir);
	TCHAR *primary_name = tstring_from_utf8(primary_name_cstr);

	sprintf(post_name_cstr, "%s\\post.fx", fx_dir);
	TCHAR *post_name = tstring_from_utf8(post_name_cstr);

	sprintf(prescale_name_cstr, "%s\\prescale.fx", fx_dir);
	TCHAR *prescale_name = tstring_from_utf8(prescale_name_cstr);

	sprintf(pincushion_name_cstr, "%s\\pincushion.fx", fx_dir);
	TCHAR *pincushion_name = tstring_from_utf8(pincushion_name_cstr);

	sprintf(phosphor_name_cstr, "%s\\phosphor.fx", fx_dir);
	TCHAR *phosphor_name = tstring_from_utf8(phosphor_name_cstr);

	sprintf(focus_name_cstr, "%s\\focus.fx", fx_dir);
	TCHAR *focus_name = tstring_from_utf8(focus_name_cstr);

	sprintf(deconverge_name_cstr, "%s\\deconverge.fx", fx_dir);
	TCHAR *deconverge_name = tstring_from_utf8(deconverge_name_cstr);

	sprintf(color_name_cstr, "%s\\color.fx", fx_dir);
	TCHAR *color_name = tstring_from_utf8(color_name_cstr);

	sprintf(yiq_encode_name_cstr, "%s\\yiq_encode.fx", fx_dir);
	TCHAR *yiq_encode_name = tstring_from_utf8(yiq_encode_name_cstr);

	sprintf(yiq_decode_name_cstr, "%s\\yiq_decode.fx", fx_dir);
	TCHAR *yiq_decode_name = tstring_from_utf8(yiq_decode_name_cstr);

	// create the regular shader
	result = (*d3dintf->device.create_effect)(d3d->device, primary_name, &effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load primary.fx\n");
		return 1;
	}

	// create the post-processing shader
	result = (*d3dintf->device.create_effect)(d3d->device, post_name, &post_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load post.fx\n");
		return 1;
	}

	// create the prescaling shader
	result = (*d3dintf->device.create_effect)(d3d->device, prescale_name, &prescale_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load prescale.fx\n");
		return 1;
	}

	// create the pincushion shader
	result = (*d3dintf->device.create_effect)(d3d->device, pincushion_name, &pincushion_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load pincushion.fx\n");
		return 1;
	}

	// create the phosphor shader
	result = (*d3dintf->device.create_effect)(d3d->device, phosphor_name, &phosphor_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load phosphor.fx\n");
		return 1;
	}

	// create the focus shader
	result = (*d3dintf->device.create_effect)(d3d->device, focus_name, &focus_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load focus.fx\n");
		return 1;
	}

	// create the deconvergence shader
	result = (*d3dintf->device.create_effect)(d3d->device, deconverge_name, &deconverge_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load deconverge.fx\n");
		return 1;
	}

	// create the color convolution shader
	result = (*d3dintf->device.create_effect)(d3d->device, color_name, &color_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load color.fx\n");
		return 1;
	}

	// create the YIQ modulation shader
	result = (*d3dintf->device.create_effect)(d3d->device, yiq_encode_name, &yiq_encode_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load yiq_encode.fx\n");
		return 1;
	}

	// create the YIQ demodulation shader
	result = (*d3dintf->device.create_effect)(d3d->device, yiq_decode_name, &yiq_decode_effect);
	if(result != D3D_OK)
	{
		printf("Direct3D: Unable to load yiq_decode.fx\n");
		return 1;
	}

	if (primary_name)
		osd_free(primary_name);
	if (post_name)
		osd_free(post_name);
	if (prescale_name)
		osd_free(prescale_name);
	if (pincushion_name)
		osd_free(pincushion_name);
	if (phosphor_name)
		osd_free(phosphor_name);
	if (focus_name)
		osd_free(focus_name);
	if (deconverge_name)
		osd_free(deconverge_name);
	if (color_name)
		osd_free(color_name);
	if (yiq_encode_name)
		osd_free(yiq_encode_name);
	if (yiq_decode_name)
		osd_free(yiq_decode_name);

	return 0;
}


//============================================================
//  hlsl_info::begin
//============================================================

void hlsl_info::begin()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	curr_effect = effect;

	(*d3dintf->effect.set_technique)(effect, "TestTechnique");
	(*d3dintf->effect.set_technique)(post_effect, "ScanMaskTechnique");
	(*d3dintf->effect.set_technique)(pincushion_effect, "TestTechnique");
	(*d3dintf->effect.set_technique)(phosphor_effect, "TestTechnique");
	(*d3dintf->effect.set_technique)(focus_effect, "TestTechnique");
	(*d3dintf->effect.set_technique)(deconverge_effect, "DeconvergeTechnique");
	(*d3dintf->effect.set_technique)(color_effect, "ColorTechnique");
	(*d3dintf->effect.set_technique)(yiq_encode_effect, "EncodeTechnique");
	(*d3dintf->effect.set_technique)(yiq_decode_effect, "DecodeTechnique");

	HRESULT result = (*d3dintf->device.get_render_target)(d3d->device, 0, &backbuffer);
	if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device get_render_target call\n", (int)result);

	for (int index = 0; index < 9; index++)
		screen_encountered[index] = false;
}


//============================================================
//  hlsl_info::render_quad
//============================================================

void hlsl_info::init_effect_info(d3d_poly_info *poly)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	if(PRIMFLAG_GET_TEXSHADE(d3d->last_texture_flags))
	{
		curr_effect = pincushion_effect;
	}
	else if(PRIMFLAG_GET_SCREENTEX(d3d->last_texture_flags) && poly->texture != NULL)
	{
		// Plug in all of the shader settings we're going to need
		// This is extremely slow, but we're not rendering models here,
		// just post-processing.
		curr_effect = post_effect;

		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
		(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
		(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "Prescale", (float)prescale_size);
		(*d3dintf->effect.set_float)(curr_effect, "RedFloor", options->red_floor);
		(*d3dintf->effect.set_float)(curr_effect, "GrnFloor", options->green_floor);
		(*d3dintf->effect.set_float)(curr_effect, "BluFloor", options->blue_floor);
		(*d3dintf->effect.set_float)(curr_effect, "SnapX", snap_width);
		(*d3dintf->effect.set_float)(curr_effect, "SnapY", snap_height);
		(*d3dintf->effect.set_float)(curr_effect, "PincushionAmount", options->pincushion);
		(*d3dintf->effect.set_float)(curr_effect, "CurvatureAmount", options->curvature);
		(*d3dintf->effect.set_float)(curr_effect, "UseShadow", shadow_texture == NULL ? 0.0f : 1.0f);
		(*d3dintf->effect.set_texture)(curr_effect, "Shadow", shadow_texture == NULL ? NULL : shadow_texture->d3dfinaltex);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowBrightness", options->shadow_mask_alpha);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowMaskSizeX", (float)options->shadow_mask_count_x);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowMaskSizeY", (float)options->shadow_mask_count_y);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowU", options->shadow_mask_u_size);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowV", options->shadow_mask_v_size);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowWidth", shadow_texture == NULL ? 1.0f : (float)shadow_texture->rawwidth);
		(*d3dintf->effect.set_float)(curr_effect, "ShadowHeight", shadow_texture == NULL ? 1.0f : (float)shadow_texture->rawheight);
		(*d3dintf->effect.set_float)(curr_effect, "ScanlineAmount", options->scanline_alpha);
		(*d3dintf->effect.set_float)(curr_effect, "ScanlineScale", options->scanline_scale);
		(*d3dintf->effect.set_float)(curr_effect, "ScanlineHeight", options->scanline_height);
		(*d3dintf->effect.set_float)(curr_effect, "ScanlineBrightScale", options->scanline_bright_scale);
		(*d3dintf->effect.set_float)(curr_effect, "ScanlineBrightOffset", options->scanline_bright_offset);
		(*d3dintf->effect.set_float)(curr_effect, "ScanlineOffset", (poly->texture->cur_frame == 0) ? 0.0f : options->scanline_offset);
	}
	else
	{
		curr_effect = effect;

		(*d3dintf->effect.set_float)(curr_effect, "FixedAlpha", 1.0f);
	}
}


//============================================================
//  hlsl_info::render_quad
//============================================================

void hlsl_info::render_quad(d3d_poly_info *poly, int vertnum)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	UINT num_passes = 0;
	d3d_info *d3d = (d3d_info *)window->drawdata;

	windows_options &winoptions = downcast<windows_options &>(window->machine().options());

	if(PRIMFLAG_GET_SCREENTEX(d3d->last_texture_flags) && poly->texture != NULL)
	{
		screen_encountered[poly->texture->target_index] = true;
		target_in_use[poly->texture->target_index] = poly->texture;

		target_use_count[poly->texture->target_index] = 60;

		if(yiq_enable)
		{
			/* Convert our signal into YIQ */
			curr_effect = yiq_encode_effect;

			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "CCValue", winoptions.screen_yiq_cc());
			(*d3dintf->effect.set_float)(curr_effect, "AValue", winoptions.screen_yiq_a());
			(*d3dintf->effect.set_float)(curr_effect, "BValue", (poly->texture->cur_frame == 2) ? 0.0f : ((float)poly->texture->cur_frame * winoptions.screen_yiq_b()));
			(*d3dintf->effect.set_float)(curr_effect, "PValue", winoptions.screen_yiq_p());
			(*d3dintf->effect.set_float)(curr_effect, "YFreqResponse", winoptions.screen_yiq_y());
			(*d3dintf->effect.set_float)(curr_effect, "IFreqResponse", winoptions.screen_yiq_i());
			(*d3dintf->effect.set_float)(curr_effect, "QFreqResponse", winoptions.screen_yiq_q());
			(*d3dintf->effect.set_float)(curr_effect, "ScanTime", winoptions.screen_yiq_scan_time());
			(*d3dintf->effect.set_float)(curr_effect, "Prescale", (float)prescale_size);

			HRESULT result = (*d3dintf->device.set_render_target)(d3d->device, 0, target4[poly->texture->target_index]);

			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
			result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			/* Convert our signal from YIQ */
			curr_effect = yiq_decode_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Composite", texture4[poly->texture->target_index]);
			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", poly->texture->d3dfinaltex);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "CCValue", winoptions.screen_yiq_cc());
			(*d3dintf->effect.set_float)(curr_effect, "AValue", winoptions.screen_yiq_a());
			(*d3dintf->effect.set_float)(curr_effect, "BValue", (poly->texture->cur_frame == 2) ? 0.0f : ((float)poly->texture->cur_frame * winoptions.screen_yiq_b()));
			(*d3dintf->effect.set_float)(curr_effect, "OValue", winoptions.screen_yiq_o());
			(*d3dintf->effect.set_float)(curr_effect, "PValue", winoptions.screen_yiq_p());
			(*d3dintf->effect.set_float)(curr_effect, "YFreqResponse", winoptions.screen_yiq_y());
			(*d3dintf->effect.set_float)(curr_effect, "IFreqResponse", winoptions.screen_yiq_i());
			(*d3dintf->effect.set_float)(curr_effect, "QFreqResponse", winoptions.screen_yiq_q());
			(*d3dintf->effect.set_float)(curr_effect, "ScanTime", winoptions.screen_yiq_scan_time());
			(*d3dintf->effect.set_float)(curr_effect, "Prescale", (float)prescale_size);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, target3[poly->texture->target_index]);

			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
			result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			curr_effect = color_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", texture3[poly->texture->target_index]);
		}

		curr_effect = color_effect;

		/* Render the initial color-convolution pass */
		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
		(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", yiq_enable ? 1.0f : (1.0f / (poly->texture->ustop - poly->texture->ustart)));
		(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", yiq_enable ? 1.0f : (1.0f / (poly->texture->vstop - poly->texture->vstart)));
		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "YIQEnable", yiq_enable ? 1.0f : 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "RedFromRed", options->red_from_red);
		(*d3dintf->effect.set_float)(curr_effect, "RedFromGrn", options->red_from_green);
		(*d3dintf->effect.set_float)(curr_effect, "RedFromBlu", options->red_from_blue);
		(*d3dintf->effect.set_float)(curr_effect, "GrnFromRed", options->green_from_red);
		(*d3dintf->effect.set_float)(curr_effect, "GrnFromGrn", options->green_from_green);
		(*d3dintf->effect.set_float)(curr_effect, "GrnFromBlu", options->green_from_blue);
		(*d3dintf->effect.set_float)(curr_effect, "BluFromRed", options->blue_from_red);
		(*d3dintf->effect.set_float)(curr_effect, "BluFromGrn", options->blue_from_green);
		(*d3dintf->effect.set_float)(curr_effect, "BluFromBlu", options->blue_from_blue);
		(*d3dintf->effect.set_float)(curr_effect, "RedOffset", options->red_offset);
		(*d3dintf->effect.set_float)(curr_effect, "GrnOffset", options->green_offset);
		(*d3dintf->effect.set_float)(curr_effect, "BluOffset", options->blue_offset);
		(*d3dintf->effect.set_float)(curr_effect, "RedScale", options->red_scale);
		(*d3dintf->effect.set_float)(curr_effect, "GrnScale", options->green_scale);
		(*d3dintf->effect.set_float)(curr_effect, "BluScale", options->blue_scale);
		(*d3dintf->effect.set_float)(curr_effect, "RedPower", options->red_power);
		(*d3dintf->effect.set_float)(curr_effect, "GrnPower", options->green_power);
		(*d3dintf->effect.set_float)(curr_effect, "BluPower", options->blue_power);
		(*d3dintf->effect.set_float)(curr_effect, "Saturation", options->saturation);

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->device, 0, smalltarget0[poly->texture->target_index]);

		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Pre-scaling pass */
		curr_effect = prescale_effect;
		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", smalltexture0[poly->texture->target_index]);

		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
		(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
		(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, prescaletarget0[poly->texture->target_index]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);


		/* Deconverge pass */
		curr_effect = deconverge_effect;
		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", prescaletexture0[poly->texture->target_index]);

		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
		(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
		(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
		(*d3dintf->effect.set_float)(curr_effect, "Prescale", prescale_size);
		(*d3dintf->effect.set_float)(curr_effect, "RedConvergeX", options->red_converge_x);
		(*d3dintf->effect.set_float)(curr_effect, "RedConvergeY", options->red_converge_y);
		(*d3dintf->effect.set_float)(curr_effect, "GrnConvergeX", options->green_converge_x);
		(*d3dintf->effect.set_float)(curr_effect, "GrnConvergeY", options->green_converge_y);
		(*d3dintf->effect.set_float)(curr_effect, "BluConvergeX", options->blue_converge_x);
		(*d3dintf->effect.set_float)(curr_effect, "BluConvergeY", options->blue_converge_y);
		(*d3dintf->effect.set_float)(curr_effect, "RedRadialConvergeX", options->red_radial_converge_x);
		(*d3dintf->effect.set_float)(curr_effect, "RedRadialConvergeY", options->red_radial_converge_y);
		(*d3dintf->effect.set_float)(curr_effect, "GrnRadialConvergeX", options->green_radial_converge_x);
		(*d3dintf->effect.set_float)(curr_effect, "GrnRadialConvergeY", options->green_radial_converge_y);
		(*d3dintf->effect.set_float)(curr_effect, "BluRadialConvergeX", options->blue_radial_converge_x);
		(*d3dintf->effect.set_float)(curr_effect, "BluRadialConvergeY", options->blue_radial_converge_y);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, target2[poly->texture->target_index]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		float defocus_x = options->defocus_x;
		float defocus_y = options->defocus_y;
		bool focus_enable = defocus_x != 0.0f || defocus_y != 0.0f;
		if(focus_enable)
		{
			/* Defocus pass 1 */
			curr_effect = focus_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", texture2[poly->texture->target_index]);

			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", poly->texture != NULL ? (1.0f / (poly->texture->ustop - poly->texture->ustart)) : 0.0f);
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", poly->texture != NULL ? (1.0f / (poly->texture->vstop - poly->texture->vstart)) : 0.0f);
			(*d3dintf->effect.set_float)(curr_effect, "DefocusX", defocus_x);
			(*d3dintf->effect.set_float)(curr_effect, "DefocusY", defocus_y);
			(*d3dintf->effect.set_float)(curr_effect, "FocusEnable", (defocus_x == 0.0f && defocus_y == 0.0f) ? 0.0f : 1.0f);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, target0[poly->texture->target_index]);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
			result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			/* Defocus pass 2 */

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", texture0[poly->texture->target_index]);

			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f);
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f);
			(*d3dintf->effect.set_float)(curr_effect, "DefocusX", defocus_x);
			(*d3dintf->effect.set_float)(curr_effect, "DefocusY", defocus_y);
			(*d3dintf->effect.set_float)(curr_effect, "FocusEnable", (defocus_x == 0.0f && defocus_y == 0.0f) ? 0.0f : 1.0f);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, target1[poly->texture->target_index]);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 7\n", (int)result);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);
		}

		// Simulate phosphorescence. This should happen after the shadow/scanline pass, but since
		// the phosphors are a direct result of the incoming texture, might as well just change the
		// input texture.
		curr_effect = phosphor_effect;

		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
		(*d3dintf->effect.set_float)(curr_effect, "RedPhosphor", options->red_phosphor_life);
		(*d3dintf->effect.set_float)(curr_effect, "GreenPhosphor", options->green_phosphor_life);
		(*d3dintf->effect.set_float)(curr_effect, "BluePhosphor", options->blue_phosphor_life);

		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", focus_enable ? texture1[poly->texture->target_index] : texture2[poly->texture->target_index]);
		(*d3dintf->effect.set_texture)(curr_effect, "LastPass", last_texture[poly->texture->target_index]);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, target0[poly->texture->target_index]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 4\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Pass along our phosphor'd screen */
		curr_effect = phosphor_effect;

		(*d3dintf->effect.set_float)(curr_effect, "FixedAlpha", 1.0f);

		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", focus_enable ? texture1[poly->texture->target_index] : texture2[poly->texture->target_index]);
		(*d3dintf->effect.set_texture)(curr_effect, "LastPass", focus_enable ? texture1[poly->texture->target_index] : texture2[poly->texture->target_index]);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, last_target[poly->texture->target_index]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 5\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, 0, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Scanlines and shadow mask, at high res for AVI logging*/
		if(avi_output_file != NULL)
		{
			curr_effect = post_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", texture0[poly->texture->target_index]);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, avi_final_target);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				HRESULT result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);
		}

		if(render_snap)
		{
			curr_effect = post_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", texture0[poly->texture->target_index]);

			(*d3dintf->effect.set_float)(curr_effect, "SnapX", snap_width);
			(*d3dintf->effect.set_float)(curr_effect, "SnapY", snap_height);
			result = (*d3dintf->device.set_render_target)(d3d->device, 0, snap_target);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				HRESULT result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			snap_rendered = true;
		}

		(*d3dintf->effect.set_float)(curr_effect, "SnapX", d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "SnapY", d3d->height);

		/* Scanlines and shadow mask */
		curr_effect = post_effect;

		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", texture0[poly->texture->target_index]);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, backbuffer);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			HRESULT result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		poly->texture->cur_frame++;
		poly->texture->cur_frame %= winoptions.screen_yiq_phase_count();
	}
	else
	{
		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", poly->texture != NULL ? (float)poly->texture->rawwidth : 8.0f);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", poly->texture != NULL ? (float)poly->texture->rawheight : 8.0f);
		(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", poly->texture != NULL ? (1.0f / (poly->texture->ustop - poly->texture->ustart)) : 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", poly->texture != NULL ? (1.0f / (poly->texture->vstop - poly->texture->vstart)) : 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "PostPass", 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "PincushionAmountX", options->pincushion);
		(*d3dintf->effect.set_float)(curr_effect, "PincushionAmountY", options->pincushion);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			HRESULT result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);
	}
}



//============================================================
//  hlsl_info::end
//============================================================

void hlsl_info::end()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	(*d3dintf->surface.release)(backbuffer);

	// Unregister any registered targets we didn't traverse in the past frame. A resolution change must
	// have occurred.
	for(int index = 0; index < 9 && master_enable && d3dintf->post_fx_available; index++)
	{
		if(!screen_encountered[index] && smalltarget0[index] != NULL)
		{
			if(target_use_count[index] > 0)
			{
				target_use_count[index]--;
			}
			else
			{
				// free all textures
				if(target_in_use[index] != NULL)
				{
					d3d_texture_info *tex = target_in_use[index];

					if(d3d->texlist == tex)
					{
						d3d->texlist = tex->next;
						if(d3d->texlist != NULL)
						{
							d3d->texlist->prev = NULL;
						}
					}
					else
					{
						if(tex->next != NULL)
						{
							tex->next->prev = tex->prev;
						}
						if(tex->prev != NULL)
						{
							tex->prev->next = tex->next;
						}
					}

					if (tex->d3dfinaltex != NULL)
						(*d3dintf->texture.release)(tex->d3dfinaltex);
					if (tex->d3dtex != NULL && tex->d3dtex != tex->d3dfinaltex)
						(*d3dintf->texture.release)(tex->d3dtex);
					if (tex->d3dsurface != NULL)
						(*d3dintf->surface.release)(tex->d3dsurface);
					global_free(tex);
				}

				if (prescaletexture0[index] != NULL)
				{
					(*d3dintf->texture.release)(prescaletexture0[index]);
					prescaletexture0[index] = NULL;
				}
				if (texture0[index] != NULL)
				{
					(*d3dintf->texture.release)(texture0[index]);
					texture0[index] = NULL;
				}
				if (texture1[index] != NULL)
				{
					(*d3dintf->texture.release)(texture1[index]);
					texture1[index] = NULL;
				}
				if (texture2[index] != NULL)
				{
					(*d3dintf->texture.release)(texture2[index]);
					texture2[index] = NULL;
				}
				if (texture3[index] != NULL)
				{
					(*d3dintf->texture.release)(texture3[index]);
					texture3[index] = NULL;
				}
				if (texture4[index] != NULL)
				{
					(*d3dintf->texture.release)(texture4[index]);
					texture4[index] = NULL;
				}
				if (smalltexture0[index] != NULL)
				{
					(*d3dintf->texture.release)(smalltexture0[index]);
					smalltexture0[index] = NULL;
				}
				if (prescaletarget0[index] != NULL)
				{
					(*d3dintf->surface.release)(prescaletarget0[index]);
					prescaletarget0[index] = NULL;
				}
				if (target0[index] != NULL)
				{
					(*d3dintf->surface.release)(target0[index]);
					target0[index] = NULL;
				}
				if (target1[index] != NULL)
				{
					(*d3dintf->surface.release)(target1[index]);
					target1[index] = NULL;
				}
				if (target2[index] != NULL)
				{
					(*d3dintf->surface.release)(target2[index]);
					target2[index] = NULL;
				}
				if (target3[index] != NULL)
				{
					(*d3dintf->surface.release)(target3[index]);
					target3[index] = NULL;
				}
				if (target4[index] != NULL)
				{
					(*d3dintf->surface.release)(target4[index]);
					target4[index] = NULL;
				}
				if (smalltarget0[index] != NULL)
				{
					(*d3dintf->surface.release)(smalltarget0[index]);
					smalltarget0[index] = NULL;
				}
				if(last_texture[index] != NULL)
				{
					(*d3dintf->texture.release)(last_texture[index]);
					last_texture[index] = NULL;
				}
				if(last_target[index] != NULL)
				{
					(*d3dintf->surface.release)(last_target[index]);
					last_target[index] = NULL;
				}
				target_use_count[index] = 0;
			}
		}
	}
}


//============================================================
//  hlsl_info::register_texture
//============================================================

int hlsl_info::register_prescaled_texture(d3d_texture_info *texture, int scwidth, int scheight)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return 0;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	int idx = registered_targets;

	// Find the nearest prescale factor that is over our screen size
	int hlsl_prescale = 1;
	while(texture->rawwidth * hlsl_prescale < d3d->width) hlsl_prescale++;
	prescale_size = hlsl_prescale;

	HRESULT result = (*d3dintf->device.create_texture)(d3d->device, scwidth * hlsl_prescale, scheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture0[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture0[idx], 0, &target0[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, scwidth * hlsl_prescale, scheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture1[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture1[idx], 0, &target1[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, scwidth * hlsl_prescale, scheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture2[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture2[idx], 0, &target2[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, scwidth * hlsl_prescale, scheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture3[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture3[idx], 0, &target3[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, scwidth, scheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture4[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture4[idx], 0, &target4[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, scwidth, scheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &smalltexture0[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(smalltexture0[idx], 0, &smalltarget0[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, scwidth * hlsl_prescale, scheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &prescaletexture0[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(prescaletexture0[idx], 0, &prescaletarget0[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, d3d->width, d3d->height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &last_texture[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(last_texture[idx], 0, &last_target[idx]);

	texture->target_index = registered_targets;
	target_use_count[texture->target_index] = 60;
	target_in_use[texture->target_index] = texture;
	registered_targets++;
	registered_targets %= 9;

	return 0;
}

//============================================================
//  hlsl_info::register_texture
//============================================================

int hlsl_info::register_texture(d3d_texture_info *texture)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return 0;

	d3d_info *d3d = (d3d_info *)window->drawdata;

	int idx = registered_targets;

	// Find the nearest prescale factor that is over our screen size
	int hlsl_prescale = 1;
	while(texture->rawwidth * hlsl_prescale < d3d->width) hlsl_prescale++;
	prescale_size = hlsl_prescale;

	HRESULT result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth * hlsl_prescale, texture->rawheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture0[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture0[idx], 0, &target0[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth * hlsl_prescale, texture->rawheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture1[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture1[idx], 0, &target1[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth * hlsl_prescale, texture->rawheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture2[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture2[idx], 0, &target2[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth, texture->rawheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture3[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture3[idx], 0, &target3[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth, texture->rawheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture4[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(texture4[idx], 0, &target4[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth, texture->rawheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &smalltexture0[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(smalltexture0[idx], 0, &smalltarget0[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, texture->rawwidth * hlsl_prescale, texture->rawheight * hlsl_prescale, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &prescaletexture0[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(prescaletexture0[idx], 0, &prescaletarget0[idx]);

	result = (*d3dintf->device.create_texture)(d3d->device, d3d->width, d3d->height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &last_texture[idx]);
	if (result != D3D_OK)
		return 1;
	(*d3dintf->texture.get_surface_level)(last_texture[idx], 0, &last_target[idx]);

	texture->target_index = registered_targets;
	target_use_count[texture->target_index] = 60;
	target_in_use[texture->target_index] = texture;
	registered_targets++;
	registered_targets %= 9;

	return 0;
}

//============================================================
//  hlsl_info::delete_resources
//============================================================

void hlsl_info::delete_resources()
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	if (effect != NULL)
	{
		(*d3dintf->effect.release)(effect);
		effect = NULL;
	}
	if (post_effect != NULL)
	{
		(*d3dintf->effect.release)(post_effect);
		post_effect = NULL;
	}
	if (prescale_effect != NULL)
	{
		(*d3dintf->effect.release)(prescale_effect);
		prescale_effect = NULL;
	}
	if (pincushion_effect != NULL)
	{
		(*d3dintf->effect.release)(pincushion_effect);
		pincushion_effect = NULL;
	}
	if (phosphor_effect != NULL)
	{
		(*d3dintf->effect.release)(phosphor_effect);
		phosphor_effect = NULL;
	}
	if (focus_effect != NULL)
	{
		(*d3dintf->effect.release)(focus_effect);
		focus_effect = NULL;
	}
	if (deconverge_effect != NULL)
	{
		(*d3dintf->effect.release)(deconverge_effect);
		deconverge_effect = NULL;
	}
	if (color_effect != NULL)
	{
		(*d3dintf->effect.release)(color_effect);
		color_effect = NULL;
	}
	if (yiq_encode_effect != NULL)
	{
		(*d3dintf->effect.release)(yiq_encode_effect);
		yiq_encode_effect = NULL;
	}
	if (yiq_decode_effect != NULL)
	{
		(*d3dintf->effect.release)(yiq_decode_effect);
		yiq_decode_effect = NULL;
	}

	for (int index = 0; index < 9; index++)
	{
		if (prescaletexture0[index] != NULL)
		{
			(*d3dintf->texture.release)(prescaletexture0[index]);
			prescaletexture0[index] = NULL;
		}
		if (texture0[index] != NULL)
		{
			(*d3dintf->texture.release)(texture0[index]);
			texture0[index] = NULL;
		}
		if (texture1[index] != NULL)
		{
			(*d3dintf->texture.release)(texture1[index]);
			texture1[index] = NULL;
		}
		if (texture2[index] != NULL)
		{
			(*d3dintf->texture.release)(texture2[index]);
			texture2[index] = NULL;
		}
		if (texture3[index] != NULL)
		{
			(*d3dintf->texture.release)(texture3[index]);
			texture3[index] = NULL;
		}
		if (texture4[index] != NULL)
		{
			(*d3dintf->texture.release)(texture4[index]);
			texture4[index] = NULL;
		}
		if (smalltexture0[index] != NULL)
		{
			(*d3dintf->texture.release)(smalltexture0[index]);
			smalltexture0[index] = NULL;
		}
		if (prescaletarget0[index] != NULL)
		{
			(*d3dintf->surface.release)(prescaletarget0[index]);
			prescaletarget0[index] = NULL;
		}
		if (target0[index] != NULL)
		{
			(*d3dintf->surface.release)(target0[index]);
			target0[index] = NULL;
		}
		if (target1[index] != NULL)
		{
			(*d3dintf->surface.release)(target1[index]);
			target1[index] = NULL;
		}
		if (target2[index] != NULL)
		{
			(*d3dintf->surface.release)(target2[index]);
			target2[index] = NULL;
		}
		if (target3[index] != NULL)
		{
			(*d3dintf->surface.release)(target3[index]);
			target3[index] = NULL;
		}
		if (target4[index] != NULL)
		{
			(*d3dintf->surface.release)(target4[index]);
			target4[index] = NULL;
		}
		if (smalltarget0[index] != NULL)
		{
			(*d3dintf->surface.release)(smalltarget0[index]);
			smalltarget0[index] = NULL;
		}
		if (last_texture[index] != NULL)
		{
			(*d3dintf->texture.release)(last_texture[index]);
			last_texture[index] = NULL;
		}
		if (last_target[index] != NULL)
		{
			(*d3dintf->surface.release)(last_target[index]);
			last_target[index] = NULL;
		}
	}

	if (avi_copy_texture != NULL)
	{
		(*d3dintf->texture.release)(avi_copy_texture);
		avi_copy_texture = NULL;
	}

	if (avi_copy_surface != NULL)
	{
		(*d3dintf->surface.release)(avi_copy_surface);
		avi_copy_surface = NULL;
	}

	if (avi_final_texture != NULL)
	{
		(*d3dintf->texture.release)(avi_final_texture);
		avi_final_texture = NULL;
	}

	if (avi_final_target != NULL)
	{
		(*d3dintf->surface.release)(avi_final_target);
		avi_final_target = NULL;
	}

	if (options != NULL)
	{
		global_free(options);
	}

	registered_targets = 0;

	if (shadow_texture != NULL)
	{
		global_free(shadow_texture);
		shadow_texture = NULL;
	}

	if (shadow_bitmap != NULL)
	{
		global_free(shadow_bitmap);
		shadow_bitmap = NULL;
	}
}


/*-------------------------------------------------
    slider_alloc - allocate a new slider entry
    currently duplicated from ui.c, this could
    be done in a more ideal way.
-------------------------------------------------*/

static slider_state *slider_alloc(running_machine &machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg)
{
	int size = sizeof(slider_state) + strlen(title);
	slider_state *state = (slider_state *)auto_alloc_array_clear(machine, UINT8, size);

	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
	strcpy(state->description, title);

	return state;
}


//============================================================
//  assorted global slider accessors
//============================================================

static INT32 slider_set(float *option, float scale, const char *fmt, astring *string, INT32 newval)
{
	if (option != NULL && newval != SLIDER_NOCHANGE) *option = (float)newval * scale;
	if (string != NULL) string->printf(fmt, *option);
	return floor(*option / scale + 0.5f);
}

static INT32 slider_shadow_mask_alpha(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_alpha), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_shadow_mask_x_count(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	hlsl_options *options = (hlsl_options*)arg;
	if (newval != SLIDER_NOCHANGE) options->shadow_mask_count_x = newval;
	if (string != NULL) string->printf("%d", options->shadow_mask_count_x);
	return options->shadow_mask_count_x;
}

static INT32 slider_shadow_mask_y_count(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	hlsl_options *options = (hlsl_options*)arg;
	if (newval != SLIDER_NOCHANGE) options->shadow_mask_count_y = newval;
	if (string != NULL) string->printf("%d", options->shadow_mask_count_y);
	return options->shadow_mask_count_y;
}

static INT32 slider_shadow_mask_usize(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_u_size), 1.0f / 32.0f, "%2.5f", string, newval);
}

static INT32 slider_shadow_mask_vsize(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_v_size), 1.0f / 32.0f, "%2.5f", string, newval);
}

static INT32 slider_curvature(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->curvature), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_pincushion(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->pincushion), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_scanline_alpha(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->scanline_alpha), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_scanline_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->scanline_scale), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_height(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->scanline_height), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_bright_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->scanline_bright_scale), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_bright_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->scanline_bright_offset), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->scanline_offset), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_defocus_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->defocus_x), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_defocus_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->defocus_y), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_red_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_converge_x), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_converge_y), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_converge_x), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_converge_y), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_converge_x), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_converge_y), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_radial_converge_x), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_radial_converge_y), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_radial_converge_x), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_radial_converge_y), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_radial_converge_x), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_radial_converge_y), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_from_red), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_from_green), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_from_blue), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_from_red), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_from_green), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_from_blue), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_from_red), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_from_green), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_from_blue), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_offset), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_offset), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_offset), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_scale), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_scale), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_scale), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_power), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_green_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_power), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_blue_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_power), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_red_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_floor), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_floor), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_floor), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->red_phosphor_life), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->green_phosphor_life), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->blue_phosphor_life), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_saturation(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	return slider_set(&(((hlsl_options*)arg)->saturation), 0.01f, "%2.2f", string, newval);
}

//============================================================
//  init_slider_list
//============================================================

slider_state *hlsl_info::init_slider_list()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		g_slider_list = NULL;
		return NULL;
	}

	d3d_info *d3d = (d3d_info *)window->drawdata;

	slider_state *listhead = NULL;
	slider_state **tailptr = &listhead;
	astring string;

	*tailptr = slider_alloc(window->machine(), "Shadow Mask Darkness", 0, 0, 100, 1, slider_shadow_mask_alpha, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask X Count", 1, 640, 1024, 1, slider_shadow_mask_x_count, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask Y Count", 1, 480, 1024, 1, slider_shadow_mask_y_count, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask Pixel Count X", 1, 3, 32, 1, slider_shadow_mask_usize, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask Pixel Count Y", 1, 3, 32, 1, slider_shadow_mask_vsize, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Screen Curvature", 0, 0, 100, 1, slider_curvature, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Image Pincushion", 0, 0, 100, 1, slider_pincushion, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Darkness", 0, 0, 100, 1, slider_scanline_alpha, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Screen Height", 1, 20, 80, 1, slider_scanline_scale, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Indiv. Height", 1, 10, 80, 1, slider_scanline_height, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Brightness", 0, 20, 40, 1, slider_scanline_bright_scale, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Brightness Overdrive", 0, 12, 20, 1, slider_scanline_bright_offset, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Jitter", 0, 0, 40, 1, slider_scanline_offset, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Defocus X", 0, 0, 64, 1, slider_defocus_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Defocus Y", 0, 0, 64, 1, slider_defocus_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Position Offset X", -1500, 0, 1500, 1, slider_red_converge_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Position Offset Y", -1500, 0, 1500, 1, slider_red_converge_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Position Offset X", -1500, 0, 1500, 1, slider_green_converge_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Position Offset Y", -1500, 0, 1500, 1, slider_green_converge_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Position Offset X", -1500, 0, 1500, 1, slider_blue_converge_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Position Offset Y", -1500, 0, 1500, 1, slider_blue_converge_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Convergence X", -1500, 0, 1500, 1, slider_red_radial_converge_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Convergence Y", -1500, 0, 1500, 1, slider_red_radial_converge_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Convergence X", -1500, 0, 1500, 1, slider_green_radial_converge_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Convergence Y", -1500, 0, 1500, 1, slider_green_radial_converge_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Convergence X", -1500, 0, 1500, 1, slider_blue_radial_converge_x, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Convergence Y", -1500, 0, 1500, 1, slider_blue_radial_converge_y, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Output from Red Input", -400, 0, 400, 5, slider_red_from_r, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Output from Green Input", -400, 0, 400, 5, slider_red_from_g, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Output from Blue Input", -400, 0, 400, 5, slider_red_from_b, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Output from Red Input", -400, 0, 400, 5, slider_green_from_r, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Output from Green Input", -400, 0, 400, 5, slider_green_from_g, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Output from Blue Input", -400, 0, 400, 5, slider_green_from_b, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Output from Red Input", -400, 0, 400, 5, slider_blue_from_r, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Output from Green Input", -400, 0, 400, 5, slider_blue_from_g, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Output from Blue Input", -400, 0, 400, 5, slider_blue_from_b, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red DC Offset", -100, 0, 100, 1, slider_red_offset, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green DC Offset", -100, 0, 100, 1, slider_green_offset, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue DC Offset", -100, 0, 100, 1, slider_blue_offset, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Scale", -200, 100, 200, 1, slider_red_scale, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Scale", -200, 100, 200, 1, slider_green_scale, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Scale", -200, 100, 200, 1, slider_blue_scale, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Power", -80, 20, 80, 1, slider_red_power, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Power", -80, 20, 80, 1, slider_green_power, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Power", -80, 20, 80, 1, slider_blue_power, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Floor", 0, 0, 100, 1, slider_red_floor, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Floor", 0, 0, 100, 1, slider_green_floor, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Floor", 0, 0, 100, 1, slider_blue_floor, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Phosphor Life", 0, 0, 100, 1, slider_red_phosphor_life, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Phosphor Life", 0, 0, 100, 1, slider_green_phosphor_life, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Phosphor Life", 0, 0, 100, 1, slider_blue_phosphor_life, (void*)d3d); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Saturation", 0, 100, 400, 1, slider_saturation, (void*)d3d); tailptr = &(*tailptr)->next;

	return listhead;
}

//============================================================
//  get_slider_list
//============================================================

void *windows_osd_interface::get_slider_list()
{
	return (void*)g_slider_list;
}



// NOTE: The function below is taken directly from src/emu/video.c and should likely be moved into a global helper function.
//-------------------------------------------------
//  open_next - open the next non-existing file of
//  type filetype according to our numbering
//  scheme
//-------------------------------------------------

static file_error open_next(d3d_info *d3d, emu_file &file, const char *extension, int idx)
{
	UINT32 origflags = file.openflags();

	// handle defaults
	const char *snapname = d3d->window->machine().options().snap_name();

	if (snapname == NULL || snapname[0] == 0)
		snapname = "%g/%i";
	astring snapstr(snapname);

	// strip any extension in the provided name
	int index = snapstr.rchr(0, '.');
	if (index != -1)
		snapstr.substr(0, index);

	// handle %d in the template (for image devices)
	astring snapdev("%d_");
	int pos = snapstr.find(0, snapdev);

	if (pos != -1)
	{
		// if more %d are found, revert to default and ignore them all
		if (snapstr.find(pos + 3, snapdev) != -1)
			snapstr.cpy("%g/%i");
		// else if there is a single %d, try to create the correct snapname
		else
		{
			int name_found = 0;

			// find length of the device name
			int end1 = snapstr.find(pos + 3, "/");
			int end2 = snapstr.find(pos + 3, "%");
			int end = -1;

			if ((end1 != -1) && (end2 != -1))
				end = MIN(end1, end2);
			else if (end1 != -1)
				end = end1;
			else if (end2 != -1)
				end = end2;
			else
				end = snapstr.len();

			if (end - pos < 3)
				fatalerror("Something very wrong is going on!!!");

			// copy the device name to an astring
			astring snapdevname;
			snapdevname.cpysubstr(snapstr, pos + 3, end - pos - 3);

			// verify that there is such a device for this system
			device_image_interface *image = NULL;
			for (bool gotone = d3d->window->machine().devicelist().first(image); gotone; gotone = image->next(image))
			{
				// get the device name
				astring tempdevname(image->brief_instance_name());

				if (snapdevname.cmp(tempdevname) == 0)
				{
					// verify that such a device has an image mounted
					if (image->basename() != NULL)
					{
						astring filename(image->basename());

						// strip extension
						filename.substr(0, filename.rchr(0, '.'));

						// setup snapname and remove the %d_
						snapstr.replace(0, snapdevname, filename);
						snapstr.del(pos, 3);

						name_found = 1;
					}
				}
			}

			// or fallback to default
			if (name_found == 0)
				snapstr.cpy("%g/%i");
		}
	}

	// add our own index
	// add our own extension
	snapstr.cat(".").cat(extension);

	// substitute path and gamename up front
	snapstr.replace(0, "/", PATH_SEPARATOR);
	snapstr.replace(0, "%g", d3d->window->machine().basename());

	// determine if the template has an index; if not, we always use the same name
	astring fname;
	if (snapstr.find(0, "%i") == -1)
		fname.cpy(snapstr);

	// otherwise, we scan for the next available filename
	else
	{
		// try until we succeed
		astring seqtext;
		file.set_openflags(OPEN_FLAG_READ);
		for (int seq = 0; ; seq++)
		{
			// build up the filename
			fname.cpy(snapstr).replace(0, "%i", seqtext.format("%04d_%d", seq, idx).cstr());

			// try to open the file; stop when we fail
			file_error filerr = file.open(fname);
			if (filerr != FILERR_NONE)
				break;
		}
	}

	// create the final file
	file.set_openflags(origflags);
    return file.open(fname);
}
