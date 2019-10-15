// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdepend.h

    OS-dependent code interface.

*******************************************************************c********/

#pragma once

#ifndef MAME_OSD_LIB_OSDOBJ_COMMON_H
#define MAME_OSD_LIB_OSDOBJ_COMMON_H

#include "osdepend.h"
#include "watchdog.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"
#include "modules/input/input_module.h"
#include "modules/sound/sound_module.h"
#include "modules/debugger/debug_module.h"
#include "modules/netdev/netdev_module.h"
#include "modules/midi/midi_module.h"
#include "modules/output/output_module.h"
#include "modules/monitor/monitor_module.h"
#include "emuopts.h"
#include "../frontend/mame/ui/menuitem.h"

//============================================================
//  Defines
//============================================================

#define OSDOPTION_UIMODEKEY             "uimodekey"

#define OSDCOMMAND_LIST_MIDI_DEVICES    "listmidi"
#define OSDCOMMAND_LIST_NETWORK_ADAPTERS "listnetwork"

#define OSDOPTION_DEBUGGER              "debugger"
#define OSDOPTION_DEBUGGER_PORT         "debugger_port"
#define OSDOPTION_DEBUGGER_FONT         "debugger_font"
#define OSDOPTION_DEBUGGER_FONT_SIZE    "debugger_font_size"
#define OSDOPTION_WATCHDOG              "watchdog"

#define OSDOPTION_NUMPROCESSORS         "numprocessors"
#define OSDOPTION_BENCH                 "bench"

#define OSDOPTION_VIDEO                 "video"
#define OSDOPTION_NUMSCREENS            "numscreens"
#define OSDOPTION_WINDOW                "window"
#define OSDOPTION_MAXIMIZE              "maximize"
#define OSDOPTION_WAITVSYNC             "waitvsync"
#define OSDOPTION_SYNCREFRESH           "syncrefresh"

#define OSDOPTION_SCREEN                "screen"
#define OSDOPTION_ASPECT                "aspect"
#define OSDOPTION_RESOLUTION            "resolution"
#define OSDOPTION_VIEW                  "view"

#define OSDOPTION_SWITCHRES             "switchres"

#define OSDOPTION_FILTER                "filter"
#define OSDOPTION_PRESCALE              "prescale"

#define OSDOPTION_SHADER_MAME           "glsl_shader_mame"
#define OSDOPTION_SHADER_SCREEN         "glsl_shader_screen"
#define OSDOPTION_GLSL_FILTER           "gl_glsl_filter"
#define OSDOPTION_GL_GLSL               "gl_glsl"
#define OSDOPTION_GL_PBO                "gl_pbo"
#define OSDOPTION_GL_VBO                "gl_vbo"
#define OSDOPTION_GL_NOTEXTURERECT      "gl_notexturerect"
#define OSDOPTION_GL_FORCEPOW2TEXTURE   "gl_forcepow2texture"

#define OSDOPTION_SOUND                 "sound"
#define OSDOPTION_AUDIO_LATENCY         "audio_latency"

#define OSDOPTION_PA_API                "pa_api"
#define OSDOPTION_PA_DEVICE             "pa_device"
#define OSDOPTION_PA_LATENCY            "pa_latency"

#define OSDOPTION_AUDIO_OUTPUT          "audio_output"
#define OSDOPTION_AUDIO_EFFECT          "audio_effect"

#define OSDOPTVAL_AUTO                  "auto"
#define OSDOPTVAL_NONE                  "none"

