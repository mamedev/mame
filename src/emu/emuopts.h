// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    emuopts.h

    Options file and command line management.

***************************************************************************/

#ifndef MAME_EMU_EMUOPTS_H
#define MAME_EMU_EMUOPTS_H

#pragma once

#include "options.h"

#define OPTION_PRIORITY_CMDLINE     OPTION_PRIORITY_HIGH + 1
// core options
#define OPTION_SYSTEMNAME           core_options::unadorned(0)
#define OPTION_SOFTWARENAME         core_options::unadorned(1)

// core configuration options
#define OPTION_READCONFIG           "readconfig"
#define OPTION_WRITECONFIG          "writeconfig"

// core search path options
#define OPTION_PLUGINDATAPATH       "homepath"
#define OPTION_MEDIAPATH            "rompath"
#define OPTION_HASHPATH             "hashpath"
#define OPTION_SAMPLEPATH           "samplepath"
#define OPTION_ARTPATH              "artpath"
#define OPTION_CTRLRPATH            "ctrlrpath"
#define OPTION_INIPATH              "inipath"
#define OPTION_FONTPATH             "fontpath"
#define OPTION_CHEATPATH            "cheatpath"
#define OPTION_CROSSHAIRPATH        "crosshairpath"
#define OPTION_PLUGINSPATH          "pluginspath"
#define OPTION_LANGUAGEPATH         "languagepath"
#define OPTION_SWPATH               "swpath"

// core directory options
#define OPTION_CFG_DIRECTORY        "cfg_directory"
#define OPTION_NVRAM_DIRECTORY      "nvram_directory"
#define OPTION_INPUT_DIRECTORY      "input_directory"
#define OPTION_STATE_DIRECTORY      "state_directory"
#define OPTION_SNAPSHOT_DIRECTORY   "snapshot_directory"
#define OPTION_DIFF_DIRECTORY       "diff_directory"
#define OPTION_COMMENT_DIRECTORY    "comment_directory"
#define OPTION_SHARE_DIRECTORY      "share_directory"

// core state/playback options
#define OPTION_STATE                "state"
#define OPTION_AUTOSAVE             "autosave"
#define OPTION_REWIND               "rewind"
#define OPTION_REWIND_CAPACITY      "rewind_capacity"
#define OPTION_PLAYBACK             "playback"
#define OPTION_RECORD               "record"
#define OPTION_EXIT_AFTER_PLAYBACK  "exit_after_playback"
#define OPTION_MNGWRITE             "mngwrite"
#define OPTION_AVIWRITE             "aviwrite"
#define OPTION_WAVWRITE             "wavwrite"
#define OPTION_SNAPNAME             "snapname"
#define OPTION_SNAPSIZE             "snapsize"
#define OPTION_SNAPVIEW             "snapview"
#define OPTION_SNAPBILINEAR         "snapbilinear"
#define OPTION_STATENAME            "statename"
#define OPTION_BURNIN               "burnin"

// core performance options
#define OPTION_AUTOFRAMESKIP        "autoframeskip"
#define OPTION_FRAMESKIP            "frameskip"
#define OPTION_SECONDS_TO_RUN       "seconds_to_run"
#define OPTION_THROTTLE             "throttle"
#define OPTION_SLEEP                "sleep"
#define OPTION_SPEED                "speed"
#define OPTION_REFRESHSPEED         "refreshspeed"
#define OPTION_LOWLATENCY           "lowlatency"

// core render options
#define OPTION_KEEPASPECT           "keepaspect"
#define OPTION_UNEVENSTRETCH        "unevenstretch"
#define OPTION_UNEVENSTRETCHX       "unevenstretchx"
#define OPTION_UNEVENSTRETCHY       "unevenstretchy"
#define OPTION_AUTOSTRETCHXY        "autostretchxy"
#define OPTION_INTOVERSCAN          "intoverscan"
#define OPTION_INTSCALEX            "intscalex"
#define OPTION_INTSCALEY            "intscaley"

// core rotation options
#define OPTION_ROTATE               "rotate"
#define OPTION_ROR                  "ror"
#define OPTION_ROL                  "rol"
#define OPTION_AUTOROR              "autoror"
#define OPTION_AUTOROL              "autorol"
#define OPTION_FLIPX                "flipx"
#define OPTION_FLIPY                "flipy"

// core artwork options
#define OPTION_ARTWORK_CROP         "artwork_crop"
#define OPTION_FALLBACK_ARTWORK     "fallback_artwork"
#define OPTION_OVERRIDE_ARTWORK     "override_artwork"

