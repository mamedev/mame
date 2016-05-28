// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  d3dhlsl.cpp - Win32 Direct3D HLSL implementation
//
//============================================================

// MAME headers
#include "emu.h"
#include "drivenum.h"
#include "render.h"
#include "rendutil.h"
#include "emuopts.h"
#include "aviio.h"
#include "png.h"
#include "screen.h"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "modules/render/drawd3d.h"
#include "d3dcomm.h"
#include "strconv.h"
#include "d3dhlsl.h"
#include "../frontend/mame/ui/slider.h"


//============================================================
//  GLOBALS
//============================================================


//============================================================
//  PROTOTYPES
//============================================================

static void get_vector(const char *data, int count, float *out, bool report_error);


//============================================================
//  shader manager constructor
//============================================================

shaders::shaders() :
	d3dintf(nullptr), machine(nullptr), d3d(nullptr), num_screens(0), curr_screen(0),
	avi_output_file(nullptr), avi_frame(0), avi_copy_surface(nullptr), avi_copy_texture(nullptr), avi_final_target(nullptr), avi_final_texture(nullptr),
	black_surface(nullptr), black_texture(nullptr), render_snap(false), snap_rendered(false), snap_copy_target(nullptr), snap_copy_texture(nullptr), snap_target(nullptr), snap_texture(nullptr),
	snap_width(0), snap_height(0), lines_pending(false), backbuffer(nullptr), curr_effect(nullptr), default_effect(nullptr), prescale_effect(nullptr), post_effect(nullptr), distortion_effect(nullptr),
	focus_effect(nullptr), phosphor_effect(nullptr), deconverge_effect(nullptr), color_effect(nullptr), ntsc_effect(nullptr), bloom_effect(nullptr),
	downsample_effect(nullptr), vector_effect(nullptr), fsfx_vertices(nullptr), curr_texture(nullptr), curr_render_target(nullptr), curr_poly(nullptr),
	d3dx_create_effect_from_file_ptr("D3DXCreateEffectFromFileW", { L"d3dx9_43.dll" })
{
	master_enable = false;
	vector_enable = true;
	oversampling_enable = false;
	shadow_texture = nullptr;
	options = nullptr;
	paused = true;
	lastidx = -1;
	targethead = nullptr;
	cachehead = nullptr;
	initialized = false;
}


//============================================================
//  shaders destructor
//============================================================

shaders::~shaders()
{
	for (slider* slider : internal_sliders)
	{
		delete slider;
	}

	cache_target *currcache = cachehead;
	while(cachehead != nullptr)
	{
		cachehead = currcache->next;
		global_free(currcache);
		currcache = cachehead;
	}

	d3d_render_target *currtarget = targethead;
	while(targethead != nullptr)
	{
		targethead = currtarget->next;
		global_free(currtarget);
		currtarget = targethead;
	}
}


//============================================================
//  shaders::window_save
//============================================================

void shaders::window_save()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	HRESULT result = d3d->get_device()->CreateTexture(snap_width, snap_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &snap_copy_texture, nullptr);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Unable to init system-memory target for HLSL snapshot (%08x), bailing\n", (UINT32)result);
		return;
	}
	snap_copy_texture->GetSurfaceLevel(0, &snap_copy_target);

	result = d3d->get_device()->CreateTexture(snap_width, snap_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &snap_texture, nullptr);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Unable to init video-memory target for HLSL snapshot (%08x), bailing\n", (UINT32)result);
		return;
	}
	snap_texture->GetSurfaceLevel(0, &snap_target);

	render_snap = true;
	snap_rendered = false;
}


//============================================================
//  shaders::window_record
//============================================================

void shaders::window_record()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	windows_options &options = downcast<windows_options &>(machine->options());
	const char *filename = options.d3d_hlsl_write();

	if (avi_output_file != nullptr)
	{
		end_avi_recording();
	}
	else if (filename[0] != 0)
	{
		begin_avi_recording(filename);
	}
}


//============================================================
//  shaders::avi_update_snap
//============================================================

void shaders::avi_update_snap(IDirect3DSurface9 *surface)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	D3DLOCKED_RECT rect;

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (!avi_snap.valid() || (int)snap_width != avi_snap.width() || (int)snap_height != avi_snap.height())
	{
		avi_snap.allocate((int)snap_width, (int)snap_height);
	}

	// copy the texture
	HRESULT result = d3d->get_device()->GetRenderTargetData(surface, avi_copy_surface);
	if (FAILED(result))
	{
		return;
	}

	// lock the texture
	result = avi_copy_surface->LockRect(&rect, nullptr, D3DLOCK_DISCARD);
	if (FAILED(result))
	{
		return;
	}

	// loop over Y
	for (int srcy = 0; srcy < (int)snap_height; srcy++)
	{
		DWORD *src = (DWORD *)((BYTE *)rect.pBits + srcy * rect.Pitch);
		UINT32 *dst = &avi_snap.pix32(srcy);

		for (int x = 0; x < snap_width; x++)
		{
			*dst++ = *src++;
		}
	}

	// unlock
	result = avi_copy_surface->UnlockRect();
	if (FAILED(result))
		osd_printf_verbose("Direct3D: Error %08lX during texture UnlockRect call\n", result);
}



//============================================================
//  hlsl_render_snapshot
//============================================================

void shaders::render_snapshot(IDirect3DSurface9 *surface)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	D3DLOCKED_RECT rect;

	render_snap = false;

	// if we don't have a bitmap, or if it's not the right size, allocate a new one
	if (!avi_snap.valid() || snap_width != avi_snap.width() || snap_height != avi_snap.height())
	{
		avi_snap.allocate(snap_width, snap_height);
	}

	// copy the texture
	HRESULT result = d3d->get_device()->GetRenderTargetData(surface, snap_copy_target);
	if (FAILED(result))
	{
		return;
	}

	// lock the texture
	result = snap_copy_target->LockRect(&rect, nullptr, D3DLOCK_DISCARD);
	if (FAILED(result))
	{
		return;
	}

	// loop over Y
	for (int srcy = 0; srcy < snap_height; srcy++)
	{
		DWORD *src = (DWORD *)((BYTE *)rect.pBits + srcy * rect.Pitch);
		UINT32 *dst = &avi_snap.pix32(srcy);

		for (int x = 0; x < snap_width; x++)
		{
			*dst++ = *src++;
		}
	}

	emu_file file(machine->options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	osd_file::error filerr = machine->video().open_next(file, "png");
	if (filerr != osd_file::error::NONE)
	{
		return;
	}

	// add two text entries describing the image
	std::string text1 = std::string(emulator_info::get_appname()).append(" ").append(emulator_info::get_build_version());
	std::string text2 = std::string(machine->system().manufacturer).append(" ").append(machine->system().description);
	png_info pnginfo = { nullptr };
	png_add_text(&pnginfo, "Software", text1.c_str());
	png_add_text(&pnginfo, "System", text2.c_str());

	// now do the actual work
	png_error error = png_write_bitmap(file, &pnginfo, avi_snap, 1 << 24, nullptr);
	if (error != PNGERR_NONE)
	{
		osd_printf_error("Error generating PNG for HLSL snapshot: png_error = %d\n", error);
	}

	// free any data allocated
	png_free(&pnginfo);

	// unlock
	result = snap_copy_target->UnlockRect();
	if (FAILED(result))
		osd_printf_verbose("Direct3D: Error %08lX during texture UnlockRect call\n", result);

	if (snap_texture != nullptr)
	{
		snap_texture->Release();
		snap_texture = nullptr;
	}

	if (snap_target != nullptr)
	{
		snap_target->Release();
		snap_target = nullptr;
	}

	if (snap_copy_texture != nullptr)
	{
		snap_copy_texture->Release();
		snap_copy_texture = nullptr;
	}

	if (snap_copy_target != nullptr)
	{
		snap_copy_target->Release();
		snap_copy_target = nullptr;
	}
}


//============================================================
//  shaders::record_texture
//============================================================

void shaders::record_texture()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	IDirect3DSurface9 *surface = avi_final_target;

	// ignore if nothing to do
	if (avi_output_file == nullptr || surface == nullptr)
	{
		return;
	}

	// get the current time
	attotime curtime = machine->time();

	avi_update_snap(surface);

	// loop until we hit the right time
	while (avi_next_frame_time <= curtime)
	{
		// handle an AVI recording
		// write the next frame
		avi_file::error avierr = avi_output_file->append_video_frame(avi_snap);
		if (avierr != avi_file::error::NONE)
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
//  shaders::end_hlsl_avi_recording
//============================================================

void shaders::end_avi_recording()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	if (avi_output_file)
	{
		avi_output_file.reset();
	}

	avi_output_file = nullptr;
	avi_frame = 0;
}


//============================================================
//  shaders::toggle
//============================================================

void shaders::toggle(std::vector<ui::menu_item>& sliders)
{
	if (master_enable)
	{
		if (initialized)
		{
			// free shader resources before renderer resources
			delete_resources(false);
		}

		master_enable = !master_enable;

		// free shader resources and re-create
		d3d->device_delete_resources();
		d3d->device_create_resources();
	}
	else
	{
		master_enable = !master_enable;

		// free shader resources and re-create
		d3d->device_delete_resources();
		d3d->device_create_resources();

		if (!initialized)
		{
			// re-create shader resources after renderer resources
			bool failed = create_resources(false, sliders);
			if (failed)
			{
				master_enable = false;
			}
		}
	}
}

//============================================================
//  shaders::begin_avi_recording
//============================================================

void shaders::begin_avi_recording(const char *name)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	// stop any existing recording
	end_avi_recording();

	// reset the state
	avi_frame = 0;
	avi_next_frame_time = machine->time();

	// build up information about this new movie
	avi_file::movie_info info;
	info.video_format = 0;
	info.video_timescale = 1000 * ((machine->first_screen() != nullptr) ? ATTOSECONDS_TO_HZ(machine->first_screen()->frame_period().m_attoseconds) : screen_device::DEFAULT_FRAME_RATE);
	info.video_sampletime = 1000;
	info.video_numsamples = 0;
	info.video_width = snap_width;
	info.video_height = snap_height;
	info.video_depth = 24;

	info.audio_format = 0;
	info.audio_timescale = machine->sample_rate();
	info.audio_sampletime = 1;
	info.audio_numsamples = 0;
	info.audio_channels = 2;
	info.audio_samplebits = 16;
	info.audio_samplerate = machine->sample_rate();

	// create a new temporary movie file
	osd_file::error filerr;
	std::string fullpath;
	{
		emu_file tempfile(machine->options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		if (name != nullptr)
		{
			filerr = tempfile.open(name);
		}
		else
		{
			filerr = machine->video().open_next(tempfile, "avi");
		}

		// compute the frame time
		{
			avi_frame_period = attotime::from_seconds(1000) / info.video_timescale;
		}

		// if we succeeded, make a copy of the name and create the real file over top
		if (filerr == osd_file::error::NONE)
		{
			fullpath = tempfile.fullpath();
		}
	}

	if (filerr == osd_file::error::NONE)
	{
		// create the file and free the string
		avi_file::error avierr = avi_file::create(fullpath, info, avi_output_file);
		if (avierr != avi_file::error::NONE)
		{
			osd_printf_error("Error creating AVI: %s\n", avi_file::error_string(avierr));
		}
	}
}


//============================================================
//  remove_cache_target - remove an active cache target when
//  refcount hits zero
//============================================================

void shaders::remove_cache_target(cache_target *cache)
{
	if (cache != nullptr)
	{
		if (cache == cachehead)
		{
			cachehead = cachehead->next;
		}

		if (cache->prev != nullptr)
		{
			cache->prev->next = cache->next;
		}

		if (cache->next != nullptr)
		{
			cache->next->prev = cache->prev;
		}

		global_free(cache);
	}
}


//============================================================
//  remove_render_target - remove an active target
//============================================================

void shaders::remove_render_target(texture_info *texture)
{
	remove_render_target(find_render_target(texture));
}

void shaders::remove_render_target(int source_width, int source_height, UINT32 screen_index, UINT32 page_index)
{
	d3d_render_target *target = find_render_target(source_width, source_height, screen_index, page_index);
	if (target != nullptr)
	{
		remove_render_target(target);
	}
}

