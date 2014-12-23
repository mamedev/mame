//============================================================
//
//  video.h - SDL implementation of MAME video routines
//
//  Copyright (c) 1996-2014, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#ifndef __SDLVIDEO__
#define __SDLVIDEO__

//============================================================
//  CONSTANTS
//============================================================

#define MAX_VIDEO_WINDOWS           (4)

enum {
	VIDEO_MODE_SOFT = 0,
	VIDEO_MODE_OPENGL,
	VIDEO_MODE_SDL2ACCEL
};

#define VIDEO_SCALE_MODE_NONE       (0)

// texture formats
// This used to be an enum, but these are now defines so we can use them as
// preprocessor conditionals
#define SDL_TEXFORMAT_ARGB32            (0) // non-16-bit textures or specials
#define SDL_TEXFORMAT_RGB32             (1)
#define SDL_TEXFORMAT_RGB32_PALETTED    (2)
#define SDL_TEXFORMAT_YUY16             (3)
#define SDL_TEXFORMAT_YUY16_PALETTED    (4)
#define SDL_TEXFORMAT_PALETTE16         (5)
#define SDL_TEXFORMAT_RGB15             (6)
#define SDL_TEXFORMAT_RGB15_PALETTED    (7)
#define SDL_TEXFORMAT_PALETTE16A        (8)
// special texture formats for 16bpp texture destination support, do not use
// to address the tex properties / tex functions arrays!
#define SDL_TEXFORMAT_PALETTE16_ARGB1555    (16)
#define SDL_TEXFORMAT_RGB15_ARGB1555        (17)
#define SDL_TEXFORMAT_RGB15_PALETTED_ARGB1555   (18)

#define GLSL_SHADER_MAX 10

//============================================================
//  TYPE DEFINITIONS
//============================================================

struct sdl_mode
{
	int                 width;
	int                 height;
};

struct sdl_monitor_info
{
	sdl_monitor_info  * next;                   // pointer to next monitor in list
#ifdef PTR64
	UINT64              handle;                 // handle to the monitor
#else
	UINT32              handle;                 // handle to the monitor
#endif
	int                 monitor_width;
	int                 monitor_height;
	char                monitor_device[64];
	float               aspect;                 // computed/configured aspect ratio of the physical device
	int                 center_width;           // width of first physical screen for centering
	int                 center_height;          // height of first physical screen for centering
	int					monitor_x;				// X position of this monitor in virtual desktop space (SDL virtual space has them all horizontally stacked, not real geometry)
};


struct sdl_window_config
{
	float               aspect;                     // decoded aspect ratio
	int                 width;                      // decoded width
	int                 height;                     // decoded height
	int                 depth;                      // decoded depth
	int                 refresh;                    // decoded refresh
};


struct sdl_video_config
{
	// performance options
	int                 novideo;                // don't draw, for pure CPU benchmarking

	// global configuration
	int                 windowed;               // start windowed?
	int                 prescale;               // prescale factor (not currently supported)
	int                 keepaspect;             // keep aspect ratio?
	int                 numscreens;             // number of screens
	int                 centerh;
	int                 centerv;

	// hardware options
	int                 mode;           // output mode
	int                 waitvsync;      // spin until vsync
	int                 syncrefresh;    // sync only to refresh rate
	int                 switchres;      // switch resolutions

	int                 fullstretch;

	// vector options
	float               beamwidth;      // beam width

	// OpenGL options
	int                 filter;         // enable filtering, disabled if glsl_filter>0
	int                 glsl;
	int                 glsl_filter;        // glsl filtering, >0 disables filter
	char *              glsl_shader_mamebm[GLSL_SHADER_MAX]; // custom glsl shader set, mame bitmap
	int                 glsl_shader_mamebm_num; // custom glsl shader set number, mame bitmap
	char *              glsl_shader_scrn[GLSL_SHADER_MAX]; // custom glsl shader set, screen bitmap
	int                 glsl_shader_scrn_num; // custom glsl shader number, screen bitmap
	int                 pbo;
	int                 vbo;
	int                 allowtexturerect;   // allow GL_ARB_texture_rectangle, default: no
	int                 forcepow2texture;   // force power of two textures, default: no

	// perftest
	int                 perftest;       // print out real video fps

	// X11 options
	int                 restrictonemonitor; // in fullscreen, confine to Xinerama monitor 0

	// YUV options
	int                 scale_mode;
};

//============================================================
//  GLOBAL VARIABLES
//============================================================

extern sdl_video_config video_config;

//============================================================
//  PROTOTYPES
//============================================================

void sdlvideo_monitor_refresh(sdl_monitor_info *monitor);
float sdlvideo_monitor_get_aspect(sdl_monitor_info *monitor);
sdl_monitor_info *sdlvideo_monitor_from_handle(UINT32 monitor); //FIXME: Remove? not referenced

#endif
