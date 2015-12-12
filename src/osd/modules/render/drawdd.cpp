// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  drawdd.c - Win32 DirectDraw implementation
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
#include "rendersw.inc"

// MAMEOS headers
#include "winmain.h"
#include "window.h"



//============================================================
//  TYPE DEFINITIONS
//============================================================

typedef HRESULT (WINAPI *directdrawcreateex_ptr)(GUID FAR *lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);
typedef HRESULT (WINAPI *directdrawenumerateex_ptr)(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);


/* dd_info is the information about DirectDraw for the current screen */
class renderer_dd : public osd_renderer
{
public:
	renderer_dd(osd_window *window)
	: osd_renderer(window, FLAG_NONE),
		width(0),
		height(0),
		refresh(0),
		//adapter(0),
		adapter_ptr(NULL),
		clearouter(0),
		blitwidth(0), blitheight(0),
		//lastdest
		ddraw(NULL),
		primary(NULL),
		back(NULL),
		blit(NULL),
		clipper(NULL),
		gamma(NULL),
		//DDSURFACEDESC2          primarydesc;
		//DDSURFACEDESC2          blitdesc;
		//DDSURFACEDESC2          origmode;
		//ddcaps(0),
		//helcaps(0),
		membuffer(NULL),
		membuffersize(0)
	{ }

	virtual ~renderer_dd() { }

	virtual int create() override;
	virtual render_primitive_list *get_primitives() override;
	virtual int draw(const int update) override;
	virtual void save() override {};
	virtual void record() override {};
	virtual void toggle_fsfx() override {};
	virtual void destroy() override;

	int                     width, height;              // current width, height
	int                     refresh;                    // current refresh rate

private:

	inline void update_outer_rects();

	// surface management
	int ddraw_create();
	int ddraw_create_surfaces();
	void ddraw_delete();
	void ddraw_delete_surfaces();
	int ddraw_verify_caps();
	int ddraw_test_cooperative();
	HRESULT create_surface(DDSURFACEDESC2 *desc, IDirectDrawSurface7 **surface, const char *type);
	int create_clipper();

	// drawing helpers
	void compute_blit_surface_size();
	void blit_to_primary(int srcwidth, int srcheight);

	// video modes
	int config_adapter_mode();
	void get_adapter_for_monitor(osd_monitor_info *monitor);
	void pick_best_mode();

	// various
	void calc_fullscreen_margins(DWORD desc_width, DWORD desc_height, RECT *margins);


	GUID                    adapter;                    // current display adapter
	GUID *                  adapter_ptr;                // pointer to current display adapter
	int                     clearouter;                 // clear the outer areas?

	INT32                   blitwidth, blitheight;      // current blit width/height values
	RECT                    lastdest;                   // last destination rectangle

	IDirectDraw7 *          ddraw;                      // pointer to the DirectDraw object
	IDirectDrawSurface7 *   primary;                    // pointer to the primary surface object
	IDirectDrawSurface7 *   back;                       // pointer to the back buffer surface object
	IDirectDrawSurface7 *   blit;                       // pointer to the blit surface object
	IDirectDrawClipper *    clipper;                    // pointer to the clipper object
	IDirectDrawGammaControl *gamma;                     // pointer to the gamma control object

	DDSURFACEDESC2          primarydesc;                // description of the primary surface
	DDSURFACEDESC2          blitdesc;                   // description of the blitting surface
	DDSURFACEDESC2          origmode;                   // original video mode

	DDCAPS                  ddcaps;                     // capabilities of the device
	DDCAPS                  helcaps;                    // capabilities of the hardware

	UINT8 *                 membuffer;                  // memory buffer for complex rendering
	UINT32                  membuffersize;              // current size of the memory buffer
};


/* monitor_enum_info holds information during a monitor enumeration */
struct monitor_enum_info
{
	osd_monitor_info *      monitor;                    // pointer to monitor we want
	GUID                    guid;                       // GUID of the one we found
	GUID *                  guid_ptr;                   // pointer to our GUID
	int                     foundit;                    // TRUE if we found what we wanted
};