void shaders::remove_render_target(d3d_render_target *rt)
{
	if (rt != nullptr)
	{
		if (rt == targethead)
		{
			targethead = targethead->next;
		}

		if (rt->prev != nullptr)
		{
			rt->prev->next = rt->next;
		}

		if (rt->next != nullptr)
		{
			rt->next->prev = rt->prev;
		}

		cache_target *cache = find_cache_target(rt->screen_index, rt->width, rt->height);
		if (cache != nullptr)
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
//  shaders::set_texture
//============================================================

void shaders::set_texture(texture_info *texture)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	if (texture != nullptr)
	{
		paused = texture->paused();
		texture->advance_frame();
	}

	// set initial texture to use
	texture_info *default_texture = d3d->get_default_texture();
	default_effect->set_texture("Diffuse", (texture == nullptr) ? default_texture->get_finaltex() : texture->get_finaltex());
	if (options->yiq_enable)
	{
		ntsc_effect->set_texture("Diffuse", (texture == nullptr) ? default_texture->get_finaltex() : texture->get_finaltex());
	}
	else
	{
		color_effect->set_texture("Diffuse", (texture == nullptr) ? default_texture->get_finaltex() : texture->get_finaltex());
	}
}


//============================================================
//  shaders::init
//============================================================

void shaders::init(d3d_base *d3dintf, running_machine *machine, renderer_d3d9 *renderer)
{
	if (!d3dx_create_effect_from_file_ptr)
	{
		printf("Direct3D: Unable to find D3DXCreateEffectFromFileW\n");
		d3dintf->post_fx_available = false;

		return;
	}

	d3dintf->post_fx_available = true;
	this->d3dintf = d3dintf;
	this->machine = machine;
	this->d3d = renderer;
	this->options = renderer->get_shaders_options();

	// check if no driver loaded (not all settings might be loaded yet)
	if (&machine->system() == &GAME_NAME(___empty))
	{
		return;
	}

	// check if another driver is loaded
	if (std::strcmp(machine->system().name, last_system_name) != 0)
	{
		strncpy(last_system_name, machine->system().name, sizeof(last_system_name));

		options->params_init = false;
		last_options.params_init = false;
	}

	enumerate_screens();

	windows_options &winoptions = downcast<windows_options &>(machine->options());

	master_enable = winoptions.d3d_hlsl_enable();
	oversampling_enable = winoptions.d3d_hlsl_oversampling();
	snap_width = winoptions.d3d_snap_width();
	snap_height = winoptions.d3d_snap_height();

	if (last_options.params_init)
	{
		options = &last_options;
	}

	if (!options->params_init)
	{
		strncpy(options->shadow_mask_texture, winoptions.screen_shadow_mask_texture(), sizeof(options->shadow_mask_texture));
		options->shadow_mask_tile_mode = winoptions.screen_shadow_mask_tile_mode();
		options->shadow_mask_alpha = winoptions.screen_shadow_mask_alpha();
		options->shadow_mask_count_x = winoptions.screen_shadow_mask_count_x();
		options->shadow_mask_count_y = winoptions.screen_shadow_mask_count_y();
		options->shadow_mask_u_size = winoptions.screen_shadow_mask_u_size();
		options->shadow_mask_v_size = winoptions.screen_shadow_mask_v_size();
		options->shadow_mask_u_offset = winoptions.screen_shadow_mask_u_offset();
		options->shadow_mask_v_offset = winoptions.screen_shadow_mask_v_offset();
		options->distortion = winoptions.screen_distortion();
		options->cubic_distortion = winoptions.screen_cubic_distortion();
		options->distort_corner = winoptions.screen_distort_corner();
		options->round_corner = winoptions.screen_round_corner();
		options->smooth_border = winoptions.screen_smooth_border();
		options->reflection = winoptions.screen_reflection();
		options->vignetting = winoptions.screen_vignetting();
		options->scanline_alpha = winoptions.screen_scanline_amount();
		options->scanline_scale = winoptions.screen_scanline_scale();
		options->scanline_height = winoptions.screen_scanline_height();
		options->scanline_variation = winoptions.screen_scanline_variation();
		options->scanline_bright_scale = winoptions.screen_scanline_bright_scale();
		options->scanline_bright_offset = winoptions.screen_scanline_bright_offset();
		options->scanline_jitter = winoptions.screen_scanline_jitter();
		options->hum_bar_alpha = winoptions.screen_hum_bar_alpha();
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
		options->yiq_enable = winoptions.screen_yiq_enable();
		options->yiq_jitter = winoptions.screen_yiq_jitter();
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
		options->vector_length_scale = winoptions.screen_vector_length_scale();
		options->vector_length_ratio = winoptions.screen_vector_length_ratio();
		options->bloom_blend_mode = winoptions.screen_bloom_blend_mode();
		options->bloom_scale = winoptions.screen_bloom_scale();
		get_vector(winoptions.screen_bloom_overdrive(), 3, options->bloom_overdrive, TRUE);
		options->bloom_level0_weight = winoptions.screen_bloom_lvl0_weight();
		options->bloom_level1_weight = winoptions.screen_bloom_lvl1_weight();
		options->bloom_level2_weight = winoptions.screen_bloom_lvl2_weight();
		options->bloom_level3_weight = winoptions.screen_bloom_lvl3_weight();
		options->bloom_level4_weight = winoptions.screen_bloom_lvl4_weight();
		options->bloom_level5_weight = winoptions.screen_bloom_lvl5_weight();
		options->bloom_level6_weight = winoptions.screen_bloom_lvl6_weight();
		options->bloom_level7_weight = winoptions.screen_bloom_lvl7_weight();
		options->bloom_level8_weight = winoptions.screen_bloom_lvl8_weight();

		options->params_init = true;
	}

	options->params_dirty = true;
}


//============================================================
//  shaders::init_fsfx_quad
//============================================================

void shaders::init_fsfx_quad(void *vertbuf)
{
	// Called at the start of each frame by the D3D code in order to reserve two triangles
	// that are guaranteed to be at a fixed position so as to simply use D3DPT_TRIANGLELIST, 0, 2
	// instead of having to do bookkeeping about a specific screen quad
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	// get a pointer to the vertex buffer
	fsfx_vertices = (vertex *)vertbuf;
	if (fsfx_vertices == nullptr)
	{
		return;
	}

	// fill in the vertexes clockwise
	fsfx_vertices[0].x = 0.0f;
	fsfx_vertices[0].y = 0.0f;
	fsfx_vertices[1].x = d3d->get_width();
	fsfx_vertices[1].y = 0.0f;
	fsfx_vertices[2].x = 0.0f;
	fsfx_vertices[2].y = d3d->get_height();
	fsfx_vertices[3].x = d3d->get_width();
	fsfx_vertices[3].y = 0.0f;
	fsfx_vertices[4].x = 0.0f;
	fsfx_vertices[4].y = d3d->get_height();
	fsfx_vertices[5].x = d3d->get_width();
	fsfx_vertices[5].y = d3d->get_height();

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

	fsfx_vertices[0].u1 = 0.0f;
	fsfx_vertices[0].v1 = 0.0f;
	fsfx_vertices[1].u1 = 0.0f;
	fsfx_vertices[1].v1 = 0.0f;
	fsfx_vertices[2].u1 = 0.0f;
	fsfx_vertices[2].v1 = 0.0f;
	fsfx_vertices[3].u1 = 0.0f;
	fsfx_vertices[3].v1 = 0.0f;
	fsfx_vertices[4].u1 = 0.0f;
	fsfx_vertices[4].v1 = 0.0f;
	fsfx_vertices[5].u1 = 0.0f;
	fsfx_vertices[5].v1 = 0.0f;

	// set the color, Z parameters to standard values
	for (int i = 0; i < 6; i++)
	{
		fsfx_vertices[i].z = 0.0f;
		fsfx_vertices[i].rhw = 1.0f;
		fsfx_vertices[i].color = D3DCOLOR_ARGB(255, 255, 255, 255);
	}
}


//============================================================
//  shaders::create_resources
//============================================================

int shaders::create_resources(bool reset, std::vector<ui::menu_item>& sliders)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return 0;
	}

	if (last_options.params_init)
	{
		options = &last_options;
	}

	HRESULT result = d3d->get_device()->GetRenderTarget(0, &backbuffer);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Error %08lX during device GetRenderTarget call\n", result);
	}

	result = d3d->get_device()->CreateTexture(4, 4, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &black_texture, nullptr);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Unable to init video-memory target for black texture (%08x)\n", (UINT32)result);
		return 1;
	}
	black_texture->GetSurfaceLevel(0, &black_surface);
	result = d3d->get_device()->SetRenderTarget(0, black_surface);
	if (FAILED(result))
		osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);
	result = d3d->get_device()->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 0, 0);
	if (FAILED(result))
		osd_printf_verbose("Direct3D: Error %08lX during device clear call\n", result);
	result = d3d->get_device()->SetRenderTarget(0, backbuffer);
	if (FAILED(result))
		osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);

	result = d3d->get_device()->CreateTexture((int)snap_width, (int)snap_height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &avi_copy_texture, nullptr);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Unable to init system-memory target for HLSL AVI dumping (%08x)\n", (UINT32)result);
		return 1;
	}
	avi_copy_texture->GetSurfaceLevel(0, &avi_copy_surface);

	result = d3d->get_device()->CreateTexture((int)snap_width, (int)snap_height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &avi_final_texture, nullptr);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Unable to init video-memory target for HLSL AVI dumping (%08x)\n", (UINT32)result);
		return 1;
	}
	avi_final_texture->GetSurfaceLevel(0, &avi_final_target);

	emu_file file(machine->options().art_path(), OPEN_FLAG_READ);
	render_load_png(shadow_bitmap, file, nullptr, options->shadow_mask_texture);

	// experimental: if we have a shadow bitmap, create a texture for it
	if (shadow_bitmap.valid())
	{
		render_texinfo texture;

		// fake in the basic data so it looks like it came from render.c
		texture.base = shadow_bitmap.raw_pixptr(0);
		texture.rowpixels = shadow_bitmap.rowpixels();
		texture.width = shadow_bitmap.width();
		texture.height = shadow_bitmap.height();
		texture.palette = nullptr;
		texture.seqid = 0;

		// now create it (no prescale, no wrap)
		shadow_texture = global_alloc(texture_info(d3d->get_texture_manager(), &texture, 1, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32)));
	}

	const char *fx_dir = downcast<windows_options &>(machine->options()).screen_post_fx_dir();

	default_effect = new effect(this, d3d->get_device(), "primary.fx", fx_dir);
	post_effect = new effect(this, d3d->get_device(), "post.fx", fx_dir);
	distortion_effect = new effect(this, d3d->get_device(), "distortion.fx", fx_dir);
	prescale_effect = new effect(this, d3d->get_device(), "prescale.fx", fx_dir);
	phosphor_effect = new effect(this, d3d->get_device(), "phosphor.fx", fx_dir);
	focus_effect = new effect(this, d3d->get_device(), "focus.fx", fx_dir);
	deconverge_effect = new effect(this, d3d->get_device(), "deconverge.fx", fx_dir);
	color_effect = new effect(this, d3d->get_device(), "color.fx", fx_dir);
	ntsc_effect = new effect(this, d3d->get_device(), "ntsc.fx", fx_dir);
	bloom_effect = new effect(this, d3d->get_device(), "bloom.fx", fx_dir);
	downsample_effect = new effect(this, d3d->get_device(), "downsample.fx", fx_dir);
	vector_effect = new effect(this, d3d->get_device(), "vector.fx", fx_dir);

	if (!default_effect->is_valid() ||
		!post_effect->is_valid() ||
		!distortion_effect->is_valid() ||
		!prescale_effect->is_valid() ||
		!phosphor_effect->is_valid() ||
		!focus_effect->is_valid() ||
		!deconverge_effect->is_valid() ||
		!color_effect->is_valid() ||
		!ntsc_effect->is_valid() ||
		!bloom_effect->is_valid() ||
		!downsample_effect->is_valid() ||
		!vector_effect->is_valid())
	{
		return 1;
	}

	effect *effects[13] = {
		default_effect,
		post_effect,
		distortion_effect,
		prescale_effect,
		phosphor_effect,
		focus_effect,
		deconverge_effect,
		color_effect,
		ntsc_effect,
		color_effect,
		bloom_effect,
		downsample_effect,
		vector_effect
	};

	for (int i = 0; i < 13; i++)
	{
		effects[i]->add_uniform("SourceDims", uniform::UT_VEC2, uniform::CU_SOURCE_DIMS);
		effects[i]->add_uniform("TargetDims", uniform::UT_VEC2, uniform::CU_TARGET_DIMS);
		effects[i]->add_uniform("ScreenDims", uniform::UT_VEC2, uniform::CU_SCREEN_DIMS);
		effects[i]->add_uniform("QuadDims", uniform::UT_VEC2, uniform::CU_QUAD_DIMS);
		effects[i]->add_uniform("SwapXY", uniform::UT_BOOL, uniform::CU_SWAP_XY);
		effects[i]->add_uniform("VectorScreen", uniform::UT_BOOL, uniform::CU_VECTOR_SCREEN);
	}

	ntsc_effect->add_uniform("CCValue", uniform::UT_FLOAT, uniform::CU_NTSC_CCFREQ);
	ntsc_effect->add_uniform("AValue", uniform::UT_FLOAT, uniform::CU_NTSC_A);
	ntsc_effect->add_uniform("BValue", uniform::UT_FLOAT, uniform::CU_NTSC_B);
	ntsc_effect->add_uniform("OValue", uniform::UT_FLOAT, uniform::CU_NTSC_O);
	ntsc_effect->add_uniform("PValue", uniform::UT_FLOAT, uniform::CU_NTSC_P);
	ntsc_effect->add_uniform("NotchHalfWidth", uniform::UT_FLOAT, uniform::CU_NTSC_NOTCH);
	ntsc_effect->add_uniform("YFreqResponse", uniform::UT_FLOAT, uniform::CU_NTSC_YFREQ);
	ntsc_effect->add_uniform("IFreqResponse", uniform::UT_FLOAT, uniform::CU_NTSC_IFREQ);
	ntsc_effect->add_uniform("QFreqResponse", uniform::UT_FLOAT, uniform::CU_NTSC_QFREQ);
	ntsc_effect->add_uniform("ScanTime", uniform::UT_FLOAT, uniform::CU_NTSC_HTIME);

	color_effect->add_uniform("RedRatios", uniform::UT_VEC3, uniform::CU_COLOR_RED_RATIOS);
	color_effect->add_uniform("GrnRatios", uniform::UT_VEC3, uniform::CU_COLOR_GRN_RATIOS);
	color_effect->add_uniform("BluRatios", uniform::UT_VEC3, uniform::CU_COLOR_BLU_RATIOS);
	color_effect->add_uniform("Offset", uniform::UT_VEC3, uniform::CU_COLOR_OFFSET);
	color_effect->add_uniform("Scale", uniform::UT_VEC3, uniform::CU_COLOR_SCALE);
	color_effect->add_uniform("Saturation", uniform::UT_FLOAT, uniform::CU_COLOR_SATURATION);

	deconverge_effect->add_uniform("ConvergeX", uniform::UT_VEC3, uniform::CU_CONVERGE_LINEAR_X);
	deconverge_effect->add_uniform("ConvergeY", uniform::UT_VEC3, uniform::CU_CONVERGE_LINEAR_Y);
	deconverge_effect->add_uniform("RadialConvergeX", uniform::UT_VEC3, uniform::CU_CONVERGE_RADIAL_X);
	deconverge_effect->add_uniform("RadialConvergeY", uniform::UT_VEC3, uniform::CU_CONVERGE_RADIAL_Y);

	focus_effect->add_uniform("Defocus", uniform::UT_VEC2, uniform::CU_FOCUS_SIZE);

	phosphor_effect->add_uniform("Phosphor", uniform::UT_VEC3, uniform::CU_PHOSPHOR_LIFE);

	post_effect->add_uniform("ShadowAlpha", uniform::UT_FLOAT, uniform::CU_POST_SHADOW_ALPHA);
	post_effect->add_uniform("ShadowCount", uniform::UT_VEC2, uniform::CU_POST_SHADOW_COUNT);
	post_effect->add_uniform("ShadowUV", uniform::UT_VEC2, uniform::CU_POST_SHADOW_UV);
	post_effect->add_uniform("ShadowUVOffset", uniform::UT_VEC2, uniform::CU_POST_SHADOW_UV_OFFSET);
	post_effect->add_uniform("ShadowDims", uniform::UT_VEC2, uniform::CU_POST_SHADOW_DIMS);
	post_effect->add_uniform("ScanlineAlpha", uniform::UT_FLOAT, uniform::CU_POST_SCANLINE_ALPHA);
	post_effect->add_uniform("ScanlineScale", uniform::UT_FLOAT, uniform::CU_POST_SCANLINE_SCALE);
	post_effect->add_uniform("ScanlineHeight", uniform::UT_FLOAT, uniform::CU_POST_SCANLINE_HEIGHT);
	post_effect->add_uniform("ScanlineVariation", uniform::UT_FLOAT, uniform::CU_POST_SCANLINE_VARIATION);
	post_effect->add_uniform("ScanlineBrightScale", uniform::UT_FLOAT, uniform::CU_POST_SCANLINE_BRIGHT_SCALE);
	post_effect->add_uniform("ScanlineBrightOffset", uniform::UT_FLOAT, uniform::CU_POST_SCANLINE_BRIGHT_OFFSET);
	post_effect->add_uniform("Power", uniform::UT_VEC3, uniform::CU_POST_POWER);
	post_effect->add_uniform("Floor", uniform::UT_VEC3, uniform::CU_POST_FLOOR);

	distortion_effect->add_uniform("VignettingAmount", uniform::UT_FLOAT, uniform::CU_POST_VIGNETTING);
	distortion_effect->add_uniform("DistortionAmount", uniform::UT_FLOAT, uniform::CU_POST_DISTORTION);
	distortion_effect->add_uniform("CubicDistortionAmount", uniform::UT_FLOAT, uniform::CU_POST_CUBIC_DISTORTION);
	distortion_effect->add_uniform("DistortCornerAmount", uniform::UT_FLOAT, uniform::CU_POST_DISTORT_CORNER);
	distortion_effect->add_uniform("RoundCornerAmount", uniform::UT_FLOAT, uniform::CU_POST_ROUND_CORNER);
	distortion_effect->add_uniform("SmoothBorderAmount", uniform::UT_FLOAT, uniform::CU_POST_SMOOTH_BORDER);
	distortion_effect->add_uniform("ReflectionAmount", uniform::UT_FLOAT, uniform::CU_POST_REFLECTION);

	initialized = true;

	std::vector<ui::menu_item> my_sliders = init_slider_list();
	sliders.insert(sliders.end(), my_sliders.begin(), my_sliders.end());

	return 0;
}


