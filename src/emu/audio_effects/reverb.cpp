// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "reverb.h"
#include "xmlfile.h"

#include <cmath>

#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

// This is a reimplementation of the "RoomReverb" by ElephantDSP.com/Christian Voigt
// which is itself built from early reflection and Progenitor 2 from Freeverb3.
//

const audio_effect_reverb::preset audio_effect_reverb::presets[] = {
	{ "Custom",                 0,     0,   0,  0,  0, 0,    0,   0,     0,  0,  0,    0,  0,  0,  0 },
	{ "Echo Chamber",           0, 12000, 100, 30, 30, 2,   80,  20,  9000, 10,  4,   90, 10, 30, 20 },
	{ "Large Room",             0,  8000,  90, 45, 45, 0.5, 40,  64,  8000, 12,  1.2, 90, 10, 30, 20 },
	{ "Large Room Bright",      0, 16000,  90, 45, 45, 0.5, 40,  59, 16000, 12,  1.2, 90, 10, 30, 20 },
	{ "Large Room Dark",        0,  3600,  90, 45, 45, 1,   20,  70,  3600, 12,  1.4, 90, 10, 30, 20 },
	{ "Large Room Drum",        0,  6500,  90, 35, 35, 1,   20,  30,  6500, 16,  1.2, 85, 10, 25, 25 },
	{ "Large Room Tiled",       0,  8500,  90, 15, 70, 2,    0,  10,  8500, 12,  1.2, 90, 10, 30, 20 },
	{ "Large Room Vocal",       0,  5500,  90, 45, 45, 2,    0,  80,  5500,  4,  1.5, 90, 10, 30, 20 },
	{ "Large Room Wooden",      0,  9000,  90, 55, 35, 1,   20, 100,  9000, 12,  1.2, 90, 10, 30, 20 },
	{ "Live",                   0, 12000,  90, 20, 20, 2,   80,  25,  9000, 25,  1.5, 90, 30, 15, 60 },
	{ "Long Reverb 12s",        0, 16000, 100, 25, 25, 1,   20,  80, 10000,  0, 12,   90, 10, 30, 20 },
	{ "Long Reverb 30s",        0, 16000, 100, 25, 25, 1,   20,  80,  9000,  0, 30,   90, 10, 30, 20 },
	{ "Medium Room",            0,  8000,  80, 30, 30, 0.5, 40,  57,  8000,  8,  0.6, 90, 10, 30, 20 },
	{ "Medium Room Bright",     0, 16000,  80, 30, 30, 0.5, 40,  52, 16000,  8,  0.6, 90, 10, 30, 20 },
	{ "Medium Room Dark",       0,  3600,  80, 30, 30, 1,   20,  65,  3600,  8,  0.8, 90, 10, 30, 20 },
	{ "Medium Room Drum",       0,  6500,  80, 25, 25, 1,   20,  25,  6500, 12,  0.6, 85, 10, 25, 25 },
	{ "Medium Room Tiled",      0,  8500,  80, 10, 65, 2,    0,  10,  8500,  8,  0.6, 90, 10, 30, 20 },
	{ "Medium Room Vocal",      0,  5500,  80, 30, 30, 2,    0,  75,  5500,  2,  0.9, 90, 10, 30, 20 },
	{ "Medium Room Wooden",     0,  9000,  80, 40, 20, 1,   20, 100,  9000,  8,  0.6, 90, 10, 30, 20 },
	{ "Shimmer",                0,  8000, 100, 15, 15, 0.5, 40,  50,  8000,  0,  6,  100,  5,  5, 20 },
	{ "Small Room",             0,  8000,  70, 15, 15, 0.5, 40,  50,  8000,  4,  0.3, 90, 10, 30, 20 },
	{ "Small Room Bright",      0, 16000,  70, 15, 15, 0.5, 40,  45, 16000,  4,  0.3, 90, 10, 30, 20 },
	{ "Small Room Dark",        0,  3600,  70, 15, 15, 1,   20,  60,  3600,  4,  0.5, 90, 10, 30, 20 },
	{ "Small Room Drum",        0,  6500,  70, 15, 15, 1,   20,  20,  6500,  8,  0.3, 85, 10, 25, 25 },
	{ "Small Room Tiled",       0,  8500,  70,  5, 60, 2,    0,  10,  8500,  4,  0.3, 90, 10, 30, 20 },
	{ "Small Room Vocal",       0,  5500,  70, 15, 15, 2,    0,  70,  5500,  0,  0.6, 90, 10, 30, 20 },
	{ "Small Room Wooden",      0,  9000,  70, 25,  5, 1,   20, 100,  9000,  4,  0.3, 90, 10, 30, 20 },
	{ "Tunnel",                 0,  8000,  50, 65, 65, 0.5, 10,  80,  6000,  0,  8,   90, 10, 30, 20 },
	{ "Very Large Room",        0,  8000, 100, 60, 60, 0.5, 40,  70,  8000, 16,  2.0, 90, 10, 30, 20 },
	{ "Very Large Room Bright", 0, 16000, 100, 60, 60, 0.5, 40,  65, 16000, 16,  2.0, 90, 10, 30, 20 },
	{ "Very Large Room Dark",   0,  3600, 100, 60, 60, 1,   20,  75,  3600, 16,  2.2, 90, 10, 30, 20 },
	{ "Very Large Room Drum",   0,  6500, 100, 45, 45, 1,   20,  35,  6500, 20,  2.0, 85, 10, 25, 25 },
	{ "Very Large Room Tiled",  0,  8500, 100, 20, 75, 2,    0,  10,  8500, 16,  2.0, 90, 10, 30, 20 },
	{ "Very Large Room Vocal",  0,  5500, 100, 60, 60, 2,    0,  85,  5500,  6,  2.3, 90, 10, 30, 20 },
	{ "Very Large Room Wooden", 0,  9000, 100, 70, 50, 1,   20, 100,  9000, 16,  2.0, 90, 10, 30, 20 },
};