// core screen options
#define OPTION_BRIGHTNESS           "brightness"
#define OPTION_CONTRAST             "contrast"
#define OPTION_GAMMA                "gamma"
#define OPTION_PAUSE_BRIGHTNESS     "pause_brightness"
#define OPTION_EFFECT               "effect"

// core vector options
#define OPTION_BEAM_WIDTH_MIN       "beam_width_min"
#define OPTION_BEAM_WIDTH_MAX       "beam_width_max"
#define OPTION_BEAM_DOT_SIZE        "beam_dot_size"
#define OPTION_BEAM_INTENSITY_WEIGHT   "beam_intensity_weight"
#define OPTION_FLICKER              "flicker"

// core sound options
#define OPTION_SAMPLERATE           "samplerate"
#define OPTION_SAMPLES              "samples"
#define OPTION_VOLUME               "volume"
#define OPTION_COMPRESSOR           "compressor"
#define OPTION_SPEAKER_REPORT       "speaker_report"

// core input options
#define OPTION_COIN_LOCKOUT         "coin_lockout"
#define OPTION_CTRLR                "ctrlr"
#define OPTION_MOUSE                "mouse"
#define OPTION_JOYSTICK             "joystick"
#define OPTION_LIGHTGUN             "lightgun"
#define OPTION_MULTIKEYBOARD        "multikeyboard"
#define OPTION_MULTIMOUSE           "multimouse"
#define OPTION_STEADYKEY            "steadykey"
#define OPTION_UI_ACTIVE            "ui_active"
#define OPTION_OFFSCREEN_RELOAD     "offscreen_reload"
#define OPTION_JOYSTICK_MAP         "joystick_map"
#define OPTION_JOYSTICK_DEADZONE    "joystick_deadzone"
#define OPTION_JOYSTICK_SATURATION  "joystick_saturation"
#define OPTION_JOYSTICK_THRESHOLD   "joystick_threshold"
#define OPTION_NATURAL_KEYBOARD     "natural"
#define OPTION_JOYSTICK_CONTRADICTORY   "joystick_contradictory"
#define OPTION_COIN_IMPULSE         "coin_impulse"

// input autoenable options
#define OPTION_PADDLE_DEVICE        "paddle_device"
#define OPTION_ADSTICK_DEVICE       "adstick_device"
#define OPTION_PEDAL_DEVICE         "pedal_device"
#define OPTION_DIAL_DEVICE          "dial_device"
#define OPTION_TRACKBALL_DEVICE     "trackball_device"
#define OPTION_LIGHTGUN_DEVICE      "lightgun_device"
#define OPTION_POSITIONAL_DEVICE    "positional_device"
#define OPTION_MOUSE_DEVICE         "mouse_device"

// core debugging options
#define OPTION_LOG                  "log"
#define OPTION_DEBUG                "debug"
#define OPTION_VERBOSE              "verbose"
#define OPTION_OSLOG                "oslog"
#define OPTION_UPDATEINPAUSE        "update_in_pause"
#define OPTION_DEBUGSCRIPT          "debugscript"
#define OPTION_DEBUGLOG             "debuglog"

// core misc options
#define OPTION_DRC                  "drc"
#define OPTION_DRC_USE_C            "drc_use_c"
#define OPTION_DRC_LOG_UML          "drc_log_uml"
#define OPTION_DRC_LOG_NATIVE       "drc_log_native"
#define OPTION_BIOS                 "bios"
#define OPTION_CHEAT                "cheat"
#define OPTION_SKIP_GAMEINFO        "skip_gameinfo"
#define OPTION_UI_FONT              "uifont"
#define OPTION_UI                   "ui"
#define OPTION_RAMSIZE              "ramsize"
#define OPTION_NVRAM_SAVE           "nvram_save"
#define OPTION_PAUSE_GAME_ON_MENU   "pause_game_on_menu"

// core comm options
#define OPTION_COMM_LOCAL_HOST      "comm_localhost"
#define OPTION_COMM_LOCAL_PORT      "comm_localport"
#define OPTION_COMM_REMOTE_HOST     "comm_remotehost"
#define OPTION_COMM_REMOTE_PORT     "comm_remoteport"
#define OPTION_COMM_FRAME_SYNC      "comm_framesync"

#define OPTION_CONFIRM_QUIT         "confirm_quit"
#define OPTION_UI_MOUSE             "ui_mouse"