#define OSDOPTION_BGFX_PATH             "bgfx_path"
#define OSDOPTION_BGFX_BACKEND          "bgfx_backend"
#define OSDOPTION_BGFX_DEBUG            "bgfx_debug"
#define OSDOPTION_BGFX_SCREEN_CHAINS    "bgfx_screen_chains"
#define OSDOPTION_BGFX_SHADOW_MASK      "bgfx_shadow_mask"
#define OSDOPTION_BGFX_LUT              "bgfx_lut"
#define OSDOPTION_BGFX_AVI_NAME         "bgfx_avi_name"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class osd_options : public emu_options
{
public:
	// construction/destruction
	osd_options();

	// keyboard mapping
	const char *ui_mode_key() const { return value(OSDOPTION_UIMODEKEY); }

	// debugging options
	const char *debugger() const { return value(OSDOPTION_DEBUGGER); }
	int debugger_port() const { return int_value(OSDOPTION_DEBUGGER_PORT); }
	const char *debugger_font() const { return value(OSDOPTION_DEBUGGER_FONT); }
	float debugger_font_size() const { return float_value(OSDOPTION_DEBUGGER_FONT_SIZE); }
	int watchdog() const { return int_value(OSDOPTION_WATCHDOG); }

	// performance options
	const char *numprocessors() const { return value(OSDOPTION_NUMPROCESSORS); }
	int bench() const { return int_value(OSDOPTION_BENCH); }

	// video options
	const char *video() const { return value(OSDOPTION_VIDEO); }
	int numscreens() const { return int_value(OSDOPTION_NUMSCREENS); }
	bool window() const { return bool_value(OSDOPTION_WINDOW); }
	bool maximize() const { return bool_value(OSDOPTION_MAXIMIZE); }
	bool wait_vsync() const { return bool_value(OSDOPTION_WAITVSYNC); }
	bool sync_refresh() const { return bool_value(OSDOPTION_SYNCREFRESH); }

	// per-window options
	const char *screen() const { return value(OSDOPTION_SCREEN); }
	const char *aspect() const { return value(OSDOPTION_ASPECT); }
	const char *resolution() const { return value(OSDOPTION_RESOLUTION); }
	const char *view() const { return value(OSDOPTION_VIEW); }
	const char *screen(int index) const { return value(string_format("%s%d", OSDOPTION_SCREEN, index).c_str()); }
	const char *aspect(int index) const { return value(string_format("%s%d", OSDOPTION_ASPECT, index).c_str()); }
	const char *resolution(int index) const { return value(string_format("%s%d", OSDOPTION_RESOLUTION, index).c_str()); }
	const char *view(int index) const { return value(string_format("%s%d", OSDOPTION_VIEW, index).c_str()); }

	// full screen options
	bool switch_res() const { return bool_value(OSDOPTION_SWITCHRES); }

	// accelerated video options
	bool filter() const { return bool_value(OSDOPTION_FILTER); }
	int prescale() const { return int_value(OSDOPTION_PRESCALE); }

	// OpenGL specific options
	bool gl_force_pow2_texture() const { return bool_value(OSDOPTION_GL_FORCEPOW2TEXTURE); }
	bool gl_no_texture_rect() const { return bool_value(OSDOPTION_GL_NOTEXTURERECT); }
	bool gl_vbo() const { return bool_value(OSDOPTION_GL_VBO); }
	bool gl_pbo() const { return bool_value(OSDOPTION_GL_PBO); }
	bool gl_glsl() const { return bool_value(OSDOPTION_GL_GLSL); }
	int glsl_filter() const { return int_value(OSDOPTION_GLSL_FILTER); }
	const char *shader_mame(int index) const { return value(string_format("%s%d", OSDOPTION_SHADER_MAME, index).c_str()); }
	const char *shader_screen(int index) const { return value(string_format("%s%d", OSDOPTION_SHADER_SCREEN, index).c_str()); }

	// sound options
	const char *sound() const { return value(OSDOPTION_SOUND); }
	int audio_latency() const { return int_value(OSDOPTION_AUDIO_LATENCY); }

	// CoreAudio specific options
	const char *audio_output() const { return value(OSDOPTION_AUDIO_OUTPUT); }
	const char *audio_effect(int index) const { return value(string_format("%s%d", OSDOPTION_AUDIO_EFFECT, index).c_str()); }

	// BGFX specific options
	const char *bgfx_path() const { return value(OSDOPTION_BGFX_PATH); }
	const char *bgfx_backend() const { return value(OSDOPTION_BGFX_BACKEND); }
	bool bgfx_debug() const { return bool_value(OSDOPTION_BGFX_DEBUG); }
	const char *bgfx_screen_chains() const { return value(OSDOPTION_BGFX_SCREEN_CHAINS); }
	const char *bgfx_shadow_mask() const { return value(OSDOPTION_BGFX_SHADOW_MASK); }
	const char *bgfx_lut() const { return value(OSDOPTION_BGFX_LUT); }
	const char *bgfx_avi_name() const { return value(OSDOPTION_BGFX_AVI_NAME); }

	// PortAudio options
	const char *pa_api() const { return value(OSDOPTION_PA_API); }
	const char *pa_device() const { return value(OSDOPTION_PA_DEVICE); }
	const float pa_latency() const { return float_value(OSDOPTION_PA_LATENCY); }

	static const options_entry s_option_entries[];
};