const audio_effect_reverb::early_reverb_tap_map audio_effect_reverb::tap_maps[15] = {
	{ "0", { 18, 18 },
	  {{ 0.0043, 0.0215, 0.0225, 0.0268, 0.0270, 0.0298, 0.0458, 0.0485, 0.0572, 0.0587, 0.0595, 0.0612, 0.0707, 0.0708, 0.0726, 0.0741, 0.0753, 0.0797 },
	   { 0.0053, 0.0225, 0.0235, 0.0278, 0.0290, 0.0288, 0.0468, 0.0475, 0.0582, 0.0577, 0.0575, 0.0622, 0.0697, 0.0718, 0.0736, 0.0751, 0.0763, 0.0817 }},
	  {{ 0.841, 0.504, 0.491, 0.379, 0.380, 0.346, 0.289, 0.272, 0.192, 0.193, 0.217, 0.181, 0.180, 0.181, 0.176, 0.142, 0.167, 0.134 },
	   { 0.842, 0.506, 0.489, 0.382, 0.300, 0.346, 0.290, 0.271, 0.193, 0.192, 0.217, 0.195, 0.192, 0.166, 0.186, 0.131, 0.168, 0.133 }}},

	{ "1", { 6, 6 },
	  {{ 0.0199, 0.0354, 0.0389, 0.0414, 0.0699, 0.0796 },
	   { 0.0209, 0.0364, 0.0399, 0.0424, 0.0709, 0.0806 }},
	  {{ 1.020, 0.818, 0.635, 0.719, 0.267, 0.242 },
	   { 1.021, 0.820, 0.633, 0.722, 0.187, 0.243 }}},

	{ "2", { 4, 4 },
	  {{ 0.0090, 0.0118, 0.0205, 0.0213 },
	   { 0.0098, 0.0145, 0.0203, 0.0230 }},
	  {{ 1.35, -1.15, 1.15, -1.14 },
	   { 1.36, -1.16, -1.00, 1.14 }}},

	{ "11", { 11, 11 },
	  {{ 0.003568, 0.011703, 0.019526, 0.024870, 0.037740, 0.048089, 0.053948, 0.061333, 0.061344, 0.070073, 0.077130 },
	   { 0.002818, 0.009115, 0.017042, 0.023885, 0.033068, 0.042307, 0.051234, 0.059896, 0.067984, 0.067995, 0.076458 }},
	  {{ 0.963333,-0.806667, 0.706667,-0.656667,-0.556667, 0.526667,-0.506667, 0.503333, 0.450000,-0.470000, 0.456667 },
	   { 0.980000,-0.850000, 0.726667,-0.660000, 0.583333, 0.553333,-0.503333, 0.486667,-0.473333,-0.466667, 0.463333 }}},

	{ "12", { 12, 12 },
	  {{ 0.006344, 0.012286, 0.023385, 0.028495, 0.036385, 0.045750, 0.053427, 0.059266, 0.063281, 0.063292, 0.066437, 0.066448 },
	   { 0.004568, 0.011266, 0.015687, 0.030203, 0.030214, 0.038740, 0.048172, 0.054073, 0.058870, 0.062417, 0.064510, 0.064521 }},
	  {{ 0.896667, 0.796667,-0.653333, 0.623333,-0.570000, 0.530000,-0.496667, 0.483333, 0.480000, 0.406667,-0.470000,-0.470000 },
	   { 0.946667, 0.823333,-0.753333, 0.593333, 0.600000, 0.556667, 0.530000,-0.516667, 0.490000,-0.480000, 0.476667, 0.410000 }}},

	{ "13", { 10, 12 },
	  {{ 0.003568, 0.011703, 0.019526, 0.024870, 0.037740, 0.048089, 0.053948, 0.061333, 0.070073, 0.077125 },
	   { 0.002818, 0.009115, 0.017042, 0.023885, 0.033062, 0.033073, 0.033120, 0.042307, 0.051234, 0.059901, 0.067990, 0.076458 }},
	  {{ 0.963333,-0.813333, 0.700000,-0.650000,-0.560000, 0.526667,-0.500000, 0.486667,-0.463333, 0.463333 },
	   { 0.980000,-0.856667, 0.723333,-0.660000, 0.580000, 0.580000, 0.166667, 0.550000,-0.506667, 0.476667,-0.466667, 0.460000 }}},

	{ "14", { 11, 13 },
	  {{ 0.002062, 0.008031, 0.023010, 0.042016, 0.055766, 0.059432, 0.064536, 0.071240, 0.076490, 0.079250, 0.079292 },
	   { 0.001396, 0.009427, 0.023260, 0.039740, 0.039786, 0.052427, 0.052479, 0.053552, 0.062042, 0.067380, 0.067391, 0.073630, 0.076490 }},
	  {{ 0.993333,-0.873333, 0.663333,-0.543333, 0.503333,-0.486667, 0.480000,-0.466667, 0.466667,-0.463333,-0.186667 },
	   { 1.013333, 0.833333,-0.660000, 0.546667, 0.190000, 0.506667, 0.150000,-0.506667, 0.493333, 0.473333, 0.473333,-0.463333, 0.473333 }}},

	{ "15", { 11, 10 },
	  {{ 0.009531, 0.014042, 0.028557, 0.037885, 0.043745, 0.051255, 0.057661, 0.066589, 0.075375, 0.084964, 0.084974 },
	   { 0.006198, 0.016792, 0.022510, 0.033880, 0.048922, 0.056766, 0.067510, 0.074745, 0.080651, 0.083865 }},
	  {{-0.636667, 1.013333, 0.770000, 0.866667, 0.983333, 0.700000, 0.786667, 0.763333, 0.796667, 0.680000, 0.690000 },
	   { 1.006667, 0.736667, 0.973333, 0.760000, 0.713333, 0.963333, 0.980000, 0.920000,-0.670000, 0.870000 }}},

	{ "16", { 12, 11 },
	  {{ 0.003021, 0.008531, 0.010703, 0.012203, 0.014682, 0.018547, 0.018604, 0.025391, 0.034026, 0.038948, 0.047807, 0.047818 },
	   { 0.002526, 0.005193, 0.005255, 0.006682, 0.013500, 0.016937, 0.020109, 0.022115, 0.028370, 0.036130, 0.043328 }},
	  {{ 0.823333, 0.773333,-0.810000,-0.873333, 0.770000, 0.723333, 0.166667,-0.766667,-0.753333, 0.656667, 0.533333, 0.533333 },
	   { 0.963333, 0.883333, 0.170000, 0.736667,-0.870000,-0.790000,-0.766667,-0.763333, 0.750000, 0.653333, 0.520000 }}},

	{ "17", { 11, 11 },
	  {{ 0.002964, 0.003031, 0.009786, 0.010870, 0.018380, 0.019984, 0.027745, 0.043411, 0.048651, 0.053406, 0.058391 },
	   { 0.004193, 0.009203, 0.010198, 0.010208, 0.010266, 0.015708, 0.016917, 0.030208, 0.036193, 0.042406, 0.048135 }},
	  {{ 0.913333, 0.143333, 0.626667, 0.610000, 0.800000, 0.643333, 0.773333,-0.773333, 0.630000,-0.806667,-0.683333 },
	   { 0.976667, 0.726667, 0.716667, 0.716667, 0.143333,-0.683333,-0.730000,-0.893333, 0.770000,-0.856667,-0.850000 }}},

	{ "18", { 11, 13 },
	  {{ 0.004693, 0.009786, 0.012036, 0.015625, 0.019521, 0.019531, 0.024615, 0.032896, 0.040786, 0.046208, 0.050464 },
	   { 0.004026, 0.009510, 0.014000, 0.015312, 0.019276, 0.024344, 0.024406, 0.031292, 0.031354, 0.042391, 0.045625, 0.045693, 0.050609 }},
	  {{ 0.810000, 0.853333,-0.733333, 1.023333, 0.690000, 0.683333, 0.906667,-0.903333, 0.666667, 0.726667, 0.660000 },
	   { 0.883333, 0.803333,-0.876667, 0.823333, 0.836667, 0.853333, 0.166667,-0.903333,-0.173333, 0.783333, 0.880000, 0.140000, 0.790000 }}},

	{ "19", { 10, 11 },
	  {{ 0.006031, 0.019214, 0.032687, 0.041682, 0.047917, 0.055474, 0.059266, 0.064068, 0.068786, 0.075062 },
	   { 0.007031, 0.012599, 0.025599, 0.034943, 0.044250, 0.050255, 0.062771, 0.066646, 0.066656, 0.071807, 0.078354 }},
	  {{ 0.933333, 0.833333, 0.793333, 0.783333, 0.840000,-0.793333, 0.860000, 0.716667,-0.810000, 0.733333 },
	   { 0.706667, 0.873333, 0.846667, 0.846667,-0.836667, 0.826667,-0.860000, 0.780000, 0.776667,-0.890000,-0.836667 }}},

	{ "20", { 12, 11 },
	  {{ 0.004292, 0.013469, 0.023667, 0.029073, 0.033005, 0.044031, 0.053677, 0.060505, 0.071667, 0.071677, 0.079833, 0.079844 },
	   { 0.007917, 0.014245, 0.026464, 0.030865, 0.041781, 0.052109, 0.057672, 0.065104, 0.074745, 0.082375, 0.082385 }},
	  {{ 0.950000,-0.786667, 0.680000, 0.620000,-0.556667, 0.520000, 0.500000,-0.476667, 0.466667, 0.470000,-0.450000,-0.456667 },
	   { 0.863333, 0.736667,-0.643333,-0.593333, 0.536667, 0.523333,-0.483333, 0.486667,-0.463333, 0.450000, 0.453333 }}},

	{ "21", { 11, 11 },
	  {{ 0.004474, 0.014313, 0.023130, 0.030646, 0.038906, 0.048729, 0.056156, 0.064510, 0.064521, 0.072859, 0.081219 },
	   { 0.007729, 0.007797, 0.017010, 0.025641, 0.033901, 0.040318, 0.051432, 0.058495, 0.066859, 0.075224, 0.085417 }},
	  {{ 0.950000, 0.756667, 0.680000, 0.616667, 0.563333, 0.523333, 0.493333, 0.493333, 0.446667, 0.460000, 0.456667 },
	   { 0.893333, 0.146667, 0.733333, 0.656667, 0.576667, 0.543333, 0.510000, 0.503333, 0.466667, 0.470000, 0.480000 }}},

	{ "22", { 10, 10 },
	  {{ 0.003276, 0.010714, 0.019526, 0.024656, 0.033474, 0.037687, 0.044661, 0.052552, 0.052599, 0.069260 },
	   { 0.003276, 0.010714, 0.019526, 0.024656, 0.033474, 0.037682, 0.044661, 0.052552, 0.058604, 0.069266 }},
	  {{ 0.963333, 0.810000, 0.720000, 0.640000, 0.596667, 0.540000, 0.520000, 0.493333, 0.173333, 0.473333 },
	   { 0.970000, 0.803333, 0.720000, 0.646667, 0.596667, 0.543333, 0.523333, 0.503333, 0.483333, 0.470000 }}},
};

u32 audio_effect_reverb::preset_count()
{
	return sizeof(presets)/sizeof(presets[0]);
}

const char *audio_effect_reverb::preset_name(u32 id)
{
	return presets[id].name;
}

u32 audio_effect_reverb::early_tap_setup_count()
{
	return sizeof(tap_maps)/sizeof(tap_maps[0]);
}

const char *audio_effect_reverb::early_tap_setup_name(u32 id)
{
	return tap_maps[id].name;
}

u32 audio_effect_reverb::find_current_preset()
{
	for(u32 id=0; id != preset_count(); id++) {
		const preset &p = presets[id];

		if(m_early_tap_setup != p.early_tap_setup) continue;
		if(m_early_damping != p.early_damping) continue;
		if(m_stereo_width != p.stereo_width) continue;
		if(m_early_room_size != p.early_room_size) continue;
		if(m_late_room_size != p.late_room_size) continue;
		if(m_late_spin != p.late_spin) continue;
		if(m_late_wander != p.late_wander) continue;
		if(m_late_diffusion != p.late_diffusion) continue;
		if(m_late_damping != p.late_damping) continue;
		if(m_late_predelay != p.late_predelay) continue;
		if(m_late_global_decay != p.late_global_decay) continue;
		if(m_dry_level != p.dry_level) continue;
		if(m_early_level != p.early_level) continue;
		if(m_late_level != p.late_level) continue;
		if(m_early_to_late_level != p.early_to_late_level) continue;

		return id;
	}

	return 0;
}