/* mode_enum_info holds information during a display mode enumeration */
struct mode_enum_info
{
	renderer_dd *           renderer;
	osd_window  *           window;
	INT32                   minimum_width, minimum_height;
	INT32                   target_width, target_height;
	double                  target_refresh;
	float                   best_score;
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

inline void renderer_dd::update_outer_rects()
{
	clearouter = (back != NULL) ? 3 : 1;
}


static inline int better_mode(int width0, int height0, int width1, int height1, float desired_aspect)
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



//============================================================
//  drawnone_create
//============================================================

static osd_renderer *drawdd_create(osd_window *window)
{
	return global_alloc(renderer_dd(window));
}


//============================================================
//  drawdd_init
//============================================================

int drawdd_init(running_machine &machine, osd_draw_callbacks *callbacks)
{
	// dynamically grab the create function from ddraw.dll
	dllhandle = LoadLibrary(TEXT("ddraw.dll"));
	if (dllhandle == NULL)
	{
		osd_printf_verbose("DirectDraw: Unable to access ddraw.dll\n");
		return 1;
	}

	// import the create function
	directdrawcreateex = (directdrawcreateex_ptr)GetProcAddress(dllhandle, "DirectDrawCreateEx");
	if (directdrawcreateex == NULL)
	{
		osd_printf_verbose("DirectDraw: Unable to find DirectDrawCreateEx\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return 1;
	}

	// import the enumerate function
	directdrawenumerateex = (directdrawenumerateex_ptr)GetProcAddress(dllhandle, "DirectDrawEnumerateExA");
	if (directdrawenumerateex == NULL)
	{
		osd_printf_verbose("DirectDraw: Unable to find DirectDrawEnumerateExA\n");
		FreeLibrary(dllhandle);
		dllhandle = NULL;
		return 1;
	}

	// fill in the callbacks
	memset(callbacks, 0, sizeof(*callbacks));
	callbacks->exit = drawdd_exit;
	callbacks->create = drawdd_create;

	osd_printf_verbose("DirectDraw: Using DirectDraw 7\n");
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

int renderer_dd::create()
{
	// configure the adapter for the mode we want
	if (config_adapter_mode())
	{
		osd_printf_error("Unable to configure adapter.\n");
		goto error;
	}

	// create the ddraw object
	if (ddraw_create())
	{
		osd_printf_error("Unable to create ddraw object.\n");
		goto error;
	}

	return 0;

error:
	destroy();
	osd_printf_error("Unable to initialize DirectDraw.\n");
	return 1;
}



//============================================================
//  drawdd_window_destroy
//============================================================

void renderer_dd::destroy()
{
	// delete the ddraw object
	ddraw_delete();
}



//============================================================
//  drawdd_window_get_primitives
//============================================================

render_primitive_list *renderer_dd::get_primitives()
{
	compute_blit_surface_size();
	window().target()->set_bounds(blitwidth, blitheight, 0);
	window().target()->set_max_update_rate((refresh == 0) ? origmode.dwRefreshRate : refresh);

	return &window().target()->get_primitives();
}



//============================================================
//  drawdd_window_draw
//============================================================

int renderer_dd::draw(const int update)
{
	render_primitive *prim;
	int usemembuffer = FALSE;
	HRESULT result;

	// if we're updating, remember to erase the outer stuff
	if (update)
		update_outer_rects();

	// if we have a ddraw object, check the cooperative level
	if (ddraw_test_cooperative())
		return 1;

	// get the size; if we're too small, delete the existing surfaces
	if (blitwidth > blitdesc.dwWidth || blitheight > blitdesc.dwHeight)
		ddraw_delete_surfaces();

	// if we need to create surfaces, do it now
	if (blit == NULL && ddraw_create_surfaces() != 0)
		return 1;

	// select our surface and lock it
	result = IDirectDrawSurface7_Lock(blit, NULL, &blitdesc, DDLOCK_WAIT, NULL);
	if (result == DDERR_SURFACELOST)
	{
		osd_printf_verbose("DirectDraw: Lost surfaces; deleting and retrying next frame\n");
		ddraw_delete_surfaces();
		return 1;
	}
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X locking blit surface\n", (int)result);
		return 1;
	}

	// render to it
	window().m_primlist->acquire_lock();

	// scan the list of primitives for tricky stuff
	for (prim = window().m_primlist->first(); prim != NULL; prim = prim->next())
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
		switch (blitdesc.ddpfPixelFormat.dwRBitMask)
		{
			case 0x00ff0000:    software_renderer<UINT32, 0,0,0, 16,8,0>::draw_primitives(*window().m_primlist, membuffer, blitwidth, blitheight, blitwidth);  break;
			case 0x000000ff:    software_renderer<UINT32, 0,0,0, 0,8,16>::draw_primitives(*window().m_primlist, membuffer, blitwidth, blitheight, blitwidth);  break;
			case 0xf800:        software_renderer<UINT16, 3,2,3, 11,5,0>::draw_primitives(*window().m_primlist, membuffer, blitwidth, blitheight, blitwidth);  break;
			case 0x7c00:        software_renderer<UINT16, 3,3,3, 10,5,0>::draw_primitives(*window().m_primlist, membuffer, blitwidth, blitheight, blitwidth);  break;
			default:
				osd_printf_verbose("DirectDraw: Unknown target mode: R=%08X G=%08X B=%08X\n", (int)blitdesc.ddpfPixelFormat.dwRBitMask, (int)blitdesc.ddpfPixelFormat.dwGBitMask, (int)blitdesc.ddpfPixelFormat.dwBBitMask);
				break;
		}

		// handle copying to both 16bpp and 32bpp destinations
		for (y = 0; y < blitheight; y++)
		{
			if (blitdesc.ddpfPixelFormat.dwRGBBitCount == 32)
			{
				UINT32 *src = (UINT32 *)membuffer + y * blitwidth;
				UINT32 *dst = (UINT32 *)((UINT8 *)blitdesc.lpSurface + y * blitdesc.lPitch);
				for (x = 0; x < blitwidth; x++)
					*dst++ = *src++;
			}
			else if (blitdesc.ddpfPixelFormat.dwRGBBitCount == 16)
			{
				UINT16 *src = (UINT16 *)membuffer + y * blitwidth;
				UINT16 *dst = (UINT16 *)((UINT8 *)blitdesc.lpSurface + y * blitdesc.lPitch);
				for (x = 0; x < blitwidth; x++)
					*dst++ = *src++;
			}
		}

	}

	// otherwise, draw directly
	else
	{
		// based on the target format, use one of our standard renderers
		switch (blitdesc.ddpfPixelFormat.dwRBitMask)
		{
			case 0x00ff0000:    software_renderer<UINT32, 0,0,0, 16,8,0, true>::draw_primitives(*window().m_primlist, blitdesc.lpSurface, blitwidth, blitheight, blitdesc.lPitch / 4); break;
			case 0x000000ff:    software_renderer<UINT32, 0,0,0, 0,8,16, true>::draw_primitives(*window().m_primlist, blitdesc.lpSurface, blitwidth, blitheight, blitdesc.lPitch / 4); break;
			case 0xf800:        software_renderer<UINT16, 3,2,3, 11,5,0, true>::draw_primitives(*window().m_primlist, blitdesc.lpSurface, blitwidth, blitheight, blitdesc.lPitch / 2); break;
			case 0x7c00:        software_renderer<UINT16, 3,3,3, 10,5,0, true>::draw_primitives(*window().m_primlist, blitdesc.lpSurface, blitwidth, blitheight, blitdesc.lPitch / 2); break;
			default:
				osd_printf_verbose("DirectDraw: Unknown target mode: R=%08X G=%08X B=%08X\n", (int)blitdesc.ddpfPixelFormat.dwRBitMask, (int)blitdesc.ddpfPixelFormat.dwGBitMask, (int)blitdesc.ddpfPixelFormat.dwBBitMask);
				break;
		}
	}
	window().m_primlist->release_lock();

	// unlock and blit
	result = IDirectDrawSurface7_Unlock(blit, NULL);
	if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X unlocking blit surface\n", (int)result);

	// sync to VBLANK
	if ((video_config.waitvsync || video_config.syncrefresh) && window().machine().video().throttled() && (!window().fullscreen() || back == NULL))
	{
		result = IDirectDraw7_WaitForVerticalBlank(ddraw, DDWAITVB_BLOCKBEGIN, NULL);
		if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X waiting for VBLANK\n", (int)result);
	}

	// complete the blitting
	blit_to_primary(blitwidth, blitheight);
	return 0;
}



//============================================================
//  ddraw_create
//============================================================

int renderer_dd::ddraw_create()
{
	HRESULT result;
	int verify;

	// if a device exists, free it
	if (ddraw != NULL)
		ddraw_delete();

	// create the DirectDraw object
	result = (*directdrawcreateex)(adapter_ptr, (LPVOID *)&ddraw, WRAP_REFIID(IID_IDirectDraw7), NULL);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X during DirectDrawCreateEx call\n", (int)result);
		goto error;
	}

