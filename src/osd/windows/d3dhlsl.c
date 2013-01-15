//============================================================
//
//  d3dhlsl.c - Win32 Direct3D HLSL implementation
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
#include "screen.h"

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

static hlsl_options g_hlsl_presets[4] =
{
	{   // 25% Shadow mask, 50% Scanlines, 3% Pincushion, 0 defocus, No Tint, 0.9 Exponent, 5% Floor, 25% Phosphor Return, 120% Saturation
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.03f, 0.03f,
		0.5f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f,
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.9f, 0.9f, 0.9f },
		{ 0.05f,0.05f,0.05f},
		{ 0.25f,0.25f,0.25f},
		1.2f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0
	},
	{   // 25% Shadow mask, 0% Scanlines, 3% Pincushion, 0 defocus, No Tint, 0.9 Exponent, 5% Floor, 25% Phosphor Return, 120% Saturation
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.03f, 0.03f,
		0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f,
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.9f, 0.9f, 0.9f },
		{ 0.05f,0.05f,0.05f},
		{ 0.25f,0.25f,0.25f},
		1.2f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0
	},
	{   // 25% Shadow mask, 0% Scanlines, 0% Pincushion, 0 defocus, No Tint, 0.9 Exponent, 5% Floor, 25% Phosphor Return, 120% Saturation
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.0f, 0.0f,
		0.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.0f,
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 0.9f, 0.9f, 0.9f },
		{ 0.05f,0.05f,0.05f},
		{ 0.25f,0.25f,0.25f},
		1.2f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0
	},
	{   // 25% Shadow mask, 100% Scanlines, 15% Pincushion, 3 defocus, 24-degree Tint Out, 1.5 Exponent, 5% Floor, 70% Phosphor Return, 80% Saturation, Bad Convergence
		true,
		0.25f, { "aperture.png" }, 320, 240, 0.09375f, 0.109375f,
		0.15f, 0.15f,
		1.0f, 1.0f, 0.5f, 1.0f, 0.0f, 0.5f,
		{ 3.0f, 3.0f, 3.0f, 3.0f },
		{ 0.5f,-0.33f,0.7f },
		{ 0.0f,-1.0f, 0.5f },
		{ 0.0f, 0.2f, 0.3f },
		{ 0.0f, 0.2f, 0.0f },
		{ 0.8f, 0.2f, 0.0f },
		{ 0.0f, 0.8f, 0.2f},
		{ 0.2f, 0.0f, 0.8f},
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f },
		{ 1.5f, 1.5f, 1.5f },
		{ 0.05f,0.05f,0.05f},
		{ 0.7f, 0.7f, 0.7f},
		0.8f,
		false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0
	},
};

static slider_state *g_slider_list;



//============================================================
//  PROTOTYPES
//============================================================

static void get_vector(const char *data, int count, float *out, int report_error);
static file_error open_next(d3d_info *d3d, emu_file &file, const char *templ, const char *extension, int idx);



//============================================================
//  hlsl_info constructor
//============================================================

hlsl_info::hlsl_info()
{
	master_enable = false;
	vector_enable = true;
	prescale_size_x = 1;
	prescale_size_y = 1;
	prescale_force_x = 0;
	prescale_force_y = 0;
	preset = -1;
	shadow_texture = NULL;
	options = NULL;
	paused = true;
	lastidx = -1;
	targethead = NULL;
	cachehead = NULL;
	initialized = false;
}



//============================================================
//  hlsl_info destructor
//============================================================

