//============================================================
//
//  winmain.h - Win32 main program and core headers
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

#include "clifront.h"
#include "osdepend.h"


//============================================================
//  CONSTANTS
//============================================================

// debugging options
#define WINOPTION_OSLOG                 "oslog"
#define WINOPTION_WATCHDOG              "watchdog"
#define WINOPTION_DEBUGGER_FONT         "debugger_font"
#define WINOPTION_DEBUGGER_FONT_SIZE    "debugger_font_size"

// performance options
#define WINOPTION_PRIORITY              "priority"
#define WINOPTION_MULTITHREADING        "multithreading"
#define WINOPTION_NUMPROCESSORS         "numprocessors"
#define WINOPTION_PROFILE               "profile"
#define WINOPTION_BENCH                 "bench"

// video options
#define WINOPTION_VIDEO                 "video"
#define WINOPTION_NUMSCREENS            "numscreens"
#define WINOPTION_WINDOW                "window"
#define WINOPTION_MAXIMIZE              "maximize"
#define WINOPTION_KEEPASPECT            "keepaspect"
#define WINOPTION_PRESCALE              "prescale"
#define WINOPTION_WAITVSYNC             "waitvsync"
#define WINOPTION_SYNCREFRESH           "syncrefresh"
#define WINOPTION_MENU                  "menu"

// DirectDraw-specific options
#define WINOPTION_HWSTRETCH             "hwstretch"

// Direct3D-specific options
#define WINOPTION_D3DVERSION            "d3dversion"
#define WINOPTION_FILTER                "filter"

// core post-processing options
#define WINOPTION_HLSL_ENABLE               "hlsl_enable"
#define WINOPTION_HLSLPATH                  "hlslpath"
#define WINOPTION_HLSL_INI_NAME             "hlslini"
#define WINOPTION_HLSL_INI_WRITE            "hlsl_ini_write"
#define WINOPTION_HLSL_INI_READ             "hlsl_ini_read"
#define WINOPTION_HLSL_PRESCALE_X           "hlsl_prescale_x"
#define WINOPTION_HLSL_PRESCALE_Y           "hlsl_prescale_y"
#define WINOPTION_HLSL_PRESET               "hlsl_preset"
#define WINOPTION_HLSL_WRITE                "hlsl_write"
#define WINOPTION_HLSL_SNAP_WIDTH           "hlsl_snap_width"
#define WINOPTION_HLSL_SNAP_HEIGHT          "hlsl_snap_height"
#define WINOPTION_SHADOW_MASK_ALPHA         "shadow_mask_alpha"
#define WINOPTION_SHADOW_MASK_TEXTURE       "shadow_mask_texture"
#define WINOPTION_SHADOW_MASK_COUNT_X       "shadow_mask_x_count"
#define WINOPTION_SHADOW_MASK_COUNT_Y       "shadow_mask_y_count"
#define WINOPTION_SHADOW_MASK_USIZE         "shadow_mask_usize"
#define WINOPTION_SHADOW_MASK_VSIZE         "shadow_mask_vsize"
#define WINOPTION_PINCUSHION                "pincushion"
#define WINOPTION_CURVATURE                 "curvature"
#define WINOPTION_SCANLINE_AMOUNT           "scanline_alpha"
#define WINOPTION_SCANLINE_SCALE            "scanline_size"
#define WINOPTION_SCANLINE_HEIGHT           "scanline_height"
#define WINOPTION_SCANLINE_BRIGHT_SCALE     "scanline_bright_scale"
#define WINOPTION_SCANLINE_BRIGHT_OFFSET    "scanline_bright_offset"
#define WINOPTION_SCANLINE_OFFSET           "scanline_jitter"
#define WINOPTION_DEFOCUS                   "defocus"
#define WINOPTION_CONVERGE_X                "converge_x"
#define WINOPTION_CONVERGE_Y                "converge_y"
#define WINOPTION_RADIAL_CONVERGE_X         "radial_converge_x"
#define WINOPTION_RADIAL_CONVERGE_Y         "radial_converge_y"
#define WINOPTION_RED_RATIO                 "red_ratio"
#define WINOPTION_GRN_RATIO                 "grn_ratio"
#define WINOPTION_BLU_RATIO                 "blu_ratio"
#define WINOPTION_OFFSET                    "offset"
#define WINOPTION_SCALE                     "scale"
#define WINOPTION_POWER                     "power"
#define WINOPTION_FLOOR                     "floor"
#define WINOPTION_PHOSPHOR                  "phosphor_life"
#define WINOPTION_SATURATION                "saturation"
#define WINOPTION_YIQ_ENABLE                "yiq_enable"
#define WINOPTION_YIQ_CCVALUE               "yiq_cc"
#define WINOPTION_YIQ_AVALUE                "yiq_a"
#define WINOPTION_YIQ_BVALUE                "yiq_b"
#define WINOPTION_YIQ_OVALUE                "yiq_o"
#define WINOPTION_YIQ_PVALUE                "yiq_p"
#define WINOPTION_YIQ_NVALUE                "yiq_n"
#define WINOPTION_YIQ_YVALUE                "yiq_y"
#define WINOPTION_YIQ_IVALUE                "yiq_i"
#define WINOPTION_YIQ_QVALUE                "yiq_q"
#define WINOPTION_YIQ_SCAN_TIME             "yiq_scan_time"
#define WINOPTION_YIQ_PHASE_COUNT           "yiq_phase_count"

