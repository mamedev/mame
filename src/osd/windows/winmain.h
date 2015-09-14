// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winmain.h - Win32 main program and core headers
//
//============================================================

#ifndef __WINDOWS_WINMAIN_H__
#define __WINDOWS_WINMAIN_H__

#include "clifront.h"
#include "osdepend.h"
#include "modules/lib/osdobj_common.h"


//============================================================
//  CONSTANTS
//============================================================

// performance options
#define WINOPTION_PRIORITY              "priority"
#define WINOPTION_PROFILE               "profile"

// video options
#define WINOPTION_MENU                  "menu"

// DirectDraw-specific options
#define WINOPTION_HWSTRETCH             "hwstretch"

// core post-processing options
#define WINOPTION_HLSL_ENABLE               "hlsl_enable"
#define WINOPTION_HLSLPATH                  "hlslpath"
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
#define WINOPTION_SHADOW_MASK_UOFFSET       "shadow_mask_uoffset"
#define WINOPTION_SHADOW_MASK_VOFFSET       "shadow_mask_voffset"
#define WINOPTION_REFLECTION                "reflection"
#define WINOPTION_CURVATURE                 "curvature"
#define WINOPTION_ROUND_CORNER              "round_corner"
#define WINOPTION_VIGNETTING                "vignetting"
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
#define WINOPTION_VECTOR_LENGTH_SCALE       "vector_length_scale"
#define WINOPTION_VECTOR_LENGTH_RATIO       "vector_length_ratio"
#define WINOPTION_VECTOR_BLOOM_SCALE        "vector_bloom_scale"
#define WINOPTION_VECTOR_TIME_PERIOD        "vector_time_period"
#define WINOPTION_RASTER_BLOOM_SCALE        "raster_bloom_scale"
#define WINOPTION_BLOOM_LEVEL0_WEIGHT       "bloom_lvl0_weight"
#define WINOPTION_BLOOM_LEVEL1_WEIGHT       "bloom_lvl1_weight"
#define WINOPTION_BLOOM_LEVEL2_WEIGHT       "bloom_lvl2_weight"
#define WINOPTION_BLOOM_LEVEL3_WEIGHT       "bloom_lvl3_weight"
#define WINOPTION_BLOOM_LEVEL4_WEIGHT       "bloom_lvl4_weight"
#define WINOPTION_BLOOM_LEVEL5_WEIGHT       "bloom_lvl5_weight"
#define WINOPTION_BLOOM_LEVEL6_WEIGHT       "bloom_lvl6_weight"
#define WINOPTION_BLOOM_LEVEL7_WEIGHT       "bloom_lvl7_weight"
#define WINOPTION_BLOOM_LEVEL8_WEIGHT       "bloom_lvl8_weight"
#define WINOPTION_BLOOM_LEVEL9_WEIGHT       "bloom_lvl9_weight"
#define WINOPTION_BLOOM_LEVEL10_WEIGHT      "bloom_lvl10_weight"

// full screen options
#define WINOPTION_TRIPLEBUFFER          "triplebuffer"
#define WINOPTION_FULLSCREENBRIGHTNESS  "full_screen_brightness"
#define WINOPTION_FULLSCREENCONTRAST    "full_screen_contrast"
#define WINOPTION_FULLSCREENGAMMA       "full_screen_gamma"

// input options
#define WINOPTION_GLOBAL_INPUTS			"global_inputs"
#define WINOPTION_DUAL_LIGHTGUN         "dual_lightgun"



//============================================================
//  TYPE DEFINITIONS
//============================================================

class windows_options : public osd_options
{
public:
	// construction/destruction
	windows_options();

	// performance options
	int priority() const { return int_value(WINOPTION_PRIORITY); }
	int profile() const { return int_value(WINOPTION_PROFILE); }

	// video options
	bool menu() const { return bool_value(WINOPTION_MENU); }

	// DirectDraw-specific options
	bool hwstretch() const { return bool_value(WINOPTION_HWSTRETCH); }