void audio_effect_reverb::load_preset(u32 id)
{
	if(id == 0)
		return;

	const preset &p = presets[id];

	set_early_tap_setup(p.early_tap_setup);
	set_early_damping(p.early_damping);
	set_stereo_width(p.stereo_width);
	set_early_room_size(p.early_room_size);
	set_late_room_size(p.late_room_size);
	set_late_spin(p.late_spin);
	set_late_wander(p.late_wander);
	set_late_diffusion(p.late_diffusion);
	set_late_damping(p.late_damping);
	set_late_predelay(p.late_predelay);
	set_late_global_decay(p.late_global_decay);
	set_dry_level(p.dry_level);
	set_early_level(p.early_level);
	set_late_level(p.late_level);
	set_early_to_late_level(p.early_to_late_level);
}

u32 audio_effect_reverb::find_preset(std::string name)
{
	for(u32 id=0; id != preset_count(); id++)
		if(preset_name(id) == name)
			return id;
	return 0;
}

audio_effect_reverb::audio_effect_reverb(speaker_device *speaker, u32 sample_rate, audio_effect *def) :
	audio_effect(speaker, sample_rate, def)
{
	m_default_preset_id = find_preset("Medium Room");
	m_default_preset = &presets[m_default_preset_id];
	assert(m_default_preset_id > 0);

	m_early_lpf_h.resize(m_channels);
	m_early_hpf_h.resize(m_channels);
	m_early_diffusion_allpass_h.resize(m_channels);
	m_early_cross_allpass_h.resize(m_channels);
	m_late_dccut_h.resize(m_channels);

	m_early_delays.resize(m_channels);
	m_early_xdelays.resize(m_channels);

	m_late_input_diffusion.resize(m_channels);
	m_late_cross_diffusion.resize(m_channels);
	for(u32 channel = 0; channel != m_channels; channel ++) {
		m_late_input_diffusion[channel].resize(INPUT_DIFFUSION_ALLPASS);
		m_late_cross_diffusion[channel].resize(CROSS_DIFFUSION_ALLPASS);
	}
	m_late_input_damping_h.resize(m_channels);
	m_late_damping_1_h.resize(m_channels);
	m_late_damping_2_h.resize(m_channels);
	m_late_output_lpf_h.resize(m_channels);
	m_late_step_1.resize(m_channels);
	m_late_step_2.resize(m_channels);
	m_late_step_3.resize(m_channels);
	m_late_step_4.resize(m_channels);
	m_late_step_5.resize(m_channels);
	m_late_step_6.resize(m_channels);
	m_late_step_7.resize(m_channels);
	m_late_step_8.resize(m_channels);
	m_late_bass_h.resize(m_channels);
	m_late_comb.resize(m_channels);
	m_late_final_delay.resize(m_channels);

	m_early_in.resize(m_channels);
	m_early_wet.resize(m_channels);
	m_early_out.resize(m_channels);
	m_late_in.resize(m_channels);
	m_late_diff.resize(m_channels);
	m_late_cross.resize(m_channels);
	m_late_cross2.resize(m_channels);
	m_late_pre_out.resize(m_channels);
	m_late_out.resize(m_channels);

	// Speaker decomposition mapping, should it go up to be shared by all effects?
	m_ch_type.resize(m_channels, T_MONO);
	m_ch_pair.resize(m_channels);
	for(u32 i=0; i != m_channels; i++) {
		// Already picked up, skip
		if(m_ch_type[i] != T_MONO)
			continue;

		// Default to mono associated with itself
		m_ch_pair[i] = i;

		const auto &pos = speaker->get_position(i);
		// Keep mono if special or in the middle
		if(pos.is_onreq() || pos.is_unknown() || pos.is_lfe() || pos.m_x == 0)
			continue;

		// Search for another channel with same y, z, and opposite x
		u32 j = i+1;
		for(;j != m_channels;j++) {
			const auto &jpos = speaker->get_position(j);
			if(jpos.is_onreq() || jpos.is_unknown() || jpos.is_lfe())
				continue;
			if(pos.m_x == -jpos.m_x && pos.m_y == jpos.m_y && pos.m_z == jpos.m_z)
				break;
		}

		// Found one, build the pair, otherwise leave as-is
		if(j != m_channels) {
			m_ch_pair[i] = j;
			m_ch_pair[j] = i;
			m_ch_type[i] = pos.m_x >= 0 ? T_LEFT : T_RIGHT;
			m_ch_type[j] = pos.m_x <  0 ? T_LEFT : T_RIGHT;
		}
	}

	// Fixed
	m_early_room_size_ratio = 1;
	m_late_room_size_ratio = 1;

	set_early_hpf(4);
	set_early_diffusion_ap(150, 4);
	set_early_cross_ap(750, 4);
	set_early_multichannel_delay(0.2);
	set_late_dccut(5);
	set_late_modulation_noise_1(0.09);
	set_late_modulation_noise_2(0.06);
	set_late_spin_limit_1(20);
	set_late_spin_limit_2(12);
	set_late_input_damping(20000);
	set_late_diffusion_1(0.375);
	set_late_diffusion_2(0.312);
	set_late_diffusion_3(0.406);
	set_late_diffusion_4(0.250);
    set_late_decay_0(0.237);
    set_late_decay_1(0.938);
    set_late_decay_2(0.844);
    set_late_decay_3(0.906);
    set_late_decay_f(1.000);
	m_late_crossfeed = 0.4;
	set_late_bass_allpass(150, 4);
	set_late_damping_2(500, 2);
	m_late_bass_boost = 0.1;

	// Variables
	reset_all();
}


void audio_effect_reverb::config_load(util::xml::data_node const *ef_node)
{
	if(ef_node->has_attribute("mode")) {
		m_mode = ef_node->get_attribute_int("mode", 0);
		m_isset_mode = true;
	} else
		reset_mode();

	if(ef_node->has_attribute("early_tap_setup")) {
		m_early_tap_setup = ef_node->get_attribute_float("early_tap_setup", 0);
		m_isset_early_tap_setup = true;
	} else
		reset_early_tap_setup();

	if(ef_node->has_attribute("early_damping")) {
		m_early_damping = ef_node->get_attribute_float("early_damping", 0);
		m_isset_early_damping = true;
	} else
		reset_early_damping();

	if(ef_node->has_attribute("stereo_width")) {
		m_stereo_width = ef_node->get_attribute_float("stereo_width", 0);
		m_isset_stereo_width = true;
	} else
		reset_stereo_width();

	if(ef_node->has_attribute("early_room_size")) {
		m_early_room_size = ef_node->get_attribute_float("early_room_size", 0);
		m_isset_early_room_size = true;
	} else
		reset_early_room_size();

	if(ef_node->has_attribute("late_room_size")) {
		m_late_room_size = ef_node->get_attribute_float("late_room_size", 0);
		m_isset_late_room_size = true;
	} else
		reset_late_room_size();

	if(ef_node->has_attribute("late_spin")) {
		m_late_spin = ef_node->get_attribute_float("late_spin", 0);
		m_isset_late_spin = true;
	} else
		reset_late_spin();

	if(ef_node->has_attribute("late_wander")) {
		m_late_wander = ef_node->get_attribute_float("late_wander", 0);
		m_isset_late_wander = true;
	} else
		reset_late_wander();

	if(ef_node->has_attribute("late_diffusion")) {
		m_late_diffusion = ef_node->get_attribute_float("late_diffusion", 0);
		m_isset_late_diffusion = true;
	} else
		reset_late_diffusion();

	if(ef_node->has_attribute("late_damping")) {
		m_late_damping = ef_node->get_attribute_float("late_damping", 0);
		m_isset_late_damping = true;
	} else
		reset_late_damping();

	if(ef_node->has_attribute("late_predelay")) {
		m_late_predelay = ef_node->get_attribute_float("late_predelay", 0);
		m_isset_late_predelay = true;
	} else
		reset_late_predelay();

	if(ef_node->has_attribute("late_global_decay")) {
		m_late_global_decay = ef_node->get_attribute_float("late_global_decay", 0);
		m_isset_late_global_decay = true;
	} else
		reset_late_global_decay();

	if(ef_node->has_attribute("dry_level")) {
		m_dry_level = ef_node->get_attribute_float("dry_level", 0);
		m_isset_dry_level = true;
	} else
		reset_dry_level();

	if(ef_node->has_attribute("early_level")) {
		m_early_level = ef_node->get_attribute_float("early_level", 0);
		m_isset_early_level = true;
	} else
		reset_early_level();

	if(ef_node->has_attribute("late_level")) {
		m_late_level = ef_node->get_attribute_float("late_level", 0);
		m_isset_late_level = true;
	} else
		reset_late_level();

	if(ef_node->has_attribute("early_to_late_level")) {
		m_early_to_late_level = ef_node->get_attribute_float("early_to_late_level", 0);
		m_isset_early_to_late_level = true;
	} else
		reset_early_to_late_level();

}