// per-window options
#define WINOPTION_SCREEN                "screen"
#define WINOPTION_ASPECT                "aspect"
#define WINOPTION_RESOLUTION            "resolution"
#define WINOPTION_VIEW                  "view"

// full screen options
#define WINOPTION_TRIPLEBUFFER          "triplebuffer"
#define WINOPTION_SWITCHRES             "switchres"
#define WINOPTION_FULLSCREENBRIGHTNESS  "full_screen_brightness"
#define WINOPTION_FULLSCREENCONTRAST    "full_screen_contrast"
#define WINOPTION_FULLSCREENGAMMA       "full_screen_gamma"

// sound options
#define WINOPTION_AUDIO_LATENCY         "audio_latency"

// input options
#define WINOPTION_DUAL_LIGHTGUN         "dual_lightgun"



//============================================================
//  TYPE DEFINITIONS
//============================================================

class windows_options : public cli_options
{
public:
	// construction/destruction
	windows_options();

	// debugging options
	bool oslog() const { return bool_value(WINOPTION_OSLOG); }
	int watchdog() const { return int_value(WINOPTION_WATCHDOG); }
	const char *debugger_font() const { return value(WINOPTION_DEBUGGER_FONT); }
	float debugger_font_size() const { return float_value(WINOPTION_DEBUGGER_FONT_SIZE); }

	// performance options
	int priority() const { return int_value(WINOPTION_PRIORITY); }
	bool multithreading() const { return bool_value(WINOPTION_MULTITHREADING); }
	const char *numprocessors() const { return value(WINOPTION_NUMPROCESSORS); }
	int profile() const { return int_value(WINOPTION_PROFILE); }
	int bench() const { return int_value(WINOPTION_BENCH); }

	// video options
	const char *video() const { return value(WINOPTION_VIDEO); }
	int numscreens() const { return int_value(WINOPTION_NUMSCREENS); }
	bool window() const { return bool_value(WINOPTION_WINDOW); }
	bool maximize() const { return bool_value(WINOPTION_MAXIMIZE); }
	bool keep_aspect() const { return bool_value(WINOPTION_KEEPASPECT); }
	int prescale() const { return int_value(WINOPTION_PRESCALE); }
	bool wait_vsync() const { return bool_value(WINOPTION_WAITVSYNC); }
	bool sync_refresh() const { return bool_value(WINOPTION_SYNCREFRESH); }
	bool menu() const { return bool_value(WINOPTION_MENU); }

	// DirectDraw-specific options
	bool hwstretch() const { return bool_value(WINOPTION_HWSTRETCH); }