	// verify the caps
	verify = ddraw_verify_caps();
	if (verify == 2)
	{
		osd_printf_error("DirectDraw: Error - Device does not meet minimum requirements for DirectDraw rendering\n");
		goto error;
	}
	if (verify == 1)
		osd_printf_verbose("DirectDraw: Warning - Device may not perform well for DirectDraw rendering\n");

	// set the cooperative level
	// for non-window modes, we will use full screen here
	result = IDirectDraw7_SetCooperativeLevel(ddraw, win_window_list->m_hwnd, DDSCL_SETFOCUSWINDOW);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X during IDirectDraw7_SetCooperativeLevel(FOCUSWINDOW) call\n", (int)result);
		goto error;
	}
	result = IDirectDraw7_SetCooperativeLevel(ddraw, window().m_hwnd, DDSCL_SETDEVICEWINDOW | (window().fullscreen() ? DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE : DDSCL_NORMAL));
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X during IDirectDraw7_SetCooperativeLevel(DEVICEWINDOW) call\n", (int)result);
		goto error;
	}

	// full screen mode: set the resolution
	if (window().fullscreen() && video_config.switchres)
	{
		result = IDirectDraw7_SetDisplayMode(ddraw, width, height, 32, refresh, 0);
		if (result != DD_OK)
		{
			osd_printf_verbose("DirectDraw: Error %08X attempting to set video mode %dx%d@%d call\n", (int)result, width, height, refresh);
			goto error;
		}
	}

	return ddraw_create_surfaces();

