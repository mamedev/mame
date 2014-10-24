
#include "options.h"
#include "osdepend.h"


//============================================================
//  TYPE DEFINITIONS
//============================================================

class retro_osd_interface : public osd_interface
{
public:
	// construction/destruction
	retro_osd_interface();
	virtual ~retro_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// audio overridables
//	virtual void update_audio_stream(const INT16 *buffer, int samples_this_frame);
//	virtual void set_mastervolume(int attenuation);
	virtual void sound_register();

	// input overridables
//	virtual void customize_input_type_list(input_type_desc *typelist);
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

private:
	void osd_exit();//static void osd_exit(running_machine &machine);
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
