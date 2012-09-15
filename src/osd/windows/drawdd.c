//============================================================
//
//  drawdd.c - Win32 DirectDraw implementation
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

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <ddraw.h>
#undef interface

// MAME headers
#include "emu.h"
#include "render.h"
#include "rendutil.h"
#include "options.h"
#include "rendersw.c"

// MAMEOS headers
#include "winmain.h"
#include "window.h"
#include "config.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef HRESULT (WINAPI *directdrawcreateex_ptr)(GUID FAR *lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);
typedef HRESULT (WINAPI *directdrawenumerateex_ptr)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);


/* dd_info is the information about DirectDraw for the current screen */
struct dd_info
{
	GUID					adapter;					// current display adapter
	GUID *					adapter_ptr;				// pointer to current display adapter
	int						width, height;				// current width, height
	int						refresh;					// current refresh rate
	int						clearouter;					// clear the outer areas?

	INT32					blitwidth, blitheight;		// current blit width/height values
	RECT					lastdest;					// last destination rectangle

	IDirectDraw7 *			ddraw;						// pointer to the DirectDraw object
	IDirectDrawSurface7 *	primary;					// pointer to the primary surface object
	IDirectDrawSurface7 *	back;						// pointer to the back buffer surface object
	IDirectDrawSurface7 *	blit;						// pointer to the blit surface object
	IDirectDrawClipper *	clipper;					// pointer to the clipper object
	IDirectDrawGammaControl *gamma;						// pointer to the gamma control object

	DDSURFACEDESC2			primarydesc;				// description of the primary surface
	DDSURFACEDESC2			blitdesc;					// description of the blitting surface
	DDSURFACEDESC2			origmode;					// original video mode

	DDCAPS					ddcaps;						// capabilities of the device
	DDCAPS					helcaps;					// capabilities of the hardware

	void *					membuffer;					// memory buffer for complex rendering
	UINT32					membuffersize;				// current size of the memory buffer
};


/* monitor_enum_info holds information during a monitor enumeration */
struct monitor_enum_info
{
	win_monitor_info *		monitor;					// pointer to monitor we want
	GUID					guid;						// GUID of the one we found
	GUID *					guid_ptr;					// pointer to our GUID
	int						foundit;					// TRUE if we found what we wanted
};


/* mode_enum_info holds information during a display mode enumeration */
struct mode_enum_info
{
	win_window_info *		window;
	INT32					minimum_width, minimum_height;
	INT32					target_width, target_height;
	double					target_refresh;
	float					best_score;
};



//============================================================
//  GLOBALS
//============================================================

static HINSTANCE dllhandle;
static directdrawcreateex_ptr directdrawcreateex;
static directdrawenumerateex_ptr directdrawenumerateex;



//============================================================
//  INLINES
//============================================================

INLINE void update_outer_rects(dd_info *dd)
{
	dd->clearouter = (dd->back != NULL) ? 3 : 1;
}


INLINE int better_mode(int width0, int height0, int width1, int height1, float desired_aspect)
{
	float aspect0 = (float)width0 / (float)height0;
	float aspect1 = (float)width1 / (float)height1;
	return (fabs(desired_aspect - aspect0) < fabs(desired_aspect - aspect1)) ? 0 : 1;
}



//============================================================
//  PROTOTYPES
//============================================================

// core functions
static void drawdd_exit(void);
static int drawdd_window_init(win_window_info *window);
static void drawdd_window_destroy(win_window_info *window);
static render_primitive_list *drawdd_window_get_primitives(win_window_info *window);
static int drawdd_window_draw(win_window_info *window, HDC dc, int update);

// surface management
static int ddraw_create(win_window_info *window);
static int ddraw_create_surfaces(win_window_info *window);
static void ddraw_delete(win_window_info *window);
static void ddraw_delete_surfaces(win_window_info *window);
static int ddraw_verify_caps(dd_info *dd);
static int ddraw_test_cooperative(win_window_info *window);
static HRESULT create_surface(dd_info *dd, DDSURFACEDESC2 *desc, IDirectDrawSurface7 **surface, const char *type);
static int create_clipper(win_window_info *window);

// drawing helpers
static void compute_blit_surface_size(win_window_info *window);
static void blit_to_primary(win_window_info *window, int srcwidth, int srcheight);

// video modes
static int config_adapter_mode(win_window_info *window);
static void get_adapter_for_monitor(dd_info *dd, win_monitor_info *monitor);
static void pick_best_mode(win_window_info *window);



//============================================================
//  drawdd_init
//============================================================