error:
	ddraw_delete();
	return 1;
}



//============================================================
//  ddraw_create_surfaces
//============================================================

int renderer_dd::ddraw_create_surfaces()
{
	HRESULT result;

	// make a description of the primary surface
	memset(&primarydesc, 0, sizeof(primarydesc));
	primarydesc.dwSize = sizeof(primarydesc);
	primarydesc.dwFlags = DDSD_CAPS;
	primarydesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// for triple-buffered full screen mode, allocate flipping surfaces
	if (window().fullscreen() && video_config.triplebuf)
	{
		primarydesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
		primarydesc.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		primarydesc.dwBackBufferCount = 2;
	}

	// create the primary surface and report errors
	result = create_surface(&primarydesc, &primary, "primary");
	if (result != DD_OK) goto error;

	// full screen mode: get the back surface
	back = NULL;
	if (window().fullscreen() && video_config.triplebuf)
	{
		DDSCAPS2 caps = { DDSCAPS_BACKBUFFER };
		result = IDirectDrawSurface7_GetAttachedSurface(primary, &caps, &back);
		if (result != DD_OK)
		{
			osd_printf_verbose("DirectDraw: Error %08X getting attached back surface\n", (int)result);
			goto error;
		}
	}

	// now make a description of our blit surface, based on the primary surface
	if (blitwidth == 0 || blitheight == 0)
		compute_blit_surface_size();
	blitdesc = primarydesc;
	blitdesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
	blitdesc.dwWidth = blitwidth;
	blitdesc.dwHeight = blitheight;
	blitdesc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

	// then create the blit surface, fall back to system memory if video mem doesn't work
	result = create_surface(&blitdesc, &blit, "blit");
	if (result != DD_OK)
	{
		blitdesc.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY;
		result = create_surface(&blitdesc, &blit, "blit");
	}
	if (result != DD_OK) goto error;

	// create a memory buffer for offscreen drawing
	if (membuffersize < blitwidth * blitheight * 4)
	{
		membuffersize = blitwidth * blitheight * 4;
		global_free_array(membuffer);
		membuffer = global_alloc_array(UINT8, membuffersize);
	}
	if (membuffer == NULL)
		goto error;

	// create a clipper for windowed mode
	if (!window().fullscreen() && create_clipper())
		goto error;

	// full screen mode: set the gamma
	if (window().fullscreen())
	{
		// only set the gamma if it's not 1.0f
		windows_options &options = downcast<windows_options &>(window().machine().options());
		float brightness = options.full_screen_brightness();
		float contrast = options.full_screen_contrast();
		float fgamma = options.full_screen_gamma();
		if (brightness != 1.0f || contrast != 1.0f || fgamma != 1.0f)
		{
			// see if we can get a GammaControl object
			result = IDirectDrawSurface_QueryInterface(primary, WRAP_REFIID(IID_IDirectDrawGammaControl), (void **)&gamma);
			if (result != DD_OK)
			{
				osd_printf_warning("DirectDraw: Warning - device does not support full screen gamma correction.\n");
				this->gamma = NULL;
			}

			// proceed if we can
			if (this->gamma != NULL)
			{
				DDGAMMARAMP ramp;
				int i;

				// create a standard ramp and set it
				for (i = 0; i < 256; i++)
					ramp.red[i] = ramp.green[i] = ramp.blue[i] = apply_brightness_contrast_gamma(i, brightness, contrast, fgamma) << 8;

				// attempt to set it
				result = IDirectDrawGammaControl_SetGammaRamp(this->gamma, 0, &ramp);
				if (result != DD_OK)
					osd_printf_verbose("DirectDraw: Error %08X attempting to set gamma correction.\n", (int)result);
			}
		}
	}

	// force some updates
	update_outer_rects();
	return 0;

error:
	ddraw_delete_surfaces();
	return 1;
}