void audio_effect_reverb::config_save(util::xml::data_node *ef_node) const
{
	if(m_isset_mode)
		ef_node->set_attribute_int("mode", m_mode);
	if(m_isset_early_tap_setup)
		ef_node->set_attribute_float("early_tap_setup", m_early_tap_setup);
	if(m_isset_early_damping)
		ef_node->set_attribute_float("early_damping", m_early_damping);
	if(m_isset_stereo_width)
		ef_node->set_attribute_float("stereo_width", m_stereo_width);
	if(m_isset_early_room_size)
		ef_node->set_attribute_float("early_room_size", m_early_room_size);
	if(m_isset_late_room_size)
		ef_node->set_attribute_float("late_room_size", m_late_room_size);
	if(m_isset_late_spin)
		ef_node->set_attribute_float("late_spin", m_late_spin);
	if(m_isset_late_wander)
		ef_node->set_attribute_float("late_wander", m_late_wander);
	if(m_isset_late_diffusion)
		ef_node->set_attribute_float("late_diffusion", m_late_diffusion);
	if(m_isset_late_damping)
		ef_node->set_attribute_float("late_damping", m_late_damping);
	if(m_isset_late_predelay)
		ef_node->set_attribute_float("late_predelay", m_late_predelay);
	if(m_isset_late_global_decay)
		ef_node->set_attribute_float("late_global_decay", m_late_global_decay);
	if(m_isset_dry_level)
		ef_node->set_attribute_float("dry_level", m_dry_level);
	if(m_isset_early_level)
		ef_node->set_attribute_float("early_level", m_early_level);
	if(m_isset_late_level)
		ef_node->set_attribute_float("late_level", m_late_level);
	if(m_isset_early_to_late_level)
		ef_node->set_attribute_float("early_to_late_level", m_early_to_late_level);
}

void audio_effect_reverb::default_changed()
{
	if(!m_default)
		return;
	if(!m_isset_mode)
		reset_mode();
	if(!m_isset_early_tap_setup)
		reset_early_tap_setup();
	if(!m_isset_early_damping)
		reset_early_damping();
	if(!m_isset_stereo_width)
		reset_stereo_width();
	if(!m_isset_early_room_size)
		reset_early_room_size();
	if(!m_isset_late_room_size)
		reset_late_room_size();
	if(!m_isset_late_spin)
		reset_late_spin();
	if(!m_isset_late_wander)
		reset_late_wander();
	if(!m_isset_late_diffusion)
		reset_late_diffusion();
	if(!m_isset_late_damping)
		reset_late_damping();
	if(!m_isset_late_predelay)
		reset_late_predelay();
	if(!m_isset_late_global_decay)
		reset_late_global_decay();
	if(!m_isset_dry_level)
		reset_dry_level();
	if(!m_isset_early_level)
		reset_early_level();
	if(!m_isset_late_level)
		reset_late_level();
	if(!m_isset_early_to_late_level)
		reset_early_to_late_level();
}

void audio_effect_reverb::set_mode(u32 mode)
{
	m_isset_mode = true;
	m_mode = mode;
}

void audio_effect_reverb::set_early_tap_setup(u32 index)
{
	m_isset_early_tap_setup = true;
	m_early_tap_setup = index;
	commit_early_tap_setup();
}

void audio_effect_reverb::set_early_damping(double cutoff)
{
	m_isset_early_damping = true;
	m_early_damping = cutoff;
	commit_early_damping();
}

void audio_effect_reverb::set_stereo_width(double width)
{
	m_isset_stereo_width = true;
	m_stereo_width = width;
	commit_stereo_width();
}

void audio_effect_reverb::set_early_room_size(double size)
{
	m_isset_early_room_size = true;
	m_early_room_size = size;
	commit_early_room_size();
}

void audio_effect_reverb::set_late_room_size(double size)
{
	m_isset_late_room_size = true;
	m_late_room_size = size;
	commit_late_room_size();
}

void audio_effect_reverb::set_late_spin(double speed)
{
	m_isset_late_spin = true;
	m_late_spin = speed;
	commit_late_spin();
}

void audio_effect_reverb::set_late_wander(double wander)
{
	m_isset_late_wander = true;
	m_late_wander = wander;
	commit_late_wander();
}

void audio_effect_reverb::set_late_diffusion(double value)
{
	m_isset_late_diffusion = true;
	m_late_diffusion = value;
	commit_late_diffusion();
}

void audio_effect_reverb::set_late_damping(double cutoff)
{
	m_isset_late_damping = true;
	m_late_damping = cutoff;
	commit_late_damping();
}

void audio_effect_reverb::set_late_predelay(double delay)
{
	m_isset_late_predelay = true;
	m_late_predelay = delay;
	commit_late_predelay();
}

void audio_effect_reverb::set_late_global_decay(float value)
{
	m_isset_late_global_decay = true;
	m_late_global_decay = value;
	commit_late_decay();
}

void audio_effect_reverb::set_dry_level(double level)
{
	m_isset_dry_level = true;
	m_dry_level = level;
	commit_dry_level();
}

void audio_effect_reverb::set_early_level(double level)
{
	m_isset_early_level = true;
	m_early_level = level;
	commit_early_level();
}

void audio_effect_reverb::set_late_level(double level)
{
	m_isset_late_level = true;
	m_late_level = level;
	commit_late_level();
}

void audio_effect_reverb::set_early_to_late_level(double level)
{
	m_isset_early_to_late_level = true;
	m_early_to_late_level = level;
	commit_early_to_late_level();
}



void audio_effect_reverb::reset_mode()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_mode = false;
	m_mode = d ? d->mode() : 0;
}

void audio_effect_reverb::reset_early_tap_setup()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_early_tap_setup = false;
	m_early_tap_setup = d ? d->early_tap_setup() : m_default_preset->early_tap_setup;
	commit_early_tap_setup();
}

void audio_effect_reverb::reset_early_damping()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_early_damping = false;
	m_early_damping = d ? d->early_damping() : m_default_preset->early_damping;
	commit_early_damping();
}

void audio_effect_reverb::reset_stereo_width()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_stereo_width = false;
	m_stereo_width = d ? d->stereo_width() : m_default_preset->stereo_width;
	commit_stereo_width();
}

void audio_effect_reverb::reset_early_room_size()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_early_room_size = false;
	m_early_room_size = d ? d->early_room_size() : m_default_preset->early_room_size;
	commit_early_room_size();
}

void audio_effect_reverb::reset_late_room_size()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_room_size = false;
	m_late_room_size = d ? d->late_room_size() : m_default_preset->late_room_size;
	commit_late_room_size();
}

void audio_effect_reverb::reset_late_spin()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_spin = false;
	m_late_spin = d ? d->late_spin() : m_default_preset->late_spin;
	commit_late_spin();
}

void audio_effect_reverb::reset_late_wander()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_wander = false;
	m_late_wander = d ? d->late_wander() : m_default_preset->late_wander;
	commit_late_wander();
}

void audio_effect_reverb::reset_late_diffusion()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_diffusion = false;
	m_late_diffusion = d ? d->late_diffusion() : m_default_preset->late_diffusion;
	commit_late_diffusion();
}

void audio_effect_reverb::reset_late_damping()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_damping = false;
	m_late_damping = d ? d->late_damping() : m_default_preset->late_damping;
	commit_late_damping();
}

void audio_effect_reverb::reset_late_predelay()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_predelay = false;
	m_late_predelay = d ? d->late_predelay() : m_default_preset->late_predelay;
	commit_late_predelay();
}

void audio_effect_reverb::reset_late_global_decay()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_global_decay = false;
	m_late_global_decay = d ? d->late_global_decay() : m_default_preset->late_global_decay;
	commit_late_decay();
}

void audio_effect_reverb::reset_dry_level()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_dry_level = false;
	m_dry_level = d ? d->dry_level() : m_default_preset->dry_level;
	commit_dry_level();
}

void audio_effect_reverb::reset_early_level()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_early_level = false;
	m_early_level = d ? d->early_level() : m_default_preset->early_level;
	commit_early_level();
}

void audio_effect_reverb::reset_late_level()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_late_level = false;
	m_late_level = d ? d->late_level() : m_default_preset->late_level;
	commit_late_level();
}

void audio_effect_reverb::reset_early_to_late_level()
{
	audio_effect_reverb *d = static_cast<audio_effect_reverb *>(m_default);
	m_isset_early_to_late_level = false;
	m_early_to_late_level = d ? d->early_to_late_level() : m_default_preset->early_to_late_level;
	commit_early_to_late_level();
}

void audio_effect_reverb::reset_all()
{
	reset_mode();
	reset_early_tap_setup();
	reset_early_damping();
	reset_stereo_width();
	reset_early_room_size();
	reset_late_room_size();
	reset_late_spin();
	reset_late_wander();
	reset_late_diffusion();
	reset_late_damping();
	reset_late_predelay();
	reset_late_global_decay();
	reset_dry_level();
	reset_early_level();
	reset_late_level();
	reset_early_to_late_level();
}



void audio_effect_reverb::commit_early_tap_setup()
{
	const auto &tmap = tap_maps[m_early_tap_setup];
	for(u32 side = 0; side != 2; side++)
		for(u32 tap = 0; tap != tmap.m_count[side]; tap++)
			m_early_tap_dists[side][tap] = m_sample_rate * m_early_room_size_ratio * tmap.m_delay[side][tap];
}

void audio_effect_reverb::commit_early_damping()
{
	m_early_lpf.prepare_lpf(m_early_damping, m_sample_rate);
}