//============================================================
//  shaders::begin_draw
//============================================================

void shaders::begin_draw()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	curr_effect = default_effect;

	default_effect->set_technique("ScreenTechnique");
	post_effect->set_technique("DefaultTechnique");
	distortion_effect->set_technique("DefaultTechnique");
	prescale_effect->set_technique("DefaultTechnique");
	phosphor_effect->set_technique("DefaultTechnique");
	focus_effect->set_technique("DefaultTechnique");
	deconverge_effect->set_technique("DefaultTechnique");
	color_effect->set_technique("DefaultTechnique");
	ntsc_effect->set_technique("DefaultTechnique");
	color_effect->set_technique("DefaultTechnique");
	bloom_effect->set_technique("DefaultTechnique");
	downsample_effect->set_technique("DefaultTechnique");
	vector_effect->set_technique("DefaultTechnique");

	HRESULT result = d3d->get_device()->GetRenderTarget(0, &backbuffer);
	if (FAILED(result))
	{
		osd_printf_verbose("Direct3D: Error %08lX during device GetRenderTarget call\n", result);
	}
}


//============================================================
//  shaders::begin_frame
//============================================================

void shaders::begin_frame()
{
	record_texture();
}


//============================================================
//  shaders::blit
//============================================================

void shaders::blit(
	IDirect3DSurface9 *dst,
	bool clear_dst,
	D3DPRIMITIVETYPE prim_type,
	UINT32 prim_index,
	UINT32 prim_count)
{
	HRESULT result;

	if (dst != nullptr)
	{
		result = d3d->get_device()->SetRenderTarget(0, dst);
		if (FAILED(result))
		{
			osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);
		}

		if (clear_dst)
		{
			result = d3d->get_device()->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1,0,0,0), 0, 0);
			if (FAILED(result))
			{
				osd_printf_verbose("Direct3D: Error %08lX during device clear call\n", result);
			}
		}
	}

	UINT num_passes = 0;
	curr_effect->begin(&num_passes, 0);

	for (UINT pass = 0; pass < num_passes; pass++)
	{
		curr_effect->begin_pass(pass);

		// add the primitives
		result = d3d->get_device()->DrawPrimitive(prim_type, prim_index, prim_count);
		if (FAILED(result))
		{
			osd_printf_verbose("Direct3D: Error %08lX during device DrawPrimitive call\n", result);
		}

		curr_effect->end_pass();
	}

	curr_effect->end();
}


//============================================================
//  shaders::end_frame
//============================================================

void shaders::end_frame()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	if (render_snap && snap_rendered)
	{
		render_snapshot(snap_target);
	}

	if (!lines_pending)
	{
		return;
	}

	lines_pending = false;
}


//============================================================
//  shaders::init_effect_info
//============================================================

void shaders::init_effect_info(poly_info *poly)
{
	// nothing to do
}


//============================================================
//  shaders::find_render_target
//============================================================

d3d_render_target* shaders::find_render_target(texture_info *texture)
{
	UINT32 screen_index_data = (UINT32)texture->get_texinfo().osddata;
	UINT32 screen_index = screen_index_data >> 1;
	UINT32 page_index = screen_index_data & 1;

	d3d_render_target *curr = targethead;
	while (curr != nullptr && (
		curr->screen_index != screen_index ||
		curr->page_index != page_index ||
		curr->width != texture->get_width() ||
		curr->height != texture->get_height()))
	{
		curr = curr->next;
	}

	return curr;
}


//============================================================
//  shaders::find_render_target
//============================================================

d3d_render_target* shaders::find_render_target(int source_width, int source_height, UINT32 screen_index, UINT32 page_index)
{
	d3d_render_target *curr = targethead;
	while (curr != nullptr && (
		curr->width != source_width ||
		curr->height != source_height ||
		curr->screen_index != screen_index ||
		curr->page_index != page_index))
	{
		curr = curr->next;
	}

	return curr;
}


//============================================================
//  shaders::find_cache_target
//============================================================

cache_target* shaders::find_cache_target(UINT32 screen_index, int width, int height)
{
	cache_target *curr = cachehead;
	while (curr != nullptr && (
		curr->screen_index != screen_index ||
		curr->width != width ||
		curr->height != height))
	{
		curr = curr->next;
	}

	return curr;
}

int shaders::ntsc_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	if (!options->yiq_enable)
	{
		return next_index;
	}

	float signal_offset = curr_texture->get_cur_frame() == 0
		? 0.0f
		: options->yiq_jitter;

	// initial "Diffuse"  texture is set in shaders::set_texture()

	curr_effect = ntsc_effect;
	curr_effect->update_uniforms();
	curr_effect->set_float("SignalOffset", signal_offset);

	next_index = rt->next_index(next_index);
	blit(rt->source_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	color_effect->set_texture("Diffuse", rt->source_texture[next_index]);

	return next_index;
}

rgb_t shaders::apply_color_convolution(rgb_t color)
{
	// this function uses the same algorithm as the color convolution shader pass

	float r = static_cast<float>(color.r()) / 255.0f;
	float g = static_cast<float>(color.g()) / 255.0f;
	float b = static_cast<float>(color.b()) / 255.0f;

	float *rRatio = options->red_ratio;
	float *gRatio = options->grn_ratio;
	float *bRatio = options->blu_ratio;
	float *offset = options->offset;
	float *scale = options->scale;
	float saturation = options->saturation;

	// RGB Tint & Shift
	float rShifted = r * rRatio[0] + g * rRatio[1] + b * rRatio[2];
	float gShifted = r * gRatio[0] + g * gRatio[1] + b * gRatio[2];
	float bShifted = r * bRatio[0] + g * bRatio[1] + b * bRatio[2];

	// RGB Scale & Offset
	r = rShifted * scale[0] + offset[0];
	g = gShifted * scale[1] + offset[1];
	b = bShifted * scale[2] + offset[2];

	// Saturation
	float grayscale[3] = { 0.299f, 0.587f, 0.114f };
	float luma = r * grayscale[0] + g * grayscale[1] + b * grayscale[2];
	float chroma[3] = { r - luma, g - luma, b - luma };

	r = chroma[0] * saturation + luma;
	g = chroma[1] * saturation + luma;
	b = chroma[2] * saturation + luma;

	return rgb_t(
		MAX(0, MIN(255, static_cast<int>(r * 255.0f))),
		MAX(0, MIN(255, static_cast<int>(g * 255.0f))),
		MAX(0, MIN(255, static_cast<int>(b * 255.0f))));
}

