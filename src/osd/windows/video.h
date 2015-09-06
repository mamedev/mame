// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  video.h - Win32 implementation of MAME video routines
//
//============================================================

#ifndef __WIN_VIDEO__
#define __WIN_VIDEO__

#include "render.h"
#include "winmain.h"

//============================================================
//  CONSTANTS
//============================================================

#define MAX_WINDOWS         4

enum {
	VIDEO_MODE_NONE,
	VIDEO_MODE_GDI,
	VIDEO_MODE_DDRAW,
#if defined (USE_BGFX)
	VIDEO_MODE_BGFX,
#endif
#if (USE_OPENGL)
	VIDEO_MODE_OPENGL,
#endif
	VIDEO_MODE_D3D
};

#define GLSL_SHADER_MAX 10

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_dim
{
public:
	osd_dim(const int &w, const int &h)
	: m_w(w), m_h(h)
	{
	}
	int width() const { return m_w; }
	int height() const { return m_h; }

	bool operator!=(const osd_dim &other) { return (m_w != other.width()) || (m_h != other.height()); }
	bool operator==(const osd_dim &other) { return (m_w == other.width()) && (m_h == other.height()); }
private:
	int m_w;
	int m_h;
};

class osd_rect
{
public:
	osd_rect()
	: m_x(0), m_y(0), m_d(0,0)
	{
	}
	osd_rect(const int x, const int y, const int &w, const int &h)
	: m_x(x), m_y(y), m_d(w,h)
	{
	}
	osd_rect(const int x, const int y, const osd_dim &d)
	: m_x(x), m_y(y), m_d(d)
	{
	}
	int top() const { return m_y; }
	int left() const { return m_x; }
	int width() const { return m_d.width(); }
	int height() const { return m_d.height(); }

	osd_dim dim() const { return m_d; }

	int bottom() const { return m_y + m_d.height(); }
	int right() const { return m_x + m_d.width(); }

	osd_rect move_by(int dx, int dy) const { return osd_rect(m_x + dx, m_y + dy, m_d); }
	osd_rect resize(int w, int h) const { return osd_rect(m_x, m_y, w, h); }

private:
	int m_x;
	int m_y;
	osd_dim m_d;
};

inline osd_rect RECT_to_osd_rect(const RECT &r)
{
	return osd_rect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}

class osd_monitor_info
{
public:

#if 0
	osd_monitor_info()
	: m_next(NULL), m_handle(NULL), m_aspect(0.0f)
		{
			strcpy(m_name, "");
		}
#endif
	osd_monitor_info(void *handle, const char *monitor_device, float aspect)
	: m_next(NULL), m_handle(handle), m_aspect(aspect)
	{
		strncpy(m_name, monitor_device, ARRAY_LENGTH(m_name) - 1);
	}

	virtual ~osd_monitor_info() { }

	virtual void refresh() = 0;

	const void *oshandle() { return m_handle; }

	const osd_rect &position_size() { return m_pos_size; }
	const osd_rect &usuable_position_size() { return m_usuable_pos_size; }

	const char *devicename() { return m_name[0] ? m_name : "UNKNOWN"; }

	float aspect();

	void set_aspect(const float a) { m_aspect = a; }
	bool is_primary() { return m_is_primary; }

	osd_monitor_info    * next() { return m_next; }   // pointer to next monitor in list

	static osd_monitor_info *pick_monitor(windows_options &options, int index);
	static osd_monitor_info *list;

	// FIXME: should be private!
	osd_monitor_info    *m_next;                   // pointer to next monitor in list
protected:
	osd_rect            m_pos_size;
	osd_rect            m_usuable_pos_size;
	bool                m_is_primary;
	char                m_name[64];
private:

	void *              m_handle;                 // handle to the monitor
	float               m_aspect;                 // computed/configured aspect ratio of the physical device
};

class win_monitor_info : public osd_monitor_info
{
public:
	win_monitor_info(const HMONITOR handle, const char *monitor_device, float aspect);
	virtual ~win_monitor_info();

	virtual void refresh();

	// static

	static BOOL CALLBACK monitor_enum_callback(HMONITOR handle, HDC dc, LPRECT rect, LPARAM data);
	static osd_monitor_info *monitor_from_handle(HMONITOR monitor);

	HMONITOR handle() { return m_handle; }

private:
	HMONITOR            m_handle;                 // handle to the monitor
	MONITORINFOEX       m_info;                   // most recently retrieved info
};

struct osd_video_config
{
	// global configuration
	int                 windowed;                   // start windowed?
	int                 prescale;                   // prescale factor
	int                 keepaspect;                 // keep aspect ratio
	int                 numscreens;                 // number of screens

	// hardware options
	int                 mode;                       // output mode
	int                 waitvsync;                  // spin until vsync
	int                 syncrefresh;                // sync only to refresh rate
	int                 switchres;                  // switch resolutions

	int                 fullstretch;    // FXIME: implement in windows!

	// ddraw options
	int                 hwstretch;                  // stretch using the hardware

	// d3d, accel, opengl
	int                 filter;                     // enable filtering
	//int                 filter;         // enable filtering, disabled if glsl_filter>0

	// OpenGL options
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

	// dd, d3d
	int                 triplebuf;                  // triple buffer

	//============================================================
	// SDL - options
	//============================================================
	int                 novideo;                // don't draw, for pure CPU benchmarking

	int                 centerh;
	int                 centerv;

	// vector options
	float               beamwidth;      // beam width

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

extern osd_video_config video_config;

#endif
