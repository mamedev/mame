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
#define WINOPTION_OSLOG					"oslog"
#define WINOPTION_WATCHDOG				"watchdog"
#define WINOPTION_DEBUGGER_FONT			"debugger_font"
#define WINOPTION_DEBUGGER_FONT_SIZE	"debugger_font_size"

// performance options
#define WINOPTION_PRIORITY				"priority"
#define WINOPTION_MULTITHREADING		"multithreading"
#define WINOPTION_NUMPROCESSORS			"numprocessors"
#define WINOPTION_PROFILE				"profile"
#define WINOPTION_BENCH					"bench"

// video options
#define WINOPTION_VIDEO					"video"
#define WINOPTION_NUMSCREENS			"numscreens"
#define WINOPTION_WINDOW				"window"
#define WINOPTION_MAXIMIZE				"maximize"
#define WINOPTION_KEEPASPECT			"keepaspect"
#define WINOPTION_PRESCALE				"prescale"
#define WINOPTION_WAITVSYNC				"waitvsync"
#define WINOPTION_SYNCREFRESH			"syncrefresh"
#define WINOPTION_MENU					"menu"

// DirectDraw-specific options
#define WINOPTION_HWSTRETCH				"hwstretch"

// Direct3D-specific options
#define WINOPTION_D3DVERSION			"d3dversion"
#define WINOPTION_FILTER				"filter"

// core post-processing options
#define WINOPTION_HLSL_ENABLE				"hlsl_enable"
#define WINOPTION_HLSLPATH					"hlslpath"
#define WINOPTION_SHADOW_MASK_ALPHA			"shadow_mask_alpha"
#define WINOPTION_SHADOW_MASK_TEXTURE		"shadow_mask_texture"
#define WINOPTION_SHADOW_MASK_COUNT_X		"shadow_mask_x_count"
#define WINOPTION_SHADOW_MASK_COUNT_Y		"shadow_mask_y_count"
#define WINOPTION_SHADOW_MASK_USIZE			"shadow_mask_usize"
#define WINOPTION_SHADOW_MASK_VSIZE			"shadow_mask_vsize"
#define WINOPTION_PINCUSHION				"pincushion"
#define WINOPTION_SCREEN_SCALE_TOP			"screen_scale_top"
#define WINOPTION_SCREEN_SCALE_BOTTOM		"screen_scale_bottom"
#define WINOPTION_CURVATURE					"curvature"
#define WINOPTION_OVERSAMPLE_X				"oversample_x"
#define WINOPTION_OVERSAMPLE_Y				"oversample_y"
#define WINOPTION_SCANLINE_AMOUNT			"scanline_alpha"
#define WINOPTION_SCANLINE_SCALE			"scanline_size"
#define WINOPTION_SCANLINE_BRIGHT_SCALE 	"scanline_bright_scale"
#define WINOPTION_SCANLINE_BRIGHT_OFFSET 	"scanline_bright_offset"
#define WINOPTION_SCANLINE_OFFSET			"scanline_jitter"
#define WINOPTION_DEFOCUS_X					"defocus_x"
#define WINOPTION_DEFOCUS_Y					"defocus_y"
#define WINOPTION_RED_CONVERGE_X			"red_converge_x"
#define WINOPTION_RED_CONVERGE_Y			"red_converge_y"
#define WINOPTION_GREEN_CONVERGE_X			"green_converge_x"
#define WINOPTION_GREEN_CONVERGE_Y			"green_converge_y"
#define WINOPTION_BLUE_CONVERGE_X			"blue_converge_x"
#define WINOPTION_BLUE_CONVERGE_Y			"blue_converge_y"
#define WINOPTION_RED_RADIAL_CONVERGE_X		"red_radial_converge_x"
#define WINOPTION_RED_RADIAL_CONVERGE_Y		"red_radial_converge_y"
#define WINOPTION_GREEN_RADIAL_CONVERGE_X	"green_radial_converge_x"
#define WINOPTION_GREEN_RADIAL_CONVERGE_Y	"green_radial_converge_y"
#define WINOPTION_BLUE_RADIAL_CONVERGE_X	"blue_radial_converge_x"
#define WINOPTION_BLUE_RADIAL_CONVERGE_Y	"blue_radial_converge_y"
#define WINOPTION_RED_MATRIX_R				"red_from_r"
#define WINOPTION_RED_MATRIX_G				"red_from_g"
#define WINOPTION_RED_MATRIX_B				"red_from_b"
#define WINOPTION_GREEN_MATRIX_R			"green_from_r"
#define WINOPTION_GREEN_MATRIX_G			"green_from_g"
#define WINOPTION_GREEN_MATRIX_B			"green_from_b"
#define WINOPTION_BLUE_MATRIX_R				"blue_from_r"
#define WINOPTION_BLUE_MATRIX_G				"blue_from_g"
#define WINOPTION_BLUE_MATRIX_B				"blue_from_b"
#define WINOPTION_RED_OFFSET				"red_offset"
#define WINOPTION_GREEN_OFFSET				"green_offset"
#define WINOPTION_BLUE_OFFSET				"blue_offset"
#define WINOPTION_RED_SCALE					"red_scale"
#define WINOPTION_GREEN_SCALE				"green_scale"
#define WINOPTION_BLUE_SCALE				"blue_scale"
#define WINOPTION_RED_POWER					"red_power"
#define WINOPTION_GREEN_POWER				"green_power"
#define WINOPTION_BLUE_POWER				"blue_power"
#define WINOPTION_RED_FLOOR					"red_floor"
#define WINOPTION_GREEN_FLOOR				"green_floor"
#define WINOPTION_BLUE_FLOOR				"blue_floor"
#define WINOPTION_RED_PHOSPHOR				"red_phosphor_life"
#define WINOPTION_GREEN_PHOSPHOR			"green_phosphor_life"
#define WINOPTION_BLUE_PHOSPHOR				"blue_phosphor_life"
#define WINOPTION_SATURATION				"saturation"
#define WINOPTION_YIQ_ENABLE				"yiq_enable"
#define WINOPTION_YIQ_WVALUE				"yiq_w"
#define WINOPTION_YIQ_AVALUE				"yiq_a"
#define WINOPTION_YIQ_BVALUE				"yiq_b"
#define WINOPTION_YIQ_FSCVALUE				"yiq_fsc"
#define WINOPTION_YIQ_FSCSCALE				"yiq_fsc_scale"
#define WINOPTION_YIQ_PHASE_COUNT			"yiq_phase_count"