#define OPTION_AUTOBOOT_COMMAND     "autoboot_command"
#define OPTION_AUTOBOOT_DELAY       "autoboot_delay"
#define OPTION_AUTOBOOT_SCRIPT      "autoboot_script"

#define OPTION_CONSOLE              "console"
#define OPTION_PLUGINS              "plugins"
#define OPTION_PLUGIN               "plugin"
#define OPTION_NO_PLUGIN            "noplugin"

#define OPTION_LANGUAGE             "language"

#define OPTION_HTTP                 "http"
#define OPTION_HTTP_PORT            "http_port"
#define OPTION_HTTP_ROOT            "http_root"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class game_driver;
class device_slot_interface;
class emu_options;

class slot_option
{
public:
	slot_option(emu_options &host, const char *default_value);
	slot_option(const slot_option &that) = delete;
	slot_option(slot_option &&that) = default;

	// accessors
	const std::string &value() const;
	std::string specified_value() const;
	const std::string &bios() const { return m_specified_bios; }
	const std::string &default_card_software() const { return m_default_card_software; }
	bool specified() const { return m_specified; }
	core_options::entry::shared_ptr option_entry() const { return m_entry.lock(); }

	// seters
	void specify(std::string_view text, bool peg_priority = true);
	void specify(std::string &&text, bool peg_priority = true);
	void specify(const char *text, bool peg_priority = true) { specify(std::string_view(text), peg_priority); }
	void set_bios(std::string &&text);
	void set_default_card_software(std::string &&s);

	// instantiates an option entry (don't call outside of emuopts.cpp)
	core_options::entry::shared_ptr setup_option_entry(const char *name);

private:
	void possibly_changed(const std::string &old_value);

	emu_options &                   m_host;
	bool                            m_specified;
	std::string                     m_specified_value;
	std::string                     m_specified_bios;
	std::string                     m_default_card_software;
	std::string                     m_default_value;
	core_options::entry::weak_ptr   m_entry;
};


class image_option
{
public:
	image_option(emu_options &host, const std::string &canonical_instance_name);
	image_option(const image_option &that) = delete;
	image_option(image_option &&that) = default;

	// accessors
	const std::string &canonical_instance_name() const { return m_canonical_instance_name; }
	const std::string &value() const { return m_value; }
	core_options::entry::shared_ptr option_entry() const { return m_entry.lock(); }

	// mutators
	void specify(std::string_view value, bool peg_priority = true);
	void specify(std::string &&value, bool peg_priority = true);
	void specify(const char *value, bool peg_priority = true) { specify(std::string_view(value), peg_priority); }

	// instantiates an option entry (don't call outside of emuopts.cpp)
	core_options::entry::shared_ptr setup_option_entry(std::vector<std::string> &&names);

private:
	emu_options &                   m_host;
	std::string                     m_canonical_instance_name;
	std::string                     m_value;
	core_options::entry::weak_ptr   m_entry;
};


class emu_options : public core_options
{
	friend class slot_option;
	friend class image_option;
public:
	enum ui_option
	{
		UI_CABINET,
		UI_SIMPLE
	};

	enum class option_support
	{
		FULL,                   // full option support
		GENERAL_AND_SYSTEM,     // support for general options and system (no softlist)
		GENERAL_ONLY            // only support for general options
	};

	// construction/destruction
	emu_options(option_support support = option_support::FULL);
	~emu_options();

	// mutation
	void set_system_name(const char *new_system_name) { set_system_name(std::string(new_system_name)); }
	void set_system_name(std::string_view new_system_name) { set_system_name(std::string(new_system_name)); }
	void set_system_name(std::string &&new_system_name);
	void set_software(std::string &&new_software);

	// core options
	const game_driver *system() const { return m_system; }
	const char *system_name() const;
	const std::string &attempted_system_name() const { return m_attempted_system_name; }
	const std::string &software_name() const { return m_software_name; }

	// core configuration options
	bool read_config() const { return bool_value(OPTION_READCONFIG); }
	bool write_config() const { return bool_value(OPTION_WRITECONFIG); }