//============================================================
//  ddraw_delete
//============================================================

void renderer_dd::ddraw_delete()
{
	// free surfaces
	ddraw_delete_surfaces();

	// restore resolutions
	if (ddraw != NULL)
		IDirectDraw7_RestoreDisplayMode(ddraw);

	// reset cooperative level
	if (ddraw != NULL && window().m_hwnd != NULL)
		IDirectDraw7_SetCooperativeLevel(ddraw, window().m_hwnd, DDSCL_NORMAL);

	// release the DirectDraw object itself
	if (ddraw != NULL)
		IDirectDraw7_Release(ddraw);
	ddraw = NULL;
}



//============================================================
//  ddraw_delete_surfaces
//============================================================

void renderer_dd::ddraw_delete_surfaces()
{
	// release the gamma control
	if (gamma != NULL)
		IDirectDrawGammaControl_Release(gamma);
	gamma = NULL;

	// release the clipper
	if (clipper != NULL)
		IDirectDrawClipper_Release(clipper);
	clipper = NULL;

	// free the memory buffer
	global_free_array(membuffer);
	membuffer = NULL;
	membuffersize = 0;

	// release the blit surface
	if (blit != NULL)
		IDirectDrawSurface7_Release(blit);
	blit = NULL;

	// release the back surface
	if (back != NULL)
		IDirectDrawSurface7_Release(back);
	back = NULL;

	// release the primary surface
	if (primary != NULL)
		IDirectDrawSurface7_Release(primary);
	primary = NULL;
}



//============================================================
//  ddraw_verify_caps
//============================================================

int renderer_dd::ddraw_verify_caps()
{
	int retval = 0;
	HRESULT result;

	// get the capabilities
	ddcaps.dwSize = sizeof(ddcaps);
	helcaps.dwSize = sizeof(helcaps);
	result = IDirectDraw7_GetCaps(ddraw, &ddcaps, &helcaps);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X during IDirectDraw7_GetCaps call\n", (int)result);
		return 1;
	}

	// determine if hardware stretching is available
	if ((ddcaps.dwCaps & DDCAPS_BLTSTRETCH) == 0)
	{
		osd_printf_verbose("DirectDraw: Warning - Device does not support hardware stretching\n");
		retval = 1;
	}

	return retval;
}



//============================================================
//  ddraw_test_cooperative
//============================================================

