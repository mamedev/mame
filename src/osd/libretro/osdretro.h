#ifndef _osdretro_h_
#define _osdretro_h_

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"

#include "libretro-internal/libretro_shared.h"

//============================================================
//  Defines
//============================================================

#define RETROOPTION_INIPATH               "inipath"

#define RETROOPTION_CENTERH               "centerh"
#define RETROOPTION_CENTERV               "centerv"

#define RETROOPTION_SCALEMODE             "scalemode"


#define RETROOPTION_SIXAXIS               "sixaxis"
#define RETROOPTION_JOYINDEX              "joy_idx"
#define RETROOPTION_KEYBINDEX             "keyb_idx"
#define RETROOPTION_MOUSEINDEX            "mouse_index"
#if (USE_XINPUT)
#define RETROOPTION_LIGHTGUNINDEX         "lightgun_index"
#endif

#define RETROOPTION_AUDIODRIVER           "audiodriver"
#define RETROOPTION_VIDEODRIVER           "videodriver"
#define RETROOPTION_RENDERDRIVER          "renderdriver"


#define RETROOPTVAL_SOFT                  "soft"

#define RETROMAME_LED(x)                  "led" #x


//============================================================
//  TYPE DEFINITIONS
//============================================================

class retro_options : public osd_options
{
public:
	// construction/destruction
	retro_options();

	// performance options
//	bool video_fps() const { return bool_value(RETROOPTION_SDLVIDEOFPS); }

	// video options
	bool centerh() const { return bool_value(RETROOPTION_CENTERH); }
	bool centerv() const { return bool_value(RETROOPTION_CENTERV); }
	const char *scale_mode() const { return value(RETROOPTION_SCALEMODE); }

	// full screen options


	// joystick mapping
	const char *joy_index(int index) const { return value(string_format("%s%d", RETROOPTION_JOYINDEX, index).c_str()); }
	bool sixaxis() const { return bool_value(RETROOPTION_SIXAXIS); }

	const char *mouse_index(int index) const { return value(string_format("%s%d", RETROOPTION_MOUSEINDEX, index).c_str()); }
	const char *keyboard_index(int index) const { return value(string_format("%s%d", RETROOPTION_KEYBINDEX, index).c_str()); }

	const char *video_driver() const { return value(RETROOPTION_VIDEODRIVER); }
	const char *render_driver() const { return value(RETROOPTION_RENDERDRIVER); }
	const char *audio_driver() const { return value(RETROOPTION_AUDIODRIVER); }


private:
	static const options_entry s_option_entries[];
};


class retro_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	retro_osd_interface(retro_options &options);
	virtual ~retro_osd_interface();

	// general overridables
	virtual void init(running_machine &machine) override;
	virtual void update(bool skip_redraw) override;
	virtual void input_update() override;

	// input overridables
	virtual void customize_input_type_list(std::vector<input_type_entry> &typelist) override;

	virtual void video_register() override;

	virtual bool video_init() override;
	virtual bool window_init() override;

	virtual void video_exit() override;
	virtual void window_exit() override;

	// retro specific
	void poll_inputs(running_machine &machine);
	void release_keys();
	bool should_hide_mouse();
	void process_events_buf();

 	void process_mouse_state(running_machine &machine);
	void process_keyboard_state(running_machine &machine);
 	void process_joypad_state(running_machine &machine);
	void process_lightgun_state(running_machine &machine);

	virtual retro_options &options() override { return m_options; }

protected:
	virtual void build_slider_list() override;
	virtual void update_slider_list() override;

private:
	virtual void osd_exit() override;

	void extract_video_config();
	void output_oslog(const char *buffer);

	retro_options &m_options;
};

//============================================================
//  work.c
//============================================================

extern int osd_num_processors;

#endif