void audio_effect_reverb::commit_stereo_width()
{
	m_wet1 = m_stereo_width / 100;
	m_wet2 = 1 - m_wet1;
}

void audio_effect_reverb::commit_early_room_size()
{
	m_early_room_size_ratio = m_early_room_size / 31.25 + 0.4;
	commit_early_tap_setup();
}

void audio_effect_reverb::commit_late_room_size()
{
	m_late_room_size_ratio = m_late_room_size / 31.25 + 0.4;

	double mult = m_sample_rate * m_late_room_size_ratio;

	static double delays_diff[2][INPUT_DIFFUSION_ALLPASS] = {
		{ 18.08, 15.68, 12.72, 10.17, 6.39, 4.75, 4.22, 3.58, 3.19, 2.17 },
		{ 17.67, 16.03, 12.19, 10.67, 6.92, 4.75, 4.10, 3.84, 3.25, 2.32 },
	};

	static double delays_cross[2][CROSS_DIFFUSION_ALLPASS] = {
		{ 12.60, 9.99, 7.74, 5.10, },
		{ 13.10, 9.49, 7.24, 5.60, },
	};

	u32 modulation_factor_input = find_prime(m_sample_rate * 0.3126e-3);
	u32 modulation_factor_step  = find_prime(m_sample_rate * 0.9377e-3);

	for(u32 channel = 0; channel != m_channels; channel++) {
		std::function<double (u32)> dlookup;
		std::function<double (u32)> clookup;
		switch(m_ch_type[channel]) {
		case T_LEFT:  dlookup = [](u32 index){ return delays_diff[0][index] * 1e-3; }; clookup = [](u32 index){ return delays_cross[0][index] * 1e-3; }; break;
		case T_RIGHT: dlookup = [](u32 index){ return delays_diff[1][index] * 1e-3; }; clookup = [](u32 index){ return delays_cross[1][index] * 1e-3; }; break;
		case T_MONO:
			dlookup = [](u32 index){ return 0.5e-3*(delays_diff [0][index] + delays_diff [1][index]); };
			clookup = [](u32 index){ return 0.5*(delays_cross[0][index] + delays_cross[1][index]); };
			break;
		}

		for(u32 index = 0; index != INPUT_DIFFUSION_ALLPASS; index++)
			m_late_input_diffusion[channel][index].set_delay(find_prime(dlookup(index)*mult), modulation_factor_input);
		for(u32 index = 0; index != CROSS_DIFFUSION_ALLPASS; index++)
			m_late_cross_diffusion[channel][index].set_delay(find_prime(clookup(index)*mult));
	}

	for(u32 cchan = 0; cchan != m_channels; cchan++) {
		int pchan = m_ch_pair[cchan];
		std::function<u32 (double, double)> sel;
		switch(m_ch_type[cchan]) {
		case T_LEFT:  sel = [mult](double left, double right) { return find_prime(left  * mult * 1e-3); }; break;
		case T_RIGHT: sel = [mult](double left, double right) { return find_prime(right * mult * 1e-3); }; break;
		case T_MONO:  sel = [mult](double left, double right) { return find_prime(0.5 * (left + right) * mult * 1e-3); }; break;
		}

		m_late_step_1[cchan].set_delay( sel( 7.004,  6.007), modulation_factor_step);
		m_late_step_2[cchan].set_tap(0, sel( 0.059,  0.029));
		m_late_step_3[cchan].set_delay( sel(11.471,  9.641), modulation_factor_step);
		m_late_step_4[cchan].set_tap(0, sel(30.916, 42.784));
		m_late_step_5[cchan].set_delays(sel(56.957, 59.546), sel(17.934, 10.784));
		m_late_step_6[cchan].set_tap(0, sel(10.081, 14.652));
		m_late_step_7[cchan].set_delays(sel(35.516, 42.549), sel( 3.546,  0.147), sel(23.912, 20.161), sel(37.040, 39.267));
		m_late_step_8[cchan].set_tap(0, sel(46.066,  0.469));

		m_late_step_4[cchan].set_tap(1, sel( 0.029, 18.315));
		m_late_step_4[pchan].set_tap(2, sel( 3.223, 23.150));

		m_late_step_5[cchan].set_tap_1(0, sel( 3.546,  0.293));
		m_late_step_5[cchan].set_tap_2(0, sel(14.066, 10.520));
		m_late_step_5[pchan].set_tap_2(1, sel( 3.018,  0.879));

		m_late_step_6[cchan].set_tap(1, sel( 1.172, 13.714));
		m_late_step_6[pchan].set_tap(2, sel( 5.626,  9.143));
		m_late_step_6[cchan].set_tap(3, sel( 8.088,  0.703));
		m_late_step_6[pchan].set_tap(4, sel(13.714,  5.538));

		m_late_step_7[cchan].set_tap_1(0, sel( 0.762,  0.293));
		m_late_step_7[cchan].set_tap_2(0, sel(22.857,  3.194));
		m_late_step_7[cchan].set_tap_3(0, sel(35.165, 38.388));
		m_late_step_7[pchan].set_tap_2(1, sel( 9.084, 23.443));

		m_late_step_8[pchan].set_tap(1, sel( 1.055,  1.055));
		m_late_step_8[cchan].set_tap(2, sel(46.066,  0.234));
		m_late_step_8[cchan].set_tap(3, sel(22.857,  0.293));
	}

	commit_late_decay();
}

void audio_effect_reverb::commit_late_decay()
{
	float r1 = m_late_global_decay / m_late_room_size_ratio;
	float r2 = r1 * m_late_decay_f;

	float d0 = std::pow(10.0f, std::log10(m_late_decay_0) / r1);
	m_late_loop_decay = d0;

	float d1 = std::pow(10.0f, std::log10(m_late_decay_1) / r2);
	for(auto &ap : m_late_step_5)
		ap.set_decay_1(d1);
	for(auto &ap : m_late_step_7) {
		ap.set_decay_1(d1);
		ap.set_decay_2(d1);
	}

	float d2 = std::pow(10.0f, std::log10(m_late_decay_2) / r2);
	for(auto &ap : m_late_step_1)
		ap.set_decay(d2);
	for(auto &ap : m_late_step_5)
		ap.set_decay_2(d2);
	for(auto &ap : m_late_step_7)
		ap.set_decay_3(d2);

	float d3 = std::pow(10.0f, std::log10(m_late_decay_3) / r2);
	for(auto &ap : m_late_step_3)
		ap.set_decay(d3);
}

void audio_effect_reverb::commit_late_spin()
{
	m_late_lfo1.prepare(m_late_spin, m_sample_rate);
	m_late_lfo2.prepare(std::sqrt(100-(10-m_late_spin)*(10-m_late_spin))/2, m_sample_rate);
}

void audio_effect_reverb::commit_late_wander()
{
	m_late_wander_actual = m_late_wander / 200 + 0.1;
}

void audio_effect_reverb::commit_late_diffusion()
{
	double gain = - m_late_diffusion / 105;
	for(auto &aps : m_late_input_diffusion)
		for(auto &ap : aps)
			ap.set_gain(gain);
	for(auto &aps : m_late_cross_diffusion)
		for(auto &ap : aps)
			ap.set_gain(gain);
}

void audio_effect_reverb::commit_late_damping()
{
	m_late_damping_1.prepare_lpf(m_late_damping, m_sample_rate);
	m_late_output_lpf.prepare_lpf(m_late_damping, 2, m_sample_rate);
}

void audio_effect_reverb::commit_late_predelay()
{
	u32 size = m_late_predelay * m_sample_rate / 1000;
	for(auto &d : m_late_final_delay)
		d.set_tap(0, size);
}

void audio_effect_reverb::commit_dry_level()
{
	m_actual_dry_level = m_dry_level / 100;
}

void audio_effect_reverb::commit_early_level()
{
	m_actual_early_level = m_early_level / 100;
}

void audio_effect_reverb::commit_late_level()
{
	m_actual_late_level = m_late_level / 100;
}

void audio_effect_reverb::commit_early_to_late_level()
{
	m_actual_early_to_late_level = m_early_to_late_level / 100;
}



void audio_effect_reverb::set_early_hpf(double cutoff)
{
	m_early_hpf.prepare_hpf(cutoff, m_sample_rate);
}

void audio_effect_reverb::set_early_diffusion_ap(double cutoff, double bw)
{
	m_early_diffusion_allpass.prepare_apf(cutoff, bw, m_sample_rate);
}

void audio_effect_reverb::set_early_cross_ap(double cutoff, double bw)
{
	m_early_cross_allpass.prepare_apf(cutoff, bw, m_sample_rate);
}

void audio_effect_reverb::set_early_multichannel_delay(double delay)
{
	m_early_xdelays_dist = m_sample_rate * delay / 1000;
}

void audio_effect_reverb::set_late_dccut(double cutoff)
{
	m_late_dccut.prepare(cutoff, m_sample_rate);
}

void audio_effect_reverb::set_late_spin_limit_1(double cutoff)
{
	m_late_lfo1_lpf.prepare_lpf(cutoff, m_sample_rate);
}

void audio_effect_reverb::set_late_spin_limit_2(double cutoff)
{
	m_late_lfo2_lpf.prepare_lpf(cutoff, m_sample_rate);
}

