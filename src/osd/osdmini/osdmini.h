// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  osdmini.h - Core header
//
//============================================================

#include "options.h"
#include "osdepend.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

class mini_osd_interface : public osd_interface
{
public:
	// construction/destruction
	mini_osd_interface();
	virtual ~mini_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// debugger overridables
//  virtual void init_debugger();
//  virtual void wait_for_debugger(device_t &device, bool firststop);

	// audio overridables
	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
	virtual void set_mastervolume(int attenuation);

	// input overridables
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

private:
	static void osd_exit(running_machine &machine);
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