	// core post-processing options
	const char *screen_post_fx_dir() const { return value(WINOPTION_HLSLPATH); }
	bool d3d_hlsl_enable() const { return bool_value(WINOPTION_HLSL_ENABLE); }
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
	float screen_shadow_mask_u_offset() const { return float_value(WINOPTION_SHADOW_MASK_UOFFSET); }
	float screen_shadow_mask_v_offset() const { return float_value(WINOPTION_SHADOW_MASK_VOFFSET); }
	float screen_scanline_amount() const { return float_value(WINOPTION_SCANLINE_AMOUNT); }
	float screen_scanline_scale() const { return float_value(WINOPTION_SCANLINE_SCALE); }
	float screen_scanline_height() const { return float_value(WINOPTION_SCANLINE_HEIGHT); }
	float screen_scanline_bright_scale() const { return float_value(WINOPTION_SCANLINE_BRIGHT_SCALE); }
	float screen_scanline_bright_offset() const { return float_value(WINOPTION_SCANLINE_BRIGHT_OFFSET); }
	float screen_scanline_offset() const { return float_value(WINOPTION_SCANLINE_OFFSET); }
	float screen_reflection() const { return float_value(WINOPTION_REFLECTION); }
	float screen_curvature() const { return float_value(WINOPTION_CURVATURE); }
	float screen_round_corner() const { return float_value(WINOPTION_ROUND_CORNER); }
	float screen_vignetting() const { return float_value(WINOPTION_VIGNETTING); }
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
	float screen_vector_length_scale() const { return float_value(WINOPTION_VECTOR_LENGTH_SCALE); }
	float screen_vector_length_ratio() const { return float_value(WINOPTION_VECTOR_LENGTH_RATIO); }
	float screen_vector_bloom_scale() const { return float_value(WINOPTION_VECTOR_BLOOM_SCALE); }
	float screen_vector_time_period() const { return float_value(WINOPTION_VECTOR_TIME_PERIOD); }
	float screen_raster_bloom_scale() const { return float_value(WINOPTION_RASTER_BLOOM_SCALE); }
	float screen_bloom_lvl0_weight() const { return float_value(WINOPTION_BLOOM_LEVEL0_WEIGHT); }
	float screen_bloom_lvl1_weight() const { return float_value(WINOPTION_BLOOM_LEVEL1_WEIGHT); }
	float screen_bloom_lvl2_weight() const { return float_value(WINOPTION_BLOOM_LEVEL2_WEIGHT); }
	float screen_bloom_lvl3_weight() const { return float_value(WINOPTION_BLOOM_LEVEL3_WEIGHT); }
	float screen_bloom_lvl4_weight() const { return float_value(WINOPTION_BLOOM_LEVEL4_WEIGHT); }
	float screen_bloom_lvl5_weight() const { return float_value(WINOPTION_BLOOM_LEVEL5_WEIGHT); }
	float screen_bloom_lvl6_weight() const { return float_value(WINOPTION_BLOOM_LEVEL6_WEIGHT); }
	float screen_bloom_lvl7_weight() const { return float_value(WINOPTION_BLOOM_LEVEL7_WEIGHT); }
	float screen_bloom_lvl8_weight() const { return float_value(WINOPTION_BLOOM_LEVEL8_WEIGHT); }
	float screen_bloom_lvl9_weight() const { return float_value(WINOPTION_BLOOM_LEVEL9_WEIGHT); }
	float screen_bloom_lvl10_weight() const { return float_value(WINOPTION_BLOOM_LEVEL10_WEIGHT); }
	const char *screen_offset() const { return value(WINOPTION_OFFSET); }
	const char *screen_scale() const { return value(WINOPTION_SCALE); }
	const char *screen_power() const { return value(WINOPTION_POWER); }
	const char *screen_floor() const { return value(WINOPTION_FLOOR); }
	const char *screen_phosphor() const { return value(WINOPTION_PHOSPHOR); }
	float screen_saturation() const { return float_value(WINOPTION_SATURATION); }

	// full screen options
	bool triple_buffer() const { return bool_value(WINOPTION_TRIPLEBUFFER); }
	float full_screen_brightness() const { return float_value(WINOPTION_FULLSCREENBRIGHTNESS); }
	float full_screen_contrast() const { return float_value(WINOPTION_FULLSCREENCONTRAST); }
	float full_screen_gamma() const { return float_value(WINOPTION_FULLSCREENGAMMA); }

	// input options
	bool global_inputs() const { return bool_value(WINOPTION_GLOBAL_INPUTS); }
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

class windows_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	windows_osd_interface(windows_options &options);
	virtual ~windows_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// video overridables
	virtual void *get_slider_list();

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

	virtual void video_register();

	virtual bool video_init();
	virtual bool window_init();
	virtual bool input_init();
	virtual void input_pause();
	virtual void input_resume();
	virtual bool output_init();

	virtual void video_exit();
	virtual void window_exit();
	virtual void input_exit();
	virtual void output_exit();

	void extract_video_config();

	windows_options &options() { return m_options; }

private:
	void osd_exit();
	windows_options &m_options;

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

// use this to ping the watchdog
void winmain_watchdog_ping(void);
void winmain_dump_stack();

#endif
