// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  winopts.cpp - Win32 options
//
//============================================================

#include "winopts.h"


namespace {

//**************************************************************************
//  OPTIONS
//**************************************************************************

options_entry const f_win_option_entries[] =
{
	// performance options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "WINDOWS PERFORMANCE OPTIONS" },
	{ WINOPTION_PRIORITY "(-15-1)",                   "0",        core_options::option_type::INTEGER,    "thread priority for the main game thread; range from -15 to 1" },
	{ WINOPTION_PROFILE,                              "0",        core_options::option_type::INTEGER,    "enables profiling, specifying the stack depth to track" },

	// video options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "WINDOWS VIDEO OPTIONS" },
	{ WINOPTION_ATTACH_WINDOW,                        "",         core_options::option_type::STRING,     "attach to arbitrary window" },

	// post-processing options
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "DIRECT3D POST-PROCESSING OPTIONS" },
	{ WINOPTION_HLSLPATH,                                       "hlsl",              core_options::option_type::PATH,       "path to HLSL support files" },
	{ WINOPTION_HLSL_ENABLE";hlsl",                             "0",                 core_options::option_type::BOOLEAN,    "enable HLSL post-processing (PS3.0 required)" },
	{ WINOPTION_HLSL_OVERSAMPLING,                              "0",                 core_options::option_type::BOOLEAN,    "enable HLSL oversampling" },
	{ WINOPTION_HLSL_WRITE,                                     OSDOPTVAL_AUTO,      core_options::option_type::PATH,       "enable HLSL AVI writing (huge disk bandwidth suggested)" },
	{ WINOPTION_HLSL_SNAP_WIDTH,                                "2048",              core_options::option_type::STRING,     "HLSL upscaled-snapshot width" },
	{ WINOPTION_HLSL_SNAP_HEIGHT,                               "1536",              core_options::option_type::STRING,     "HLSL upscaled-snapshot height" },
	{ WINOPTION_SHADOW_MASK_TILE_MODE,                          "0",                 core_options::option_type::INTEGER,    "shadow mask tile mode (0 for screen based, 1 for source based)" },
	{ WINOPTION_SHADOW_MASK_ALPHA ";fs_shadwa(0.0-1.0)",        "0.0",               core_options::option_type::FLOAT,      "shadow mask alpha-blend value (1.0 is fully blended, 0.0 is no mask)" },
	{ WINOPTION_SHADOW_MASK_TEXTURE ";fs_shadwt(0.0-1.0)",      "shadow-mask.png",   core_options::option_type::STRING,     "shadow mask texture name" },
	{ WINOPTION_SHADOW_MASK_COUNT_X ";fs_shadww",               "6",                 core_options::option_type::INTEGER,    "shadow mask tile width, in screen dimensions" },
	{ WINOPTION_SHADOW_MASK_COUNT_Y ";fs_shadwh",               "4",                 core_options::option_type::INTEGER,    "shadow mask tile height, in screen dimensions" },
	{ WINOPTION_SHADOW_MASK_USIZE ";fs_shadwu(0.0-1.0)",        "0.1875",            core_options::option_type::FLOAT,      "shadow mask texture width, in U/V dimensions" },
	{ WINOPTION_SHADOW_MASK_VSIZE ";fs_shadwv(0.0-1.0)",        "0.25",              core_options::option_type::FLOAT,      "shadow mask texture height, in U/V dimensions" },
	{ WINOPTION_SHADOW_MASK_UOFFSET ";fs_shadwou(-1.0-1.0)",    "0.0",               core_options::option_type::FLOAT,      "shadow mask texture offset, in U direction" },
	{ WINOPTION_SHADOW_MASK_VOFFSET ";fs_shadwov(-1.0-1.0)",    "0.0",               core_options::option_type::FLOAT,      "shadow mask texture offset, in V direction" },
	{ WINOPTION_DISTORTION ";fs_dist(-1.0-1.0)",                "0.0",               core_options::option_type::FLOAT,      "screen distortion amount" },
	{ WINOPTION_CUBIC_DISTORTION ";fs_cubedist(-1.0-1.0)",      "0.0",               core_options::option_type::FLOAT,      "screen cubic distortion amount" },
	{ WINOPTION_DISTORT_CORNER ";fs_distc(0.0-1.0)",            "0.0",               core_options::option_type::FLOAT,      "screen distort corner amount" },
	{ WINOPTION_ROUND_CORNER ";fs_rndc(0.0-1.0)",               "0.0",               core_options::option_type::FLOAT,      "screen round corner amount" },
	{ WINOPTION_SMOOTH_BORDER ";fs_smob(0.0-1.0)",              "0.0",               core_options::option_type::FLOAT,      "screen smooth border amount" },
	{ WINOPTION_REFLECTION ";fs_ref(0.0-1.0)",                  "0.0",               core_options::option_type::FLOAT,      "screen reflection amount" },
	{ WINOPTION_VIGNETTING ";fs_vig(0.0-1.0)",                  "0.0",               core_options::option_type::FLOAT,      "image vignetting amount" },
	// Beam-related values below this line
	{ WINOPTION_SCANLINE_AMOUNT ";fs_scanam(0.0-4.0)",          "0.0",               core_options::option_type::FLOAT,      "overall alpha scaling value for scanlines" },
	{ WINOPTION_SCANLINE_SCALE ";fs_scansc(0.0-4.0)",           "1.0",               core_options::option_type::FLOAT,      "overall height scaling value for scanlines" },
	{ WINOPTION_SCANLINE_HEIGHT ";fs_scanh(0.0-4.0)",           "1.0",               core_options::option_type::FLOAT,      "individual height scaling value for scanlines" },
	{ WINOPTION_SCANLINE_VARIATION ";fs_scanv(0.0-4.0)",        "1.0",               core_options::option_type::FLOAT,      "individual height varying value for scanlines" },
	{ WINOPTION_SCANLINE_BRIGHT_SCALE ";fs_scanbs(0.0-2.0)",    "1.0",               core_options::option_type::FLOAT,      "overall brightness scaling value for scanlines (multiplicative)" },
	{ WINOPTION_SCANLINE_BRIGHT_OFFSET ";fs_scanbo(0.0-1.0)",   "0.0",               core_options::option_type::FLOAT,      "overall brightness offset value for scanlines (additive)" },
	{ WINOPTION_SCANLINE_JITTER ";fs_scanjt(0.0-4.0)",          "0.0",               core_options::option_type::FLOAT,      "overall interlace jitter scaling value for scanlines" },
	{ WINOPTION_HUM_BAR_ALPHA ";fs_humba(0.0-1.0)",             "0.0",               core_options::option_type::FLOAT,      "overall alpha scaling value for hum bar" },
	{ WINOPTION_DEFOCUS ";fs_focus",                            "0.0,0.0",           core_options::option_type::STRING,     "overall defocus value in screen-relative coords" },
	{ WINOPTION_CONVERGE_X ";fs_convx",                         "0.0,0.0,0.0",       core_options::option_type::STRING,     "convergence in screen-relative X direction" },
	{ WINOPTION_CONVERGE_Y ";fs_convy",                         "0.0,0.0,0.0",       core_options::option_type::STRING,     "convergence in screen-relative Y direction" },
	{ WINOPTION_RADIAL_CONVERGE_X ";fs_rconvx",                 "0.0,0.0,0.0",       core_options::option_type::STRING,     "radial convergence in screen-relative X direction" },
	{ WINOPTION_RADIAL_CONVERGE_Y ";fs_rconvy",                 "0.0,0.0,0.0",       core_options::option_type::STRING,     "radial convergence in screen-relative Y direction" },
	// RGB colorspace convolution below this line
	{ WINOPTION_RED_RATIO ";fs_redratio",                       "1.0,0.0,0.0",       core_options::option_type::STRING,     "red output signal generated by input signal" },
	{ WINOPTION_GRN_RATIO ";fs_grnratio",                       "0.0,1.0,0.0",       core_options::option_type::STRING,     "green output signal generated by input signal" },
	{ WINOPTION_BLU_RATIO ";fs_bluratio",                       "0.0,0.0,1.0",       core_options::option_type::STRING,     "blue output signal generated by input signal" },
	{ WINOPTION_SATURATION ";fs_sat(0.0-4.0)",                  "1.0",               core_options::option_type::FLOAT,      "saturation scaling value" },
	{ WINOPTION_OFFSET ";fs_offset",                            "0.0,0.0,0.0",       core_options::option_type::STRING,     "signal offset value (additive)" },
	{ WINOPTION_SCALE ";fs_scale",                              "1.0,1.0,1.0",       core_options::option_type::STRING,     "signal scaling value (multiplicative)" },
	{ WINOPTION_POWER ";fs_power",                              "1.0,1.0,1.0",       core_options::option_type::STRING,     "signal power value (exponential)" },
	{ WINOPTION_FLOOR ";fs_floor",                              "0.0,0.0,0.0",       core_options::option_type::STRING,     "signal floor level" },
	{ WINOPTION_PHOSPHOR";fs_phosphor",                         "0.0,0.0,0.0",       core_options::option_type::STRING,     "phosphorescence decay rate (0.0 is instant, 1.0 is forever)" },
	{ WINOPTION_CHROMA_MODE,                                    "3",                 core_options::option_type::INTEGER,    "number of phosphors to use: 1 - monochrome, 2 - dichrome, 3 - trichrome (color)" },
	{ WINOPTION_CHROMA_CONVERSION_GAIN,                         "0.299,0.587,0.114", core_options::option_type::STRING,     "gain to be applied when summing RGB signal for monochrome and dichrome modes" },
	{ WINOPTION_CHROMA_A,                                       "0.64,0.33",         core_options::option_type::STRING,     "chromaticity coordinate for first phosphor" },
	{ WINOPTION_CHROMA_B,                                       "0.30,0.60",         core_options::option_type::STRING,     "chromaticity coordinate for second phosphor" },
	{ WINOPTION_CHROMA_C,                                       "0.15,0.06",         core_options::option_type::STRING,     "chromaticity coordinate for third phosphor" },
	{ WINOPTION_CHROMA_Y_GAIN,                                  "0.2126,0.7152,0.0722", core_options::option_type::STRING,  "gain to be applied for each phosphor" },
	// NTSC simulation below this line
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "NTSC POST-PROCESSING OPTIONS" },
	{ WINOPTION_YIQ_ENABLE ";yiq",                              "0",                 core_options::option_type::BOOLEAN,    "enable YIQ-space HLSL post-processing" },
	{ WINOPTION_YIQ_JITTER ";yiqj",                             "0.0",               core_options::option_type::FLOAT,      "jitter for the NTSC signal processing" },
	{ WINOPTION_YIQ_CCVALUE ";yiqcc",                           "3.57954545",        core_options::option_type::FLOAT,      "color carrier frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_AVALUE ";yiqa",                             "0.5",               core_options::option_type::FLOAT,      "A value for NTSC signal processing" },
	{ WINOPTION_YIQ_BVALUE ";yiqb",                             "0.5",               core_options::option_type::FLOAT,      "B value for NTSC signal processing" },
	{ WINOPTION_YIQ_OVALUE ";yiqo",                             "0.0",               core_options::option_type::FLOAT,      "outgoing Color Carrier phase offset for NTSC signal processing" },
	{ WINOPTION_YIQ_PVALUE ";yiqp",                             "1.0",               core_options::option_type::FLOAT,      "incoming Pixel Clock scaling value for NTSC signal processing" },
	{ WINOPTION_YIQ_NVALUE ";yiqn",                             "1.0",               core_options::option_type::FLOAT,      "Y filter notch width for NTSC signal processing" },
	{ WINOPTION_YIQ_YVALUE ";yiqy",                             "6.0",               core_options::option_type::FLOAT,      "Y filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_IVALUE ";yiqi",                             "1.2",               core_options::option_type::FLOAT,      "I filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_QVALUE ";yiqq",                             "0.6",               core_options::option_type::FLOAT,      "Q filter cutoff frequency for NTSC signal processing" },
	{ WINOPTION_YIQ_SCAN_TIME ";yiqsc",                         "52.6",              core_options::option_type::FLOAT,      "horizontal scanline duration for NTSC signal processing (microseconds)" },
	{ WINOPTION_YIQ_PHASE_COUNT ";yiqpc",                       "2",                 core_options::option_type::INTEGER,    "phase count value for NTSC signal processing" },
	// Vector simulation below this line
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "VECTOR POST-PROCESSING OPTIONS" },
	{ WINOPTION_VECTOR_BEAM_SMOOTH ";vecsmooth",                "0.0",               core_options::option_type::FLOAT,      "vector beam smoothness" },
	{ WINOPTION_VECTOR_LENGTH_SCALE ";vecscale",                "0.5",               core_options::option_type::FLOAT,      "maximum vector attenuation" },
	{ WINOPTION_VECTOR_LENGTH_RATIO ";vecratio",                "0.5",               core_options::option_type::FLOAT,      "minimum vector length affected by attenuation (vector length to screen size ratio)" },
	// Bloom below this line
	{ nullptr,                                                  nullptr,             core_options::option_type::HEADER,     "BLOOM POST-PROCESSING OPTIONS" },
	{ WINOPTION_BLOOM_BLEND_MODE,                               "0",                 core_options::option_type::INTEGER,    "bloom blend mode (0 for brighten, 1 for darken)" },
	{ WINOPTION_BLOOM_SCALE,                                    "0.0",               core_options::option_type::FLOAT,      "intensity factor for bloom" },
	{ WINOPTION_BLOOM_OVERDRIVE,                                "1.0,1.0,1.0",       core_options::option_type::STRING,     "overdrive factor for bloom" },
	{ WINOPTION_BLOOM_LEVEL0_WEIGHT,                            "1.0",               core_options::option_type::FLOAT,      "bloom level 0 weight (full-size target)" },
	{ WINOPTION_BLOOM_LEVEL1_WEIGHT,                            "0.64",              core_options::option_type::FLOAT,      "bloom level 1 weight (1/4 smaller that level 0 target)" },
	{ WINOPTION_BLOOM_LEVEL2_WEIGHT,                            "0.32",              core_options::option_type::FLOAT,      "bloom level 2 weight (1/4 smaller that level 1 target)" },
	{ WINOPTION_BLOOM_LEVEL3_WEIGHT,                            "0.16",              core_options::option_type::FLOAT,      "bloom level 3 weight (1/4 smaller that level 2 target)" },
	{ WINOPTION_BLOOM_LEVEL4_WEIGHT,                            "0.08",              core_options::option_type::FLOAT,      "bloom level 4 weight (1/4 smaller that level 3 target)" },
	{ WINOPTION_BLOOM_LEVEL5_WEIGHT,                            "0.06",              core_options::option_type::FLOAT,      "bloom level 5 weight (1/4 smaller that level 4 target)" },
	{ WINOPTION_BLOOM_LEVEL6_WEIGHT,                            "0.04",              core_options::option_type::FLOAT,      "bloom level 6 weight (1/4 smaller that level 5 target)" },
	{ WINOPTION_BLOOM_LEVEL7_WEIGHT,                            "0.02",              core_options::option_type::FLOAT,      "bloom level 7 weight (1/4 smaller that level 6 target)" },
	{ WINOPTION_BLOOM_LEVEL8_WEIGHT,                            "0.01",              core_options::option_type::FLOAT,      "bloom level 8 weight (1/4 smaller that level 7 target)" },
	{ WINOPTION_LUT_TEXTURE,                                    "lut-default.png",   core_options::option_type::PATH,       "3D LUT texture filename for screen, PNG format" },
	{ WINOPTION_LUT_ENABLE,                                     "0",                 core_options::option_type::BOOLEAN,    "Enables 3D LUT to be applied to screen after post-processing" },
	{ WINOPTION_UI_LUT_TEXTURE,                                 "lut-default.png",   core_options::option_type::PATH,       "3D LUT texture filename of UI, PNG format" },
	{ WINOPTION_UI_LUT_ENABLE,                                  "0",                 core_options::option_type::BOOLEAN,    "enable 3D LUT to be applied to UI and artwork after post-processing" },

	// full screen options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "FULL SCREEN OPTIONS" },
	{ WINOPTION_TRIPLEBUFFER ";tb",                   "0",        core_options::option_type::BOOLEAN,    "enable triple buffering" },
	{ WINOPTION_FULLSCREENBRIGHTNESS ";fsb(0.1-2.0)", "1.0",      core_options::option_type::FLOAT,      "brightness value in full screen mode" },
	{ WINOPTION_FULLSCREENCONTRAST ";fsc(0.1-2.0)",   "1.0",      core_options::option_type::FLOAT,      "contrast value in full screen mode" },
	{ WINOPTION_FULLSCREENGAMMA ";fsg(0.1-3.0)",      "1.0",      core_options::option_type::FLOAT,      "gamma value in full screen mode" },

	// input options
	{ nullptr,                                        nullptr,    core_options::option_type::HEADER,     "INPUT DEVICE OPTIONS" },
	{ WINOPTION_DUAL_LIGHTGUN ";dual",                "0",        core_options::option_type::BOOLEAN,    "enable dual lightgun input" },

	{ nullptr }
};

} // anonymous namespace


//============================================================
//  windows_options
//============================================================

windows_options::windows_options() : osd_options()
{
	add_entries(f_win_option_entries);
}


//============================================================
//  osd_setup_osd_specific_emu_options
//============================================================

void osd_setup_osd_specific_emu_options(emu_options &opts)
{
	opts.add_entries(osd_options::s_option_entries);
	opts.add_entries(f_win_option_entries);
}