int drawdd_init(running_machine &machine, win_draw_callbacks *callbacks)
{
	// dynamically grab the create function from ddraw.dll
	dllhandle = LoadLibrary(TEXT("ddraw.dll"));
	if (dllhandle == NULL)
	{
		mame_printf_verbose("DirectDraw: Unable to access ddraw.dll\n");
		return 1;
	}

	// import the create function
	directdrawcreateex = (directdrawcreateex_ptr)GetProcAddress(dllhandle, "DirectDrawCreateEx");
	if (directdrawcreateex == NULL)
	{
		mame_printf_verbose("DirectDraw: Unable to find DirectDrawCreateEx\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return 1;
	}

	// import the enumerate function
	directdrawenumerateex = (directdrawenumerateex_ptr)GetProcAddress(dllhandle, "DirectDrawEnumerateExA");
	if (directdrawenumerateex == NULL)
	{
		mame_printf_verbose("DirectDraw: Unable to find DirectDrawEnumerateExA\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return 1;
	}

	// fill in the callbacks
	callbacks->exit = drawdd_exit;
	callbacks->window_init = drawdd_window_init;
	callbacks->window_get_primitives = drawdd_window_get_primitives;
	callbacks->window_draw = drawdd_window_draw;
	callbacks->window_save = NULL;
	callbacks->window_record = NULL;
	callbacks->window_destroy = drawdd_window_destroy;

	mame_printf_verbose("DirectDraw: Using DirectDraw 7\n");
	return 0;
}



//============================================================
//  drawdd_exit
//============================================================

static void drawdd_exit(void)
{
	if (dllhandle != NULL)
		FreeLibrary(dllhandle);
}



//============================================================
//  drawdd_window_init
//============================================================

static int drawdd_window_init(win_window_info *window)
{
	dd_info *dd;

	// allocate memory for our structures
	dd = global_alloc_clear(dd_info);
	window->drawdata = dd;

	// configure the adapter for the mode we want
	if (config_adapter_mode(window))
		goto error;

	// create the ddraw object
	if (ddraw_create(window))
		goto error;

	return 0;

error:
	drawdd_window_destroy(window);
	mame_printf_error("Unable to initialize DirectDraw.\n");
	return 1;
}



//============================================================
//  drawdd_window_destroy
//============================================================

static void drawdd_window_destroy(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;

	// skip if nothing
	if (dd == NULL)
		return;

	// delete the ddraw object
	ddraw_delete(window);

	// free the memory in the window
	global_free(dd);
	window->drawdata = NULL;
}



//============================================================
//  drawdd_window_get_primitives
//============================================================

static render_primitive_list *drawdd_window_get_primitives(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;

	compute_blit_surface_size(window);
	window->target->set_bounds(dd->blitwidth, dd->blitheight, 0);
	window->target->set_max_update_rate((dd->refresh == 0) ? dd->origmode.dwRefreshRate : dd->refresh);

	return &window->target->get_primitives();
}



//============================================================
//  drawdd_window_draw
//============================================================

static int drawdd_window_draw(win_window_info *window, HDC dc, int update)
{
	dd_info *dd = (dd_info *)window->drawdata;
	render_primitive *prim;
	int usemembuffer = FALSE;
	HRESULT result;

	// if we haven't been created, just punt
	if (dd == NULL)
		return 1;

	// if we're updating, remember to erase the outer stuff
	if (update)
		update_outer_rects(dd);

	// if we have a ddraw object, check the cooperative level
	if (ddraw_test_cooperative(window))
		return 1;

	// get the size; if we're too small, delete the existing surfaces
	if (dd->blitwidth > dd->blitdesc.dwWidth || dd->blitheight > dd->blitdesc.dwHeight)
		ddraw_delete_surfaces(window);

	// if we need to create surfaces, do it now
	if (dd->blit == NULL && ddraw_create_surfaces(window) != 0)
		return 1;

	// select our surface and lock it
	result = IDirectDrawSurface7_Lock(dd->blit, NULL, &dd->blitdesc, DDLOCK_WAIT, NULL);
	if (result == DDERR_SURFACELOST)
	{
		mame_printf_verbose("DirectDraw: Lost surfaces; deleting and retrying next frame\n");
		ddraw_delete_surfaces(window);
		return 1;
	}
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X locking blit surface\n", (int)result);
		return 1;
	}

	// render to it
	window->primlist->acquire_lock();

	// scan the list of primitives for tricky stuff
	for (prim = window->primlist->first(); prim != NULL; prim = prim->next())
		if (PRIMFLAG_GET_BLENDMODE(prim->flags) != BLENDMODE_NONE ||
			(prim->texture.base != NULL && PRIMFLAG_GET_TEXFORMAT(prim->flags) == TEXFORMAT_ARGB32))
		{
			usemembuffer = TRUE;
			break;
		}

	// if we're using the memory buffer, draw offscreen first and then copy
	if (usemembuffer)
	{
		int x, y;

		// based on the target format, use one of our standard renderers
		switch (dd->blitdesc.ddpfPixelFormat.dwRBitMask)
		{
			case 0x00ff0000:	software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window->primlist, dd->membuffer, dd->blitwidth, dd->blitheight, dd->blitwidth);	break;
			case 0x000000ff:	software_renderer<UINT32, 0,0,0, 0,8,16>::draw_primitives(*window->primlist, dd->membuffer, dd->blitwidth, dd->blitheight, dd->blitwidth);	break;
			case 0xf800:		software_renderer<UINT16, 3,2,3, 11,5,0>::draw_primitives(*window->primlist, dd->membuffer, dd->blitwidth, dd->blitheight, dd->blitwidth);	break;
			case 0x7c00:		software_renderer<UINT16, 3,3,3, 10,5,0>::draw_primitives(*window->primlist, dd->membuffer, dd->blitwidth, dd->blitheight, dd->blitwidth);	break;
			default:
				mame_printf_verbose("DirectDraw: Unknown target mode: R=%08X G=%08X B=%08X\n", (int)dd->blitdesc.ddpfPixelFormat.dwRBitMask, (int)dd->blitdesc.ddpfPixelFormat.dwGBitMask, (int)dd->blitdesc.ddpfPixelFormat.dwBBitMask);
				break;
		}

		// handle copying to both 16bpp and 32bpp destinations
		for (y = 0; y < dd->blitheight; y++)
		{
			if (dd->blitdesc.ddpfPixelFormat.dwRGBBitCount == 32)
			{
				UINT32 *src = (UINT32 *)dd->membuffer + y * dd->blitwidth;
				UINT32 *dst = (UINT32 *)((UINT8 *)dd->blitdesc.lpSurface + y * dd->blitdesc.lPitch);
				for (x = 0; x < dd->blitwidth; x++)
					*dst++ = *src++;
			}
			else if (dd->blitdesc.ddpfPixelFormat.dwRGBBitCount == 16)
			{
				UINT16 *src = (UINT16 *)dd->membuffer + y * dd->blitwidth;
				UINT16 *dst = (UINT16 *)((UINT8 *)dd->blitdesc.lpSurface + y * dd->blitdesc.lPitch);
				for (x = 0; x < dd->blitwidth; x++)
					*dst++ = *src++;
			}
		}

	}

	// otherwise, draw directly
	else
	{
		// based on the target format, use one of our standard renderers
		switch (dd->blitdesc.ddpfPixelFormat.dwRBitMask)
		{
			case 0x00ff0000:	software_renderer<UINT32, 0,0,0, 16,8,0, true>::draw_primitives(*window->primlist, dd->blitdesc.lpSurface, dd->blitwidth, dd->blitheight, dd->blitdesc.lPitch / 4);	break;
			case 0x000000ff:	software_renderer<UINT32, 0,0,0, 0,8,16, true>::draw_primitives(*window->primlist, dd->blitdesc.lpSurface, dd->blitwidth, dd->blitheight, dd->blitdesc.lPitch / 4);	break;
			case 0xf800:		software_renderer<UINT16, 3,2,3, 11,5,0, true>::draw_primitives(*window->primlist, dd->blitdesc.lpSurface, dd->blitwidth, dd->blitheight, dd->blitdesc.lPitch / 2);	break;
			case 0x7c00:		software_renderer<UINT16, 3,3,3, 10,5,0, true>::draw_primitives(*window->primlist, dd->blitdesc.lpSurface, dd->blitwidth, dd->blitheight, dd->blitdesc.lPitch / 2);	break;
			default:
				mame_printf_verbose("DirectDraw: Unknown target mode: R=%08X G=%08X B=%08X\n", (int)dd->blitdesc.ddpfPixelFormat.dwRBitMask, (int)dd->blitdesc.ddpfPixelFormat.dwGBitMask, (int)dd->blitdesc.ddpfPixelFormat.dwBBitMask);
				break;
		}
	}
	window->primlist->release_lock();

	// unlock and blit
	result = IDirectDrawSurface7_Unlock(dd->blit, NULL);
	if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X unlocking blit surface\n", (int)result);

	// sync to VBLANK
	if ((video_config.waitvsync || video_config.syncrefresh) && window->machine().video().throttled() && (!window->fullscreen || dd->back == NULL))
	{
		result = IDirectDraw7_WaitForVerticalBlank(dd->ddraw, DDWAITVB_BLOCKBEGIN, NULL);
		if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X waiting for VBLANK\n", (int)result);
	}

	// complete the blitting
	blit_to_primary(window, dd->blitwidth, dd->blitheight);
	return 0;
}