int renderer_dd::ddraw_test_cooperative()
{
	HRESULT result;

	// check our current status; if we lost the device, punt to GDI
	result = IDirectDraw7_TestCooperativeLevel(ddraw);
	switch (result)
	{
		// punt to GDI if someone else has exclusive mode
		case DDERR_NOEXCLUSIVEMODE:
		case DDERR_EXCLUSIVEMODEALREADYSET:
			ddraw_delete_surfaces();
			return 1;

		// if we're ok, but we don't have a primary surface, create one
		default:
		case DD_OK:
			if (primary == NULL)
				return ddraw_create_surfaces();
			return 0;
	}
}



//============================================================
//  create_surface
//============================================================

HRESULT renderer_dd::create_surface(DDSURFACEDESC2 *desc, IDirectDrawSurface7 **surface, const char *type)
{
	HRESULT result;

	// create the surface as requested
	result = IDirectDraw7_CreateSurface(ddraw, desc, surface, NULL);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X creating %s surface\n", (int)result, type);
		return result;
	}

	// get a description of the primary surface
	result = IDirectDrawSurface7_GetSurfaceDesc(*surface, desc);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X getting %s surface desciption\n", (int)result, type);
		IDirectDrawSurface7_Release(*surface);
		*surface = NULL;
		return result;
	}

	// print out the good stuff
	osd_printf_verbose("DirectDraw: %s surface created: %dx%dx%d (R=%08X G=%08X B=%08X)\n",
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

int renderer_dd::create_clipper()
{
	HRESULT result;

	// create a clipper for the primary surface
	result = IDirectDraw7_CreateClipper(ddraw, 0, &clipper, NULL);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X creating clipper\n", (int)result);
		return 1;
	}

	// set the clipper's hwnd
	result = IDirectDrawClipper_SetHWnd(clipper, 0, window().m_hwnd);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X setting clipper hwnd\n", (int)result);
		return 1;
	}

	// set the clipper on the primary surface
	result = IDirectDrawSurface7_SetClipper(primary, clipper);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X setting clipper on primary surface\n", (int)result);
		return 1;
	}
	return 0;
}



//============================================================
//  compute_blit_surface_size
//============================================================