void audio_effect_reverb::set_late_modulation_noise_1(double mod)
{
	m_late_modulation_noise_1 = mod;
}

void audio_effect_reverb::set_late_modulation_noise_2(double mod)
{
	m_late_modulation_noise_2 = mod;
}

void audio_effect_reverb::set_late_diffusion_1(double value)
{
	for(auto &ap : m_late_step_1)
		ap.set_gain(value);
}

void audio_effect_reverb::set_late_diffusion_2(double value)
{
	for(auto &ap : m_late_step_3)
		ap.set_gain(value);
}

void audio_effect_reverb::set_late_diffusion_3(double value)
{
	for(auto &ap : m_late_step_5)
		ap.set_gain_2(value);
	for(auto &ap : m_late_step_7)
		ap.set_gain_3(value);
}

void audio_effect_reverb::set_late_diffusion_4(double value)
{
	for(auto &ap : m_late_step_5)
		ap.set_gain_1(value);
	for(auto &ap : m_late_step_7) {
		ap.set_gain_1(value);
		ap.set_gain_2(value);
	}
}

void audio_effect_reverb::set_late_input_damping(double cutoff)
{
	m_late_input_damping.prepare_lpf(cutoff, m_sample_rate);
}

void audio_effect_reverb::set_late_decay_0(float value)
{
	m_late_decay_0 = value;
}

void audio_effect_reverb::set_late_decay_1(float value)
{
	m_late_decay_1 = value;
}

void audio_effect_reverb::set_late_decay_2(float value)
{
	m_late_decay_2 = value;
}

void audio_effect_reverb::set_late_decay_3(float value)
{
	m_late_decay_3 = value;
}

void audio_effect_reverb::set_late_decay_f(float value)
{
	m_late_decay_f = value;
}

void audio_effect_reverb::set_late_bass_allpass(double cutoff, double bw)
{
	m_late_bass.prepare_apf(cutoff, bw, m_sample_rate);
}

void audio_effect_reverb::set_late_damping_2(double cutoff, double bw)
{
	m_late_damping_2.prepare_lpf(cutoff, bw, m_sample_rate);
}

void audio_effect_reverb::set_late_spin_to_wander(double value)
{
	u32 size = find_prime(value * m_sample_rate / 1000.0);
	for(auto &c : m_late_comb)
		c.set_size(size);
}


// An IIR1 filter, either lowpass or highpass

void audio_effect_reverb::iir1::prepare_lpf(double cutoff, u32 sample_rate)
{
	double w2 = M_PI*cutoff/sample_rate;
	double tw2 = std::tan(w2);
	m_b1 = tw2/(1+tw2);
	m_b2 = m_b1;
	m_a2 = (1-tw2)/(1+tw2);
}

void audio_effect_reverb::iir1::prepare_hpf(double cutoff, u32 sample_rate)
{
	double w2 = M_PI*cutoff/sample_rate;
	double tw2 = std::tan(w2);
	m_b1 = 1/(1+tw2);
	m_b2 = -m_b1;
	m_a2 = (1-tw2)/(1+tw2);
}

void audio_effect_reverb::iir1h::clear()
{
	m_y1 = 0;
}

audio_effect_reverb::sample_t audio_effect_reverb::iir1::process(iir1h &h, sample_t x0)
{
	sample_t y0 = x0 * m_b1 + h.m_y1;
	h.m_y1 = y0 * m_a2 + x0 * m_b2;
	return y0;
}


// An IIR2 filter, lowpass (changes the phases without touching the amplitudes)

void audio_effect_reverb::iir2::prepare_lpf(double cutoff, double bw, u32 sample_rate)
{
	double w = 2*M_PI*cutoff/sample_rate;
	double s = std::sin(w);
	double c = std::cos(w);
	double alpha = s * std::sinh(M_LN2 / 2 * bw * w / s);
	double a0 = 1/(1+alpha);
	m_b0 = a0*0.5*(1 - c);
	m_b1 = a0*(1 - c);
	m_b2 = a0*0.5*(1 - c);
	m_a1 = a0*(-2 * c);
	m_a2 = a0*(1-alpha);
}

// An IIR2 filter, allpass (changes the phases without touching the amplitudes)

void audio_effect_reverb::iir2::prepare_apf(double cutoff, double bw, u32 sample_rate)
{
	double w = 2*M_PI*cutoff/sample_rate;
	double s = std::sin(w);
	double c = std::cos(w);
	double alpha = s * std::sinh(M_LN2 / 2 * bw * w / s);
	double a0 = 1/(1+alpha);
	m_b0 = a0*(1-alpha);
	m_b1 = a0*(-2 * c);
	m_b2 = a0*(1+alpha);
	m_a1 = a0*(-2 * c);
	m_a2 = a0*(1-alpha);
}


void audio_effect_reverb::iir2h::clear()
{
	m_x1 = m_x2 = m_y1 = m_y2 = 0;
}

audio_effect_reverb::sample_t audio_effect_reverb::iir2::process(iir2h &h, sample_t x0)
{
	sample_t y0 = x0 * m_b0 + h.m_x1 * m_b1 + h.m_x2 * m_b2 - m_a1 * h.m_y1 - m_a2 * h.m_y2;
	h.m_x2 = h.m_x1;
	h.m_x1 = x0;
	h.m_y2 = h.m_y1;
	h.m_y1 = y0;
	return y0;
}

// A filter to cut off DC

void audio_effect_reverb::dccut::prepare(double cutoff, u32 sample_rate)
{
	double w = 2*M_PI*cutoff/sample_rate;
	double s = std::sin(w);
	double c = std::cos(w);
	constexpr double s3 = 1.73205080757; // std::sqrt(3.0);

	m_gain = (s3 - 2*s)/(s + s3*c);
}

void audio_effect_reverb::dccuth::clear()
{
	m_y1 = 0;
	m_y2 = 0;
}

audio_effect_reverb::sample_t audio_effect_reverb::dccut::process(dccuth &h, sample_t x0)
{
	sample_t y0 = x0 - h.m_y1 + m_gain * h.m_y2;
	h.m_y1 = x0;
	h.m_y2 = y0;
	return y0;
}


// A delay buffer, also known as a reverb buffer

void audio_effect_reverb::delay_buffer::clear()
{
	memset(m_samples, 0, sizeof(m_samples));
	m_index = 0;
}

void audio_effect_reverb::delay_buffer::push(sample_t value)
{
	m_index = (m_index + 1) & D_MASK;
	m_samples[m_index] = value;
}

audio_effect_reverb::sample_t audio_effect_reverb::delay_buffer::get(u32 dist) const
{
	return m_samples[(m_index - dist) & D_MASK];
}

audio_effect_reverb::sample_t audio_effect_reverb::delay_buffer::geti(float dist) const
{
	u32 di = u32(dist);
	double t = dist - di;
	sample_t s1 = m_samples[(m_index - di) & D_MASK];
	sample_t s2 = m_samples[(m_index - di - 1) & D_MASK];
	return s1 + (s2-s1)*t;
}


// A delay with the storage for taps, e.g. the distances in it to
// lookup samples

void audio_effect_reverb::delay::clear()
{
	m_delay_buffer.clear();
	// Don't clear the taps positions
}

void audio_effect_reverb::delay::push(sample_t value)
{
	m_delay_buffer.push(value);
}

void audio_effect_reverb::delay::set_tap(u32 tap, u32 dist)
{
	if(m_taps.size() <= tap)
		m_taps.resize(tap+1);
	m_taps[tap] = dist;
}

audio_effect_reverb::sample_t audio_effect_reverb::delay::get(u32 tap) const
{
	return m_delay_buffer.get(m_taps[tap]);
}




// Allpass.  One category of allpass filters (there are others) is a
// delay line with a loop.  The design developed by Manfred Schroeder
// has:
//   - output      = delay_output * (decay - gain**2) - input * gain
//   - delay_input = delay_output * gain + input
//
// When disabled, decay is 1.
//
// Some variants have modulation, which change the delay and the gain
// dynamically.  Some variants work with multiple delay lines, for a
// final result built similarly.


// Allpass variant without modulation or decay.

void audio_effect_reverb::allpass::clear()
{
	m_delay_buffer.clear();
	m_gain = 0;
	m_delay = 0;
}

void audio_effect_reverb::allpass::set_gain(float base)
{
	m_gain = base;
}