//============================================================
//  ddraw_create
//============================================================

static int ddraw_create(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;
	HRESULT result;
	int verify;

	// if a device exists, free it
	if (dd->ddraw != NULL)
		ddraw_delete(window);

	// create the DirectDraw object
	result = (*directdrawcreateex)(dd->adapter_ptr, (LPVOID *)&dd->ddraw, WRAP_REFIID(IID_IDirectDraw7), NULL);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X during DirectDrawCreateEx call\n", (int)result);
		goto error;
	}

	// verify the caps
	verify = ddraw_verify_caps(dd);
	if (verify == 2)
	{
		mame_printf_error("DirectDraw: Error - Device does not meet minimum requirements for DirectDraw rendering\n");
		goto error;
	}
	if (verify == 1)
		mame_printf_verbose("DirectDraw: Warning - Device may not perform well for DirectDraw rendering\n");

	// set the cooperative level
	// for non-window modes, we will use full screen here
	result = IDirectDraw7_SetCooperativeLevel(dd->ddraw, win_window_list->hwnd, DDSCL_SETFOCUSWINDOW);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X during IDirectDraw7_SetCooperativeLevel(FOCUSWINDOW) call\n", (int)result);
		goto error;
	}
	result = IDirectDraw7_SetCooperativeLevel(dd->ddraw, window->hwnd, DDSCL_SETDEVICEWINDOW | (window->fullscreen ? DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE : DDSCL_NORMAL));
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X during IDirectDraw7_SetCooperativeLevel(DEVICEWINDOW) call\n", (int)result);
		goto error;
	}

	// full screen mode: set the resolution
	if (window->fullscreen && video_config.switchres)
	{
		result = IDirectDraw7_SetDisplayMode(dd->ddraw, dd->width, dd->height, 32, dd->refresh, 0);
		if (result != DD_OK)
		{
			mame_printf_verbose("DirectDraw: Error %08X attempting to set video mode %dx%d@%d call\n", (int)result, dd->width, dd->height, dd->refresh);
			goto error;
		}
	}

	return ddraw_create_surfaces(window);