// ======================> osd_interface
class osd_window;

// description of the currently-running machine
class osd_common_t : public osd_interface, osd_output
{
public:
	// construction/destruction
	osd_common_t(osd_options &options);
	virtual ~osd_common_t();

	// FIXME: simply option handling
	virtual void register_options();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;

	// debugger overridables
	virtual void init_debugger() override;
	virtual void wait_for_debugger(device_t &device, bool firststop) override;

	// audio overridables
	virtual void update_audio_stream(const int16_t *buffer, int samples_this_frame) override;
	virtual void set_mastervolume(int attenuation) override;
	virtual bool no_sound() override;

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist) override;

	// video overridables
	virtual void add_audio_to_recording(const int16_t *buffer, int samples_this_frame) override;
	virtual std::vector<ui::menu_item> get_slider_list() override;

	// command option overrides
	virtual bool execute_command(const char *command) override;

	virtual osd_font::ptr font_alloc() override { return m_font_module->font_alloc(); }
	virtual bool get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result) override { return m_font_module->get_font_families(font_path, result); }

	virtual osd_midi_device *create_midi_device() override { return m_midi->create_midi_device(); }

	// FIXME: everything below seems to be osd specific and not part of
	//        this INTERFACE but part of the osd IMPLEMENTATION

	// getters
	running_machine &machine() const { assert(m_machine != nullptr); return *m_machine; }

	virtual void debugger_update();

	virtual void init_subsystems();

	virtual bool video_init();
	virtual void video_register();
	virtual bool window_init();

	virtual void input_resume();

	virtual void exit_subsystems();
	virtual void video_exit();
	virtual void window_exit();

	virtual void osd_exit();

	virtual void video_options_add(const char *name, void *type);

	virtual osd_options &options() { return m_options; }

	// osd_output interface ...
	virtual void output_callback(osd_output_channel channel, const util::format_argument_pack<std::ostream> &args)  override;
	bool verbose() const { return m_print_verbose; }
	virtual void set_verbose(bool print_verbose) override { m_print_verbose = print_verbose; }

	void notify(const char *outname, int32_t value) const { m_output->notify(outname, value); }

	static std::list<std::shared_ptr<osd_window>> s_window_list;

protected:
	virtual bool input_init();
	virtual void input_pause();

	virtual void build_slider_list() { }
	virtual void update_slider_list() { }

private:
	// internal state
	running_machine *   m_machine;
	osd_options& m_options;

	bool m_print_verbose;

	osd_module_manager m_mod_man;
	font_module *m_font_module;

	void update_option(const std::string &key, std::vector<const char *> &values);
	// FIXME: should be elsewhere
	osd_module *select_module_options(const core_options &opts, const std::string &opt_name)
	{
		std::string opt_val = opts.exists(opt_name) ? opts.value(opt_name.c_str()) : "";
		if (opt_val.compare("auto")==0)
			opt_val = "";
		else if (!m_mod_man.type_has_name(opt_name.c_str(), opt_val.c_str()))
		{
			osd_printf_warning("Value %s not supported for option %s - falling back to auto\n", opt_val, opt_name);
			opt_val = "";
		}
		return m_mod_man.select_module(opt_name.c_str(), opt_val.c_str());
	}

	template<class C>
	C select_module_options(const core_options &opts, const std::string &opt_name)
	{
		return dynamic_cast<C>(select_module_options(opts, opt_name));
	}

protected:
	sound_module*   m_sound;
	debug_module*   m_debugger;
	midi_module*    m_midi;
	input_module*   m_keyboard_input;
	input_module*   m_mouse_input;
	input_module*   m_lightgun_input;
	input_module*   m_joystick_input;
	output_module*  m_output;
	monitor_module* m_monitor_module;
	std::unique_ptr<osd_watchdog> m_watchdog;
	std::vector<ui::menu_item> m_sliders;

private:
	std::vector<const char *> m_video_names;
	std::unordered_map<std::string, std::string> m_option_descs;
};


// this template function creates a stub which constructs a debugger
template<class _DeviceClass>
debug_module *osd_debugger_creator()
{
	return global_alloc(_DeviceClass());
}

#endif  // MAME_OSD_LIB_OSDOBJ_COMMON_H
