
//============================================================
//
//  input_retro.h - Common code used by Windows input modules
//
//============================================================

#ifndef INPUT_RETRO_H_
#define INPUT_RETRO_H_



//============================================================
//  TYPEDEFS
//============================================================

typedef struct joystate_t
{
   int button[RETRO_MAX_BUTTONS];
   int a1[2];
   int a2[2];
   int a3[2];
}Joystate;

struct KeyPressEventArgs
{
	int event_id;
	uint8_t vkey;
	uint8_t scancode;
};

struct kt_table
{
   const char  *   mame_key_name;
   int retro_key_name;
   input_item_id   mame_key;
};

extern uint16_t retrokbd_state[RETROK_LAST];
extern uint16_t retrokbd_state2[RETROK_LAST];
extern kt_table ktable[];

extern int mouseLX;
extern int mouseLY;
extern int mouseBUT[4];

extern Joystate joystate[4];

extern int fb_width;
extern int fb_height;

class retroinput_module : public input_module_base
{
protected:
	bool  m_global_inputs_enabled;

public:
	retroinput_module(const char * type, const char * name)
		: input_module_base(type, name),
			m_global_inputs_enabled(false)
	{
	}

	virtual bool should_hide_mouse()
	{
		if (/*winwindow_has_focus()  // has focus
			&& (!video_config.windowed || !osd_common_t::s_window_list.front()->win_has_menu()) // not windowed or doesn't have a menu
			&&*/ (input_enabled() && !input_paused()) // input enabled and not paused
			&& (mouse_enabled() || lightgun_enabled())) // either mouse or lightgun enabled in the core
		{
			return true;
		}

		return false;
	}

	virtual bool handle_input_event(void)
	{
		return false;
	}

protected:

	void before_poll(running_machine& machine) override
	{
		// periodically process events, in case they're not coming through
		// this also will make sure the mouse state is up-to-date
		//winwindow_process_events_periodic(machine);
	}

	bool should_poll_devices(running_machine &machine) override
	{
		return input_enabled() && (m_global_inputs_enabled /*|| winwindow_has_focus()*/);
	}
};

#endif