error:
	ddraw_delete(window);
	return 1;
}



//============================================================
//  ddraw_create_surfaces
//============================================================

static int ddraw_create_surfaces(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;
	HRESULT result;

	// make a description of the primary surface
	memset(&dd->primarydesc, 0, sizeof(dd->primarydesc));
	dd->primarydesc.dwSize = sizeof(dd->primarydesc);
	dd->primarydesc.dwFlags = DDSD_CAPS;
	dd->primarydesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// for triple-buffered full screen mode, allocate flipping surfaces
	if (window->fullscreen && video_config.triplebuf)
	{
		dd->primarydesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
		dd->primarydesc.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		dd->primarydesc.dwBackBufferCount = 2;
	}

	// create the primary surface and report errors
	result = create_surface(dd, &dd->primarydesc, &dd->primary, "primary");
	if (result != DD_OK) goto error;

	// full screen mode: get the back surface
	dd->back = NULL;
	if (window->fullscreen && video_config.triplebuf)
	{
		DDSCAPS2 caps = { DDSCAPS_BACKBUFFER };
		result = IDirectDrawSurface7_GetAttachedSurface(dd->primary, &caps, &dd->back);
		if (result != DD_OK)
		{
			mame_printf_verbose("DirectDraw: Error %08X getting attached back surface\n", (int)result);
			goto error;
		}
	}

	// now make a description of our blit surface, based on the primary surface
	if (dd->blitwidth == 0 || dd->blitheight == 0)
		compute_blit_surface_size(window);
	dd->blitdesc = dd->primarydesc;
	dd->blitdesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
	dd->blitdesc.dwWidth = dd->blitwidth;
	dd->blitdesc.dwHeight = dd->blitheight;
	dd->blitdesc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

	// then create the blit surface, fall back to system memory if video mem doesn't work
	result = create_surface(dd, &dd->blitdesc, &dd->blit, "blit");
	if (result != DD_OK)
	{
		dd->blitdesc.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
		result = create_surface(dd, &dd->blitdesc, &dd->blit, "blit");
	}
	if (result != DD_OK) goto error;

	// create a memory buffer for offscreen drawing
	if (dd->membuffersize < dd->blitwidth * dd->blitheight * 4)
	{
		dd->membuffersize = dd->blitwidth * dd->blitheight * 4;
		global_free(dd->membuffer);
		dd->membuffer = global_alloc_array(UINT8, dd->membuffersize);
	}
	if (dd->membuffer == NULL)
		goto error;

	// create a clipper for windowed mode
	if (!window->fullscreen && create_clipper(window))
		goto error;

	// full screen mode: set the gamma
	if (window->fullscreen)
	{
		// only set the gamma if it's not 1.0f
		windows_options &options = downcast<windows_options &>(window->machine().options());
		float brightness = options.full_screen_brightness();
		float contrast = options.full_screen_contrast();
		float gamma = options.full_screen_gamma();
		if (brightness != 1.0f || contrast != 1.0f || gamma != 1.0f)
		{
			// see if we can get a GammaControl object
			result = IDirectDrawSurface_QueryInterface(dd->primary, WRAP_REFIID(IID_IDirectDrawGammaControl), (void **)&dd->gamma);
			if (result != DD_OK)
			{
				mame_printf_warning("DirectDraw: Warning - device does not support full screen gamma correction.\n");
				dd->gamma = NULL;
			}

			// proceed if we can
			if (dd->gamma != NULL)
			{
				DDGAMMARAMP ramp;
				int i;

				// create a standard ramp and set it
				for (i = 0; i < 256; i++)
					ramp.red[i] = ramp.green[i] = ramp.blue[i] = apply_brightness_contrast_gamma(i, brightness, contrast, gamma) << 8;

				// attempt to set it
				result = IDirectDrawGammaControl_SetGammaRamp(dd->gamma, 0, &ramp);
				if (result != DD_OK)
					mame_printf_verbose("DirectDraw: Error %08X attempting to set gamma correction.\n", (int)result);
			}
		}
	}

	// force some updates
	update_outer_rects(dd);
	return 0;

error:
	ddraw_delete_surfaces(window);
	return 1;
}



//============================================================
//  ddraw_delete
//============================================================

static void ddraw_delete(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;

	// free surfaces
	ddraw_delete_surfaces(window);

	// restore resolutions
	if (dd->ddraw != NULL)
		IDirectDraw7_RestoreDisplayMode(dd->ddraw);

	// reset cooperative level
	if (dd->ddraw != NULL && window->hwnd != NULL)
		IDirectDraw7_SetCooperativeLevel(dd->ddraw, window->hwnd, DDSCL_NORMAL);

	// release the DirectDraw object itself
	if (dd->ddraw != NULL)
		IDirectDraw7_Release(dd->ddraw);
	dd->ddraw = NULL;
}



//============================================================
//  ddraw_delete_surfaces
//============================================================

