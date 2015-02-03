
#include "options.h"
#include "osdepend.h"
#include "modules/lib/osdobj_common.h"

//============================================================
//  TYPE DEFINITIONS
//============================================================

class retro_osd_interface : public osd_common_t
{
public:
	// construction/destruction
	retro_osd_interface(osd_options &options);
	virtual ~retro_osd_interface();

	// general overridables
	virtual void init(running_machine &machine);
	virtual void update(bool skip_redraw);

	// input overridables
//	virtual void customize_input_type_list(input_type_desc *typelist);
	virtual void customize_input_type_list(simple_list<input_type_entry> &typelist);

private:
	virtual void osd_exit();
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