	// Direct3D-specific options
	int d3d_version() const { return int_value(WINOPTION_D3DVERSION); }
	bool filter() const { return bool_value(WINOPTION_FILTER); }

	// core post-processing options
	const char *screen_post_fx_dir() const { return value(WINOPTION_HLSLPATH); }
	const char *hlsl_ini_name() const { return value(WINOPTION_HLSL_INI_NAME); }
	bool d3d_hlsl_enable() const { return bool_value(WINOPTION_HLSL_ENABLE); }
	bool hlsl_write_ini() const { return bool_value(WINOPTION_HLSL_INI_WRITE); }
	bool hlsl_read_ini() const { return bool_value(WINOPTION_HLSL_INI_READ); }
	const char *d3d_hlsl_write() const { return value(WINOPTION_HLSL_WRITE); }
	int d3d_hlsl_prescale_x() const { return int_value(WINOPTION_HLSL_PRESCALE_X); }
	int d3d_hlsl_prescale_y() const { return int_value(WINOPTION_HLSL_PRESCALE_Y); }
	int d3d_hlsl_preset() const { return int_value(WINOPTION_HLSL_PRESET); }
	int d3d_snap_width() const { return int_value(WINOPTION_HLSL_SNAP_WIDTH); }
	int d3d_snap_height() const { return int_value(WINOPTION_HLSL_SNAP_HEIGHT); }
	float screen_shadow_mask_alpha() const { return float_value(WINOPTION_SHADOW_MASK_ALPHA); }
	const char *screen_shadow_mask_texture() const { return value(WINOPTION_SHADOW_MASK_TEXTURE); }
	int screen_shadow_mask_count_x() const { return int_value(WINOPTION_SHADOW_MASK_COUNT_X); }
	int screen_shadow_mask_count_y() const { return int_value(WINOPTION_SHADOW_MASK_COUNT_Y); }
	float screen_shadow_mask_u_size() const { return float_value(WINOPTION_SHADOW_MASK_USIZE); }
	float screen_shadow_mask_v_size() const { return float_value(WINOPTION_SHADOW_MASK_VSIZE); }
	float screen_scanline_amount() const { return float_value(WINOPTION_SCANLINE_AMOUNT); }
	float screen_scanline_scale() const { return float_value(WINOPTION_SCANLINE_SCALE); }
	float screen_scanline_height() const { return float_value(WINOPTION_SCANLINE_HEIGHT); }
	float screen_scanline_bright_scale() const { return float_value(WINOPTION_SCANLINE_BRIGHT_SCALE); }
	float screen_scanline_bright_offset() const { return float_value(WINOPTION_SCANLINE_BRIGHT_OFFSET); }
	float screen_scanline_offset() const { return float_value(WINOPTION_SCANLINE_OFFSET); }
	float screen_pincushion() const { return float_value(WINOPTION_PINCUSHION); }
	float screen_curvature() const { return float_value(WINOPTION_CURVATURE); }
	const char *screen_defocus() const { return value(WINOPTION_DEFOCUS); }
	const char *screen_converge_x() const { return value(WINOPTION_CONVERGE_X); }
	const char *screen_converge_y() const { return value(WINOPTION_CONVERGE_Y); }
	const char *screen_radial_converge_x() const { return value(WINOPTION_RADIAL_CONVERGE_X); }
	const char *screen_radial_converge_y() const { return value(WINOPTION_RADIAL_CONVERGE_Y); }
	const char *screen_red_ratio() const { return value(WINOPTION_RED_RATIO); }
	const char *screen_grn_ratio() const { return value(WINOPTION_GRN_RATIO); }
	const char *screen_blu_ratio() const { return value(WINOPTION_BLU_RATIO); }
	bool screen_yiq_enable() const { return bool_value(WINOPTION_YIQ_ENABLE); }
	float screen_yiq_cc() const { return float_value(WINOPTION_YIQ_CCVALUE); }
	float screen_yiq_a() const { return float_value(WINOPTION_YIQ_AVALUE); }
	float screen_yiq_b() const { return float_value(WINOPTION_YIQ_BVALUE); }
	float screen_yiq_o() const { return float_value(WINOPTION_YIQ_OVALUE); }
	float screen_yiq_p() const { return float_value(WINOPTION_YIQ_PVALUE); }
	float screen_yiq_n() const { return float_value(WINOPTION_YIQ_NVALUE); }
	float screen_yiq_y() const { return float_value(WINOPTION_YIQ_YVALUE); }
	float screen_yiq_i() const { return float_value(WINOPTION_YIQ_IVALUE); }
	float screen_yiq_q() const { return float_value(WINOPTION_YIQ_QVALUE); }
	float screen_yiq_scan_time() const { return float_value(WINOPTION_YIQ_SCAN_TIME); }
	int screen_yiq_phase_count() const { return int_value(WINOPTION_YIQ_PHASE_COUNT); }
	const char *screen_offset() const { return value(WINOPTION_OFFSET); }
	const char *screen_scale() const { return value(WINOPTION_SCALE); }
	const char *screen_power() const { return value(WINOPTION_POWER); }
	const char *screen_floor() const { return value(WINOPTION_FLOOR); }
	const char *screen_phosphor() const { return value(WINOPTION_PHOSPHOR); }
	float screen_saturation() const { return float_value(WINOPTION_SATURATION); }