hlsl_info::~hlsl_info()
{
	global_free(options);
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
	if (!avi_snap.valid() || (int)snap_width != avi_snap.width() || (int)snap_height != avi_snap.height())
	{
		avi_snap.allocate((int)snap_width, (int)snap_height);
	}

	// copy the texture
	HRESULT result = (*d3dintf->device.get_render_target_data)(d3d->device, surface, avi_copy_surface);
	if (result != D3D_OK)
	{
		return;
	}

	// lock the texture
	result = (*d3dintf->surface.lock_rect)(avi_copy_surface, &rect, NULL, D3DLOCK_DISCARD);
	if (result != D3D_OK)
	{
		return;
	}

	// loop over Y
	for (int srcy = 0; srcy < (int)snap_height; srcy++)
	{
		DWORD *src = (DWORD *)((BYTE *)rect.pBits + srcy * rect.Pitch);
		UINT32 *dst = &avi_snap.pix32(srcy);

		for(int x = 0; x < snap_width; x++)
			*dst++ = *src++;
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
	if (!avi_snap.valid() || snap_width != (avi_snap.width() / 2) || snap_height != (avi_snap.height() / 2))
	{
		avi_snap.allocate(snap_width / 2, snap_height / 2);
	}

	// copy the texture
	HRESULT result = (*d3dintf->device.get_render_target_data)(d3d->device, surface, snap_copy_target);
	if (result != D3D_OK)
	{
		return;
	}

	// lock the texture
	result = (*d3dintf->surface.lock_rect)(snap_copy_target, &rect, NULL, D3DLOCK_DISCARD);
	if (result != D3D_OK)
	{
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
				DWORD *src = (DWORD *)((BYTE *)rect.pBits + toty * rect.Pitch + totx * 4);
				UINT32 *dst = &avi_snap.pix32(srcy);

				for(int x = 0; x < snap_width / 2; x++)
					*dst++ = *src++;
			}

			int idx = cy * 2 + cx;

			emu_file file(window->machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			file_error filerr = open_next(d3d, file, NULL, "png", idx);
			if (filerr != FILERR_NONE)
				return;

			// add two text entries describing the image
			astring text1(emulator_info::get_appname(), " ", build_version);
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
		avi_error avierr = avi_append_video_frame(avi_output_file, avi_snap);
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
//  hlsl_info::set_texture
//============================================================

void hlsl_info::toggle()
{
	if (master_enable)
	{
		if (initialized)
		{
			delete_resources(false);
		}
	}
	else
	{
		if (!initialized)
		{
			create_resources(false);
		}
	}

	master_enable = !master_enable;
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
			filerr = open_next(d3d, tempfile, NULL, "avi", 0);
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
//  remove_cache_target - remove an active cache target when
//  refcount hits zero
//============================================================

void hlsl_info::remove_cache_target(d3d_cache_target *cache)
{
	if (cache != NULL)
	{
		if (cache == cachehead)
		{
			cachehead = cachehead->next;
		}

		if (cache->prev != NULL)
		{
			cache->prev->next = cache->next;
		}

		if (cache->next != NULL)
		{
			cache->next->prev = cache->prev;
		}

		global_free(cache);
	}
}


//============================================================
//  remove_render_target - remove an active target
//============================================================

void hlsl_info::remove_render_target(d3d_texture_info *texture)
{
	remove_render_target(find_render_target(texture));
}

void hlsl_info::remove_render_target(int width, int height, UINT32 screen_index, UINT32 page_index)
{
	d3d_render_target *target = find_render_target(width, height, screen_index, page_index);
	if (target != NULL)
	{
		remove_render_target(target);
	}
}

void hlsl_info::remove_render_target(d3d_render_target *rt)
{
	if (rt != NULL)
	{
		if (rt == targethead)
		{
			targethead = targethead->next;
		}

		if (rt->prev != NULL)
		{
			rt->prev->next = rt->next;
		}

		if (rt->next != NULL)
		{
			rt->next->prev = rt->prev;
		}

		d3d_cache_target *cache = find_cache_target(rt->screen_index, rt->width, rt->height);
		if (cache != NULL)
		{
			remove_cache_target(cache);
		}

		int screen_index = rt->screen_index;
		int other_page = 1 - rt->page_index;
		int width = rt->width;
		int height = rt->height;

		global_free(rt);

		// Remove other double-buffered page (if it exists)
		remove_render_target(width, height, screen_index, other_page);
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

	if(texture != NULL)
	{
		if(texture->prev_frame == texture->cur_frame)
		{
			paused = true;
		}
		else
		{
			paused = false;
		}

		texture->prev_frame = texture->cur_frame;
	}

	(*d3dintf->effect.set_texture)(effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
	if (options->yiq_enable)
		(*d3dintf->effect.set_texture)(yiq_encode_effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
	else
		(*d3dintf->effect.set_texture)(color_effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
	(*d3dintf->effect.set_texture)(pincushion_effect, "Diffuse", (texture == NULL) ? d3d->default_texture->d3dfinaltex : texture->d3dfinaltex);
}


//============================================================
//  hlsl_info::init
//============================================================

void hlsl_info::init(d3d_base *d3dintf, win_window_info *window)
{
	if (!d3dintf->post_fx_available)
		return;

	g_slider_list = init_slider_list();

	this->d3dintf = d3dintf;
	this->window = window;

	master_enable = downcast<windows_options &>(window->machine().options()).d3d_hlsl_enable();
	prescale_size_x = 1;
	prescale_size_y = 1;
	preset = downcast<windows_options &>(window->machine().options()).d3d_hlsl_preset();
	if (preset < -1 || preset > 3)
	{
		preset = -1;
	}

	snap_width = downcast<windows_options &>(window->machine().options()).d3d_snap_width();
	snap_height = downcast<windows_options &>(window->machine().options()).d3d_snap_height();

	prescale_force_x = 0;
	prescale_force_y = 0;

	windows_options &winoptions = downcast<windows_options &>(window->machine().options());

	options = (hlsl_options*)global_alloc_clear(hlsl_options);

	options->params_dirty = true;
	strcpy(options->shadow_mask_texture, downcast<windows_options &>(window->machine().options()).screen_shadow_mask_texture()); // unsafe

	write_ini = downcast<windows_options &>(window->machine().options()).hlsl_write_ini();
	read_ini = downcast<windows_options &>(window->machine().options()).hlsl_read_ini();

	if(read_ini)
	{
		emu_file ini_file(downcast<windows_options &>(window->machine().options()).screen_post_fx_dir(), OPEN_FLAG_READ | OPEN_FLAG_CREATE_PATHS);
		file_error filerr = open_next((d3d_info*)window->drawdata, ini_file, downcast<windows_options &>(window->machine().options()).hlsl_ini_name(), "ini", 0);

		read_ini = false;
		if (filerr == FILERR_NONE)
		{
			ini_file.seek(0, SEEK_END);
			if (ini_file.tell() >= 1000)
			{
				read_ini = true;
				ini_file.seek(0, SEEK_SET);

				int en = 0;
				char buf[1024];
				ini_file.gets(buf, 1024);
				sscanf(buf, "hlsl_enable %d\n", &en);
				master_enable = en == 1;

				ini_file.gets(buf, 1024);
				sscanf(buf, "hlsl_prescale_x %d\n", &prescale_force_x);

				ini_file.gets(buf, 1024);
				sscanf(buf, "hlsl_prescale_y %d\n", &prescale_force_y);

				ini_file.gets(buf, 1024);
				sscanf(buf, "hlsl_preset %d\n", &preset);

				ini_file.gets(buf, 1024);
				sscanf(buf, "hlsl_snap_width %d\n", &snap_width);

				ini_file.gets(buf, 1024);
				sscanf(buf, "hlsl_snap_height %d\n", &snap_height);

				ini_file.gets(buf, 1024);
				sscanf(buf, "shadow_mask_alpha %f\n", &options->shadow_mask_alpha);

				ini_file.gets(buf, 1024);
				sscanf(buf, "shadow_mask_texture %s\n", options->shadow_mask_texture);

				ini_file.gets(buf, 1024);
				sscanf(buf, "shadow_mask_x_count %d\n", &options->shadow_mask_count_x);

				ini_file.gets(buf, 1024);
				sscanf(buf, "shadow_mask_y_count %d\n", &options->shadow_mask_count_y);

				ini_file.gets(buf, 1024);
				sscanf(buf, "shadow_mask_usize %f\n", &options->shadow_mask_u_size);

				ini_file.gets(buf, 1024);
				sscanf(buf, "shadow_mask_vsize %f\n", &options->shadow_mask_v_size);

				ini_file.gets(buf, 1024);
				sscanf(buf, "curvature %f\n", &options->curvature);

				ini_file.gets(buf, 1024);
				sscanf(buf, "pincushion %f\n", &options->pincushion);

				ini_file.gets(buf, 1024);
				sscanf(buf, "scanline_alpha %f\n", &options->scanline_alpha);

				ini_file.gets(buf, 1024);
				sscanf(buf, "scanline_size %f\n", &options->scanline_scale);

				ini_file.gets(buf, 1024);
				sscanf(buf, "scanline_height %f\n", &options->scanline_height);

				ini_file.gets(buf, 1024);
				sscanf(buf, "scanline_bright_scale %f\n", &options->scanline_bright_scale);

				ini_file.gets(buf, 1024);
				sscanf(buf, "scanline_bright_offset %f\n", &options->scanline_bright_offset);

				ini_file.gets(buf, 1024);
				sscanf(buf, "scanline_jitter %f\n", &options->scanline_offset);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "defocus %f %f\n", &options->defocus[0], &options->defocus[1]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "converge_x %f %f %f\n", &options->converge_x[0], &options->converge_x[1], &options->converge_x[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "converge_y %f %f %f\n", &options->converge_y[0], &options->converge_y[1], &options->converge_y[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "radial_converge_x %f %f %f\n", &options->radial_converge_x[0], &options->radial_converge_x[1], &options->radial_converge_x[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "radial_converge_y %f %f %f\n", &options->radial_converge_y[0], &options->radial_converge_y[1], &options->radial_converge_y[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "red_ratio %f %f %f\n", &options->red_ratio[0], &options->red_ratio[1], &options->red_ratio[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "grn_ratio %f %f %f\n", &options->grn_ratio[0], &options->grn_ratio[1], &options->grn_ratio[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "blu_ratio %f %f %f\n", &options->blu_ratio[0], &options->blu_ratio[1], &options->blu_ratio[2]);

				ini_file.gets(buf, 1024);
				sscanf(buf, "saturation %f\n", &options->saturation);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "offset %f %f %f\n", &options->offset[0], &options->offset[1], &options->offset[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "scale %f %f %f\n", &options->scale[0], &options->scale[1], &options->scale[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "power %f %f %f\n", &options->power[0], &options->power[1], &options->power[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "floor %f %f %f\n", &options->floor[0], &options->floor[1], &options->floor[2]);

				ini_file.gets(buf, 1024);
				for(int idx = 0; idx < strlen(buf); idx++) if(buf[idx] == ',') buf[idx] = ' ';
				sscanf(buf, "phosphor_life %f %f %f\n", &options->phosphor[0], &options->phosphor[1], &options->phosphor[2]);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_enable %d\n", &en);
				options->yiq_enable = en == 1;

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_cc %f\n", &options->yiq_cc);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_a %f\n", &options->yiq_a);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_b %f\n", &options->yiq_b);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_o %f\n", &options->yiq_o);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_p %f\n", &options->yiq_p);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_n %f\n", &options->yiq_n);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_y %f\n", &options->yiq_y);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_i %f\n", &options->yiq_i);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_q %f\n", &options->yiq_q);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_scan_time %f\n", &options->yiq_scan_time);

				ini_file.gets(buf, 1024);
				sscanf(buf, "yiq_phase_count %d\n", &options->yiq_phase_count);
			}
		}
	}
	else
	{
		prescale_force_x = winoptions.d3d_hlsl_prescale_x();
		prescale_force_y = winoptions.d3d_hlsl_prescale_y();
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
			get_vector(winoptions.screen_defocus(), 2, options->defocus, TRUE);
			get_vector(winoptions.screen_converge_x(), 3, options->converge_x, TRUE);
			get_vector(winoptions.screen_converge_y(), 3, options->converge_y, TRUE);
			get_vector(winoptions.screen_radial_converge_x(), 3, options->radial_converge_x, TRUE);
			get_vector(winoptions.screen_radial_converge_y(), 3, options->radial_converge_y, TRUE);
			get_vector(winoptions.screen_red_ratio(), 3, options->red_ratio, TRUE);
			get_vector(winoptions.screen_grn_ratio(), 3, options->grn_ratio, TRUE);
			get_vector(winoptions.screen_blu_ratio(), 3, options->blu_ratio, TRUE);
			get_vector(winoptions.screen_offset(), 3, options->offset, TRUE);
			get_vector(winoptions.screen_scale(), 3, options->scale, TRUE);
			get_vector(winoptions.screen_power(), 3, options->power, TRUE);
			get_vector(winoptions.screen_floor(), 3, options->floor, TRUE);
			get_vector(winoptions.screen_phosphor(), 3, options->phosphor, TRUE);
			options->saturation = winoptions.screen_saturation();
		}
		else
		{
			options = &g_hlsl_presets[preset];
		}

		options->yiq_enable = winoptions.screen_yiq_enable();
		options->yiq_cc = winoptions.screen_yiq_cc();
		options->yiq_a = winoptions.screen_yiq_a();
		options->yiq_b = winoptions.screen_yiq_b();
		options->yiq_o = winoptions.screen_yiq_o();
		options->yiq_p = winoptions.screen_yiq_p();
		options->yiq_n = winoptions.screen_yiq_n();
		options->yiq_y = winoptions.screen_yiq_y();
		options->yiq_i = winoptions.screen_yiq_i();
		options->yiq_q = winoptions.screen_yiq_q();
		options->yiq_scan_time = winoptions.screen_yiq_scan_time();
		options->yiq_phase_count = winoptions.screen_yiq_phase_count();
	}

	options->params_dirty = true;
	// experimental: load a PNG to use for vector rendering; it is treated
	// as a brightness map
	emu_file file(window->machine().options().art_path(), OPEN_FLAG_READ);

	render_load_png(shadow_bitmap, file, NULL, options->shadow_mask_texture);
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
	fsfx_vertices[0].x = 0.0f;
	fsfx_vertices[0].y = 0.0f;
	fsfx_vertices[1].x = d3d->width;
	fsfx_vertices[1].y = 0.0f;
	fsfx_vertices[2].x = 0.0f;
	fsfx_vertices[2].y = d3d->height;
	fsfx_vertices[3].x = d3d->width;
	fsfx_vertices[3].y = 0.0f;
	fsfx_vertices[4].x = 0.0f;
	fsfx_vertices[4].y = d3d->height;
	fsfx_vertices[5].x = d3d->width;
	fsfx_vertices[5].y = d3d->height;

	fsfx_vertices[0].u0 = 0.0f;
	fsfx_vertices[0].v0 = 0.0f;

	fsfx_vertices[1].u0 = 1.0f;
	fsfx_vertices[1].v0 = 0.0f;

	fsfx_vertices[2].u0 = 0.0f;
	fsfx_vertices[2].v0 = 1.0f;

	fsfx_vertices[3].u0 = 1.0f;
	fsfx_vertices[3].v0 = 0.0f;

	fsfx_vertices[4].u0 = 0.0f;
	fsfx_vertices[4].v0 = 1.0f;

	fsfx_vertices[5].u0 = 1.0f;
	fsfx_vertices[5].v0 = 1.0f;

	// set the color, Z parameters to standard values
	for (int i = 0; i < 6; i++)
	{
		fsfx_vertices[i].z = 0.0f;
		fsfx_vertices[i].rhw = 1.0f;
		fsfx_vertices[i].color = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
}


//============================================================
//  hlsl_info::create_resources
//============================================================

int hlsl_info::create_resources(bool reset)
{
	initialized = true;

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

	// experimental: if we have a shadow bitmap, create a texture for it
	if (shadow_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = shadow_bitmap.raw_pixptr(0);
		texture.rowpixels = shadow_bitmap.rowpixels();
		texture.width = shadow_bitmap.width();
		texture.height = shadow_bitmap.height();
		texture.palette = NULL;
		texture.seqid = 0;

		// now create it
		shadow_texture = texture_create(d3d, &texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32));
	}

	const char *fx_dir = downcast<windows_options &>(window->machine().options()).screen_post_fx_dir();

	// Replace all this garbage with a proper data-driven system
	char primary_name_cstr[1024];
	sprintf(primary_name_cstr, "%s\\primary.fx", fx_dir);
	TCHAR *primary_name = tstring_from_utf8(primary_name_cstr);

	char post_name_cstr[1024];
	sprintf(post_name_cstr, "%s\\post.fx", fx_dir);
	TCHAR *post_name = tstring_from_utf8(post_name_cstr);

	char prescale_name_cstr[1024];
	sprintf(prescale_name_cstr, "%s\\prescale.fx", fx_dir);
	TCHAR *prescale_name = tstring_from_utf8(prescale_name_cstr);

	char pincushion_name_cstr[1024];
	sprintf(pincushion_name_cstr, "%s\\pincushion.fx", fx_dir);
	TCHAR *pincushion_name = tstring_from_utf8(pincushion_name_cstr);

	char phosphor_name_cstr[1024];
	sprintf(phosphor_name_cstr, "%s\\phosphor.fx", fx_dir);
	TCHAR *phosphor_name = tstring_from_utf8(phosphor_name_cstr);

	char focus_name_cstr[1024];
	sprintf(focus_name_cstr, "%s\\focus.fx", fx_dir);
	TCHAR *focus_name = tstring_from_utf8(focus_name_cstr);

	char deconverge_name_cstr[1024];
	sprintf(deconverge_name_cstr, "%s\\deconverge.fx", fx_dir);
	TCHAR *deconverge_name = tstring_from_utf8(deconverge_name_cstr);

	char color_name_cstr[1024];
	sprintf(color_name_cstr, "%s\\color.fx", fx_dir);
	TCHAR *color_name = tstring_from_utf8(color_name_cstr);

	char yiq_encode_name_cstr[1024];
	sprintf(yiq_encode_name_cstr, "%s\\yiq_encode.fx", fx_dir);
	TCHAR *yiq_encode_name = tstring_from_utf8(yiq_encode_name_cstr);

	char yiq_decode_name_cstr[1024];
	sprintf(yiq_decode_name_cstr, "%s\\yiq_decode.fx", fx_dir);
	TCHAR *yiq_decode_name = tstring_from_utf8(yiq_decode_name_cstr);

	// create the regular shader
	result = (*d3dintf->device.create_effect)(d3d->device, primary_name, &effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load primary.fx\n");
		return 1;
	}

	// create the post-processing shader
	result = (*d3dintf->device.create_effect)(d3d->device, post_name, &post_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load post.fx\n");
		return 1;
	}

	// create the prescaling shader
	result = (*d3dintf->device.create_effect)(d3d->device, prescale_name, &prescale_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load prescale.fx\n");
		return 1;
	}

	// create the pincushion shader
	result = (*d3dintf->device.create_effect)(d3d->device, pincushion_name, &pincushion_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load pincushion.fx\n");
		return 1;
	}

	// create the phosphor shader
	result = (*d3dintf->device.create_effect)(d3d->device, phosphor_name, &phosphor_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load phosphor.fx\n");
		return 1;
	}

	// create the focus shader
	result = (*d3dintf->device.create_effect)(d3d->device, focus_name, &focus_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load focus.fx\n");
		return 1;
	}

	// create the deconvergence shader
	result = (*d3dintf->device.create_effect)(d3d->device, deconverge_name, &deconverge_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load deconverge.fx\n");
		return 1;
	}

	// create the color convolution shader
	result = (*d3dintf->device.create_effect)(d3d->device, color_name, &color_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load color.fx\n");
		return 1;
	}

	// create the YIQ modulation shader
	result = (*d3dintf->device.create_effect)(d3d->device, yiq_encode_name, &yiq_encode_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load yiq_encode.fx\n");
		return 1;
	}

	// create the YIQ demodulation shader
	result = (*d3dintf->device.create_effect)(d3d->device, yiq_decode_name, &yiq_decode_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load yiq_decode.fx\n");
		return 1;
	}

	// create the vector shader
#if HLSL_VECTOR
	char vector_cstr[1024];
	sprintf(vector_cstr, "%s\\vector.fx", fx_dir);
	TCHAR *vector_name = tstring_from_utf8(vector_cstr);

	result = (*d3dintf->device.create_effect)(d3d->device, vector_name, &vector_effect);
	if(result != D3D_OK)
	{
		mame_printf_verbose("Direct3D: Unable to load vector.fx\n");
		return 1;
	}
	if (vector_name)
		osd_free(vector_name);
#endif

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
}


//============================================================
//  hlsl_info::init_effect_info
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

		(*d3dintf->effect.set_float)(curr_effect, "ScanlineOffset", (poly->texture->cur_frame == 0) ? 0.0f : options->scanline_offset);

		if(options->params_dirty)
		{
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_vector)(curr_effect, "Floor", 3, options->floor);
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
			//(*d3dintf->effect.set_float)(curr_effect, "ScanlineOffset", (poly->texture->cur_frame == 0) ? 0.0f : options->scanline_offset);
			(*d3dintf->effect.set_vector)(curr_effect, "Power", 3, options->power);
		}
	}
	else
	{
		curr_effect = effect;

		(*d3dintf->effect.set_float)(curr_effect, "FixedAlpha", 1.0f);
	}
}


//============================================================
//  hlsl_info::find_render_target
//============================================================

d3d_render_target* hlsl_info::find_render_target(d3d_texture_info *info)
{
	d3d_render_target *curr = targethead;

	UINT32 screen_index_data = (UINT32)info->texinfo.osddata;
	UINT32 screen_index = screen_index_data >> 1;
	UINT32 page_index = screen_index_data & 1;

	while (curr != NULL && (curr->screen_index != screen_index || curr->page_index != page_index || curr->width != info->texinfo.width || curr->height != info->texinfo.height))
	{
		curr = curr->next;
	}

	return curr;
}


//============================================================
//  hlsl_info::find_render_target
//============================================================

d3d_render_target* hlsl_info::find_render_target(int width, int height, UINT32 screen_index, UINT32 page_index)
{
	d3d_render_target *curr = targethead;

	while (curr != NULL && (curr->width != width || curr->height != height || curr->screen_index != screen_index || curr->page_index != page_index))
	{
		curr = curr->next;
	}

	return curr;
}


//============================================================
//  hlsl_info::find_cache_target
//============================================================

d3d_cache_target* hlsl_info::find_cache_target(UINT32 screen_index, int width, int height)
{
	d3d_cache_target *curr = cachehead;

	while (curr != NULL && (curr->screen_index != screen_index || curr->width != width || curr->height != height))
	{
		curr = curr->next;
	}

	return curr;
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

#if HLSL_VECTOR
	if(PRIMFLAG_GET_VECTOR(poly->flags) && vector_enable)
	{
		lines_pending = true;
	}
	else if (PRIMFLAG_GET_VECTORBUF(poly->flags) && vector_enable)
	{
	}
#endif

	if(PRIMFLAG_GET_SCREENTEX(d3d->last_texture_flags) && poly->texture != NULL)
	{
		d3d_render_target *rt = find_render_target(poly->texture);
		if (rt == NULL)
		{
			return;
		}
		d3d_cache_target *ct = find_cache_target(rt->screen_index, poly->texture->texinfo.width, poly->texture->texinfo.height);

		if(options->yiq_enable)
		{
			/* Convert our signal into YIQ */
			curr_effect = yiq_encode_effect;

			if(options->params_dirty)
			{
				(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
				(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
				(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
				(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
				(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
				(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
				(*d3dintf->effect.set_float)(curr_effect, "CCValue", options->yiq_cc);
				(*d3dintf->effect.set_float)(curr_effect, "AValue", options->yiq_a);
				(*d3dintf->effect.set_float)(curr_effect, "BValue", (poly->texture->cur_frame == 2) ? 0.0f : ((float)poly->texture->cur_frame * options->yiq_b));
				(*d3dintf->effect.set_float)(curr_effect, "PValue", options->yiq_p);
				(*d3dintf->effect.set_float)(curr_effect, "NotchHalfWidth", options->yiq_n);
				(*d3dintf->effect.set_float)(curr_effect, "YFreqResponse", options->yiq_y);
				(*d3dintf->effect.set_float)(curr_effect, "IFreqResponse", options->yiq_i);
				(*d3dintf->effect.set_float)(curr_effect, "QFreqResponse", options->yiq_q);
				(*d3dintf->effect.set_float)(curr_effect, "ScanTime", options->yiq_scan_time);
			}

			HRESULT result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->target[4]);

			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
			result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			/* Convert our signal from YIQ */
			curr_effect = yiq_decode_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Composite", rt->texture[4]);
			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", poly->texture->d3dfinaltex);
			if(options->params_dirty)
			{
				(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
				(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
				(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
				(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
				(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
				(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
				(*d3dintf->effect.set_float)(curr_effect, "CCValue", options->yiq_cc);
				(*d3dintf->effect.set_float)(curr_effect, "AValue", options->yiq_a);
				(*d3dintf->effect.set_float)(curr_effect, "BValue", (poly->texture->cur_frame == 2) ? 0.0f : ((float)poly->texture->cur_frame * options->yiq_b));
				(*d3dintf->effect.set_float)(curr_effect, "OValue", options->yiq_o);
				(*d3dintf->effect.set_float)(curr_effect, "PValue", options->yiq_p);
				(*d3dintf->effect.set_float)(curr_effect, "NotchHalfWidth", options->yiq_n);
				(*d3dintf->effect.set_float)(curr_effect, "YFreqResponse", options->yiq_y);
				(*d3dintf->effect.set_float)(curr_effect, "IFreqResponse", options->yiq_i);
				(*d3dintf->effect.set_float)(curr_effect, "QFreqResponse", options->yiq_q);
				(*d3dintf->effect.set_float)(curr_effect, "ScanTime", options->yiq_scan_time);
			}

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->target[3]);

			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
			result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			curr_effect = color_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[3]);
		}

		curr_effect = color_effect;

		/* Render the initial color-convolution pass */
		if(options->params_dirty)
		{
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", options->yiq_enable ? 1.0f : (1.0f / (poly->texture->ustop - poly->texture->ustart)));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", options->yiq_enable ? 1.0f : (1.0f / (poly->texture->vstop - poly->texture->vstart)));
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "YIQEnable", options->yiq_enable ? 1.0f : 0.0f);
			(*d3dintf->effect.set_vector)(curr_effect, "RedRatios", 3, options->red_ratio);
			(*d3dintf->effect.set_vector)(curr_effect, "GrnRatios", 3, options->grn_ratio);
			(*d3dintf->effect.set_vector)(curr_effect, "BluRatios", 3, options->blu_ratio);
			(*d3dintf->effect.set_vector)(curr_effect, "Offset", 3, options->offset);
			(*d3dintf->effect.set_vector)(curr_effect, "Scale", 3, options->scale);
			(*d3dintf->effect.set_float)(curr_effect, "Saturation", options->saturation);
		}

		HRESULT result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->smalltarget);

		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Pre-scaling pass */
		curr_effect = prescale_effect;
		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->smalltexture);

		if(options->params_dirty)
		{
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
		}

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->prescaletarget);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Deconverge pass */
		curr_effect = deconverge_effect;
		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->prescaletexture);

		if(options->params_dirty)
		{
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
			(*d3dintf->effect.set_vector)(curr_effect, "ConvergeX", 3, options->converge_x);
			(*d3dintf->effect.set_vector)(curr_effect, "ConvergeY", 3, options->converge_y);
			(*d3dintf->effect.set_vector)(curr_effect, "RadialConvergeX", 3, options->radial_converge_x);
			(*d3dintf->effect.set_vector)(curr_effect, "RadialConvergeY", 3, options->radial_converge_y);
		}

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->target[2]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		float defocus_x = options->defocus[0];
		float defocus_y = options->defocus[1];
		bool focus_enable = defocus_x != 0.0f || defocus_y != 0.0f;
		if(focus_enable)
		{
			/* Defocus pass 1 */
			curr_effect = focus_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[2]);

			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", poly->texture != NULL ? (1.0f / (poly->texture->ustop - poly->texture->ustart)) : 0.0f);
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", poly->texture != NULL ? (1.0f / (poly->texture->vstop - poly->texture->vstart)) : 0.0f);
			(*d3dintf->effect.set_vector)(curr_effect, "Defocus", 2, &options->defocus[0]);
			(*d3dintf->effect.set_float)(curr_effect, "FocusEnable", (defocus_x == 0.0f && defocus_y == 0.0f) ? 0.0f : 1.0f);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->target[0]);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 6\n", (int)result);
			result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			/* Defocus pass 2 */

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[0]);

			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f);
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f);
			(*d3dintf->effect.set_vector)(curr_effect, "Defocus", 2, &options->defocus[0]);
			(*d3dintf->effect.set_float)(curr_effect, "FocusEnable", (defocus_x == 0.0f && defocus_y == 0.0f) ? 0.0f : 1.0f);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->target[1]);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 7\n", (int)result);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);
		}

		// Simulate phosphorescence. This should happen after the shadow/scanline pass, but since
		// the phosphors are a direct result of the incoming texture, might as well just change the
		// input texture.
		curr_effect = phosphor_effect;

		if(options->params_dirty)
		{
			(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
			(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
			(*d3dintf->effect.set_float)(curr_effect, "RawWidth", (float)poly->texture->rawwidth);
			(*d3dintf->effect.set_float)(curr_effect, "RawHeight", (float)poly->texture->rawheight);
			(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", 1.0f / (poly->texture->ustop - poly->texture->ustart));
			(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", 1.0f / (poly->texture->vstop - poly->texture->vstart));
			(*d3dintf->effect.set_vector)(curr_effect, "Phosphor", 3, options->phosphor);
		}
		(*d3dintf->effect.set_float)(curr_effect, "TextureWidth", (float)rt->target_width);
		(*d3dintf->effect.set_float)(curr_effect, "TextureHeight", (float)rt->target_height);
		(*d3dintf->effect.set_float)(curr_effect, "Passthrough", 0.0f);

		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", focus_enable ? rt->texture[1] : rt->texture[2]);
		(*d3dintf->effect.set_texture)(curr_effect, "LastPass", ct->last_texture);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, rt->target[0]);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 4\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Pass along our phosphor'd screen */
		curr_effect = phosphor_effect;

		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[0]);
		(*d3dintf->effect.set_texture)(curr_effect, "LastPass", rt->texture[0]);
		(*d3dintf->effect.set_float)(curr_effect, "Passthrough", 1.0f);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, ct->last_target); // Avoid changing targets due to page flipping
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call 5\n", (int)result);
		result = (*d3dintf->device.clear)(d3d->device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device clear call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, D3DPT_TRIANGLELIST, 0, 2);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		/* Scanlines and shadow mask, at high res for AVI logging*/
		if(avi_output_file != NULL)
		{
			curr_effect = post_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[0]);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, avi_final_target);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);
		}

		if(render_snap)
		{
			curr_effect = post_effect;

			(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[0]);

			result = (*d3dintf->device.set_render_target)(d3d->device, 0, snap_target);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

			(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

			for (UINT pass = 0; pass < num_passes; pass++)
			{
				(*d3dintf->effect.begin_pass)(curr_effect, pass);
				// add the primitives
				result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
				if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
				(*d3dintf->effect.end_pass)(curr_effect);
			}

			(*d3dintf->effect.end)(curr_effect);

			snap_rendered = true;
		}

		/* Scanlines and shadow mask */
		curr_effect = post_effect;

		(*d3dintf->effect.set_texture)(curr_effect, "Diffuse", rt->texture[0]);

		result = (*d3dintf->device.set_render_target)(d3d->device, 0, backbuffer);
		if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device set_render_target call\n", (int)result);

		(*d3dintf->effect.begin)(curr_effect, &num_passes, 0);

		for (UINT pass = 0; pass < num_passes; pass++)
		{
			(*d3dintf->effect.begin_pass)(curr_effect, pass);
			// add the primitives
			result = (*d3dintf->device.draw_primitive)(d3d->device, poly->type, vertnum, poly->count);
			if (result != D3D_OK) mame_printf_verbose("Direct3D: Error %08X during device draw_primitive call\n", (int)result);
			(*d3dintf->effect.end_pass)(curr_effect);
		}

		(*d3dintf->effect.end)(curr_effect);

		poly->texture->cur_frame++;
		poly->texture->cur_frame %= options->yiq_phase_count;

		options->params_dirty = false;
	}
#if HLSL_VECTOR
	else if(PRIMFLAG_GET_VECTOR(poly->flags) && vector_enable)
	{
	}
#endif
	else
	{
		(*d3dintf->effect.set_float)(curr_effect, "RawWidth", poly->texture != NULL ? (float)poly->texture->rawwidth : 8.0f);
		(*d3dintf->effect.set_float)(curr_effect, "RawHeight", poly->texture != NULL ? (float)poly->texture->rawheight : 8.0f);
		(*d3dintf->effect.set_float)(curr_effect, "WidthRatio", poly->texture != NULL ? (1.0f / (poly->texture->ustop - poly->texture->ustart)) : 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "HeightRatio", poly->texture != NULL ? (1.0f / (poly->texture->vstop - poly->texture->vstart)) : 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "TargetWidth", (float)d3d->width);
		(*d3dintf->effect.set_float)(curr_effect, "TargetHeight", (float)d3d->height);
		(*d3dintf->effect.set_float)(curr_effect, "PostPass", 0.0f);
		(*d3dintf->effect.set_float)(curr_effect, "PincushionAmount", options->pincushion);

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

	(*d3dintf->surface.release)(backbuffer);
}


//============================================================
//  hlsl_info::register_prescaled_texture
//============================================================

bool hlsl_info::register_prescaled_texture(d3d_texture_info *texture)
{
	return register_texture(texture, texture->rawwidth, texture->rawheight, texture->xprescale, texture->yprescale);
}


//============================================================
//  hlsl_info::add_cache_target - register a cache target
//============================================================
bool hlsl_info::add_cache_target(d3d_info* d3d, d3d_texture_info* info, int width, int height, int xprescale, int yprescale, int screen_index)
{
	d3d_cache_target* target = (d3d_cache_target*)global_alloc_clear(d3d_cache_target);

	if (!target->init(d3d, d3dintf, width, height, xprescale, yprescale))
	{
		global_free(target);
		return false;
	}

	target->width = info->texinfo.width;
	target->height = info->texinfo.height;

	target->next = cachehead;
	target->prev = NULL;

	target->screen_index = screen_index;

	if (cachehead != NULL)
	{
		cachehead->prev = target;
	}
	cachehead = target;

	return true;
}

d3d_render_target* hlsl_info::get_vector_target(d3d_info *d3d)
{
#if HLSL_VECTOR
	if (!vector_enable)
	{
		return false;
	}

	return find_render_target(d3d->width, d3d->height, 0, 0);
#endif
	return NULL;
}

void hlsl_info::create_vector_target(d3d_info *d3d, render_primitive *prim)
{
#if HLSL_VECTOR
	if (!add_render_target(d3d, NULL, d3d->width, d3d->height, 1, 1))
	{
		vector_enable = false;
	}
#endif
}

//============================================================
//  hlsl_info::add_render_target - register a render target
//============================================================

bool hlsl_info::add_render_target(d3d_info* d3d, d3d_texture_info* info, int width, int height, int xprescale, int yprescale)
{
	UINT32 screen_index = 0;
	UINT32 page_index = 0;
	if (info != NULL)
	{
		if (find_render_target(info))
		{
			remove_render_target(info);
		}

		UINT32 screen_index_data = (UINT32)info->texinfo.osddata;
		screen_index = screen_index_data >> 1;
		page_index = screen_index_data & 1;
	}

	d3d_render_target* target = (d3d_render_target*)global_alloc_clear(d3d_render_target);

	if (!target->init(d3d, d3dintf, width, height, xprescale, yprescale))
	{
		global_free(target);
		return false;
	}

	if (info != NULL)
	{
		target->width = info->texinfo.width;
		target->height = info->texinfo.height;
	}
	else
	{
		target->width = d3d->width;
		target->height = d3d->height;
	}

	target->screen_index = screen_index;
	target->page_index = page_index;

	d3d_cache_target* cache = find_cache_target(target->screen_index, target->width, target->height);
	if (cache == NULL)
	{
		if (!add_cache_target(d3d, info, width, height, xprescale, yprescale, target->screen_index))
		{
			global_free(target);
			return false;
		}
	}

	target->next = targethead;
	target->prev = NULL;

	if (targethead != NULL)
	{
		targethead->prev = target;
	}
	targethead = target;

	return true;
}

//============================================================
//  hlsl_info::enumerate_screens
//============================================================
void hlsl_info::enumerate_screens()
{
	screen_device_iterator iter(window->machine().root_device());
	num_screens = iter.count();
}


//============================================================
//  hlsl_info::register_texture
//============================================================

bool hlsl_info::register_texture(d3d_texture_info *texture)
{
	return register_texture(texture, texture->rawwidth, texture->rawheight, 1, 1);
}


//============================================================
//  hlsl_info::register_texture(d3d_texture_info, int, int, int, int)
//============================================================

bool hlsl_info::register_texture(d3d_texture_info *texture, int width, int height, int xscale, int yscale)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return 0;

	enumerate_screens();

	d3d_info *d3d = (d3d_info *)window->drawdata;

	int hlsl_prescale_x = prescale_force_x;
	int hlsl_prescale_y = prescale_force_y;

	// Find the nearest prescale factor that is over our screen size
	if (hlsl_prescale_x == 0)
	{
		hlsl_prescale_x = 1;
		while (width * xscale * hlsl_prescale_x <= d3d->width)
		{
			hlsl_prescale_x++;
		}
		hlsl_prescale_x--;
	}

	if (hlsl_prescale_y == 0)
	{
		hlsl_prescale_y = 1;
		while (height * yscale * hlsl_prescale_y <= d3d->height)
		{
			hlsl_prescale_y++;
		}
		hlsl_prescale_y--;
	}

	hlsl_prescale_x = ((hlsl_prescale_x == 0) ? 1 : hlsl_prescale_x);
	hlsl_prescale_y = ((hlsl_prescale_y == 0) ? 1 : hlsl_prescale_y);

	if (!add_render_target(d3d, texture, width, height, xscale * hlsl_prescale_x, yscale * hlsl_prescale_y))
		return false;

	options->params_dirty = true;

	return true;
}

//============================================================
//  hlsl_info::delete_resources
//============================================================

void hlsl_info::delete_resources(bool reset)
{
	if (!master_enable || !d3dintf->post_fx_available)
		return;

	initialized = false;

	if(write_ini && !reset)
	{
		emu_file file(downcast<windows_options &>(window->machine().options()).screen_post_fx_dir(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		file_error filerr = open_next((d3d_info*)window->drawdata, file, downcast<windows_options &>(window->machine().options()).hlsl_ini_name(), "ini", 0);

		if (filerr != FILERR_NONE)
			return;

		file.printf("hlsl_enable            %d\n", master_enable ? 1 : 0);
		file.printf("hlsl_prescale_x        %d\n", prescale_force_x);
		file.printf("hlsl_prescale_y        %d\n", prescale_force_y);
		file.printf("hlsl_preset            %d\n", preset);
		file.printf("hlsl_snap_width        %d\n", snap_width);
		file.printf("hlsl_snap_height       %d\n", snap_height);
		file.printf("shadow_mask_alpha      %f\n", options->shadow_mask_alpha);
		file.printf("shadow_mask_texture    %s\n", options->shadow_mask_texture);
		file.printf("shadow_mask_x_count    %d\n", options->shadow_mask_count_x);
		file.printf("shadow_mask_y_count    %d\n", options->shadow_mask_count_y);
		file.printf("shadow_mask_usize      %f\n", options->shadow_mask_u_size);
		file.printf("shadow_mask_vsize      %f\n", options->shadow_mask_v_size);
		file.printf("curvature              %f\n", options->curvature);
		file.printf("pincushion             %f\n", options->pincushion);
		file.printf("scanline_alpha         %f\n", options->scanline_alpha);
		file.printf("scanline_size          %f\n", options->scanline_scale);
		file.printf("scanline_height        %f\n", options->scanline_height);
		file.printf("scanline_bright_scale  %f\n", options->scanline_bright_scale);
		file.printf("scanline_bright_offset %f\n", options->scanline_bright_offset);
		file.printf("scanline_jitter        %f\n", options->scanline_offset);
		file.printf("defocus                %f,%f\n", options->defocus[0], options->defocus[1]);
		file.printf("converge_x             %f,%f,%f\n", options->converge_x[0], options->converge_x[1], options->converge_x[2]);
		file.printf("converge_y             %f,%f,%f\n", options->converge_y[0], options->converge_y[1], options->converge_y[2]);
		file.printf("radial_converge_x      %f,%f,%f\n", options->radial_converge_x[0], options->radial_converge_x[1], options->radial_converge_x[2]);
		file.printf("radial_converge_y      %f,%f,%f\n", options->radial_converge_y[0], options->radial_converge_y[1], options->radial_converge_y[2]);
		file.printf("red_ratio              %f,%f,%f\n", options->red_ratio[0], options->red_ratio[1], options->red_ratio[2]);
		file.printf("grn_ratio              %f,%f,%f\n", options->grn_ratio[0], options->grn_ratio[1], options->grn_ratio[2]);
		file.printf("blu_ratio              %f,%f,%f\n", options->blu_ratio[0], options->blu_ratio[1], options->blu_ratio[2]);
		file.printf("saturation             %f\n", options->saturation);
		file.printf("offset                 %f,%f,%f\n", options->offset[0], options->offset[1], options->offset[2]);
		file.printf("scale                  %f,%f,%f\n", options->scale[0], options->scale[1], options->scale[2]);
		file.printf("power                  %f,%f,%f\n", options->power[0], options->power[1], options->power[2]);
		file.printf("floor                  %f,%f,%f\n", options->floor[0], options->floor[1], options->floor[2]);
		file.printf("phosphor_life          %f,%f,%f\n", options->phosphor[0], options->phosphor[1], options->phosphor[2]);
		file.printf("yiq_enable             %d\n", options->yiq_enable ? 1 : 0);
		file.printf("yiq_cc                 %f\n", options->yiq_cc);
		file.printf("yiq_a                  %f\n", options->yiq_a);
		file.printf("yiq_b                  %f\n", options->yiq_b);
		file.printf("yiq_o                  %f\n", options->yiq_o);
		file.printf("yiq_p                  %f\n", options->yiq_p);
		file.printf("yiq_n                  %f\n", options->yiq_n);
		file.printf("yiq_y                  %f\n", options->yiq_y);
		file.printf("yiq_i                  %f\n", options->yiq_i);
		file.printf("yiq_q                  %f\n", options->yiq_q);
		file.printf("yiq_scan_time          %f\n", options->yiq_scan_time);
		file.printf("yiq_phase_count        %d\n", options->yiq_phase_count);
	}

	while (targethead != NULL)
	{
		remove_render_target(targethead);
	}

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

	shadow_bitmap.reset();
}


//============================================================
//  get_vector
//============================================================

static void get_vector(const char *data, int count, float *out, int report_error)
{
	if (count > 3)
	{
		if (sscanf(data, "%f,%f,%f,%f", &out[0], &out[1], &out[2], &out[3]) < 4 && report_error)
			mame_printf_error("Illegal quad vector value = %s\n", data);
	}
	else if(count > 2)
	{
		if (sscanf(data, "%f,%f,%f", &out[0], &out[1], &out[2]) < 3 && report_error)
			mame_printf_error("Illegal triple vector value = %s\n", data);
	}
	else if(count > 1)
	{
		if (sscanf(data, "%f,%f", &out[0], &out[1]) < 2 && report_error)
			mame_printf_error("Illegal double vector value = %s\n", data);
	}
	else if(count > 0)
	{
		if (sscanf(data, "%f", &out[0]) < 1 && report_error)
			mame_printf_error("Illegal single vector value = %s\n", data);
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
	options->params_dirty = true;
	return options->shadow_mask_count_x;
}

static INT32 slider_shadow_mask_y_count(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	hlsl_options *options = (hlsl_options*)arg;
	if (newval != SLIDER_NOCHANGE) options->shadow_mask_count_y = newval;
	if (string != NULL) string->printf("%d", options->shadow_mask_count_y);
	options->params_dirty = true;
	return options->shadow_mask_count_y;
}

static INT32 slider_shadow_mask_usize(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_u_size), 1.0f / 32.0f, "%2.5f", string, newval);
}

static INT32 slider_shadow_mask_vsize(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->shadow_mask_v_size), 1.0f / 32.0f, "%2.5f", string, newval);
}

static INT32 slider_curvature(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->curvature), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_pincushion(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->pincushion), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_scanline_alpha(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_alpha), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_scanline_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_scale), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_height(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_height), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_bright_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_bright_scale), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_bright_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_bright_offset), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_scanline_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scanline_offset), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_defocus_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->defocus[0]), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_defocus_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->defocus[1]), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_post_defocus_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->defocus[2]), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_post_defocus_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->defocus[3]), 0.5f, "%2.1f", string, newval);
}