static void ddraw_delete_surfaces(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;

	// release the gamma control
	if (dd->gamma != NULL)
		IDirectDrawGammaControl_Release(dd->gamma);
	dd->gamma = NULL;

	// release the clipper
	if (dd->clipper != NULL)
		IDirectDrawClipper_Release(dd->clipper);
	dd->clipper = NULL;

	// free the memory buffer
	global_free(dd->membuffer);
	dd->membuffer = NULL;
	dd->membuffersize = 0;

	// release the blit surface
	if (dd->blit != NULL)
		IDirectDrawSurface7_Release(dd->blit);
	dd->blit = NULL;

	// release the back surface
	if (dd->back != NULL)
		IDirectDrawSurface7_Release(dd->back);
	dd->back = NULL;

	// release the primary surface
	if (dd->primary != NULL)
		IDirectDrawSurface7_Release(dd->primary);
	dd->primary = NULL;
}



//============================================================
//  ddraw_verify_caps
//============================================================

static int ddraw_verify_caps(dd_info *dd)
{
	int retval = 0;
	HRESULT result;

	// get the capabilities
	dd->ddcaps.dwSize = sizeof(dd->ddcaps);
	dd->helcaps.dwSize = sizeof(dd->helcaps);
	result = IDirectDraw7_GetCaps(dd->ddraw, &dd->ddcaps, &dd->helcaps);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X during IDirectDraw7_GetCaps call\n", (int)result);
		return 1;
	}

	// determine if hardware stretching is available
	if ((dd->ddcaps.dwCaps & DDCAPS_BLTSTRETCH) == 0)
	{
		mame_printf_verbose("DirectDraw: Warning - Device does not support hardware stretching\n");
		retval = 1;
	}

	return retval;
}



//============================================================
//  ddraw_test_cooperative
//============================================================

static int ddraw_test_cooperative(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;
	HRESULT result;

	// check our current status; if we lost the device, punt to GDI
	result = IDirectDraw7_TestCooperativeLevel(dd->ddraw);
	switch (result)
	{
		// punt to GDI if someone else has exclusive mode
		case DDERR_NOEXCLUSIVEMODE:
		case DDERR_EXCLUSIVEMODEALREADYSET:
			ddraw_delete_surfaces(window);
			return 1;

		// if we're ok, but we don't have a primary surface, create one
		default:
		case DD_OK:
			if (dd->primary == NULL)
				return ddraw_create_surfaces(window);
			return 0;
	}
}



//============================================================
//  create_surface
//============================================================

static HRESULT create_surface(dd_info *dd, DDSURFACEDESC2 *desc, IDirectDrawSurface7 **surface, const char *type)
{
	HRESULT result;

	// create the surface as requested
	result = IDirectDraw7_CreateSurface(dd->ddraw, desc, surface, NULL);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X creating %s surface\n", (int)result, type);
		return result;
	}

	// get a description of the primary surface
	result = IDirectDrawSurface7_GetSurfaceDesc(*surface, desc);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X getting %s surface desciption\n", (int)result, type);
		IDirectDrawSurface7_Release(*surface);
		*surface = NULL;
		return result;
	}

	// print out the good stuff
	mame_printf_verbose("DirectDraw: %s surface created: %dx%dx%d (R=%08X G=%08X B=%08X)\n",
				type,
				(int)desc->dwWidth,
				(int)desc->dwHeight,
				(int)desc->ddpfPixelFormat.dwRGBBitCount,
				(UINT32)desc->ddpfPixelFormat.dwRBitMask,
				(UINT32)desc->ddpfPixelFormat.dwGBitMask,
				(UINT32)desc->ddpfPixelFormat.dwBBitMask);
	return result;
}



//============================================================
//  create_clipper
//============================================================

static int create_clipper(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;
	HRESULT result;

	// create a clipper for the primary surface
	result = IDirectDraw7_CreateClipper(dd->ddraw, 0, &dd->clipper, NULL);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X creating clipper\n", (int)result);
		return 1;
	}

	// set the clipper's hwnd
	result = IDirectDrawClipper_SetHWnd(dd->clipper, 0, window->hwnd);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X setting clipper hwnd\n", (int)result);
		return 1;
	}

	// set the clipper on the primary surface
	result = IDirectDrawSurface7_SetClipper(dd->primary, dd->clipper);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X setting clipper on primary surface\n", (int)result);
		return 1;
	}
	return 0;
}



//============================================================
//  compute_blit_surface_size
//============================================================