int shaders::color_convolution_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	curr_effect = color_effect;
	curr_effect->update_uniforms();

	// initial "Diffuse" texture is set in shaders::set_texture() or the result of shaders::ntsc_pass()

	next_index = rt->next_index(next_index);
	blit(rt->source_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::prescale_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	curr_effect = prescale_effect;
	curr_effect->update_uniforms();
	curr_effect->set_texture("Diffuse", rt->source_texture[next_index]);

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::deconverge_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	// skip deconverge if no influencing settings
	if (options->converge_x[0] == 0.0f && options->converge_x[1] == 0.0f && options->converge_x[2] == 0.0f &&
		options->converge_y[0] == 0.0f && options->converge_y[1] == 0.0f && options->converge_y[2] == 0.0f &&
		options->radial_converge_x[0] == 0.0f && options->radial_converge_x[1] == 0.0f && options->radial_converge_x[2] == 0.0f &&
		options->radial_converge_y[0] == 0.0f && options->radial_converge_y[1] == 0.0f && options->radial_converge_y[2] == 0.0f)
	{
		return next_index;
	}

	curr_effect = deconverge_effect;
	curr_effect->update_uniforms();
	curr_effect->set_texture("Diffuse", rt->target_texture[next_index]);

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::defocus_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	// skip defocus if no influencing settings
	if (options->defocus[0] == 0.0f && options->defocus[1] == 0.0f)
	{
		return next_index;
	}

	curr_effect = focus_effect;
	curr_effect->update_uniforms();
	curr_effect->set_texture("Diffuse", rt->target_texture[next_index]);

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::phosphor_pass(d3d_render_target *rt, cache_target *ct, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	// skip phosphor if no influencing settings
	if (options->phosphor[0] == 0.0f && options->phosphor[1] == 0.0f && options->phosphor[2] == 0.0f)
	{
		return next_index;
	}

	curr_effect = phosphor_effect;
	curr_effect->update_uniforms();
	curr_effect->set_texture("Diffuse", rt->target_texture[next_index]);
	curr_effect->set_texture("LastPass", ct->last_texture);
	curr_effect->set_bool("Passthrough", false);

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	// Pass along our phosphor'd screen
	curr_effect->update_uniforms();
	curr_effect->set_texture("Diffuse", rt->target_texture[next_index]);
	curr_effect->set_texture("LastPass", rt->target_texture[next_index]);
	curr_effect->set_bool("Passthrough", true);

	// Avoid changing targets due to page flipping
	blit(ct->last_target, true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::post_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum, bool prepare_bloom)
{
	int next_index = source_index;

	screen_device_iterator screen_iterator(machine->root_device());
	screen_device *screen = screen_iterator.byindex(curr_screen);
	render_container &screen_container = screen->container();

	float xscale = 1.0f / screen_container.xscale();
	float yscale = 1.0f / screen_container.yscale();
	float xoffset = -screen_container.xoffset();
	float yoffset = -screen_container.yoffset();
	float screen_scale[2] = { xscale, yscale };
	float screen_offset[2] = { xoffset, yoffset };

	rgb_t back_color_rgb = screen->has_palette()
		? screen->palette().palette()->entry_color(0)
		: rgb_t(0, 0, 0);
	back_color_rgb = apply_color_convolution(back_color_rgb);
	float back_color[3] = {
		static_cast<float>(back_color_rgb.r()) / 255.0f,
		static_cast<float>(back_color_rgb.g()) / 255.0f,
		static_cast<float>(back_color_rgb.b()) / 255.0f };

	curr_effect = post_effect;
	curr_effect->update_uniforms();
	curr_effect->set_texture("ShadowTexture", shadow_texture == nullptr ? nullptr : shadow_texture->get_finaltex());
	curr_effect->set_int("ShadowTileMode", options->shadow_mask_tile_mode);
	curr_effect->set_texture("DiffuseTexture", rt->target_texture[next_index]);
	curr_effect->set_vector("BackColor", 3, back_color);
	curr_effect->set_vector("ScreenScale", 2, screen_scale);
	curr_effect->set_vector("ScreenOffset", 2, screen_offset);
	curr_effect->set_float("ScanlineOffset", curr_texture->get_cur_frame() == 0 ? 0.0f : options->scanline_jitter);
	curr_effect->set_float("TimeMilliseconds", (float)machine->time().as_double() * 1000.0f);
	curr_effect->set_float("HumBarAlpha", options->hum_bar_alpha);
	curr_effect->set_bool("PrepareBloom", prepare_bloom);

	next_index = rt->next_index(next_index);
	blit(prepare_bloom ? rt->source_surface[next_index] : rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::downsample_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	// skip downsample if no influencing settings
	if (options->bloom_scale == 0.0f)
	{
		return next_index;
	}

	curr_effect = downsample_effect;
	curr_effect->update_uniforms();

	for (int bloom_index = 0; bloom_index < rt->bloom_count; bloom_index++)
	{
		curr_effect->set_vector("TargetDims", 2, rt->bloom_dims[bloom_index]);
		curr_effect->set_texture("DiffuseTexture",
			bloom_index == 0
				? rt->source_texture[next_index]
				: rt->bloom_texture[bloom_index - 1]);

		blit(rt->bloom_surface[bloom_index], true, D3DPT_TRIANGLELIST, 0, 2);
	}

	return next_index;
}

int shaders::bloom_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	// skip bloom if no influencing settings
	if (options->bloom_scale == 0.0f)
	{
		return next_index;
	}

	curr_effect = bloom_effect;
	curr_effect->update_uniforms();

	curr_effect->set_float("Level0Weight", options->bloom_level0_weight);
	curr_effect->set_float("Level1Weight", options->bloom_level1_weight);
	curr_effect->set_float("Level2Weight", options->bloom_level2_weight);
	curr_effect->set_float("Level3Weight", options->bloom_level3_weight);
	curr_effect->set_float("Level4Weight", options->bloom_level4_weight);
	curr_effect->set_float("Level5Weight", options->bloom_level5_weight);
	curr_effect->set_float("Level6Weight", options->bloom_level6_weight);
	curr_effect->set_float("Level7Weight", options->bloom_level7_weight);
	curr_effect->set_float("Level8Weight", options->bloom_level8_weight);

	curr_effect->set_int("BloomBlendMode", options->bloom_blend_mode);
	curr_effect->set_float("BloomScale", options->bloom_scale);
	curr_effect->set_vector("BloomOverdrive", 3, options->bloom_overdrive);

	curr_effect->set_texture("DiffuseTexture", rt->target_texture[next_index]);

	char name[14] = "BloomTexture*";
	for (int index = 1; index < rt->bloom_count; index++)
	{
		name[12] = 'A' + index - 1;
		curr_effect->set_texture(name, rt->bloom_texture[index - 1]);
	}
	for (int index = rt->bloom_count; index < MAX_BLOOM_COUNT; index++)
	{
		name[12] = 'A' + index - 1;
		curr_effect->set_texture(name, black_texture);
	}

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::distortion_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	// skip distortion if no influencing settings
	if (options->reflection == 0 &&
		options->vignetting == 0 &&
		options->distortion == 0 &&
		options->cubic_distortion == 0 &&
		options->distort_corner == 0 &&
		options->round_corner == 0 &&
		options->smooth_border == 0)
	{
		return next_index;
	}

	curr_effect = distortion_effect;
	curr_effect->update_uniforms();
	curr_effect->set_texture("DiffuseTexture", rt->target_texture[next_index]);

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::vector_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	curr_effect = vector_effect;
	curr_effect->update_uniforms();
	// curr_effect->set_float("TimeRatio", options->vector_time_ratio);
	// curr_effect->set_float("TimeScale", options->vector_time_scale);
	curr_effect->set_float("LengthRatio", options->vector_length_ratio);
	curr_effect->set_float("LengthScale", options->vector_length_scale);

	blit(rt->target_surface[next_index], true, poly->get_type(), vertnum, poly->get_count());

	return next_index;
}

int shaders::vector_buffer_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	curr_effect = default_effect;
	curr_effect->update_uniforms();
	curr_effect->set_technique("VectorBufferTechnique");

	curr_effect->set_texture("Diffuse", rt->target_texture[next_index]);

	next_index = rt->next_index(next_index);
	blit(rt->target_surface[next_index], true, D3DPT_TRIANGLELIST, 0, 2);

	return next_index;
}

int shaders::screen_pass(d3d_render_target *rt, int source_index, poly_info *poly, int vertnum)
{
	int next_index = source_index;

	curr_effect = default_effect;
	curr_effect->update_uniforms();
	curr_effect->set_technique("ScreenTechnique");

	curr_effect->set_texture("Diffuse", rt->target_texture[next_index]);

	// we do not clear the backbuffer here because multiple screens might be rendered into
	blit(backbuffer, false, poly->get_type(), vertnum, poly->get_count());

	if (avi_output_file != nullptr)
	{
		blit(avi_final_target, false, poly->get_type(), vertnum, poly->get_count());

		HRESULT result = d3d->get_device()->SetRenderTarget(0, backbuffer);
		if (FAILED(result))
		{
			osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);
		}
	}

	if (render_snap)
	{
		blit(snap_target, false, poly->get_type(), vertnum, poly->get_count());

		HRESULT result = d3d->get_device()->SetRenderTarget(0, backbuffer);
		if (FAILED(result))
		{
			osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);
		}

		snap_rendered = true;
	}

	return next_index;
}

void shaders::ui_pass(poly_info *poly, int vertnum)
{
	curr_effect = default_effect;
	curr_effect->update_uniforms();
	curr_effect->set_technique("UiTechnique");

	blit(nullptr, false, poly->get_type(), vertnum, poly->get_count());
}


//============================================================
//  shaders::render_quad
//============================================================

void shaders::render_quad(poly_info *poly, int vertnum)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	curr_texture = poly->get_texture();
	curr_poly = poly;

	auto win = d3d->assert_window();

	if (PRIMFLAG_GET_SCREENTEX(d3d->get_last_texture_flags()) && curr_texture != nullptr)
	{
		curr_screen = curr_screen < num_screens ? curr_screen : 0;

		curr_render_target = find_render_target(curr_texture);

		d3d_render_target *rt = curr_render_target;
		if (rt == nullptr)
		{
			osd_printf_verbose("Direct3D: No raster render target\n");
			return;
		}

		cache_target *ct = find_cache_target(rt->screen_index, curr_texture->get_width(), curr_texture->get_height());

		int next_index = 0;

		next_index = ntsc_pass(rt, next_index, poly, vertnum); // handled in bgfx
		next_index = color_convolution_pass(rt, next_index, poly, vertnum); // handled in bgfx
		next_index = prescale_pass(rt, next_index, poly, vertnum); // handled in bgfx
		next_index = deconverge_pass(rt, next_index, poly, vertnum); // handled in bgfx
		next_index = defocus_pass(rt, next_index, poly, vertnum); // 1st pass
		next_index = defocus_pass(rt, next_index, poly, vertnum); // 2nd pass
		next_index = phosphor_pass(rt, ct, next_index, poly, vertnum);

		// create bloom textures
		int phosphor_index = next_index;
		next_index = post_pass(rt, next_index, poly, vertnum, true);
		next_index = downsample_pass(rt, next_index, poly, vertnum);

		// apply bloom textures
		next_index = phosphor_index;
		next_index = post_pass(rt, next_index, poly, vertnum, false);
		next_index = bloom_pass(rt, next_index, poly, vertnum);

		next_index = distortion_pass(rt, next_index, poly, vertnum);

		// render on screen
		d3d->set_wrap(D3DTADDRESS_MIRROR);
		next_index = screen_pass(rt, next_index, poly, vertnum);
		d3d->set_wrap(PRIMFLAG_GET_TEXWRAP(curr_texture->get_flags()) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);

		curr_texture->increment_frame_count();
		curr_texture->mask_frame_count(options->yiq_phase_count);

		curr_screen++;
	}
	else if (PRIMFLAG_GET_VECTOR(poly->get_flags()) && vector_enable)
	{
		lines_pending = true;

		int source_width = int(poly->get_prim_width() + 0.5f);
		int source_height = int(poly->get_prim_height() + 0.5f);
		if (win->swap_xy())
		{
			std::swap(source_width, source_height);
		}
		curr_render_target = find_render_target(source_width, source_height, 0, 0);

		d3d_render_target *rt = curr_render_target;
		if (rt == nullptr)
		{
			osd_printf_verbose("Direct3D: No vector render target\n");
			return;
		}

		int next_index = 0;

		next_index = vector_pass(rt, next_index, poly, vertnum);

		HRESULT result = d3d->get_device()->SetRenderTarget(0, backbuffer);
		if (FAILED(result))
		{
			osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);
		}
	}
	else if (PRIMFLAG_GET_VECTORBUF(poly->get_flags()) && vector_enable)
	{
		curr_screen = curr_screen < num_screens ? curr_screen : 0;

		int source_width = int(poly->get_prim_width() + 0.5f);
		int source_height = int(poly->get_prim_height() + 0.5f);
		if (win->swap_xy())
		{
			std::swap(source_width, source_height);
		}
		curr_render_target = find_render_target(source_width, source_height, 0, 0);

		d3d_render_target *rt = curr_render_target;
		if (rt == nullptr)
		{
			osd_printf_verbose("Direct3D: No vector buffer render target\n");
			return;
		}

		cache_target *ct = find_cache_target(rt->screen_index, rt->width, rt->height);

		int next_index = 0;

		next_index = vector_buffer_pass(rt, next_index, poly, vertnum);
		next_index = deconverge_pass(rt, next_index, poly, vertnum);
		next_index = defocus_pass(rt, next_index, poly, vertnum); // 1st pass
		next_index = defocus_pass(rt, next_index, poly, vertnum); // 2nd pass
		next_index = phosphor_pass(rt, ct, next_index, poly, vertnum);

		// create bloom textures
		int phosphor_index = next_index;
		next_index = post_pass(rt, next_index, poly, vertnum, true);
		next_index = downsample_pass(rt, next_index, poly, vertnum);

		// apply bloom textures
		next_index = phosphor_index;
		next_index = post_pass(rt, next_index, poly, vertnum, false);
		next_index = bloom_pass(rt, next_index, poly, vertnum);

		next_index = distortion_pass(rt, next_index, poly, vertnum);

		// render on screen
		d3d->set_wrap(D3DTADDRESS_MIRROR);
		next_index = screen_pass(rt, next_index, poly, vertnum);
		d3d->set_wrap(PRIMFLAG_GET_TEXWRAP(curr_texture->get_flags()) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);

		HRESULT result = d3d->get_device()->SetRenderTarget(0, backbuffer);
		if (FAILED(result))
		{
			osd_printf_verbose("Direct3D: Error %08lX during device SetRenderTarget call\n", result);
		}

		lines_pending = false;

		curr_screen++;
	}
	else
	{
		ui_pass(poly, vertnum);
	}

	options->params_dirty = false;

	curr_render_target = nullptr;
	curr_texture = nullptr;
	curr_poly = nullptr;
}