	// per-window options
	const char *screen() const { return value(WINOPTION_SCREEN); }
	const char *aspect() const { return value(WINOPTION_ASPECT); }
	const char *resolution() const { return value(WINOPTION_RESOLUTION); }
	const char *view() const { return value(WINOPTION_VIEW); }
	const char *screen(int index) const { astring temp; return value(temp.format("%s%d", WINOPTION_SCREEN, index)); }
	const char *aspect(int index) const { astring temp; return value(temp.format("%s%d", WINOPTION_ASPECT, index)); }
	const char *resolution(int index) const { astring temp; return value(temp.format("%s%d", WINOPTION_RESOLUTION, index)); }
	const char *view(int index) const { astring temp; return value(temp.format("%s%d", WINOPTION_VIEW, index)); }

	// full screen options
	bool triple_buffer() const { return bool_value(WINOPTION_TRIPLEBUFFER); }
	bool switch_res() const { return bool_value(WINOPTION_SWITCHRES); }
	float full_screen_brightness() const { return float_value(WINOPTION_FULLSCREENBRIGHTNESS); }
	float full_screen_contrast() const { return float_value(WINOPTION_FULLSCREENCONTRAST); }
	float full_screen_gamma() const { return float_value(WINOPTION_FULLSCREENGAMMA); }

	// sound options
	int audio_latency() const { return int_value(WINOPTION_AUDIO_LATENCY); }

	// input options
	bool dual_lightgun() const { return bool_value(WINOPTION_DUAL_LIGHTGUN); }

private:
	static const options_entry s_option_entries[];
};



//============================================================
//  MACROS
//============================================================

#ifdef __cplusplus
#define WRAP_REFIID(x)      x
#else
#define WRAP_REFIID(x)      &x
#endif



//============================================================
//  TYPE DEFINITIONS
//============================================================

class windows_osd_interface : public osd_interface
{
public:
	// construction/destruction
	windows_osd_interface();
	virtual ~windows_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// debugger overridables
//  virtual void init_debugger();
	virtual void wait_for_debugger(device_t &device, bool firststop);

	// audio overridables
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

	// video overridables
	virtual void *get_slider_list();

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bool font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs);

private:
	static void osd_exit(running_machine &machine);

	static const int DEFAULT_FONT_HEIGHT = 200;
};



//============================================================
//  GLOBAL VARIABLES
//============================================================

extern const options_entry mame_win_options[];

// defined in winwork.c
extern int osd_num_processors;



//============================================================
//  FUNCTION PROTOTYPES
//============================================================

// use if you want to print something with the verbose flag
void CLIB_DECL mame_printf_verbose(const char *text, ...) ATTR_PRINTF(1,2);

// use this to ping the watchdog
void winmain_watchdog_ping(void);
void winmain_dump_stack();