// per-window options
#define WINOPTION_SCREEN				"screen"
#define WINOPTION_ASPECT				"aspect"
#define WINOPTION_RESOLUTION			"resolution"
#define WINOPTION_VIEW					"view"

// full screen options
#define WINOPTION_TRIPLEBUFFER			"triplebuffer"
#define WINOPTION_SWITCHRES				"switchres"
#define WINOPTION_FULLSCREENBRIGHTNESS	"full_screen_brightness"
#define WINOPTION_FULLLSCREENCONTRAST	"full_screen_contrast"
#define WINOPTION_FULLSCREENGAMMA		"full_screen_gamma"

// sound options
#define WINOPTION_AUDIO_LATENCY			"audio_latency"

// input options
#define WINOPTION_DUAL_LIGHTGUN			"dual_lightgun"



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
	bool d3d_hlsl_enable() const { return bool_value(WINOPTION_HLSL_ENABLE); }
	float screen_shadow_mask_alpha() const { return float_value(WINOPTION_SHADOW_MASK_ALPHA); }
	const char *screen_shadow_mask_texture() const { return value(WINOPTION_SHADOW_MASK_TEXTURE); }
	float screen_shadow_mask_count_x() const { return float_value(WINOPTION_SHADOW_MASK_COUNT_X); }
	float screen_shadow_mask_count_y() const { return float_value(WINOPTION_SHADOW_MASK_COUNT_Y); }
	float screen_shadow_mask_u_size() const { return float_value(WINOPTION_SHADOW_MASK_USIZE); }
	float screen_shadow_mask_v_size() const { return float_value(WINOPTION_SHADOW_MASK_VSIZE); }
	float screen_oversample_x() const { return float_value(WINOPTION_OVERSAMPLE_X); }
	float screen_oversample_y() const { return float_value(WINOPTION_OVERSAMPLE_Y); }
	float screen_scanline_amount() const { return float_value(WINOPTION_SCANLINE_AMOUNT); }
	float screen_scanline_scale() const { return float_value(WINOPTION_SCANLINE_SCALE); }
	float screen_scanline_bright_scale() const { return float_value(WINOPTION_SCANLINE_BRIGHT_SCALE); }
	float screen_scanline_bright_offset() const { return float_value(WINOPTION_SCANLINE_BRIGHT_OFFSET); }
	float screen_scanline_offset() const { return float_value(WINOPTION_SCANLINE_OFFSET); }
	float screen_pincushion() const { return float_value(WINOPTION_PINCUSHION); }
	float screen_scale_top() const { return float_value(WINOPTION_SCREEN_SCALE_TOP); }
	float screen_scale_bottom() const { return float_value(WINOPTION_SCREEN_SCALE_BOTTOM); }
	float screen_curvature() const { return float_value(WINOPTION_CURVATURE); }
	float screen_defocus_x() const { return float_value(WINOPTION_DEFOCUS_X); }
	float screen_defocus_y() const { return float_value(WINOPTION_DEFOCUS_Y); }
	float screen_red_converge_x() const { return float_value(WINOPTION_RED_CONVERGE_X); }
	float screen_red_converge_y() const { return float_value(WINOPTION_RED_CONVERGE_Y); }
	float screen_green_converge_x() const { return float_value(WINOPTION_GREEN_CONVERGE_X); }
	float screen_green_converge_y() const { return float_value(WINOPTION_GREEN_CONVERGE_Y); }
	float screen_blue_converge_x() const { return float_value(WINOPTION_BLUE_CONVERGE_X); }
	float screen_blue_converge_y() const { return float_value(WINOPTION_BLUE_CONVERGE_Y); }
	float screen_red_radial_converge_x() const { return float_value(WINOPTION_RED_RADIAL_CONVERGE_X); }
	float screen_red_radial_converge_y() const { return float_value(WINOPTION_RED_RADIAL_CONVERGE_Y); }
	float screen_green_radial_converge_x() const { return float_value(WINOPTION_GREEN_RADIAL_CONVERGE_X); }
	float screen_green_radial_converge_y() const { return float_value(WINOPTION_GREEN_RADIAL_CONVERGE_Y); }
	float screen_blue_radial_converge_x() const { return float_value(WINOPTION_BLUE_RADIAL_CONVERGE_X); }
	float screen_blue_radial_converge_y() const { return float_value(WINOPTION_BLUE_RADIAL_CONVERGE_Y); }
	float screen_red_from_red() const { return float_value(WINOPTION_RED_MATRIX_R); }
	float screen_red_from_green() const { return float_value(WINOPTION_RED_MATRIX_G); }
	float screen_red_from_blue() const { return float_value(WINOPTION_RED_MATRIX_B); }
	float screen_green_from_red() const { return float_value(WINOPTION_GREEN_MATRIX_R); }
	float screen_green_from_green() const { return float_value(WINOPTION_GREEN_MATRIX_G); }
	float screen_green_from_blue() const { return float_value(WINOPTION_GREEN_MATRIX_B); }
	float screen_blue_from_red() const { return float_value(WINOPTION_BLUE_MATRIX_R); }
	float screen_blue_from_green() const { return float_value(WINOPTION_BLUE_MATRIX_G); }
	float screen_blue_from_blue() const { return float_value(WINOPTION_BLUE_MATRIX_B); }
	bool screen_yiq_enable() const { return bool_value(WINOPTION_YIQ_ENABLE); }
	float screen_yiq_w() const { return float_value(WINOPTION_YIQ_WVALUE); }
	float screen_yiq_a() const { return float_value(WINOPTION_YIQ_AVALUE); }
	float screen_yiq_b() const { return float_value(WINOPTION_YIQ_BVALUE); }
	float screen_yiq_fsc() const { return float_value(WINOPTION_YIQ_FSCVALUE); }
	float screen_yiq_fsc_scale() const { return float_value(WINOPTION_YIQ_FSCSCALE); }
	int screen_yiq_phase_count() const { return int_value(WINOPTION_YIQ_PHASE_COUNT); }
	float screen_red_offset() const { return float_value(WINOPTION_RED_OFFSET); }
	float screen_green_offset() const { return float_value(WINOPTION_GREEN_OFFSET); }
	float screen_blue_offset() const { return float_value(WINOPTION_BLUE_OFFSET); }
	float screen_red_scale() const { return float_value(WINOPTION_RED_SCALE); }
	float screen_green_scale() const { return float_value(WINOPTION_GREEN_SCALE); }
	float screen_blue_scale() const { return float_value(WINOPTION_BLUE_SCALE); }
	float screen_red_power() const { return float_value(WINOPTION_RED_POWER); }
	float screen_green_power() const { return float_value(WINOPTION_GREEN_POWER); }
	float screen_blue_power() const { return float_value(WINOPTION_BLUE_POWER); }
	float screen_red_floor() const { return float_value(WINOPTION_RED_FLOOR); }
	float screen_green_floor() const { return float_value(WINOPTION_GREEN_FLOOR); }
	float screen_blue_floor() const { return float_value(WINOPTION_BLUE_FLOOR); }
	float screen_saturation() const { return float_value(WINOPTION_SATURATION); }
	float screen_red_phosphor() const { return float_value(WINOPTION_RED_PHOSPHOR); }
	float screen_green_phosphor() const { return float_value(WINOPTION_GREEN_PHOSPHOR); }
	float screen_blue_phosphor() const { return float_value(WINOPTION_BLUE_PHOSPHOR); }

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
	float full_screen_contrast() const { return float_value(WINOPTION_FULLLSCREENCONTRAST); }
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
#define WRAP_REFIID(x)		x
#else
#define WRAP_REFIID(x)		&x
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

	// input overridables
	virtual void customize_input_type_list(input_type_desc *typelist);

	// font overridables
	virtual osd_font font_open(const char *name, int &height);
	virtual void font_close(osd_font font);
	virtual bitmap_t *font_get_bitmap(osd_font font, unicode_char chnum, INT32 &width, INT32 &xoffs, INT32 &yoffs);

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