//============================================================
//  shaders::end_draw
//============================================================

void shaders::end_draw()
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	backbuffer->Release();
}


//============================================================
//  shaders::add_cache_target - register a cache target
//============================================================

bool shaders::add_cache_target(renderer_d3d9* d3d, texture_info* texture, int source_width, int source_height, int target_width, int target_height, int screen_index)
{
	cache_target* target = (cache_target*)global_alloc_clear<cache_target>();

	if (!target->init(d3d, d3dintf, source_width, source_height, target_width, target_height))
	{
		global_free(target);
		return false;
	}

	target->screen_index = screen_index;
	target->next = cachehead;
	target->prev = nullptr;

	if (cachehead != nullptr)
	{
		cachehead->prev = target;
	}
	cachehead = target;

	return true;
}

//============================================================
//  shaders::get_texture_target(render_primitive::prim, texture_info::texture)
//============================================================

d3d_render_target* shaders::get_texture_target(render_primitive *prim, texture_info *texture)
{
	auto win = d3d->assert_window();

	int target_width = int(prim->get_quad_width() + 0.5f);
	int target_height = int(prim->get_quad_height() + 0.5f);
	target_width *= oversampling_enable ? 2 : 1;
	target_height *= oversampling_enable ? 2 : 1;
	if (win->swap_xy())
	{
		std::swap(target_width, target_height);
	}

	// find render target and check if the size of the target quad has changed
	d3d_render_target *target = find_render_target(texture);
	if (target != nullptr)
	{
		if (PRIMFLAG_GET_SCREENTEX(prim->flags))
		{
			// check if the size of the screen quad has changed
			if (target->target_width != target_width || target->target_height != target_height)
			{
				osd_printf_verbose("get_texture_target() - invalid size\n");
				return nullptr;
			}
		}
	}

	return target;
}

d3d_render_target* shaders::get_vector_target(render_primitive *prim)
{
	if (!vector_enable)
	{
		return nullptr;
	}

	auto win = d3d->assert_window();

	// source and target size are the same for vector targets
	int source_width = int(prim->get_quad_width() + 0.5f);
	int source_height = int(prim->get_quad_height() + 0.5f);
	int target_width = source_width;
	int target_height = source_height;
	target_width *= oversampling_enable ? 2 : 1;
	target_height *= oversampling_enable ? 2 : 1;
	if (win->swap_xy())
	{
		std::swap(source_width, source_height);
		std::swap(target_width, target_height);
	}

	// find render target
	d3d_render_target *target = find_render_target(source_width, source_height, 0, 0);
	if (target != nullptr)
	{
		if (PRIMFLAG_GET_VECTORBUF(prim->flags))
		{
			// check if the size of the screen quad has changed
			if (target->target_width != target_width || target->target_height != target_height)
			{
				osd_printf_verbose("get_vector_target() - invalid size\n");
				return nullptr;
			}
		}
	}

	return target;
}

void shaders::create_vector_target(render_primitive *prim)
{
	auto win = d3d->assert_window();

	// source and target size are the same for vector targets
	int source_width = int(prim->get_quad_width() + 0.5f);
	int source_height = int(prim->get_quad_height() + 0.5f);
	int target_width = source_width;
	int target_height = source_height;
	target_width *= oversampling_enable ? 2 : 1;
	target_height *= oversampling_enable ? 2 : 1;
	if (win->swap_xy())
	{
		std::swap(source_width, source_height);
		std::swap(target_width, target_height);
	}

	osd_printf_verbose("create_vector_target() - %d, %d\n", target_width, target_height);
	if (!add_render_target(d3d, prim, nullptr, source_width, source_height, target_width, target_height))
	{
		vector_enable = false;
	}
}


//============================================================
//  shaders::add_render_target - register a render target
//============================================================

bool shaders::add_render_target(renderer_d3d9* d3d, render_primitive *prim, texture_info* texture, int source_width, int source_height, int target_width, int target_height)
{
	UINT32 screen_index = 0;
	UINT32 page_index = 0;
	if (texture != nullptr)
	{
		d3d_render_target *existing_target = find_render_target(texture);
		if (existing_target != nullptr)
		{
			remove_render_target(existing_target);
		}

		UINT32 screen_index_data = (UINT32)texture->get_texinfo().osddata;
		screen_index = screen_index_data >> 1;
		page_index = screen_index_data & 1;
	}
	else
	{
		d3d_render_target *existing_target = find_render_target(source_width, source_height, 0, 0);
		if (existing_target != nullptr)
		{
			remove_render_target(existing_target);
		}
	}

	d3d_render_target* target = (d3d_render_target*)global_alloc_clear<d3d_render_target>();

	if (!target->init(d3d, d3dintf, source_width, source_height, target_width, target_height))
	{
		global_free(target);
		return false;
	}

	target->screen_index = screen_index;
	target->page_index = page_index;
	target->next = targethead;
	target->prev = nullptr;

	if (targethead != nullptr)
	{
		targethead->prev = target;
	}
	targethead = target;

	// cached target only for screen texture and vector buffer
	if (PRIMFLAG_GET_SCREENTEX(prim->flags) || PRIMFLAG_GET_VECTORBUF(prim->flags))
	{
		cache_target* cache = find_cache_target(target->screen_index, source_width, source_height);
		if (cache == nullptr)
		{
			if (!add_cache_target(d3d, texture, source_width, source_height, target_width, target_height, target->screen_index))
			{
				global_free(target);
				return false;
			}
		}
	}

	return true;
}


//============================================================
//  shaders::enumerate_screens
//============================================================
void shaders::enumerate_screens()
{
	screen_device_iterator iter(machine->root_device());
	num_screens = iter.count();
}


//============================================================
//  shaders::register_texture(texture::info)
//============================================================

bool shaders::register_texture(render_primitive *prim, texture_info *texture)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return false;
	}

	auto win = d3d->assert_window();

	int source_width = texture->get_width();
	int source_height = texture->get_height();
	int target_width = int(prim->get_quad_width() + 0.5f);
	int target_height = int(prim->get_quad_height() + 0.5f);
	target_width *= oversampling_enable ? 2 : 1;
	target_height *= oversampling_enable ? 2 : 1;
	if (win->swap_xy())
	{
		// source texture is already swapped
		std::swap(target_width, target_height);
	}

	osd_printf_verbose("register_texture() - %d, %d\n", target_width, target_height);
	if (!add_render_target(d3d, prim, texture, source_width, source_height, target_width, target_height))
	{
		return false;
	}

	return true;
}


//============================================================
//  shaders::delete_resources
//============================================================

void shaders::delete_resources(bool reset)
{
	if (!master_enable || !d3dintf->post_fx_available)
	{
		return;
	}

	if (options != nullptr)
	{
		last_options = *options;
		options = nullptr;
	}

	initialized = false;

	cache_target *currcache = cachehead;
	while(cachehead != nullptr)
	{
		cachehead = currcache->next;
		global_free(currcache);
		currcache = cachehead;
	}

	d3d_render_target *currtarget = targethead;
	while(targethead != nullptr)
	{
		targethead = currtarget->next;
		global_free(currtarget);
		currtarget = targethead;
	}

	if (downsample_effect != nullptr)
	{
		delete downsample_effect;
		downsample_effect = nullptr;
	}
	if (bloom_effect != nullptr)
	{
		delete bloom_effect;
		bloom_effect = nullptr;
	}
	if (vector_effect != nullptr)
	{
		delete vector_effect;
		vector_effect = nullptr;
	}
	if (default_effect != nullptr)
	{
		delete default_effect;
		default_effect = nullptr;
	}
	if (post_effect != nullptr)
	{
		delete post_effect;
		post_effect = nullptr;
	}
	if (distortion_effect != nullptr)
	{
		delete distortion_effect;
		distortion_effect = nullptr;
	}
	if (prescale_effect != nullptr)
	{
		delete prescale_effect;
		prescale_effect = nullptr;
	}
	if (phosphor_effect != nullptr)
	{
		delete phosphor_effect;
		phosphor_effect = nullptr;
	}
	if (focus_effect != nullptr)
	{
		delete focus_effect;
		focus_effect = nullptr;
	}
	if (deconverge_effect != nullptr)
	{
		delete deconverge_effect;
		deconverge_effect = nullptr;
	}
	if (color_effect != nullptr)
	{
		delete color_effect;
		color_effect = nullptr;
	}
	if (ntsc_effect != nullptr)
	{
		delete ntsc_effect;
		ntsc_effect = nullptr;
	}

	if (backbuffer != nullptr)
	{
		backbuffer->Release();
		backbuffer = nullptr;
	}

	if (black_surface != nullptr)
	{
		black_surface->Release();
		black_surface = nullptr;
	}
	if (black_texture != nullptr)
	{
		black_texture->Release();
		black_texture = nullptr;
	}

	if (avi_copy_texture != nullptr)
	{
		avi_copy_texture->Release();
		avi_copy_texture = nullptr;
	}

	if (avi_copy_surface != nullptr)
	{
		avi_copy_surface->Release();
		avi_copy_surface = nullptr;
	}

	if (avi_final_texture != nullptr)
	{
		avi_final_texture->Release();
		avi_final_texture = nullptr;
	}

	if (avi_final_target != nullptr)
	{
		avi_final_target->Release();
		avi_final_target = nullptr;
	}

	shadow_bitmap.reset();
}


//============================================================
//  get_vector
//============================================================

static void get_vector(const char *data, int count, float *out, bool report_error)
{
	if (count > 3 &&
		sscanf(data, "%f,%f,%f,%f", &out[0], &out[1], &out[2], &out[3]) < 4 && report_error)
	{
		osd_printf_error("Illegal quad vector value = %s\n", data);
	}
	else if (count > 2 &&
		sscanf(data, "%f,%f,%f", &out[0], &out[1], &out[2]) < 3 && report_error)
	{
		osd_printf_error("Illegal triple vector value = %s\n", data);
	}
	else if (count > 1 &&
		sscanf(data, "%f,%f", &out[0], &out[1]) < 2 && report_error)
	{
		osd_printf_error("Illegal double vector value = %s\n", data);
	}
	else if (count > 0 &&
		sscanf(data, "%f", &out[0]) < 1 && report_error)
	{
		osd_printf_error("Illegal single vector value = %s\n", data);
	}
}