	// core search path options
	const char *plugin_data_path() const { return value(OPTION_PLUGINDATAPATH); }
	const char *media_path() const { return value(OPTION_MEDIAPATH); }
	const char *hash_path() const { return value(OPTION_HASHPATH); }
	const char *sample_path() const { return value(OPTION_SAMPLEPATH); }
	const char *art_path() const { return value(OPTION_ARTPATH); }
	const char *ctrlr_path() const { return value(OPTION_CTRLRPATH); }
	const char *ini_path() const { return value(OPTION_INIPATH); }
	const char *font_path() const { return value(OPTION_FONTPATH); }
	const char *cheat_path() const { return value(OPTION_CHEATPATH); }
	const char *crosshair_path() const { return value(OPTION_CROSSHAIRPATH); }
	const char *plugins_path() const { return value(OPTION_PLUGINSPATH); }
	const char *language_path() const { return value(OPTION_LANGUAGEPATH); }
	const char *sw_path() const { return value(OPTION_SWPATH); }

	// core directory options
	const char *cfg_directory() const { return value(OPTION_CFG_DIRECTORY); }
	const char *nvram_directory() const { return value(OPTION_NVRAM_DIRECTORY); }
	const char *input_directory() const { return value(OPTION_INPUT_DIRECTORY); }
	const char *state_directory() const { return value(OPTION_STATE_DIRECTORY); }
	const char *snapshot_directory() const { return value(OPTION_SNAPSHOT_DIRECTORY); }
	const char *diff_directory() const { return value(OPTION_DIFF_DIRECTORY); }
	const char *comment_directory() const { return value(OPTION_COMMENT_DIRECTORY); }
	const char *share_directory() const { return value(OPTION_SHARE_DIRECTORY); }

	// core state/playback options
	const char *state() const { return value(OPTION_STATE); }
	bool autosave() const { return bool_value(OPTION_AUTOSAVE); }
	int rewind() const { return bool_value(OPTION_REWIND); }
	int rewind_capacity() const { return int_value(OPTION_REWIND_CAPACITY); }
	const char *playback() const { return value(OPTION_PLAYBACK); }
	const char *record() const { return value(OPTION_RECORD); }
	bool exit_after_playback() const { return bool_value(OPTION_EXIT_AFTER_PLAYBACK); }
	const char *mng_write() const { return value(OPTION_MNGWRITE); }
	const char *avi_write() const { return value(OPTION_AVIWRITE); }
	const char *wav_write() const { return value(OPTION_WAVWRITE); }
	const char *snap_name() const { return value(OPTION_SNAPNAME); }
	const char *snap_size() const { return value(OPTION_SNAPSIZE); }
	const char *snap_view() const { return value(OPTION_SNAPVIEW); }
	bool snap_bilinear() const { return bool_value(OPTION_SNAPBILINEAR); }
	const char *state_name() const { return value(OPTION_STATENAME); }
	bool burnin() const { return bool_value(OPTION_BURNIN); }

	// core performance options
	bool auto_frameskip() const { return bool_value(OPTION_AUTOFRAMESKIP); }
	int frameskip() const { return int_value(OPTION_FRAMESKIP); }
	int seconds_to_run() const { return int_value(OPTION_SECONDS_TO_RUN); }
	bool throttle() const { return bool_value(OPTION_THROTTLE); }
	bool sleep() const { return m_sleep; }
	float speed() const { return float_value(OPTION_SPEED); }
	bool refresh_speed() const { return m_refresh_speed; }
	bool low_latency() const { return bool_value(OPTION_LOWLATENCY); }

	// core render options
	bool keep_aspect() const { return bool_value(OPTION_KEEPASPECT); }
	bool uneven_stretch() const { return bool_value(OPTION_UNEVENSTRETCH); }
	bool uneven_stretch_x() const { return bool_value(OPTION_UNEVENSTRETCHX); }
	bool uneven_stretch_y() const { return bool_value(OPTION_UNEVENSTRETCHY); }
	bool auto_stretch_xy() const { return bool_value(OPTION_AUTOSTRETCHXY); }
	bool int_overscan() const { return bool_value(OPTION_INTOVERSCAN); }
	int int_scale_x() const { return int_value(OPTION_INTSCALEX); }
	int int_scale_y() const { return int_value(OPTION_INTSCALEY); }

	// core rotation options
	bool rotate() const { return bool_value(OPTION_ROTATE); }
	bool ror() const { return bool_value(OPTION_ROR); }
	bool rol() const { return bool_value(OPTION_ROL); }
	bool auto_ror() const { return bool_value(OPTION_AUTOROR); }
	bool auto_rol() const { return bool_value(OPTION_AUTOROL); }
	bool flipx() const { return bool_value(OPTION_FLIPX); }
	bool flipy() const { return bool_value(OPTION_FLIPY); }