static void compute_blit_surface_size(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;
	INT32 newwidth, newheight;
	int xscale, yscale;
	RECT client;

	// start with the minimum size
	window->target->compute_minimum_size(newwidth, newheight);

	// get the window's client rectangle
	GetClientRect(window->hwnd, &client);

	// hardware stretch case: apply prescale
	if (video_config.hwstretch)
	{
		int prescale = (video_config.prescale < 1) ? 1 : video_config.prescale;

		// clamp the prescale to something smaller than the target bounds
		xscale = prescale;
		while (xscale > 1 && newwidth * xscale > rect_width(&client))
			xscale--;
		yscale = prescale;
		while (yscale > 1 && newheight * yscale > rect_height(&client))
			yscale--;
	}

	// non stretch case
	else
	{
		INT32 target_width = rect_width(&client);
		INT32 target_height = rect_height(&client);
		float desired_aspect = 1.0f;

		// compute the appropriate visible area if we're trying to keepaspect
		if (video_config.keepaspect)
		{
			win_monitor_info *monitor = winwindow_video_window_monitor(window, NULL);
			window->target->compute_visible_area(target_width, target_height, winvideo_monitor_get_aspect(monitor), window->target->orientation(), target_width, target_height);
			desired_aspect = (float)target_width / (float)target_height;
		}

		// compute maximum integral scaling to fit the window
		xscale = (target_width + 2) / newwidth;
		yscale = (target_height + 2) / newheight;

		// try a little harder to keep the aspect ratio if desired
		if (video_config.keepaspect)
		{
			// if we could stretch more in the X direction, and that makes a better fit, bump the xscale
			while (newwidth * (xscale + 1) <= rect_width(&client) &&
				better_mode(newwidth * xscale, newheight * yscale, newwidth * (xscale + 1), newheight * yscale, desired_aspect))
				xscale++;

			// if we could stretch more in the Y direction, and that makes a better fit, bump the yscale
			while (newheight * (yscale + 1) <= rect_height(&client) &&
				better_mode(newwidth * xscale, newheight * yscale, newwidth * xscale, newheight * (yscale + 1), desired_aspect))
				yscale++;

			// now that we've maxed out, see if backing off the maximally stretched one makes a better fit
			if (rect_width(&client) - newwidth * xscale < rect_height(&client) - newheight * yscale)
			{
				while (xscale > 1 && better_mode(newwidth * xscale, newheight * yscale, newwidth * (xscale - 1), newheight * yscale, desired_aspect))
					xscale--;
			}
			else
			{
				while (yscale > 1 && better_mode(newwidth * xscale, newheight * yscale, newwidth * xscale, newheight * (yscale - 1), desired_aspect))
					yscale--;
			}
		}
	}

	// ensure at least a scale factor of 1
	if (xscale == 0) xscale = 1;
	if (yscale == 0) yscale = 1;

	// apply the final scale
	newwidth *= xscale;
	newheight *= yscale;
	if (newwidth != dd->blitwidth || newheight != dd->blitheight)
	{
		// force some updates
		update_outer_rects(dd);
		mame_printf_verbose("DirectDraw: New blit size = %dx%d\n", newwidth, newheight);
	}
	dd->blitwidth = newwidth;
	dd->blitheight = newheight;
}



//============================================================
//  calc_fullscreen_margins
//============================================================

static void calc_fullscreen_margins(win_window_info *window, DWORD desc_width, DWORD desc_height, RECT *margins)
{
	margins->left = 0;
	margins->top = 0;
	margins->right = desc_width;
	margins->bottom = desc_height;

	if (win_has_menu(window))
	{
		static int height_with_menubar = 0;
		if (height_with_menubar == 0)
		{
			RECT with_menu = { 100, 100, 200, 200 };
			RECT without_menu = { 100, 100, 200, 200 };
			AdjustWindowRect(&with_menu, WS_OVERLAPPED, TRUE);
			AdjustWindowRect(&without_menu, WS_OVERLAPPED, FALSE);
			height_with_menubar = (with_menu.bottom - with_menu.top) - (without_menu.bottom - without_menu.top);
		}
		margins->top = height_with_menubar;
	}
}



//============================================================
//  blit_to_primary
//============================================================