//============================================================
//  slider_alloc - allocate a new slider entry
//  currently duplicated from ui.c, this could
//  be done in a more ideal way.
//============================================================

static slider_state *slider_alloc(running_machine &machine, int id, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg)
{
	int size = sizeof(slider_state) + strlen(title);
	slider_state *state = reinterpret_cast<slider_state *>(auto_alloc_array_clear(machine, UINT8, size));

	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
	state->id = id;
	strcpy(state->description, title);

	return state;
}


//============================================================
//  assorted global slider accessors
//============================================================

enum slider_type
{
	SLIDER_FLOAT,
	SLIDER_INT_ENUM,
	SLIDER_INT,
	SLIDER_COLOR,
	SLIDER_VEC2
};

INT32 slider::update(std::string *str, INT32 newval)
{
	switch (m_desc->slider_type)
	{
		case SLIDER_INT_ENUM:
		{
			INT32 *val_ptr = reinterpret_cast<INT32 *>(m_value);
			*m_dirty = true;
			if (newval != SLIDER_NOCHANGE)
			{
				*val_ptr = newval;
			}
			if (str != nullptr)
			{
				*str = string_format(m_desc->format, m_desc->strings[*val_ptr]);
			}
			return *val_ptr;
		}

		case SLIDER_INT:
		{
			int *val_ptr = reinterpret_cast<int *>(m_value);
			*m_dirty = true;
			if (newval != SLIDER_NOCHANGE)
			{
				*val_ptr = newval;
			}
			if (str != nullptr)
			{
				*str = string_format(m_desc->format, *val_ptr);
			}
			return *val_ptr;
		}

		default:
		{
			float *val_ptr = reinterpret_cast<float *>(m_value);
			*m_dirty = true;
			if (newval != SLIDER_NOCHANGE)
			{
				*val_ptr = (float)newval * m_desc->scale;
			}
			if (str != nullptr)
			{
				*str = string_format(m_desc->format, *val_ptr);
			}
			return (INT32)floor(*val_ptr / m_desc->scale + 0.5f);
		}
	}
	return 0;
}

static INT32 slider_update_trampoline(running_machine &machine, void *arg, int id, std::string *str, INT32 newval)
{
	if (arg != nullptr)
	{
		return reinterpret_cast<slider *>(arg)->update(str, newval);
	}
	return 0;
}

char shaders::last_system_name[16];

hlsl_options shaders::last_options = { false };

enum slider_option
{
	SLIDER_VECTOR_ATT_MAX = 0,
	SLIDER_VECTOR_ATT_LEN_MIN,
	SLIDER_SHADOW_MASK_TILE_MODE,
	SLIDER_SHADOW_MASK_ALPHA,
	SLIDER_SHADOW_MASK_X_COUNT,
	SLIDER_SHADOW_MASK_Y_COUNT,
	SLIDER_SHADOW_MASK_U_SIZE,
	SLIDER_SHADOW_MASK_V_SIZE,
	SLIDER_SHADOW_MASK_U_OFFSET,
	SLIDER_SHADOW_MASK_V_OFFSET,
	SLIDER_DISTORTION,
	SLIDER_CUBIC_DISTORTION,
	SLIDER_DISTORT_CORNER,
	SLIDER_ROUND_CORNER,
	SLIDER_SMOOTH_BORDER,
	SLIDER_REFLECTION,
	SLIDER_VIGNETTING,
	SLIDER_SCANLINE_ALPHA,
	SLIDER_SCANLINE_SCALE,
	SLIDER_SCANLINE_HEIGHT,
	SLIDER_SCANLINE_VARIATION,
	SLIDER_SCANLINE_BRIGHT_SCALE,
	SLIDER_SCANLINE_BRIGHT_OFFSET,
	SLIDER_SCANLINE_JITTER,
	SLIDER_HUM_BAR_ALPHA,
	SLIDER_DEFOCUS,
	SLIDER_CONVERGE_X,
	SLIDER_CONVERGE_Y,
	SLIDER_RADIAL_CONVERGE_X,
	SLIDER_RADIAL_CONVERGE_Y,
	SLIDER_RED_RATIO,
	SLIDER_GREEN_RATIO,
	SLIDER_BLUE_RATIO,
	SLIDER_SATURATION,
	SLIDER_OFFSET,
	SLIDER_SCALE,
	SLIDER_POWER,
	SLIDER_FLOOR,
	SLIDER_PHOSPHOR,
	SLIDER_BLOOM_BLEND_MODE,
	SLIDER_BLOOM_SCALE,
	SLIDER_BLOOM_OVERDRIVE,
	SLIDER_BLOOM_LVL0_SCALE,
	SLIDER_BLOOM_LVL1_SCALE,
	SLIDER_BLOOM_LVL2_SCALE,
	SLIDER_BLOOM_LVL3_SCALE,
	SLIDER_BLOOM_LVL4_SCALE,
	SLIDER_BLOOM_LVL5_SCALE,
	SLIDER_BLOOM_LVL6_SCALE,
	SLIDER_BLOOM_LVL7_SCALE,
	SLIDER_BLOOM_LVL8_SCALE,
	SLIDER_NTSC_ENABLE,
	SLIDER_NTSC_JITTER,
	SLIDER_NTSC_A_VALUE,
	SLIDER_NTSC_B_VALUE,
	SLIDER_NTSC_P_VALUE,
	SLIDER_NTSC_O_VALUE,
	SLIDER_NTSC_CC_VALUE,
	SLIDER_NTSC_N_VALUE,
	SLIDER_NTSC_Y_VALUE,
	SLIDER_NTSC_I_VALUE,
	SLIDER_NTSC_Q_VALUE,
	SLIDER_NTSC_SCAN_TIME
};

enum slider_screen_type
{
	SLIDER_SCREEN_TYPE_NONE = 0,
	SLIDER_SCREEN_TYPE_RASTER = 1,
	SLIDER_SCREEN_TYPE_VECTOR = 2,
	SLIDER_SCREEN_TYPE_LCD = 4,
	SLIDER_SCREEN_TYPE_LCD_OR_RASTER = SLIDER_SCREEN_TYPE_RASTER | SLIDER_SCREEN_TYPE_LCD,
	SLIDER_SCREEN_TYPE_ANY = SLIDER_SCREEN_TYPE_RASTER | SLIDER_SCREEN_TYPE_VECTOR | SLIDER_SCREEN_TYPE_LCD
};

slider_desc shaders::s_sliders[] =
{
	{ "Vector Attenuation Maximum",         0,    50,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_VECTOR,        SLIDER_VECTOR_ATT_MAX,          0.01f,    "%1.2f", {} },
	{ "Vector Attenuation Length Minimum",  1,   500,  1000, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_VECTOR,        SLIDER_VECTOR_ATT_LEN_MIN,      0.001f,   "%1.3f", {} },
	{ "Shadow Mask Tile Mode",              0,     0,     1, 1, SLIDER_INT_ENUM, SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_TILE_MODE,   0,        "%s",    { "Screen", "Source" } },
	{ "Shadow Mask Amount",                 0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_ALPHA,       0.01f,    "%1.2f", {} },
	{ "Shadow Mask Pixel X Count",          1,     1,  1024, 1, SLIDER_INT,      SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_X_COUNT,     0,        "%d",    {} },
	{ "Shadow Mask Pixel Y Count",          1,     1,  1024, 1, SLIDER_INT,      SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_Y_COUNT,     0,        "%d",    {} },
	{ "Shadow Mask U Size",                 1,     1,    32, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_U_SIZE,      0.03125f, "%2.5f", {} },
	{ "Shadow Mask V Size",                 1,     1,    32, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_V_SIZE,      0.03125f, "%2.5f", {} },
	{ "Shadow Mask U Offset",            -100,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_U_OFFSET,    0.01f,    "%1.2f", {} },
	{ "Shadow Mask V Offset",            -100,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SHADOW_MASK_V_OFFSET,    0.01f,    "%1.2f", {} },
	{ "Quadric Distortion Amount",       -200,     0,   200, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_DISTORTION,              0.01f,    "%2.2f", {} },
	{ "Cubic Distortion Amount",         -200,     0,   200, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_CUBIC_DISTORTION,        0.01f,    "%2.2f", {} },
	{ "Distorted Corner Amount",            0,     0,   200, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_DISTORT_CORNER,          0.01f,    "%1.2f", {} },
	{ "Rounded Corner Amount",              0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_ROUND_CORNER,            0.01f,    "%1.2f", {} },
	{ "Smooth Border Amount",               0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SMOOTH_BORDER,           0.01f,    "%1.2f", {} },
	{ "Reflection Amount",                  0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_REFLECTION,              0.01f,    "%1.2f", {} },
	{ "Vignetting Amount",                  0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_VIGNETTING,              0.01f,    "%1.2f", {} },
	{ "Scanline Amount",                    0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_ALPHA,          0.01f,    "%1.2f", {} },
	{ "Overall Scanline Scale",             0,   100,   400, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_SCALE,          0.01f,    "%1.2f", {} },
	{ "Individual Scanline Scale",          0,   100,   400, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_HEIGHT,         0.01f,    "%1.2f", {} },
	{ "Scanline Variation",                 0,   100,   400, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_VARIATION,      0.01f,    "%1.2f", {} },
	{ "Scanline Brightness Scale",          0,   100,   200, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_BRIGHT_SCALE,   0.01f,    "%1.2f", {} },
	{ "Scanline Brightness Offset",         0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_BRIGHT_OFFSET,  0.01f,    "%1.2f", {} },
	{ "Scanline Jitter Amount",             0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_SCANLINE_JITTER,         0.01f,    "%1.2f", {} },
	{ "Hum Bar Amount",                     0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_HUM_BAR_ALPHA,           0.01f,    "%2.2f", {} },
	{ "Defocus",                            0,     0,   100, 1, SLIDER_VEC2,     SLIDER_SCREEN_TYPE_ANY,           SLIDER_DEFOCUS,                 0.1f,     "%2.1f", {} },
	{ "Linear Convergence X,",           -100,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_CONVERGE_X,              0.1f,     "%3.1f",{} },
	{ "Linear Convergence Y,",           -100,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_CONVERGE_Y,              0.1f,     "%3.1f", {} },
	{ "Radial Convergence X,",           -100,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_RADIAL_CONVERGE_X,       0.1f,     "%3.1f", {} },
	{ "Radial Convergence Y,",           -100,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_RADIAL_CONVERGE_Y,       0.1f,     "%3.1f", {} },
	{ "Red Output from",                 -400,     0,   400, 5, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_RED_RATIO,               0.005f,   "%2.3f", {} },
	{ "Green Output from",               -400,     0,   400, 5, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_GREEN_RATIO,             0.005f,   "%2.3f", {} },
	{ "Blue Output from",                -400,     0,   400, 5, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLUE_RATIO,              0.005f,   "%2.3f", {} },
	{ "Color Saturation",                   0,  1000,  4000, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SATURATION,              0.01f,    "%2.2f", {} },
	{ "Signal Offset,",                  -100,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_OFFSET,                  0.01f,    "%2.2f", {} },
	{ "Signal Scale,",                   -200,   100,   200, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_SCALE,                   0.01f,    "%2.2f", {} },
	{ "Signal Exponent,",                -800,     0,   800, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_POWER,                   0.01f,    "%2.2f", {} },
	{ "Signal Floor,",                      0,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_FLOOR,                   0.01f,    "%2.2f", {} },
	{ "Phosphor Persistence,",              0,     0,   100, 1, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_PHOSPHOR,                0.01f,    "%2.2f", {} },
	{ "Bloom Blend Mode",                   0,     0,     1, 1, SLIDER_INT_ENUM, SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_BLEND_MODE,        0,        "%s",    { "Brighten", "Darken" } },
	{ "Bloom Scale",                        0,     0,  2000, 5, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_SCALE,             0.001f,   "%1.3f", {} },
	{ "Bloom Overdrive,",                   0,     0,  2000, 5, SLIDER_COLOR,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_OVERDRIVE,         0.001f,   "%1.3f", {} },
	{ "Bloom Level 0 Scale",                0,   100,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL0_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 1 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL1_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 2 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL2_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 3 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL3_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 4 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL4_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 5 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL5_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 6 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL6_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 7 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL7_SCALE,        0.01f,    "%1.2f", {} },
	{ "Bloom Level 8 Scale",                0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_ANY,           SLIDER_BLOOM_LVL8_SCALE,        0.01f,    "%1.2f", {} },
	{ "NTSC Processing",                    0,     0,     1, 1, SLIDER_INT_ENUM, SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_ENABLE,             0,        "%s",    { "Off", "On" } },
	{ "NTSC Frame Jitter Offset",           0,     0,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_JITTER,             0.01f,    "%1.2f", {} },
	{ "NTSC A Value",                    -100,    50,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_A_VALUE,            0.01f,    "%1.2f", {} },
	{ "NTSC B Value",                    -100,    50,   100, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_B_VALUE,            0.01f,    "%1.2f", {} },
	{ "NTSC Incoming Phase Pixel Clock Scale",-300,   100,   300, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_P_VALUE,            0.01f,    "%1.2f", {} },
	{ "NTSC Outgoing Phase Offset",      -300,     0,   300, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_O_VALUE,            0.01f,    "%1.2f", {} },
	{ "NTSC Color Carrier (Hz)",            0, 35795, 60000, 5, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_CC_VALUE,           0.001f,   "%1.4f", {} },
	{ "NTSC Color Notch Filter Width",      0,   100,   600, 5, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_N_VALUE,            0.01f,    "%1.4f", {} },
	{ "NTSC Y Signal Bandwidth (Hz)",       0,   600,   600, 5, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_Y_VALUE,            0.01f,    "%1.4f", {} },
	{ "NTSC I Signal Bandwidth (Hz)",       0,   120,   600, 5, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_I_VALUE,            0.01f,    "%1.4f", {} },
	{ "NTSC Q Signal Bandwidth (Hz)",       0,    60,   600, 5, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_Q_VALUE,            0.01f,    "%1.4f", {} },
	{ "NTSC Scanline Duration (uSec)",      0,  5260, 10000, 1, SLIDER_FLOAT,    SLIDER_SCREEN_TYPE_LCD_OR_RASTER, SLIDER_NTSC_SCAN_TIME,          0.01f,    "%1.2f", {} },
	{ nullptr, 0, 0, 0, 0, 0, 0, -1, 0, nullptr, {} }
};


