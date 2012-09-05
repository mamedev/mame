/***************************************************************************

    emuopts.h

    Options file and command line management.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMUOPTS_H__
#define __EMUOPTS_H__

#include "options.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// option priorities
enum
{
	// command-line options are HIGH priority
	OPTION_PRIORITY_CMDLINE = OPTION_PRIORITY_HIGH,

	// INI-based options are NORMAL priority, in increasing order:
	OPTION_PRIORITY_INI = OPTION_PRIORITY_NORMAL,
	OPTION_PRIORITY_MAME_INI,
	OPTION_PRIORITY_DEBUG_INI,
	OPTION_PRIORITY_ORIENTATION_INI,
	OPTION_PRIORITY_VECTOR_INI,
	OPTION_PRIORITY_SOURCE_INI,
	OPTION_PRIORITY_GPARENT_INI,
	OPTION_PRIORITY_PARENT_INI,
	OPTION_PRIORITY_DRIVER_INI
};

// core options
#define OPTION_SYSTEMNAME			core_options::unadorned(0)
#define OPTION_SOFTWARENAME			core_options::unadorned(1)

// core configuration options
#define OPTION_READCONFIG			"readconfig"
#define OPTION_WRITECONFIG			"writeconfig"

// core search path options
#define OPTION_MEDIAPATH			"rompath"
#define OPTION_HASHPATH				"hashpath"
#define OPTION_SAMPLEPATH			"samplepath"
#define OPTION_ARTPATH				"artpath"
#define OPTION_CTRLRPATH			"ctrlrpath"
#define OPTION_INIPATH				"inipath"
#define OPTION_FONTPATH				"fontpath"
#define OPTION_CHEATPATH			"cheatpath"
#define OPTION_CROSSHAIRPATH		"crosshairpath"

// core directory options
#define OPTION_CFG_DIRECTORY		"cfg_directory"
#define OPTION_NVRAM_DIRECTORY		"nvram_directory"
#define OPTION_MEMCARD_DIRECTORY	"memcard_directory"
#define OPTION_INPUT_DIRECTORY		"input_directory"
#define OPTION_STATE_DIRECTORY		"state_directory"
#define OPTION_SNAPSHOT_DIRECTORY	"snapshot_directory"
#define OPTION_DIFF_DIRECTORY		"diff_directory"
#define OPTION_COMMENT_DIRECTORY	"comment_directory"

// core state/playback options
#define OPTION_STATE				"state"
#define OPTION_AUTOSAVE				"autosave"
#define OPTION_PLAYBACK				"playback"
#define OPTION_RECORD				"record"
#define OPTION_MNGWRITE				"mngwrite"
#define OPTION_AVIWRITE				"aviwrite"
#define OPTION_WAVWRITE				"wavwrite"
#define OPTION_SNAPNAME				"snapname"
#define OPTION_SNAPSIZE				"snapsize"
#define OPTION_SNAPVIEW				"snapview"
#define OPTION_BURNIN				"burnin"

// core performance options
#define OPTION_AUTOFRAMESKIP		"autoframeskip"
#define OPTION_FRAMESKIP			"frameskip"
#define OPTION_SECONDS_TO_RUN		"seconds_to_run"
#define OPTION_THROTTLE				"throttle"
#define OPTION_SLEEP				"sleep"
#define OPTION_SPEED				"speed"
#define OPTION_REFRESHSPEED			"refreshspeed"

// core rotation options
#define OPTION_ROTATE				"rotate"
#define OPTION_ROR					"ror"
#define OPTION_ROL					"rol"
#define OPTION_AUTOROR				"autoror"
#define OPTION_AUTOROL				"autorol"
#define OPTION_FLIPX				"flipx"
#define OPTION_FLIPY				"flipy"

// core artwork options
#define OPTION_ARTWORK_CROP			"artwork_crop"
#define OPTION_USE_BACKDROPS		"use_backdrops"
#define OPTION_USE_OVERLAYS			"use_overlays"
#define OPTION_USE_BEZELS			"use_bezels"
#define OPTION_USE_CPANELS			"use_cpanels"
#define OPTION_USE_MARQUEES			"use_marquees"

// core screen options
#define OPTION_BRIGHTNESS			"brightness"
#define OPTION_CONTRAST				"contrast"
#define OPTION_GAMMA				"gamma"
#define OPTION_PAUSE_BRIGHTNESS		"pause_brightness"
#define OPTION_EFFECT				"effect"

// core vector options
#define OPTION_ANTIALIAS			"antialias"
#define OPTION_BEAM					"beam"
#define OPTION_FLICKER				"flicker"

// core sound options
#define OPTION_SOUND				"sound"
#define OPTION_SAMPLERATE			"samplerate"
#define OPTION_SAMPLES				"samples"
#define OPTION_VOLUME				"volume"

// core input options
#define OPTION_COIN_LOCKOUT			"coin_lockout"
#define OPTION_CTRLR				"ctrlr"
#define OPTION_MOUSE				"mouse"
#define OPTION_JOYSTICK				"joystick"
#define OPTION_LIGHTGUN				"lightgun"
#define OPTION_MULTIKEYBOARD		"multikeyboard"
#define OPTION_MULTIMOUSE			"multimouse"
#define OPTION_STEADYKEY			"steadykey"
#define OPTION_UI_ACTIVE			"ui_active"
#define OPTION_OFFSCREEN_RELOAD		"offscreen_reload"
#define OPTION_JOYSTICK_MAP			"joystick_map"
#define OPTION_JOYSTICK_DEADZONE	"joystick_deadzone"
#define OPTION_JOYSTICK_SATURATION	"joystick_saturation"
#define OPTION_NATURAL_KEYBOARD		"natural"
#define OPTION_JOYSTICK_CONTRADICTORY	"joystick_contradictory"
#define OPTION_COIN_IMPULSE			"coin_impulse"

// input autoenable options
#define OPTION_PADDLE_DEVICE		"paddle_device"
#define OPTION_ADSTICK_DEVICE		"adstick_device"
#define OPTION_PEDAL_DEVICE			"pedal_device"
#define OPTION_DIAL_DEVICE			"dial_device"
#define OPTION_TRACKBALL_DEVICE		"trackball_device"
#define OPTION_LIGHTGUN_DEVICE		"lightgun_device"
#define OPTION_POSITIONAL_DEVICE	"positional_device"
#define OPTION_MOUSE_DEVICE			"mouse_device"

// core debugging options
#define OPTION_LOG					"log"
#define OPTION_VERBOSE				"verbose"
#define OPTION_UPDATEINPAUSE		"update_in_pause"
#define OPTION_DEBUG				"debug"
#define OPTION_DEBUG_INTERNAL		"debug_internal"
#define OPTION_DEBUGSCRIPT			"debugscript"

// core misc options
#define OPTION_BIOS					"bios"
#define OPTION_CHEAT				"cheat"
#define OPTION_SKIP_GAMEINFO		"skip_gameinfo"
#define OPTION_UI_FONT				"uifont"
#define OPTION_RAMSIZE				"ramsize"

#define OPTION_CONFIRM_QUIT			"confirm_quit"
#define OPTION_UI_MOUSE				"ui_mouse"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward references
struct game_driver;


class emu_options : public core_options
{
	static const UINT32 OPTION_FLAG_DEVICE = 0x80000000;

public:
	// construction/destruction
	emu_options();

	// parsing wrappers
	bool parse_command_line(int argc, char *argv[], astring &error_string);
	void parse_standard_inis(astring &error_string);
	bool parse_slot_devices(int argc, char *argv[], astring &error_string, const char *name, const char *value);

	// core options
	const char *system_name() const { return value(OPTION_SYSTEMNAME); }
	const char *software_name() const { return value(OPTION_SOFTWARENAME); }
	const game_driver *system() const;
	void set_system_name(const char *name);

	// core configuration options
	bool read_config() const { return bool_value(OPTION_READCONFIG); }
	bool write_config() const { return bool_value(OPTION_WRITECONFIG); }

	// core search path options
	const char *media_path() const { return value(OPTION_MEDIAPATH); }
	const char *hash_path() const { return value(OPTION_HASHPATH); }
	const char *sample_path() const { return value(OPTION_SAMPLEPATH); }
	const char *art_path() const { return value(OPTION_ARTPATH); }
	const char *ctrlr_path() const { return value(OPTION_CTRLRPATH); }
	const char *ini_path() const { return value(OPTION_INIPATH); }
	const char *font_path() const { return value(OPTION_FONTPATH); }
	const char *cheat_path() const { return value(OPTION_CHEATPATH); }
	const char *crosshair_path() const { return value(OPTION_CROSSHAIRPATH); }

	// core directory options
	const char *cfg_directory() const { return value(OPTION_CFG_DIRECTORY); }
	const char *nvram_directory() const { return value(OPTION_NVRAM_DIRECTORY); }
	const char *memcard_directory() const { return value(OPTION_MEMCARD_DIRECTORY); }
	const char *input_directory() const { return value(OPTION_INPUT_DIRECTORY); }
	const char *state_directory() const { return value(OPTION_STATE_DIRECTORY); }
	const char *snapshot_directory() const { return value(OPTION_SNAPSHOT_DIRECTORY); }
	const char *diff_directory() const { return value(OPTION_DIFF_DIRECTORY); }
	const char *comment_directory() const { return value(OPTION_COMMENT_DIRECTORY); }

	// core state/playback options
	const char *state() const { return value(OPTION_STATE); }
	bool autosave() const { return bool_value(OPTION_AUTOSAVE); }
	const char *playback() const { return value(OPTION_PLAYBACK); }
	const char *record() const { return value(OPTION_RECORD); }
	const char *mng_write() const { return value(OPTION_MNGWRITE); }
	const char *avi_write() const { return value(OPTION_AVIWRITE); }
	const char *wav_write() const { return value(OPTION_WAVWRITE); }
	const char *snap_name() const { return value(OPTION_SNAPNAME); }
	const char *snap_size() const { return value(OPTION_SNAPSIZE); }
	const char *snap_view() const { return value(OPTION_SNAPVIEW); }
	bool burnin() const { return bool_value(OPTION_BURNIN); }

	// core performance options
	bool auto_frameskip() const { return bool_value(OPTION_AUTOFRAMESKIP); }
	int frameskip() const { return int_value(OPTION_FRAMESKIP); }
	int seconds_to_run() const { return int_value(OPTION_SECONDS_TO_RUN); }
	bool throttle() const { return bool_value(OPTION_THROTTLE); }
	bool sleep() const { return bool_value(OPTION_SLEEP); }
	float speed() const { return float_value(OPTION_SPEED); }
	bool refresh_speed() const { return bool_value(OPTION_REFRESHSPEED); }

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
	bool use_backdrops() const { return bool_value(OPTION_USE_BACKDROPS); }
	bool use_overlays() const { return bool_value(OPTION_USE_OVERLAYS); }
	bool use_bezels() const { return bool_value(OPTION_USE_BEZELS); }
	bool use_cpanels() const { return bool_value(OPTION_USE_CPANELS); }
	bool use_marquees() const { return bool_value(OPTION_USE_MARQUEES); }

	// core screen options
	float brightness() const { return float_value(OPTION_BRIGHTNESS); }
	float contrast() const { return float_value(OPTION_CONTRAST); }
	float gamma() const { return float_value(OPTION_GAMMA); }
	float pause_brightness() const { return float_value(OPTION_PAUSE_BRIGHTNESS); }
	const char *effect() const { return value(OPTION_EFFECT); }

	// core vector options
	bool antialias() const { return bool_value(OPTION_ANTIALIAS); }
	float beam() const { return float_value(OPTION_BEAM); }
	float flicker() const { return float_value(OPTION_FLICKER); }

	// core sound options
	bool sound() const { return bool_value(OPTION_SOUND); }
	int sample_rate() const { return int_value(OPTION_SAMPLERATE); }
	bool samples() const { return bool_value(OPTION_SAMPLES); }
	int volume() const { return int_value(OPTION_VOLUME); }

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
	bool steadykey() const { return bool_value(OPTION_STEADYKEY); }
	bool ui_active() const { return bool_value(OPTION_UI_ACTIVE); }
	bool offscreen_reload() const { return bool_value(OPTION_OFFSCREEN_RELOAD); }
	bool natural_keyboard() const { return bool_value(OPTION_NATURAL_KEYBOARD); }
	bool joystick_contradictory() const { return bool_value(OPTION_JOYSTICK_CONTRADICTORY); }
	int coin_impulse() const { return int_value(OPTION_COIN_IMPULSE); }

	// core debugging options
	bool verbose() const { return bool_value(OPTION_VERBOSE); }
	bool log() const { return bool_value(OPTION_LOG); }
	bool debug() const { return bool_value(OPTION_DEBUG); }
	bool debug_internal() const { return bool_value(OPTION_DEBUG_INTERNAL); }
	const char *debug_script() const { return value(OPTION_DEBUGSCRIPT); }
	bool update_in_pause() const { return bool_value(OPTION_UPDATEINPAUSE); }

	// core misc options
	const char *bios() const { return value(OPTION_BIOS); }
	bool cheat() const { return bool_value(OPTION_CHEAT); }
	bool skip_gameinfo() const { return bool_value(OPTION_SKIP_GAMEINFO); }
	const char *ui_font() const { return value(OPTION_UI_FONT); }
	const char *ram_size() const { return value(OPTION_RAMSIZE); }

	bool confirm_quit() const { return bool_value(OPTION_CONFIRM_QUIT); }
	bool ui_mouse() const { return bool_value(OPTION_UI_MOUSE); }

	// device-specific options
	const char *device_option(device_image_interface &image);

	void remove_device_options();

	const char *main_value(astring &buffer, const char *option) const;
	const char *sub_value(astring &buffer, const char *name, const char *subname) const;
private:
	// device-specific option handling
	void add_device_options(bool isfirst);
	bool add_slot_options(bool isfirst);
	void update_slot_options();

	// INI parsing helper
	bool parse_one_ini(const char *basename, int priority, astring *error_string = NULL);

	static const options_entry s_option_entries[];
};


#endif	/* __EMUOPTS_H__ */