static INT32 slider_red_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_x[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_y[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_x[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_y[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_x[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->converge_y[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_x[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_y[0]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_x[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_green_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_y[1]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_radial_converge_x(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_x[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_blue_radial_converge_y(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->radial_converge_y[2]), 0.1f, "%3.1f", string, newval);
}

static INT32 slider_red_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->red_ratio[0]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->red_ratio[1]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->red_ratio[2]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->grn_ratio[0]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->grn_ratio[1]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_green_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->grn_ratio[2]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_r(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->blu_ratio[0]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_g(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->blu_ratio[1]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_blue_from_b(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->blu_ratio[2]), 0.005f, "%2.3f", string, newval);
}

static INT32 slider_red_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->offset[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->offset[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_offset(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->offset[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scale[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scale[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_scale(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->scale[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->power[0]), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_green_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->power[1]), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_blue_power(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->power[2]), 0.05f, "%2.2f", string, newval);
}

static INT32 slider_red_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->floor[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->floor[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_floor(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->floor[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_red_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->phosphor[0]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_green_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->phosphor[1]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_blue_phosphor_life(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
	return slider_set(&(((hlsl_options*)arg)->phosphor[2]), 0.01f, "%2.2f", string, newval);
}

static INT32 slider_saturation(running_machine &machine, void *arg, astring *string, INT32 newval)
{
	((hlsl_options*)arg)->params_dirty = true;
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

	slider_state *listhead = NULL;
	slider_state **tailptr = &listhead;
	astring string;

	*tailptr = slider_alloc(window->machine(), "Shadow Mask Darkness", 0, 0, 100, 1, slider_shadow_mask_alpha, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask X Count", 1, 640, 1024, 1, slider_shadow_mask_x_count, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask Y Count", 1, 480, 1024, 1, slider_shadow_mask_y_count, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask Pixel Count X", 1, 3, 32, 1, slider_shadow_mask_usize, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Shadow Mask Pixel Count Y", 1, 3, 32, 1, slider_shadow_mask_vsize, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Screen Curvature", 0, 0, 100, 1, slider_curvature, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Image Pincushion", 0, 0, 100, 1, slider_pincushion, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Darkness", 0, 0, 100, 1, slider_scanline_alpha, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Screen Height", 1, 20, 80, 1, slider_scanline_scale, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Indiv. Height", 1, 10, 80, 1, slider_scanline_height, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Brightness", 0, 20, 40, 1, slider_scanline_bright_scale, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Brightness Overdrive", 0, 12, 20, 1, slider_scanline_bright_offset, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Scanline Jitter", 0, 0, 40, 1, slider_scanline_offset, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Defocus X", 0, 0, 64, 1, slider_defocus_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Defocus Y", 0, 0, 64, 1, slider_defocus_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Phosphor Defocus X", 0, 0, 64, 1, slider_post_defocus_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Phosphor Defocus Y", 0, 0, 64, 1, slider_post_defocus_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Position Offset X", -1500, 0, 1500, 1, slider_red_converge_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Position Offset Y", -1500, 0, 1500, 1, slider_red_converge_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Position Offset X", -1500, 0, 1500, 1, slider_green_converge_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Position Offset Y", -1500, 0, 1500, 1, slider_green_converge_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Position Offset X", -1500, 0, 1500, 1, slider_blue_converge_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Position Offset Y", -1500, 0, 1500, 1, slider_blue_converge_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Convergence X", -1500, 0, 1500, 1, slider_red_radial_converge_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Convergence Y", -1500, 0, 1500, 1, slider_red_radial_converge_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Convergence X", -1500, 0, 1500, 1, slider_green_radial_converge_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Convergence Y", -1500, 0, 1500, 1, slider_green_radial_converge_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Convergence X", -1500, 0, 1500, 1, slider_blue_radial_converge_x, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Convergence Y", -1500, 0, 1500, 1, slider_blue_radial_converge_y, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Output from Red Input", -400, 0, 400, 5, slider_red_from_r, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Output from Green Input", -400, 0, 400, 5, slider_red_from_g, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Output from Blue Input", -400, 0, 400, 5, slider_red_from_b, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Output from Red Input", -400, 0, 400, 5, slider_green_from_r, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Output from Green Input", -400, 0, 400, 5, slider_green_from_g, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Output from Blue Input", -400, 0, 400, 5, slider_green_from_b, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Output from Red Input", -400, 0, 400, 5, slider_blue_from_r, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Output from Green Input", -400, 0, 400, 5, slider_blue_from_g, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Output from Blue Input", -400, 0, 400, 5, slider_blue_from_b, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red DC Offset", -100, 0, 100, 1, slider_red_offset, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green DC Offset", -100, 0, 100, 1, slider_green_offset, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue DC Offset", -100, 0, 100, 1, slider_blue_offset, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Scale", -200, 100, 200, 1, slider_red_scale, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Scale", -200, 100, 200, 1, slider_green_scale, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Scale", -200, 100, 200, 1, slider_blue_scale, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Power", -80, 20, 80, 1, slider_red_power, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Power", -80, 20, 80, 1, slider_green_power, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Power", -80, 20, 80, 1, slider_blue_power, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Floor", 0, 0, 100, 1, slider_red_floor, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Floor", 0, 0, 100, 1, slider_green_floor, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Floor", 0, 0, 100, 1, slider_blue_floor, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Red Phosphor Life", 0, 0, 100, 1, slider_red_phosphor_life, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Green Phosphor Life", 0, 0, 100, 1, slider_green_phosphor_life, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Blue Phosphor Life", 0, 0, 100, 1, slider_blue_phosphor_life, (void*)options); tailptr = &(*tailptr)->next;
	*tailptr = slider_alloc(window->machine(), "Saturation", 0, 100, 400, 1, slider_saturation, (void*)options); tailptr = &(*tailptr)->next;

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

static file_error open_next(d3d_info *d3d, emu_file &file, const char *templ, const char *extension, int idx)
{
	UINT32 origflags = file.openflags();

	// handle defaults
	const char *snapname = templ ? templ : d3d->window->machine().options().snap_name();

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
				fatalerror("Something very wrong is going on!!!\n");

			// copy the device name to an astring
			astring snapdevname;
			snapdevname.cpysubstr(snapstr, pos + 3, end - pos - 3);

			// verify that there is such a device for this system
			image_interface_iterator iter(d3d->window->machine().root_device());
			for (device_image_interface *image = iter.first(); image != NULL; iter.next())
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