void renderer_dd::compute_blit_surface_size()
{
	INT32 newwidth, newheight;
	int xscale, yscale;
	RECT client;

	// start with the minimum size
	window().target()->compute_minimum_size(newwidth, newheight);

	// get the window's client rectangle
	GetClientRect(window().m_hwnd, &client);

	// hardware stretch case: apply prescale
	if (video_config.hwstretch)
	{
		int prescale = (window().prescale() < 1) ? 1 : window().prescale();

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
			osd_monitor_info *monitor = window().winwindow_video_window_monitor(NULL);
			window().target()->compute_visible_area(target_width, target_height, monitor->aspect(), window().target()->orientation(), target_width, target_height);
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
	if (newwidth != blitwidth || newheight != blitheight)
	{
		// force some updates
		update_outer_rects();
		osd_printf_verbose("DirectDraw: New blit size = %dx%d\n", newwidth, newheight);
	}
	blitwidth = newwidth;
	blitheight = newheight;
}



//============================================================
//  calc_fullscreen_margins
//============================================================

void renderer_dd::calc_fullscreen_margins(DWORD desc_width, DWORD desc_height, RECT *margins)
{
	margins->left = 0;
	margins->top = 0;
	margins->right = desc_width;
	margins->bottom = desc_height;

	if (window().win_has_menu())
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

void renderer_dd::blit_to_primary(int srcwidth, int srcheight)
{
	IDirectDrawSurface7 *target = (back != NULL) ? back : primary;
	osd_monitor_info *monitor = window().winwindow_video_window_monitor(NULL);
	DDBLTFX blitfx = { sizeof(DDBLTFX) };
	RECT clear, outer, dest, source;
	INT32 dstwidth, dstheight;
	HRESULT result;

	// compute source rect
	source.left = source.top = 0;
	source.right = srcwidth;
	source.bottom = srcheight;

	// compute outer rect -- windowed version
	if (!window().fullscreen())
	{
		GetClientRect(window().m_hwnd, &outer);
		ClientToScreen(window().m_hwnd, &((LPPOINT)&outer)[0]);
		ClientToScreen(window().m_hwnd, &((LPPOINT)&outer)[1]);

		// adjust to be relative to the monitor
		osd_rect pos = monitor->position_size();
		outer.left -= pos.left();
		outer.right -= pos.left();
		outer.top -= pos.top();
		outer.bottom -= pos.top();
	}

	// compute outer rect -- full screen version
	else
	{
		calc_fullscreen_margins(primarydesc.dwWidth, primarydesc.dwHeight, &outer);
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
		window().target()->compute_visible_area(rect_width(&outer), rect_height(&outer), monitor->aspect(), window().target()->orientation(), dstwidth, dstheight);
	}

	// center within
	dest.left = outer.left + (rect_width(&outer) - dstwidth) / 2;
	dest.right = dest.left + dstwidth;
	dest.top = outer.top + (rect_height(&outer) - dstheight) / 2;
	dest.bottom = dest.top + dstheight;

	// compare against last destination; if different, force a redraw
	if (dest.left != lastdest.left || dest.right != lastdest.right || dest.top != lastdest.top || dest.bottom != lastdest.bottom)
	{
		lastdest = dest;
		update_outer_rects();
	}

	// clear outer rects if we need to
	if (clearouter != 0)
	{
		clearouter--;

		// clear the left edge
		if (dest.left > outer.left)
		{
			clear = outer;
			clear.right = dest.left;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}

		// clear the right edge
		if (dest.right < outer.right)
		{
			clear = outer;
			clear.left = dest.right;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}

		// clear the top edge
		if (dest.top > outer.top)
		{
			clear = outer;
			clear.bottom = dest.top;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}

		// clear the bottom edge
		if (dest.bottom < outer.bottom)
		{
			clear = outer;
			clear.top = dest.bottom;
			result = IDirectDrawSurface_Blt(target, &clear, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blitfx);
			if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X clearing the screen\n", (int)result);
		}
	}

	// do the blit
	result = IDirectDrawSurface7_Blt(target, &dest, blit, &source, DDBLT_WAIT, NULL);
	if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X blitting to the screen\n", (int)result);

	// page flip if triple buffered
	if (window().fullscreen() && back != NULL)
	{
		result = IDirectDrawSurface7_Flip(primary, NULL, DDFLIP_WAIT);
		if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X waiting for VBLANK\n", (int)result);
	}
}



//============================================================
//  config_adapter_mode
//============================================================

int renderer_dd::config_adapter_mode()
{
	DDDEVICEIDENTIFIER2 identifier;
	HRESULT result;

	// choose the monitor number
	get_adapter_for_monitor(window().monitor());

	// create a temporary DirectDraw object
	result = (*directdrawcreateex)(adapter_ptr, (LPVOID *)&ddraw, WRAP_REFIID(IID_IDirectDraw7), NULL);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X during DirectDrawCreateEx call\n", (int)result);
		return 1;
	}

	// get the identifier
	result = IDirectDraw7_GetDeviceIdentifier(ddraw, &identifier, 0);
	if (result != DD_OK)
	{
		osd_printf_error("Error getting identifier for device\n");
		return 1;
	}
	osd_printf_verbose("DirectDraw: Configuring device %s\n", identifier.szDescription);

	// get the current display mode
	memset(&origmode, 0, sizeof(origmode));
	origmode.dwSize = sizeof(origmode);
	result = IDirectDraw7_GetDisplayMode(ddraw, &origmode);
	if (result != DD_OK)
	{
		osd_printf_verbose("DirectDraw: Error %08X getting current display mode\n", (int)result);
		IDirectDraw7_Release(ddraw);
		return 1;
	}

	// choose a resolution: full screen mode case
	if (window().fullscreen())
	{
		// default to the current mode exactly
		width = origmode.dwWidth;
		height = origmode.dwHeight;
		refresh = origmode.dwRefreshRate;

		// if we're allowed to switch resolutions, override with something better
		if (video_config.switchres)
			pick_best_mode();
	}

	// release the DirectDraw object
	IDirectDraw7_Release(ddraw);
	ddraw = NULL;

	// if we're not changing resolutions, make sure we have a resolution we can handle
	if (!window().fullscreen() || !video_config.switchres)
	{
		switch (origmode.ddpfPixelFormat.dwRBitMask)
		{
			case 0x00ff0000:
			case 0x000000ff:
			case 0xf800:
			case 0x7c00:
				break;

			default:
				osd_printf_verbose("DirectDraw: Unknown target mode: R=%08X G=%08X B=%08X\n", (int)origmode.ddpfPixelFormat.dwRBitMask, (int)origmode.ddpfPixelFormat.dwGBitMask, (int)origmode.ddpfPixelFormat.dwBBitMask);
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
	if (hmonitor == *((HMONITOR *)einfo->monitor->oshandle()) || (hmonitor == NULL && einfo->monitor->is_primary()))
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

void renderer_dd::get_adapter_for_monitor(osd_monitor_info *monitor)
{
	monitor_enum_info einfo;
	HRESULT result;

	// try to find our monitor
	memset(&einfo, 0, sizeof(einfo));
	einfo.monitor = monitor;
	result = (*directdrawenumerateex)(monitor_enum_callback, &einfo, DDENUM_ATTACHEDSECONDARYDEVICES);
	if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X during DirectDrawEnumerateEx call\n", (int)result);

	// set up the adapter
	if (einfo.foundit && einfo.guid_ptr != NULL)
	{
		adapter = einfo.guid;
		adapter_ptr = &adapter;
	}
	else
		adapter_ptr = NULL;
}



//============================================================
//  enum_modes_callback
//============================================================

static HRESULT WINAPI enum_modes_callback(LPDDSURFACEDESC2 desc, LPVOID context)
{
	float size_score, refresh_score, final_score;
	mode_enum_info *einfo = (mode_enum_info *)context;
	renderer_dd *dd = einfo->renderer;

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
	if (desc->dwWidth == einfo->window->m_win_config.width && desc->dwHeight == einfo->window->m_win_config.height)
		size_score = 2.0f;

	// compute refresh score
	refresh_score = 1.0f / (1.0f + fabs((double)desc->dwRefreshRate - einfo->target_refresh));

	// if refresh is smaller than we'd like, it only scores up to 0.1
	if ((double)desc->dwRefreshRate < einfo->target_refresh)
		refresh_score *= 0.1f;

	// if we're looking for a particular refresh, make sure it matches
	if (desc->dwRefreshRate == einfo->window->m_win_config.refresh)
		refresh_score = 2.0f;

	// weight size and refresh equally
	final_score = size_score + refresh_score;

	// best so far?
	osd_printf_verbose("  %4dx%4d@%3dHz -> %f\n", (int)desc->dwWidth, (int)desc->dwHeight, (int)desc->dwRefreshRate, final_score * 1000.0f);
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

void renderer_dd::pick_best_mode()
{
	mode_enum_info einfo;
	HRESULT result;

	// determine the minimum width/height for the selected target
	// note: technically we should not be calling this from an alternate window
	// thread; however, it is only done during init time, and the init code on
	// the main thread is waiting for us to finish, so it is safe to do so here
	window().target()->compute_minimum_size(einfo.minimum_width, einfo.minimum_height);

	// use those as the target for now
	einfo.target_width = einfo.minimum_width * MAX(1, window().prescale());
	einfo.target_height = einfo.minimum_height * MAX(1, window().prescale());

	// determine the refresh rate of the primary screen
	einfo.target_refresh = 60.0;
	const screen_device *primary_screen = window().machine().config().first_screen();
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
	else if (window().prescale() <= 1)
	{
		einfo.target_width *= 2;
		einfo.target_height *= 2;
	}

	// fill in the rest of the data
	einfo.window = &window();
	einfo.renderer = this;
	einfo.best_score = 0.0f;

	// enumerate the modes
	osd_printf_verbose("DirectDraw: Selecting video mode...\n");
	result = IDirectDraw7_EnumDisplayModes(ddraw, DDEDM_REFRESHRATES, NULL, &einfo, enum_modes_callback);
	if (result != DD_OK) osd_printf_verbose("DirectDraw: Error %08X during EnumDisplayModes call\n", (int)result);
	osd_printf_verbose("DirectDraw: Mode selected = %4dx%4d@%3dHz\n", width, height, refresh);
}