void *shaders::get_slider_option(int id, int index)
{
	switch (id)
	{
		case SLIDER_VECTOR_ATT_MAX: return &(options->vector_length_scale);
		case SLIDER_VECTOR_ATT_LEN_MIN: return &(options->vector_length_ratio);
		case SLIDER_SHADOW_MASK_TILE_MODE: return &(options->shadow_mask_tile_mode);
		case SLIDER_SHADOW_MASK_ALPHA: return &(options->shadow_mask_alpha);
		case SLIDER_SHADOW_MASK_X_COUNT: return &(options->shadow_mask_count_x);
		case SLIDER_SHADOW_MASK_Y_COUNT: return &(options->shadow_mask_count_y);
		case SLIDER_SHADOW_MASK_U_SIZE: return &(options->shadow_mask_u_size);
		case SLIDER_SHADOW_MASK_V_SIZE: return &(options->shadow_mask_v_size);
		case SLIDER_SHADOW_MASK_U_OFFSET: return &(options->shadow_mask_u_offset);
		case SLIDER_SHADOW_MASK_V_OFFSET: return &(options->shadow_mask_v_offset);
		case SLIDER_DISTORTION: return &(options->distortion);
		case SLIDER_CUBIC_DISTORTION: return &(options->cubic_distortion);
		case SLIDER_DISTORT_CORNER: return &(options->distort_corner);
		case SLIDER_ROUND_CORNER: return &(options->round_corner);
		case SLIDER_SMOOTH_BORDER: return &(options->smooth_border);
		case SLIDER_REFLECTION: return &(options->reflection);
		case SLIDER_VIGNETTING: return &(options->vignetting);
		case SLIDER_SCANLINE_ALPHA: return &(options->scanline_alpha);
		case SLIDER_SCANLINE_SCALE: return &(options->scanline_scale);
		case SLIDER_SCANLINE_HEIGHT: return &(options->scanline_height);
		case SLIDER_SCANLINE_VARIATION: return &(options->scanline_variation);
		case SLIDER_SCANLINE_BRIGHT_SCALE: return &(options->scanline_bright_scale);
		case SLIDER_SCANLINE_BRIGHT_OFFSET: return &(options->scanline_bright_offset);
		case SLIDER_SCANLINE_JITTER: return &(options->scanline_jitter);
		case SLIDER_HUM_BAR_ALPHA: return &(options->hum_bar_alpha);
		case SLIDER_DEFOCUS: return &(options->defocus[index]);
		case SLIDER_CONVERGE_X: return &(options->converge_x[index]);
		case SLIDER_CONVERGE_Y: return &(options->converge_y[index]);
		case SLIDER_RADIAL_CONVERGE_X: return &(options->radial_converge_x[index]);
		case SLIDER_RADIAL_CONVERGE_Y: return &(options->radial_converge_y[index]);
		case SLIDER_RED_RATIO: return &(options->red_ratio[index]);
		case SLIDER_GREEN_RATIO: return &(options->grn_ratio[index]);
		case SLIDER_BLUE_RATIO: return &(options->blu_ratio[index]);
		case SLIDER_SATURATION: return &(options->saturation);
		case SLIDER_OFFSET: return &(options->offset[index]);
		case SLIDER_SCALE: return &(options->scale[index]);
		case SLIDER_POWER: return &(options->power[index]);
		case SLIDER_FLOOR: return &(options->floor[index]);
		case SLIDER_PHOSPHOR: return &(options->phosphor[index]);
		case SLIDER_BLOOM_BLEND_MODE: return &(options->bloom_blend_mode);
		case SLIDER_BLOOM_SCALE: return &(options->bloom_scale);
		case SLIDER_BLOOM_OVERDRIVE: return &(options->bloom_overdrive[index]);
		case SLIDER_BLOOM_LVL0_SCALE: return &(options->bloom_level0_weight);
		case SLIDER_BLOOM_LVL1_SCALE: return &(options->bloom_level1_weight);
		case SLIDER_BLOOM_LVL2_SCALE: return &(options->bloom_level2_weight);
		case SLIDER_BLOOM_LVL3_SCALE: return &(options->bloom_level3_weight);
		case SLIDER_BLOOM_LVL4_SCALE: return &(options->bloom_level4_weight);
		case SLIDER_BLOOM_LVL5_SCALE: return &(options->bloom_level5_weight);
		case SLIDER_BLOOM_LVL6_SCALE: return &(options->bloom_level6_weight);
		case SLIDER_BLOOM_LVL7_SCALE: return &(options->bloom_level7_weight);
		case SLIDER_BLOOM_LVL8_SCALE: return &(options->bloom_level8_weight);
		case SLIDER_NTSC_ENABLE: return &(options->yiq_enable);
		case SLIDER_NTSC_JITTER: return &(options->yiq_jitter);
		case SLIDER_NTSC_A_VALUE: return &(options->yiq_a);
		case SLIDER_NTSC_B_VALUE: return &(options->yiq_b);
		case SLIDER_NTSC_P_VALUE: return &(options->yiq_p);
		case SLIDER_NTSC_O_VALUE: return &(options->yiq_o);
		case SLIDER_NTSC_CC_VALUE: return &(options->yiq_cc);
		case SLIDER_NTSC_N_VALUE: return &(options->yiq_n);
		case SLIDER_NTSC_Y_VALUE: return &(options->yiq_y);
		case SLIDER_NTSC_I_VALUE: return &(options->yiq_i);
		case SLIDER_NTSC_Q_VALUE: return &(options->yiq_q);
		case SLIDER_NTSC_SCAN_TIME: return &(options->yiq_scan_time);
	}
	return nullptr;
}

std::vector<ui::menu_item> shaders::init_slider_list()
{
	std::vector<ui::menu_item> sliders;

	for (slider* slider : internal_sliders)
	{
		delete slider;
	}
	internal_sliders.clear();

	auto first_screen = machine->first_screen();
	if (first_screen == nullptr)
	{
		return sliders;
	}
	int screen_type = first_screen->screen_type();

	for (int i = 0; s_sliders[i].name != nullptr; i++)
	{
		slider_desc *desc = &s_sliders[i];
		if ((screen_type == SCREEN_TYPE_VECTOR && (desc->screen_type & SLIDER_SCREEN_TYPE_VECTOR) == SLIDER_SCREEN_TYPE_VECTOR) ||
			(screen_type == SCREEN_TYPE_RASTER && (desc->screen_type & SLIDER_SCREEN_TYPE_RASTER) == SLIDER_SCREEN_TYPE_RASTER) ||
			(screen_type == SCREEN_TYPE_LCD    && (desc->screen_type & SLIDER_SCREEN_TYPE_LCD)    == SLIDER_SCREEN_TYPE_LCD))
		{
			int count;
			switch (desc->slider_type)
			{
				case SLIDER_VEC2:
					count = 2;
					break;
				case SLIDER_COLOR:
					count = 3;
					break;
				default:
					count = 1;
					break;
			}

			for (int j = 0; j < count; j++)
			{
				slider* slider_arg = new slider(desc, get_slider_option(desc->id, j), &options->params_dirty);
				internal_sliders.push_back(slider_arg);
				std::string name = desc->name;
				switch (desc->slider_type)
				{
					case SLIDER_VEC2:
					{
						std::string names[2] = { " X", " Y" };
						name = name + names[j];
						break;
					}
					case SLIDER_COLOR:
					{
						std::string names[3] = { " Red", " Green", " Blue" };
						name = name + names[j];
						break;
					}
					default:
						break;
				}

				slider_state* core_slider = slider_alloc(*machine, desc->id, name.c_str(), desc->minval, desc->defval, desc->maxval, desc->step, slider_update_trampoline, slider_arg);

				ui::menu_item item;
				item.text = core_slider->description;
				item.subtext = "";
				item.flags = 0;
				item.ref = core_slider;
				item.type = ui::menu_item_type::SLIDER;
				sliders.push_back(item);
			}
		}
	}

	return sliders;
}


//============================================================
//  uniform functions
//============================================================

uniform::uniform(effect *shader, const char *name, uniform_type type, int id)
{
	m_shader = shader;
	m_type = type;
	m_next = nullptr;
	m_handle = m_shader->get_parameter(nullptr, name);
	m_ival = 0;
	m_bval = false;
	memset(m_vec, 0, sizeof(float) * 4);
	m_mval = nullptr;
	m_texture = nullptr;
	m_id = id;

	switch (type)
	{
		case UT_BOOL:
		case UT_INT:
		case UT_FLOAT:
		case UT_MATRIX:
		case UT_SAMPLER:
			m_count = 1;
			break;
		case UT_VEC2:
			m_count = 2;
			break;
		case UT_VEC3:
			m_count = 3;
			break;
		case UT_VEC4:
			m_count = 4;
			break;
		default:
			m_count = 1;
			break;
	}
}

void uniform::set_next(uniform *next)
{
	m_next = next;
}