void audio_effect_reverb::allpass::set_delay(u32 base)
{
	m_delay = base;
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass::process(sample_t input)
{
	sample_t tap = m_delay_buffer.get(m_delay);
	sample_t loop = input + tap * m_gain;
	sample_t output = tap - loop * m_gain;
	m_delay_buffer.push(loop);
	return output;
}


// Allpass variant with modulation, no decay.

void audio_effect_reverb::allpass_m::clear()
{
	m_delay_buffer.clear();
	m_base_gain = 0;
	m_base_delay = 0;
	m_mod_delay = 0;
}

void audio_effect_reverb::allpass_m::set_gain(float base)
{
	m_base_gain = base;
}

void audio_effect_reverb::allpass_m::set_delay(float base, float mod)
{
	m_base_delay = base;
	m_mod_delay = mod;
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass_m::process(sample_t input, float delay_mod, float gain_mod)
{
	float gain = m_base_gain + gain_mod;
	float delay = m_base_delay + (1 + delay_mod) * m_mod_delay;

	sample_t tap = m_delay_buffer.geti(delay);
	sample_t loop = input + tap * gain;
	sample_t output = tap - loop * gain;
	m_delay_buffer.push(loop);
	return output;
}


// Allpass variant with modulation and decay.

void audio_effect_reverb::allpass_md::clear()
{
	m_delay_buffer.clear();
	m_base_gain = 0;
	m_base_delay = 0;
	m_mod_delay = 0;
	m_decay = 0;
}

void audio_effect_reverb::allpass_md::set_gain(float base)
{
	m_base_gain = base;
}

void audio_effect_reverb::allpass_md::set_delay(float base, float mod)
{
	m_base_delay = base;
	m_mod_delay = mod;
}

void audio_effect_reverb::allpass_md::set_decay(float base)
{
	m_decay = base;
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass_md::process(sample_t input, float delay_mod, float gain_mod)
{
	float gain = m_base_gain + gain_mod;
	float delay = m_base_delay + (1 + delay_mod) * m_mod_delay;

	sample_t tap = m_delay_buffer.geti(delay);
	sample_t loop = input + tap * gain;
	sample_t output = m_decay * tap - loop * gain;
	m_delay_buffer.push(loop);
	return output;
}


// Allpass variant with dual buffer, decay and taps

void audio_effect_reverb::allpass2::clear()
{
	m_delay_buffer_1.clear();
	m_delay_buffer_2.clear();
	m_gain_1 = 0;
	m_gain_2 = 0;
	m_delay_1 = 0;
	m_delay_2 = 0;
	m_decay_1 = 0;
	m_decay_2 = 0;
	// Don't clear the taps positions
}

void audio_effect_reverb::allpass2::set_gain_1(float base)
{
	m_gain_1 = base;
}

void audio_effect_reverb::allpass2::set_gain_2(float base)
{
	m_gain_2 = base;
}

void audio_effect_reverb::allpass2::set_delays(u32 base_1, u32 base_2)
{
	m_delay_1 = base_1;
	m_delay_2 = base_2;
}

void audio_effect_reverb::allpass2::set_decay_1(float base)
{
	m_decay_1 = base;
}

void audio_effect_reverb::allpass2::set_decay_2(float base)
{
	m_decay_2 = base;
}

void audio_effect_reverb::allpass2::set_tap_1(u32 tap, u32 dist)
{
	if(m_taps_1.size() <= tap)
		m_taps_1.resize(tap+1);
	m_taps_1[tap] = dist;
}

void audio_effect_reverb::allpass2::set_tap_2(u32 tap, u32 dist)
{
	if(m_taps_2.size() <= tap)
		m_taps_2.resize(tap+1);
	m_taps_2[tap] = dist;
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass2::get_1(u32 tap) const
{
	return m_delay_buffer_1.get(m_taps_1[tap]);
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass2::get_2(u32 tap) const
{
	return m_delay_buffer_2.get(m_taps_2[tap]);
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass2::process(sample_t input)
{
	sample_t tap_2 = m_delay_buffer_2.get(m_delay_2);
	sample_t loop_2 = input + tap_2 * m_gain_2;
	sample_t output_2 = m_decay_2 * tap_2 - loop_2 * m_gain_2;

	sample_t tap_1 = m_delay_buffer_1.get(m_delay_1);
	sample_t loop_1 = loop_2 + tap_1 * m_gain_1;
	sample_t output_1 = m_decay_1 * tap_1 - loop_1 * m_gain_1;

	m_delay_buffer_2.push(output_1);
	m_delay_buffer_1.push(loop_1);

	return output_2;
}


// Allpass variant with triple buffer, decay, taps and modulation for the first buffer delay

void audio_effect_reverb::allpass3m::clear()
{
	m_delay_buffer_1.clear();
	m_delay_buffer_2.clear();
	m_delay_buffer_3.clear();
	m_gain_1 = 0;
	m_gain_2 = 0;
	m_gain_3 = 0;
	m_base_delay_1 = 0;
	m_mod_delay_1 = 0;
	m_delay_2 = 0;
	m_delay_3 = 0;
	m_decay_1 = 0;
	m_decay_2 = 0;
	m_decay_3 = 0;
	// Don't clear the taps positions
}


void audio_effect_reverb::allpass3m::set_gain_1(float base)
{
	m_gain_1 = base;
}

void audio_effect_reverb::allpass3m::set_gain_2(float base)
{
	m_gain_2 = base;
}

void audio_effect_reverb::allpass3m::set_gain_3(float base)
{
	m_gain_3 = base;
}

void audio_effect_reverb::allpass3m::set_delays(u32 base_1, u32 mod_1, u32 base_2, u32 base_3)
{
	m_base_delay_1 = base_1;
	m_mod_delay_1 = mod_1;
	m_delay_2 = base_2;
	m_delay_3 = base_3;
}

void audio_effect_reverb::allpass3m::set_decay_1(float base)
{
	m_decay_1 = base;
}

void audio_effect_reverb::allpass3m::set_decay_2(float base)
{
	m_decay_2 = base;
}

void audio_effect_reverb::allpass3m::set_decay_3(float base)
{
	m_decay_3 = base;
}

void audio_effect_reverb::allpass3m::set_tap_1(u32 tap, u32 dist)
{
	if(m_taps_1.size() <= tap)
		m_taps_1.resize(tap+1);
	m_taps_1[tap] = dist;
}

void audio_effect_reverb::allpass3m::set_tap_2(u32 tap, u32 dist)
{
	if(m_taps_2.size() <= tap)
		m_taps_2.resize(tap+1);
	m_taps_2[tap] = dist;
}

void audio_effect_reverb::allpass3m::set_tap_3(u32 tap, u32 dist)
{
	if(m_taps_3.size() <= tap)
		m_taps_3.resize(tap+1);
	m_taps_3[tap] = dist;
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass3m::get_1(u32 tap) const
{
	return m_delay_buffer_1.get(m_taps_1[tap]);
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass3m::get_2(u32 tap) const
{
	return m_delay_buffer_2.get(m_taps_2[tap]);
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass3m::get_3(u32 tap) const
{
	return m_delay_buffer_3.get(m_taps_3[tap]);
}

audio_effect_reverb::sample_t audio_effect_reverb::allpass3m::process(sample_t input, float delay_mod)
{
	sample_t tap_3 = m_delay_buffer_3.get(m_delay_3);
	sample_t loop_3 = input + tap_3 * m_gain_3;
	sample_t output_3 = m_decay_3 * tap_3 - loop_3 * m_gain_3;

	sample_t tap_2 = m_delay_buffer_2.get(m_delay_2);
	sample_t loop_2 = loop_3 + tap_2 * m_gain_2;
	sample_t output_2 = m_decay_2 * tap_2 - loop_2 * m_gain_2;

	float delay_1 = m_base_delay_1 + (1+delay_mod) * m_mod_delay_1;

	sample_t tap_1 = m_delay_buffer_1.geti(delay_1);
	sample_t loop_1 = loop_2 + tap_1 * m_gain_1;
	sample_t output_1 = m_decay_1 * tap_1 - loop_1 * m_gain_1;

	m_delay_buffer_3.push(output_2);
	m_delay_buffer_2.push(output_1);
	m_delay_buffer_1.push(loop_1);

	return output_3;
}

// A comb filter, another kind of delay which loops

void audio_effect_reverb::comb::clear()
{
	m_delay_buffer.clear();
	m_filter_history = 0;
	m_size = 0;
}

void audio_effect_reverb::comb::set_size(u32 size)
{
	m_size = size;
}

audio_effect_reverb::sample_t audio_effect_reverb::comb::process(sample_t input, float feedback)
{
	sample_t tap = m_delay_buffer.get(m_size) * feedback + input;
	m_delay_buffer.push(tap);
	return tap;
}



// Pink noise generator

void audio_effect_reverb::pink::clear()
{
	m_index = SIZE;
	memset(m_buffer, 0, sizeof(m_buffer));
}

audio_effect_reverb::sample_t audio_effect_reverb::pink::process()
{
	if(m_index == SIZE) {
		m_index = 0;
		// Generate a fractal pattern using the Midpoint Displacement
		// Method with a fractal dimension of 0.5.

		m_buffer[0] = 0;
		sample_t r = 2*(0.5*0.5) + 0.3;
		for(u32 l = SIZE; l > 1; l >>= 1) {
			for(u32 c = 0; c != SIZE; c += l)
				m_buffer[c + l/2] = std::clamp((m_buffer[c] + m_buffer[(c+l) & (SIZE-1)]) / 2 + r * m_dis(m_rng), -1.f, 1.f);

			r /= M_SQRT2; // 2**0.5
		}
	}
	return m_buffer[m_index++];
}


// LFO with sine-type output

void audio_effect_reverb::lfo::clear()
{
	m_count = 0;
	m_c = 1;
	m_s = 0;
}

void audio_effect_reverb::lfo::prepare(double speed, u32 sample_rate)
{
	float w = 2*M_PI*speed/sample_rate;
	m_rc = std::cos(w);
	m_rs = std::sin(w);
}

float audio_effect_reverb::lfo::process()
{
	float r = m_s;
	float nc = m_c * m_rc - m_s * m_rs;
	float ns = m_c * m_rs + m_s * m_rc;
	if(m_count++ == 10000) {
		m_count = 0;
		float l = std::sqrt(nc*nc + ns*ns);
		nc /= l;
		ns /= l;
	}
	m_c = nc;
	m_s = ns;
	return r;
}


// Find the lowest prime (or 1) more-or-equal to a given number
bool audio_effect_reverb::is_prime(u32 value)
{
	if(value == 1 || value == 2)
		return value;
	if(!(value & 1))
		return false;
	for(u32 d = 3; d*d <= value; d += 2)
		if(!(value % d))
			return false;
	return true;
}

u32 audio_effect_reverb::find_prime(u32 value)
{
	if(value <= 1)
		return 1;

	while(!is_prime(value))
		value++;

	return value;
}


// The complete effect implementation

void audio_effect_reverb::apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest)
{
	if(m_mode == 0) {
		copy(src, dest);
		return;
	}

	u32 samples = src.available_samples();
	dest.prepare_space(samples);

	for(u32 i=0; i != samples; i++) {
		for(u32 channel = 0; channel != m_channels; channel++)
			m_early_in[channel] = *src.ptrs(channel, i);

		// We start by the early reflection

		for(u32 channel = 0; channel != m_channels; channel++) {
			auto &d = m_early_delays[channel];

			// The input audio is pushed to a reverb buffer and a
			// number of taps are applied to it.

			// The taps each have a delay and a gain, they are set in
			// a table with 15 different presets.  How they were build
			// does not seem documented.  The delays are multiplied by
			// the room size factor, between 0.4 and 3.6, build from
			// the 0-100 room size.

			// Taps are different for left and right, we use left for
			// mono.  We can't just mean left and right because some
			// presets have different numbers of taps.

			// RoomReverb only uses preset 0, we keep the others
			// available though.

			d.push(m_early_in[channel]);
			sample_t w1 = 0;
			const auto &tmap = tap_maps[m_early_tap_setup];
			u32 side = m_ch_type[channel] == T_RIGHT ? 1 : 0;
			for(u32 tap = 0; tap != tmap.m_count[side]; tap++)
				w1 += d.get(m_early_tap_dists[side][tap]) * tmap.m_gain[side][tap];

			// We end up with a current value for the channel and we
			// push it in another delay for cross-channel reflections
			// with the original signal added

			m_early_xdelays[channel].push(w1 + m_early_in[channel]);
			m_early_wet[channel] = w1;
		}

		for(u32 channel = 0; channel != m_channels; channel++) {
			sample_t w2;
			if(m_ch_type[channel] == T_MONO)
				w2 = m_early_wet[channel];

			else {
				// For stereo pairs, once we have the per-channel
				// early reverb signal we use a proportion between
				// that raw signal and all the signals coming from the
				// other channels after they go through an allpass to
				// muddy the phases under 750Hz, as reflections are
				// known to do, without changing the actual levels.
				// That gives us a new composite per-channel signal.
				// The proportion comes from the stereo width knob,
				// where 0 is pure cross-channel reflection and 100 is
				// pure raw signal.
				int other = m_ch_pair[channel];
				w2 = m_wet1 * m_early_wet[channel] + m_wet2 * m_early_cross_allpass.process(m_early_cross_allpass_h[channel], m_early_xdelays[other].get(m_early_xdelays_dist));
			}

			// The per-channel result then goes through an allpass
			// with cutoff at 150Hz (again, phases only), a 4Hz
			// highpass that is essentially a DC cut, and a lowpass
			// set by the knob "early damping".

			m_early_out[channel] = m_early_lpf.process(m_early_lpf_h[channel], m_early_hpf.process(m_early_hpf_h[channel], m_early_diffusion_allpass.process(m_early_diffusion_allpass_h[channel], w2)));
		}

		// The late reverb input is the original signal plus a part of
		// the early reverb signal, depending on the early-to-late
		// level.

		for(u32 channel = 0; channel != m_channels; channel++)
			m_late_in[channel] = m_early_in[channel] + m_actual_early_to_late_level * m_early_out[channel];


		// Then it's the Progenitor 2 reverb, and it's... complicated
		sample_t noise = m_late_noise.process();
		float lfo1 = m_late_lfo1_lpf.process(m_late_lfo1_lpf_h, (m_late_lfo1.process() + m_late_modulation_noise_1*noise)*m_late_wander_actual);
		float lfo2 = m_late_lfo2_lpf.process(m_late_lfo2_lpf_h, m_late_lfo2.process() * m_late_wander_actual);
		noise *= m_late_modulation_noise_2;

		for(u32 channel = 0; channel != m_channels; channel++) {
			sample_t value = m_late_dccut.process(m_late_dccut_h[channel], m_late_in[channel]);
			if(m_ch_type[channel] == T_RIGHT)
				for(auto &ap : m_late_input_diffusion[channel])
					value = ap.process(value, lfo1, (i & 1) ? noise : -noise);
			else
				for(auto &ap : m_late_input_diffusion[channel])
					value = ap.process(value, (i & 1) ? lfo1 : -lfo1, noise);
			m_late_diff[channel] = value;
			for(auto &ap : m_late_cross_diffusion[channel])
				value = ap.process(value);
			m_late_cross[channel] = value;
		}

		for(u32 channel = 0; channel != m_channels; channel++)
			if(m_ch_type[channel] == T_MONO)
				m_late_diff[channel] = m_late_input_damping.process(m_late_input_damping_h[channel], m_late_diff[channel] + m_late_crossfeed * m_late_cross[channel]);
			else
				m_late_diff[channel] = m_late_input_damping.process(m_late_input_damping_h[channel], m_late_diff[channel] + m_late_crossfeed * m_late_cross[m_ch_pair[channel]]);

		for(u32 cchan = 0; cchan != m_channels; cchan++)
			m_late_cross2[cchan] = m_late_step_8[m_ch_pair[cchan]].get(0);

		for(u32 cchan = 0; cchan != m_channels; cchan++) {
			bool right = m_ch_type[cchan] == T_RIGHT;
			sample_t c1 = m_late_cross2[cchan];
			sample_t c2 = m_late_bass.process(m_late_bass_h[cchan], c1);
			sample_t c3 = m_late_damping_2.process(m_late_damping_2_h[cchan], c2);
			sample_t v0 = m_late_diff[cchan] + m_late_loop_decay * (c1 + m_late_bass_boost * c3);
			sample_t v1 = m_late_damping_1.process(m_late_damping_1_h[cchan], v0);
			sample_t v2 = m_late_step_1[cchan].process(v1, right ? -lfo1 : lfo1, right ? -noise : noise);
			sample_t v3 = m_late_step_2[cchan].process(v2);
			sample_t v4 = m_late_step_3[cchan].process(v3, right ? lfo1 : -lfo1, right ? noise : -noise);
			sample_t v5 = m_late_step_4[cchan].process(v4);
			sample_t v6 = m_late_step_5[cchan].process(v5);
			sample_t v7 = m_late_step_6[cchan].process(v6);
			sample_t v8 = m_late_step_7[cchan].process(v7, right ? lfo1 : -lfo1);
			m_late_step_8[cchan].push(v8);
		}

		for(u32 cchan = 0; cchan != m_channels; cchan++) {
			u32 pchan = m_ch_pair[cchan];
			bool right = m_ch_type[cchan] == T_RIGHT;
			sample_t v8 = 0.469 * m_late_step_4[cchan].get(1) +
				0.219 * (m_late_step_6[cchan].get(1)
						 - m_late_step_6[pchan].get(2)
						 + m_late_step_6[cchan].get(3)
						 - m_late_step_4[pchan].get(2)
						 - m_late_step_6[pchan].get(4)) +
				0.064 * (m_late_step_8[cchan].get(2)
						 + m_late_step_5[cchan].get_1(0)
						 + m_late_step_5[cchan].get_2(0)
						 - m_late_step_5[pchan].get_2(1)
						 + m_late_step_7[cchan].get_1(0)
						 + m_late_step_7[cchan].get_2(0)
						 + m_late_step_7[cchan].get_3(0)
						 - m_late_step_7[pchan].get_2(1)) +
				0.045 * m_late_step_8[cchan].get(3);

			if(right)
				v8 -= 0.219 * m_late_step_8[pchan].get(1);

			sample_t v9 = m_late_comb[cchan].process(v8, right ? -lfo2 : lfo2);
			sample_t v10 = m_late_output_lpf.process(m_late_output_lpf_h[cchan], v9);
			sample_t v11 = m_late_final_delay[cchan].process(v10);

			m_late_pre_out[cchan] = v11;
		}

		for(u32 cchan = 0; cchan != m_channels; cchan++)
			if(m_ch_type[cchan] == T_MONO)
				m_late_out[cchan] = m_late_pre_out[cchan];
			else
				m_late_out[cchan] = m_wet1 * m_late_pre_out[cchan] + m_wet2 * m_late_pre_out[m_ch_pair[cchan]];

		for(u32 channel = 0; channel != m_channels; channel++)
			*dest.ptrw(channel, i) = m_early_in[channel] * m_actual_dry_level + m_early_out[channel] * m_actual_early_level + m_late_out[channel] * m_actual_late_level;
	}

	dest.commit(samples);
}