	// core artwork options
	bool artwork_crop() const { return bool_value(OPTION_ARTWORK_CROP); }
	const char *fallback_artwork() const { return value(OPTION_FALLBACK_ARTWORK); }
	const char *override_artwork() const { return value(OPTION_OVERRIDE_ARTWORK); }

	// core screen options
	float brightness() const { return float_value(OPTION_BRIGHTNESS); }
	float contrast() const { return float_value(OPTION_CONTRAST); }
	float gamma() const { return float_value(OPTION_GAMMA); }
	float pause_brightness() const { return float_value(OPTION_PAUSE_BRIGHTNESS); }
	const char *effect() const { return value(OPTION_EFFECT); }

	// core vector options
	float beam_width_min() const { return float_value(OPTION_BEAM_WIDTH_MIN); }
	float beam_width_max() const { return float_value(OPTION_BEAM_WIDTH_MAX); }
	float beam_dot_size() const { return float_value(OPTION_BEAM_DOT_SIZE); }
	float beam_intensity_weight() const { return float_value(OPTION_BEAM_INTENSITY_WEIGHT); }
	float flicker() const { return float_value(OPTION_FLICKER); }

	// core sound options
	int sample_rate() const { return int_value(OPTION_SAMPLERATE); }
	bool samples() const { return bool_value(OPTION_SAMPLES); }
	int volume() const { return int_value(OPTION_VOLUME); }
	bool compressor() const { return bool_value(OPTION_COMPRESSOR); }
	int speaker_report() const { return int_value(OPTION_SPEAKER_REPORT); }

	// core input options
	bool coin_lockout() const { return bool_value(OPTION_COIN_LOCKOUT); }
	const char *ctrlr() const { return value(OPTION_CTRLR); }
	bool mouse() const { return bool_value(OPTION_MOUSE); }
	bool joystick() const { return bool_value(OPTION_JOYSTICK); }
	bool lightgun() const { return bool_value(OPTION_LIGHTGUN); }
	bool multi_keyboard() const { return bool_value(OPTION_MULTIKEYBOARD); }
	bool multi_mouse() const { return bool_value(OPTION_MULTIMOUSE); }
	const char *paddle_device() const { return value(OPTION_PADDLE_DEVICE); }
	const char *adstick_device() const { return value(OPTION_ADSTICK_DEVICE); }
	const char *pedal_device() const { return value(OPTION_PEDAL_DEVICE); }
	const char *dial_device() const { return value(OPTION_DIAL_DEVICE); }
	const char *trackball_device() const { return value(OPTION_TRACKBALL_DEVICE); }
	const char *lightgun_device() const { return value(OPTION_LIGHTGUN_DEVICE); }
	const char *positional_device() const { return value(OPTION_POSITIONAL_DEVICE); }
	const char *mouse_device() const { return value(OPTION_MOUSE_DEVICE); }
	const char *joystick_map() const { return value(OPTION_JOYSTICK_MAP); }
	float joystick_deadzone() const { return float_value(OPTION_JOYSTICK_DEADZONE); }
	float joystick_saturation() const { return float_value(OPTION_JOYSTICK_SATURATION); }
	float joystick_threshold() const { return float_value(OPTION_JOYSTICK_THRESHOLD); }
	bool steadykey() const { return bool_value(OPTION_STEADYKEY); }
	bool ui_active() const { return bool_value(OPTION_UI_ACTIVE); }
	bool offscreen_reload() const { return bool_value(OPTION_OFFSCREEN_RELOAD); }
	bool natural_keyboard() const { return bool_value(OPTION_NATURAL_KEYBOARD); }
	bool joystick_contradictory() const { return m_joystick_contradictory; }
	int coin_impulse() const { return m_coin_impulse; }

	// core debugging options
	bool log() const { return bool_value(OPTION_LOG); }
	bool debug() const { return bool_value(OPTION_DEBUG); }
	bool verbose() const { return bool_value(OPTION_VERBOSE); }
	bool oslog() const { return bool_value(OPTION_OSLOG); }
	const char *debug_script() const { return value(OPTION_DEBUGSCRIPT); }
	bool update_in_pause() const { return bool_value(OPTION_UPDATEINPAUSE); }
	bool debuglog() const { return bool_value(OPTION_DEBUGLOG); }