void uniform::update()
{
	if (m_id >= CU_COUNT)
	{
		return;
	}

	shaders *shadersys = m_shader->m_shaders;
	hlsl_options *options = shadersys->options;
	renderer_d3d9 *d3d = shadersys->d3d;

	auto win = d3d->assert_window();
	auto first_screen = win->machine().first_screen();

	bool vector_screen =
		first_screen != nullptr &&
		first_screen->screen_type() == SCREEN_TYPE_VECTOR;

	switch (m_id)
	{
		case CU_SCREEN_DIMS:
		{
			vec2f screendims = d3d->get_dims();
			m_shader->set_vector("ScreenDims", 2, &screendims.c.x);
			break;
		}
		case CU_SOURCE_DIMS:
		{
			if (vector_screen)
			{
				if (shadersys->curr_render_target)
				{
					// vector screen has no source texture, so take the source dimensions of the render target
					float sourcedims[2] = {
						static_cast<float>(shadersys->curr_render_target->width),
						static_cast<float>(shadersys->curr_render_target->height) };
					m_shader->set_vector("SourceDims", 2, sourcedims);
				}
			}
			else
			{
				if (shadersys->curr_texture)
				{
					vec2f sourcedims = shadersys->curr_texture->get_rawdims();
					m_shader->set_vector("SourceDims", 2, &sourcedims.c.x);
				}
			}
			break;
		}
		case CU_TARGET_DIMS:
		{
			if (shadersys->curr_render_target)
			{
				float targetdims[2] = {
					static_cast<float>(shadersys->curr_render_target->target_width),
					static_cast<float>(shadersys->curr_render_target->target_height) };
				m_shader->set_vector("TargetDims", 2, targetdims);
			}
			break;
		}
		case CU_QUAD_DIMS:
		{
			if (shadersys->curr_poly)
			{
				float quaddims[2] = {
					// round
					static_cast<float>(static_cast<int>(shadersys->curr_poly->get_prim_width() + 0.5f)),
					static_cast<float>(static_cast<int>(shadersys->curr_poly->get_prim_height() + 0.5f)) };
				m_shader->set_vector("QuadDims", 2, quaddims);
			}
			break;
		}

		case CU_SWAP_XY:
		{
			m_shader->set_bool("SwapXY", win->swap_xy());
			break;
		}
		case CU_VECTOR_SCREEN:
		{
			m_shader->set_bool("VectorScreen", vector_screen);
			break;
		}

		case CU_NTSC_CCFREQ:
			m_shader->set_float("CCValue", options->yiq_cc);
			break;
		case CU_NTSC_A:
			m_shader->set_float("AValue", options->yiq_a);
			break;
		case CU_NTSC_B:
			m_shader->set_float("BValue", options->yiq_b);
			break;
		case CU_NTSC_O:
			m_shader->set_float("OValue", options->yiq_o);
			break;
		case CU_NTSC_P:
			m_shader->set_float("PValue", options->yiq_p);
			break;
		case CU_NTSC_NOTCH:
			m_shader->set_float("NotchHalfWidth", options->yiq_n);
			break;
		case CU_NTSC_YFREQ:
			m_shader->set_float("YFreqResponse", options->yiq_y);
			break;
		case CU_NTSC_IFREQ:
			m_shader->set_float("IFreqResponse", options->yiq_i);
			break;
		case CU_NTSC_QFREQ:
			m_shader->set_float("QFreqResponse", options->yiq_q);
			break;
		case CU_NTSC_HTIME:
			m_shader->set_float("ScanTime", options->yiq_scan_time);
			break;
		case CU_NTSC_ENABLE:
			m_shader->set_float("YIQEnable", options->yiq_enable ? 1.0f : 0.0f);
			break;

		case CU_COLOR_RED_RATIOS:
			m_shader->set_vector("RedRatios", 3, options->red_ratio);
			break;
		case CU_COLOR_GRN_RATIOS:
			m_shader->set_vector("GrnRatios", 3, options->grn_ratio);
			break;
		case CU_COLOR_BLU_RATIOS:
			m_shader->set_vector("BluRatios", 3, options->blu_ratio);
			break;
		case CU_COLOR_OFFSET:
			m_shader->set_vector("Offset", 3, options->offset);
			break;
		case CU_COLOR_SCALE:
			m_shader->set_vector("Scale", 3, options->scale);
			break;
		case CU_COLOR_SATURATION:
			m_shader->set_float("Saturation", options->saturation);
			break;

		case CU_CONVERGE_LINEAR_X:
			m_shader->set_vector("ConvergeX", 3, options->converge_x);
			break;
		case CU_CONVERGE_LINEAR_Y:
			m_shader->set_vector("ConvergeY", 3, options->converge_y);
			break;
		case CU_CONVERGE_RADIAL_X:
			m_shader->set_vector("RadialConvergeX", 3, options->radial_converge_x);
			break;
		case CU_CONVERGE_RADIAL_Y:
			m_shader->set_vector("RadialConvergeY", 3, options->radial_converge_y);
			break;

		case CU_FOCUS_SIZE:
			m_shader->set_vector("Defocus", 2, &options->defocus[0]);
			break;

		case CU_PHOSPHOR_LIFE:
			m_shader->set_vector("Phosphor", 3, options->phosphor);
			break;

		case CU_POST_REFLECTION:
			m_shader->set_float("ReflectionAmount", options->reflection);
			break;
		case CU_POST_VIGNETTING:
			m_shader->set_float("VignettingAmount", options->vignetting);
			break;
		case CU_POST_DISTORTION:
			m_shader->set_float("DistortionAmount", options->distortion);
			break;
		case CU_POST_CUBIC_DISTORTION:
			m_shader->set_float("CubicDistortionAmount", options->cubic_distortion);
			break;
		case CU_POST_DISTORT_CORNER:
			m_shader->set_float("DistortCornerAmount", options->distort_corner);
			break;
		case CU_POST_ROUND_CORNER:
			m_shader->set_float("RoundCornerAmount", options->round_corner);
			break;
		case CU_POST_SMOOTH_BORDER:
			m_shader->set_float("SmoothBorderAmount", options->smooth_border);
			break;
		case CU_POST_SHADOW_ALPHA:
			m_shader->set_float("ShadowAlpha", shadersys->shadow_texture == nullptr ? 0.0f : options->shadow_mask_alpha);
			break;
		case CU_POST_SHADOW_COUNT:
		{
			float shadowcount[2] = { static_cast<float>(options->shadow_mask_count_x), static_cast<float>(options->shadow_mask_count_y) };
			m_shader->set_vector("ShadowCount", 2, shadowcount);
			break;
		}
		case CU_POST_SHADOW_UV:
		{
			float shadowuv[2] = { options->shadow_mask_u_size, options->shadow_mask_v_size };
			m_shader->set_vector("ShadowUV", 2, shadowuv);
			break;
		}
		case CU_POST_SHADOW_UV_OFFSET:
		{
			float shadowuv[2] = { options->shadow_mask_u_offset, options->shadow_mask_v_offset };
			m_shader->set_vector("ShadowUVOffset", 2, shadowuv);
			break;
		}
		case CU_POST_SHADOW_DIMS:
		{
			vec2f shadow_dims;

			if (shadersys->shadow_texture)
			{
				shadow_dims = shadersys->shadow_texture->get_rawdims();
			}
			else
			{
				shadow_dims.c.x = 1.0f;
				shadow_dims.c.y = 1.0f;
			}

			m_shader->set_vector("ShadowDims", 2, &shadow_dims.c.x);
			break;
		}
		case CU_POST_SCANLINE_ALPHA:
			m_shader->set_float("ScanlineAlpha", options->scanline_alpha);
			break;
		case CU_POST_SCANLINE_SCALE:
			m_shader->set_float("ScanlineScale", options->scanline_scale);
			break;
		case CU_POST_SCANLINE_HEIGHT:
			m_shader->set_float("ScanlineHeight", options->scanline_height);
			break;
		case CU_POST_SCANLINE_VARIATION:
			m_shader->set_float("ScanlineVariation", options->scanline_variation);
			break;
		case CU_POST_SCANLINE_BRIGHT_SCALE:
			m_shader->set_float("ScanlineBrightScale", options->scanline_bright_scale);
			break;
		case CU_POST_SCANLINE_BRIGHT_OFFSET:
			m_shader->set_float("ScanlineBrightOffset", options->scanline_bright_offset);
			break;
		case CU_POST_POWER:
			m_shader->set_vector("Power", 3, options->power);
			break;
		case CU_POST_FLOOR:
			m_shader->set_vector("Floor", 3, options->floor);
			break;
	}
}

void uniform::set(float x, float y, float z, float w)
{
	m_vec[0] = x;
	m_vec[1] = y;
	m_vec[2] = z;
	m_vec[3] = w;
}

void uniform::set(float x, float y, float z)
{
	m_vec[0] = x;
	m_vec[1] = y;
	m_vec[2] = z;
}

void uniform::set(float x, float y)
{
	m_vec[0] = x;
	m_vec[1] = y;
}

void uniform::set(float x)
{
	m_vec[0] = x;
}

void uniform::set(int x)
{
	m_ival = x;
}

void uniform::set(bool x)
{
	m_bval = x;
}

void uniform::set(D3DMATRIX *mat)
{
	m_mval = mat;
}

void uniform::set(IDirect3DTexture9 *tex)
{
	m_texture = tex;
}

void uniform::upload()
{
	switch (m_type)
	{
		case UT_BOOL:
			m_shader->set_bool(m_handle, m_bval);
			break;
		case UT_INT:
			m_shader->set_int(m_handle, m_ival);
			break;
		case UT_FLOAT:
			m_shader->set_float(m_handle, m_vec[0]);
			break;
		case UT_VEC2:
		case UT_VEC3:
		case UT_VEC4:
			m_shader->set_vector(m_handle, m_count, m_vec);
			break;
		case UT_MATRIX:
			m_shader->set_matrix(m_handle, m_mval);
			break;
		case UT_SAMPLER:
			m_shader->set_texture(m_handle, m_texture);
			break;
	}
}


//============================================================
//  effect functions
//============================================================

effect::effect(shaders *shadersys, IDirect3DDevice9 *dev, const char *name, const char *path)
{
	LPD3DXBUFFER buffer_errors = nullptr;

	m_shaders = shadersys;
	m_uniform_head = nullptr;
	m_uniform_tail = nullptr;
	m_effect = nullptr;
	m_valid = false;

	char name_cstr[1024];
	sprintf(name_cstr, "%s\\%s", path, name);
	TCHAR *effect_name = tstring_from_utf8(name_cstr);

	HRESULT hr = (*shadersys->d3dx_create_effect_from_file_ptr)(dev, effect_name, nullptr, nullptr, 0, nullptr, &m_effect, &buffer_errors);
	if (FAILED(hr))
	{
		if (buffer_errors != nullptr)
		{
			LPVOID compile_errors = buffer_errors->GetBufferPointer();
			printf("Unable to compile shader: %s\n", (const char*)compile_errors); fflush(stdout);
		}
		else
		{
			printf("Shader %s is missing, corrupt or cannot be compiled.\n", (const char*)name); fflush(stdout);
		}
	}
	else
	{
		m_valid = true;
	}

	osd_free(effect_name);
}

effect::~effect()
{
	m_effect->Release();
	m_effect = nullptr;
	uniform *curr = m_uniform_head;
	while (curr != nullptr)
	{
		uniform *next = curr->get_next();
		delete curr;
		curr = next;
	}
	m_uniform_head = nullptr;
	m_uniform_tail = nullptr;
}

void effect::add_uniform(const char *name, uniform::uniform_type type, int id)
{
	uniform *newuniform = new uniform(this, name, type, id);
	if (newuniform == nullptr)
	{
		return;
	}

	if (m_uniform_head == nullptr)
	{
		m_uniform_head = newuniform;
	}
	if (m_uniform_tail != nullptr)
	{
		m_uniform_tail->set_next(newuniform);
	}
	m_uniform_tail = newuniform;
}

void effect::update_uniforms()
{
	uniform *curr = m_uniform_head;
	while(curr != nullptr)
	{
		curr->update();
		curr = curr->get_next();
	}
}

void effect::begin(UINT *passes, DWORD flags)
{
	m_effect->Begin(passes, flags);
}

void effect::end()
{
	m_effect->End();
}

void effect::begin_pass(UINT pass)
{
	m_effect->BeginPass(pass);
}

void effect::end_pass()
{
	m_effect->EndPass();
}

void effect::set_technique(const char *name)
{
	m_effect->SetTechnique(name);
}

void effect::set_vector(D3DXHANDLE param, int count, float *vector)
{
	static D3DXVECTOR4 out_vector;
	if (count > 0)
	{
		out_vector.x = vector[0];
	}
	if (count > 1)
	{
		out_vector.y = vector[1];
	}
	if (count > 2)
	{
		out_vector.z = vector[2];
	}
	if (count > 3)
	{
		out_vector.w = vector[3];
	}
	m_effect->SetVector(param, &out_vector);
}

void effect::set_float(D3DXHANDLE param, float value)
{
	m_effect->SetFloat(param, value);
}

void effect::set_int(D3DXHANDLE param, int value)
{
	m_effect->SetInt(param, value);
}

void effect::set_bool(D3DXHANDLE param, bool value)
{
	m_effect->SetBool(param, value);
}

void effect::set_matrix(D3DXHANDLE param, D3DMATRIX *matrix)
{
	m_effect->SetMatrix(param, (D3DXMATRIX*)matrix);
}

void effect::set_texture(D3DXHANDLE param, IDirect3DTexture9 *tex)
{
	m_effect->SetTexture(param, tex);
}

D3DXHANDLE effect::get_parameter(D3DXHANDLE param, const char *name)
{
	return m_effect->GetParameterByName(param, name);
}

ULONG effect::release()
{
	return m_effect->Release();
}