static void blit_to_primary(win_window_info *window, int srcwidth, int srcheight)
{
	dd_info *dd = (dd_info *)window->drawdata;
	IDirectDrawSurface7 *target = (dd->back != NULL) ? dd->back : dd->primary;
	win_monitor_info *monitor = winwindow_video_window_monitor(window, NULL);
	DDBLTFX blitfx = { sizeof(DDBLTFX) };
	RECT clear, outer, dest, source;
	INT32 dstwidth, dstheight;
	HRESULT result;

	// compute source rect
	source.left = source.top = 0;
	source.right = srcwidth;
	source.bottom = srcheight;

	// compute outer rect -- windowed version
	if (!window->fullscreen)
	{
		GetClientRect(window->hwnd, &outer);
		ClientToScreen(window->hwnd, &((LPPOINT)&outer)[0]);
		ClientToScreen(window->hwnd, &((LPPOINT)&outer)[1]);

		// adjust to be relative to the monitor
		outer.left -= monitor->info.rcMonitor.left;
		outer.right -= monitor->info.rcMonitor.left;
		outer.top -= monitor->info.rcMonitor.top;
		outer.bottom -= monitor->info.rcMonitor.top;
	}

	// compute outer rect -- full screen version
	else
	{
		calc_fullscreen_margins(window, dd->primarydesc.dwWidth, dd->primarydesc.dwHeight, &outer);
	}

	// if we're respecting the aspect ratio, we need to adjust to fit
	dstwidth = rect_width(&outer);
	dstheight = rect_height(&outer);
	if (!video_config.hwstretch)
	{
		// trim the source if necessary
		if (rect_width(&outer) < srcwidth)
		{
			source.left += (srcwidth - rect_width(&outer)) / 2;
			source.right = source.left + rect_width(&outer);
		}
		if (rect_height(&outer) < srcheight)
		{
			source.top += (srcheight - rect_height(&outer)) / 2;
			source.bottom = source.top + rect_height(&outer);
		}

		// match the destination and source sizes
		dstwidth = srcwidth = source.right - source.left;
		dstheight = srcheight = source.bottom - source.top;
	}
	else if (video_config.keepaspect)
	{
		// compute the appropriate visible area
		window->target->compute_visible_area(rect_width(&outer), rect_height(&outer), winvideo_monitor_get_aspect(monitor), window->target->orientation(), dstwidth, dstheight);
	}

	// center within
	dest.left = outer.left + (rect_width(&outer) - dstwidth) / 2;
	dest.right = dest.left + dstwidth;
	dest.top = outer.top + (rect_height(&outer) - dstheight) / 2;
	dest.bottom = dest.top + dstheight;

	// compare against last destination; if different, force a redraw
	if (dest.left != dd->lastdest.left || dest.right != dd->lastdest.right || dest.top != dd->lastdest.top || dest.bottom != dd->lastdest.bottom)
	{
		dd->lastdest = dest;
		update_outer_rects(dd);
	}

	// clear outer rects if we need to
	if (dd->clearouter != 0)
	{
		dd->clearouter--;

		// clear the left edge
		if (dest.left > outer.left)
		{
			clear = outer;
			clear.right = dest.left;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}

		// clear the right edge
		if (dest.right < outer.right)
		{
			clear = outer;
			clear.left = dest.right;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}

		// clear the top edge
		if (dest.top > outer.top)
		{
			clear = outer;
			clear.bottom = dest.top;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}

		// clear the bottom edge
		if (dest.bottom < outer.bottom)
		{
			clear = outer;
			clear.top = dest.bottom;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}
	}

	// do the blit
	result = IDirectDrawSurface7_Blt(target, &dest, dd->blit, &source, DDBLT_WAIT, NULL);
	if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X blitting to the screen\n", (int)result);

	// page flip if triple buffered
	if (window->fullscreen && dd->back != NULL)
	{
		result = IDirectDrawSurface7_Flip(dd->primary, NULL, DDFLIP_WAIT);
		if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X waiting for VBLANK\n", (int)result);
	}
}



//============================================================
//  config_adapter_mode
//============================================================

static int config_adapter_mode(win_window_info *window)
{
	DDDEVICEIDENTIFIER2 identifier;
	dd_info *dd = (dd_info *)window->drawdata;
	HRESULT result;

	// choose the monitor number
	get_adapter_for_monitor(dd, window->monitor);

	// create a temporary DirectDraw object
	result = (*directdrawcreateex)(dd->adapter_ptr, (LPVOID *)&dd->ddraw, WRAP_REFIID(IID_IDirectDraw7), NULL);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X during DirectDrawCreateEx call\n", (int)result);
		return 1;
	}

	// get the identifier
	result = IDirectDraw7_GetDeviceIdentifier(dd->ddraw, &identifier, 0);
	if (result != DD_OK)
	{
		mame_printf_error("Error getting identifier for device\n");
		return 1;
	}
	mame_printf_verbose("DirectDraw: Configuring device %s\n", identifier.szDescription);

	// get the current display mode
	memset(&dd->origmode, 0, sizeof(dd->origmode));
	dd->origmode.dwSize = sizeof(dd->origmode);
	result = IDirectDraw7_GetDisplayMode(dd->ddraw, &dd->origmode);
	if (result != DD_OK)
	{
		mame_printf_verbose("DirectDraw: Error %08X getting current display mode\n", (int)result);
		IDirectDraw7_Release(dd->ddraw);
		return 1;
	}

	// choose a resolution: full screen mode case
	if (window->fullscreen)
	{
		// default to the current mode exactly
		dd->width = dd->origmode.dwWidth;
		dd->height = dd->origmode.dwHeight;
		dd->refresh = dd->origmode.dwRefreshRate;

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			pick_best_mode(window);
	}

	// release the DirectDraw object
	IDirectDraw7_Release(dd->ddraw);
	dd->ddraw = NULL;

	// if we're not changing resolutions, make sure we have a resolution we can handle
	if (!window->fullscreen || !video_config.switchres)
	{
		switch (dd->origmode.ddpfPixelFormat.dwRBitMask)
		{
			case 0x00ff0000:
			case 0x000000ff:
			case 0xf800:
			case 0x7c00:
				break;

			default:
				mame_printf_verbose("DirectDraw: Unknown target mode: R=%08X G=%08X B=%08X\n", (int)dd->origmode.ddpfPixelFormat.dwRBitMask, (int)dd->origmode.ddpfPixelFormat.dwGBitMask, (int)dd->origmode.ddpfPixelFormat.dwBBitMask);
				return 1;
		}
	}

	return 0;
}



//============================================================
//  monitor_enum_callback
//============================================================

static BOOL WINAPI monitor_enum_callback(GUID FAR *guid, LPSTR description, LPSTR name, LPVOID context, HMONITOR hmonitor)
{
	monitor_enum_info *einfo = (monitor_enum_info *)context;

	// do we match the desired monitor?
	if (hmonitor == einfo->monitor->handle || (hmonitor == NULL && (einfo->monitor->info.dwFlags & MONITORINFOF_PRIMARY) != 0))
	{
		einfo->guid_ptr = (guid != NULL) ? &einfo->guid : NULL;
		if (guid != NULL)
			einfo->guid = *guid;
		einfo->foundit = TRUE;
	}
	return 1;
}