	// core misc options
	bool drc() const { return bool_value(OPTION_DRC); }
	bool drc_use_c() const { return bool_value(OPTION_DRC_USE_C); }
	bool drc_log_uml() const { return bool_value(OPTION_DRC_LOG_UML); }
	bool drc_log_native() const { return bool_value(OPTION_DRC_LOG_NATIVE); }
	const char *bios() const { return value(OPTION_BIOS); }
	bool cheat() const { return bool_value(OPTION_CHEAT); }
	bool skip_gameinfo() const { return bool_value(OPTION_SKIP_GAMEINFO); }
	const char *ui_font() const { return value(OPTION_UI_FONT); }
	ui_option ui() const { return m_ui; }
	const char *ram_size() const { return value(OPTION_RAMSIZE); }
	bool nvram_save() const { return bool_value(OPTION_NVRAM_SAVE); }
	bool pause_game_on_menu() const { return bool_value(OPTION_PAUSE_GAME_ON_MENU); }

	// core comm options
	const char *comm_localhost() const { return value(OPTION_COMM_LOCAL_HOST); }
	const char *comm_localport() const { return value(OPTION_COMM_LOCAL_PORT); }
	const char *comm_remotehost() const { return value(OPTION_COMM_REMOTE_HOST); }
	const char *comm_remoteport() const { return value(OPTION_COMM_REMOTE_PORT); }
	bool comm_framesync() const { return bool_value(OPTION_COMM_FRAME_SYNC); }


	bool confirm_quit() const { return bool_value(OPTION_CONFIRM_QUIT); }
	bool ui_mouse() const { return bool_value(OPTION_UI_MOUSE); }

	const char *autoboot_command() const { return value(OPTION_AUTOBOOT_COMMAND); }
	int autoboot_delay() const { return int_value(OPTION_AUTOBOOT_DELAY); }
	const char *autoboot_script() const { return value(OPTION_AUTOBOOT_SCRIPT); }

	bool console() const { return bool_value(OPTION_CONSOLE); }

	bool plugins() const { return bool_value(OPTION_PLUGINS); }

	const char *plugin() const { return value(OPTION_PLUGIN); }
	const char *no_plugin() const { return value(OPTION_NO_PLUGIN); }

	const char *language() const { return value(OPTION_LANGUAGE); }

	// Web server specific options
	bool  http() const { return bool_value(OPTION_HTTP); }
	short http_port() const { return int_value(OPTION_HTTP_PORT); }
	const char *http_root() const { return value(OPTION_HTTP_ROOT); }

	// slots and devices - the values for these are stored outside of the core_options
	// structure
	const ::slot_option &slot_option(const std::string &device_name) const;
	::slot_option &slot_option(const std::string &device_name);
	const ::slot_option *find_slot_option(const std::string &device_name) const;
	::slot_option *find_slot_option(const std::string &device_name);
	bool has_slot_option(const std::string &device_name) const { return find_slot_option(device_name) ? true : false; }
	const ::image_option &image_option(const std::string &device_name) const;
	::image_option &image_option(const std::string &device_name);
	bool has_image_option(const std::string &device_name) const { return m_image_options.find(device_name) != m_image_options.end(); }

protected:
	virtual void command_argument_processed() override;

private:
	struct software_options
	{
		std::unordered_map<std::string, std::string>    slot;
		std::unordered_map<std::string, std::string>    slot_defaults;
		std::unordered_map<std::string, std::string>    image;
	};

	// slot/image/softlist calculus
	software_options evaluate_initial_softlist_options(const std::string &software_identifier);
	void update_slot_and_image_options();
	bool add_and_remove_slot_options();
	bool add_and_remove_image_options();
	void reevaluate_default_card_software();
	std::string get_default_card_software(device_slot_interface &slot);

	// static list of options entries
	static const options_entry                          s_option_entries[];

	// the basics
	option_support                                      m_support;
	const game_driver *                                 m_system;

	// slots and devices
	std::unordered_map<std::string, ::slot_option>      m_slot_options;
	std::unordered_map<std::string, ::image_option>     m_image_options_canonical;
	std::unordered_map<std::string, ::image_option *>   m_image_options;

	// cached options, for scenarios where parsing core_options is too slow
	int                                                 m_coin_impulse;
	bool                                                m_joystick_contradictory;
	bool                                                m_sleep;
	bool                                                m_refresh_speed;
	ui_option                                           m_ui;

	// special option; the system name we tried to specify
	std::string                                         m_attempted_system_name;

	// special option; the software set name that we did specify
	std::string                                         m_software_name;
};

// takes an existing emu_options and adds system specific options
void osd_setup_osd_specific_emu_options(emu_options &opts);

#endif  // MAME_EMU_EMUOPTS_H