//============================================================
//  get_adapter_for_monitor
//============================================================

static void get_adapter_for_monitor(dd_info *dd, win_monitor_info *monitor)
{
	monitor_enum_info einfo;
	HRESULT result;

	// try to find our monitor
	memset(&einfo, 0, sizeof(einfo));
	einfo.monitor = monitor;
	result = (*directdrawenumerateex)(monitor_enum_callback, &einfo, DDENUM_ATTACHEDSECONDARYDEVICES);
	if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X during DirectDrawEnumerateEx call\n", (int)result);

	// set up the adapter
	if (einfo.foundit && einfo.guid_ptr != NULL)
	{
		dd->adapter = einfo.guid;
		dd->adapter_ptr = &dd->adapter;
	}
	else
		dd->adapter_ptr = NULL;
}



//============================================================
//  enum_modes_callback
//============================================================

static HRESULT WINAPI enum_modes_callback(LPDDSURFACEDESC2 desc, LPVOID context)
{
	float size_score, refresh_score, final_score;
	mode_enum_info *einfo = (mode_enum_info *)context;
	dd_info *dd = (dd_info *)einfo->window->drawdata;

	// skip non-32 bit modes
	if (desc->ddpfPixelFormat.dwRGBBitCount != 32)
		return DDENUMRET_OK;

	// compute initial score based on difference between target and current
	size_score = 1.0f / (1.0f + fabs((float)((INT32)desc->dwWidth - einfo->target_width)) + fabs((float)((INT32)desc->dwHeight - einfo->target_height)));

	// if the mode is too small, give a big penalty
	if (desc->dwWidth < einfo->minimum_width || desc->dwHeight < einfo->minimum_height)
		size_score *= 0.01f;

	// if mode is smaller than we'd like, it only scores up to 0.1
	if (desc->dwWidth < einfo->target_width || desc->dwHeight < einfo->target_height)
		size_score *= 0.1f;

	// if we're looking for a particular mode, that's a winner
	if (desc->dwWidth == einfo->window->maxwidth && desc->dwHeight == einfo->window->maxheight)
		size_score = 2.0f;

	// compute refresh score
	refresh_score = 1.0f / (1.0f + fabs((double)desc->dwRefreshRate - einfo->target_refresh));

	// if refresh is smaller than we'd like, it only scores up to 0.1
	if ((double)desc->dwRefreshRate < einfo->target_refresh)
		refresh_score *= 0.1f;

	// if we're looking for a particular refresh, make sure it matches
	if (desc->dwRefreshRate == einfo->window->refresh)
		refresh_score = 2.0f;

	// weight size and refresh equally
	final_score = size_score + refresh_score;

	// best so far?
	mame_printf_verbose("  %4dx%4d@%3dHz -> %f\n", (int)desc->dwWidth, (int)desc->dwHeight, (int)desc->dwRefreshRate, final_score * 1000.0f);
	if (final_score > einfo->best_score)
	{
		einfo->best_score = final_score;
		dd->width = desc->dwWidth;
		dd->height = desc->dwHeight;
		dd->refresh = desc->dwRefreshRate;
	}
	return DDENUMRET_OK;
}



//============================================================
//  pick_best_mode
//============================================================

static void pick_best_mode(win_window_info *window)
{
	dd_info *dd = (dd_info *)window->drawdata;
	mode_enum_info einfo;
	HRESULT result;

	// determine the minimum width/height for the selected target
	// note: technically we should not be calling this from an alternate window
	// thread; however, it is only done during init time, and the init code on
	// the main thread is waiting for us to finish, so it is safe to do so here
	window->target->compute_minimum_size(einfo.minimum_width, einfo.minimum_height);

	// use those as the target for now
	einfo.target_width = einfo.minimum_width * MAX(1, video_config.prescale);
	einfo.target_height = einfo.minimum_height * MAX(1, video_config.prescale);

	// determine the refresh rate of the primary screen
	einfo.target_refresh = 60.0;
	const screen_device *primary_screen = window->machine().config().first_screen();
	if (primary_screen != NULL)
		einfo.target_refresh = ATTOSECONDS_TO_HZ(primary_screen->refresh_attoseconds());
	printf("Target refresh = %f\n", einfo.target_refresh);

	// if we're not stretching, allow some slop on the minimum since we can handle it
	if (!video_config.hwstretch)
	{
		einfo.minimum_width -= 4;
		einfo.minimum_height -= 4;
	}

	// if we are stretching, aim for a mode approximately 2x the game's resolution
	else if (video_config.prescale <= 1)
	{
		einfo.target_width *= 2;
		einfo.target_height *= 2;
	}

	// fill in the rest of the data
	einfo.window = window;
	einfo.best_score = 0.0f;

	// enumerate the modes
	mame_printf_verbose("DirectDraw: Selecting video mode...\n");
	result = IDirectDraw7_EnumDisplayModes(dd->ddraw, DDEDM_REFRESHRATES, NULL, &einfo, enum_modes_callback);
	if (result != DD_OK) mame_printf_verbose("DirectDraw: Error %08X during EnumDisplayModes call\n", (int)result);
	mame_printf_verbose("DirectDraw: Mode selected = %4dx%4d@%3dHz\n", dd->width, dd->height, dd->refresh);
}
