// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdepend.c

    OS-dependent code interface.

*******************************************************************c********/


#include "emu.h"
#include "osdepend.h"
#include "modules/sound/none.h"
#include "modules/debugger/none.h"
#include "modules/debugger/debugint.h"
#include "modules/lib/osdobj_common.h"

extern bool g_print_verbose;

const options_entry osd_options::s_option_entries[] =
{
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD CLI OPTIONS" },
    { OSDCOMMAND_LIST_MIDI_DEVICES ";mlist",  "0",        OPTION_COMMAND,    "list available MIDI I/O devices" },
    { OSDCOMMAND_LIST_NETWORK_ADAPTERS ";nlist", "0",     OPTION_COMMAND,    "list available network adapters" },

    // debugging options
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD DEBUGGING OPTIONS" },
    { OSDOPTION_DEBUGGER,                     OSDOPTVAL_AUTO,      OPTION_STRING,    "debugger used : " },
    { OSDOPTION_WATCHDOG ";wdog",             "0",        OPTION_INTEGER,    "force the program to terminate if no updates within specified number of seconds" },

    // performance options
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD PERFORMANCE OPTIONS" },
    { OSDOPTION_MULTITHREADING ";mt",         "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
    { OSDOPTION_NUMPROCESSORS ";np",          OSDOPTVAL_AUTO,      OPTION_STRING,     "number of processors; this overrides the number the system reports" },
    { OSDOPTION_BENCH,                        "0",        OPTION_INTEGER,    "benchmark for the given number of emulated seconds; implies -video none -sound none -nothrottle" },
    // video options
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD VIDEO OPTIONS" },
// OS X can be trusted to have working hardware OpenGL, so default to it on for the best user experience
    { OSDOPTION_VIDEO,                        OSDOPTVAL_AUTO,     OPTION_STRING,     "video output method: " },
    { OSDOPTION_NUMSCREENS "(1-4)",           "1",        OPTION_INTEGER,    "number of screens to create; usually, you want just one" },
    { OSDOPTION_WINDOW ";w",                  "0",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
    { OSDOPTION_MAXIMIZE ";max",              "1",        OPTION_BOOLEAN,    "default to maximized windows; otherwise, windows will be minimized" },
    { OSDOPTION_KEEPASPECT ";ka",             "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
    { OSDOPTION_UNEVENSTRETCH ";ues",         "1",        OPTION_BOOLEAN,    "allow non-integer stretch factors" },
    { OSDOPTION_WAITVSYNC ";vs",              "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
    { OSDOPTION_SYNCREFRESH ";srf",           "0",        OPTION_BOOLEAN,    "enable using the start of VBLANK for throttling instead of the game time" },

    // per-window options
    { NULL,                                   NULL,             OPTION_HEADER,    "OSD PER-WINDOW VIDEO OPTIONS" },
    { OSDOPTION_SCREEN,                   OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT ";screen_aspect",  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION ";r",          OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW,                     OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for all screens" },

    { OSDOPTION_SCREEN "0",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "0",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "0;r0",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "0",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the first screen" },

    { OSDOPTION_SCREEN "1",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "1",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "1;r1",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "1",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the second screen" },

    { OSDOPTION_SCREEN "2",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "2",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "2;r2",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "2",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the third screen" },

    { OSDOPTION_SCREEN "3",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "3",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "3;r3",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "3",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the fourth screen" },

    // full screen options
    { NULL,                                   NULL,  OPTION_HEADER,     "OSD FULL SCREEN OPTIONS" },
    { OSDOPTION_SWITCHRES,                    "0",   OPTION_BOOLEAN,    "enable resolution switching" },

    // sound options
    { NULL,                                   NULL,  OPTION_HEADER,     "OSD SOUND OPTIONS" },
    { OSDOPTION_SOUND,                        OSDOPTVAL_AUTO, OPTION_STRING,     "sound output method: " },
    { OSDOPTION_AUDIO_LATENCY "(1-5)",        "2",   OPTION_INTEGER,    "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

    // End of list
    { NULL }
};

osd_options::osd_options()
: cli_options()
{
    add_entries(osd_options::s_option_entries);
};


//-------------------------------------------------
//  osd_interface - constructor
//-------------------------------------------------

osd_common_t::osd_common_t(osd_options &options)
	: m_machine(NULL),
	  m_options(options),
	  m_sound(NULL),
	  m_debugger(NULL)

{
}


void osd_common_t::register_options()
{
    // Register video options and update options
    video_options_add("none", NULL);
    video_register();
    update_option(OSDOPTION_VIDEO, m_video_names);

    // Register sound options and update options
    sound_options_add("none", OSD_SOUND_NONE);
    sound_register();
    update_option(OSDOPTION_SOUND, m_sound_names);

    // Register debugger options and update options
    debugger_options_add("none", OSD_DEBUGGER_NONE);
    debugger_options_add("internal", OSD_DEBUGGER_INTERNAL);
    debugger_register();
    update_option(OSDOPTION_DEBUGGER, m_debugger_names);
}

void osd_common_t::update_option(const char * key, dynamic_array<const char *> &values)
{
	astring current_value(m_options.description(key));
	astring new_option_value("");
	for (int index = 0; index < values.count(); index++)
	{
		astring t(values[index]);
		if (new_option_value.len() > 0)
		{
			if( index != (values.count()-1))
				new_option_value.cat(", ");
			else
				new_option_value.cat(" or ");
		}
		new_option_value.cat(t);
	}
	// TODO: core_strdup() is leaked
	m_options.set_description(key, core_strdup(current_value.cat(new_option_value).cstr()));
}

//-------------------------------------------------
//  osd_interface - destructor
//-------------------------------------------------

osd_common_t::~osd_common_t()
{
	for(int i= 0; i < m_video_names.count(); ++i)
		osd_free(const_cast<char*>(m_video_names[i]));
	//m_video_options,reset();

	for(int i= 0; i < m_sound_names.count(); ++i)
		osd_free(const_cast<char*>(m_sound_names[i]));
	m_sound_options.reset();

	for(int i= 0; i < m_debugger_names.count(); ++i)
		osd_free(const_cast<char*>(m_debugger_names[i]));
	m_debugger_options.reset();
}


//-------------------------------------------------
//  init - initialize the OSD system.
//-------------------------------------------------

void osd_common_t::init(running_machine &machine)
{
	//
	// This function is responsible for initializing the OSD-specific
	// video and input functionality, and registering that functionality
	// with the MAME core.
	//
	// In terms of video, this function is expected to create one or more
	// render_targets that will be used by the MAME core to provide graphics
	// data to the system. Although it is possible to do this later, the
	// assumption in the MAME core is that the user interface will be
	// visible starting at init() time, so you will have some work to
	// do to avoid these assumptions.
	//
	// In terms of input, this function is expected to enumerate all input
	// devices available and describe them to the MAME core by adding
	// input devices and their attached items (buttons/axes) via the input
	// system.
	//
	// Beyond these core responsibilities, init() should also initialize
	// any other OSD systems that require information about the current
	// running_machine.
	//
	// This callback is also the last opportunity to adjust the options
	// before they are consumed by the rest of the core.
	//
	// Future work/changes:
	//
	// Audio initialization may eventually move into here as well,
	// instead of relying on independent callbacks from each system.
	//

	m_machine = &machine;

	osd_options &options = downcast<osd_options &>(machine.options());
	// extract the verbose printing option
	if (options.verbose())
		g_print_verbose = true;

	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(osd_common_t::osd_exit), this));
}


//-------------------------------------------------
//  update - periodic system update
//-------------------------------------------------

void osd_common_t::update(bool skip_redraw)
{
	//
	// This method is called periodically to flush video updates to the
	// screen, and also to allow the OSD a chance to update other systems
	// on a regular basis. In general this will be called at the frame
	// rate of the system being run; however, it may be called at more
	// irregular intervals in some circumstances (e.g., multi-screen games
	// or games with asynchronous updates).
	//
}


//-------------------------------------------------
//  init_debugger - perform debugger-specific
//  initialization
//-------------------------------------------------

void osd_common_t::init_debugger()
{
	//
	// Unlike init() above, this method is only called if the debugger
	// is active. This gives any OSD debugger interface a chance to
	// create all of its structures.
	//
	osd_debugger_type debugger = m_debugger_options.find(options().debugger());
	if (debugger==NULL)
	{
		osd_printf_warning("debugger_init: option %s not found switching to auto\n",options().debugger());
		debugger = m_debugger_options.find("auto");
	}
	m_debugger = (*debugger)(*this);

	m_debugger->init_debugger(machine());
}


//-------------------------------------------------
//  wait_for_debugger - wait for a debugger
//  command to be processed
//-------------------------------------------------

void osd_common_t::wait_for_debugger(device_t &device, bool firststop)
{
	//
	// When implementing an OSD-driver debugger, this method should be
	// overridden to wait for input, process it, and return. It will be
	// called repeatedly until a command is issued that resumes
	// execution.
	//
	m_debugger->wait_for_debugger(device, firststop);
}

void osd_common_t::debugger_update()
{
	if (m_debugger) m_debugger->debugger_update();
}

void osd_common_t::debugger_exit()
{
	if (m_debugger)
	{
		m_debugger->debugger_exit();
		global_free(m_debugger);
		m_debugger = NULL;
	}
}

//-------------------------------------------------
//  update_audio_stream - update the stereo audio
//  stream
//-------------------------------------------------

void osd_common_t::update_audio_stream(const INT16 *buffer, int samples_this_frame)
{
	//
	// This method is called whenever the system has new audio data to stream.
	// It provides an array of stereo samples in L-R order which should be
	// output at the configured sample_rate.
	//
    if (m_sound != NULL)
        m_sound->update_audio_stream(buffer,samples_this_frame);
}


//-------------------------------------------------
//  set_mastervolume - set the system volume
//-------------------------------------------------

void osd_common_t::set_mastervolume(int attenuation)
{
	//
	// Attenuation is the attenuation in dB (a negative number).
	// To convert from dB to a linear volume scale do the following:
	//    volume = MAX_VOLUME;
	//    while (attenuation++ < 0)
	//       volume /= 1.122018454;      //  = (10 ^ (1/20)) = 1dB
	//
    if (m_sound != NULL)
        m_sound->set_mastervolume(attenuation);
}


//-------------------------------------------------
//  customize_input_type_list - provide OSD
//  additions/modifications to the input list
//-------------------------------------------------

void osd_common_t::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	//
	// inptport.c defines some general purpose defaults for key and joystick bindings.
	// They may be further adjusted by the OS dependent code to better match the
	// available keyboard, e.g. one could map pause to the Pause key instead of P, or
	// snapshot to PrtScr instead of F12. Of course the user can further change the
	// settings to anything he/she likes.
	//
	// This function is called on startup, before reading the configuration from disk.
	// Scan the list, and change the keys/joysticks you want.
	//
}


//-------------------------------------------------
//  font_open - attempt to "open" a handle to the
//  font with the given name
//-------------------------------------------------

osd_font osd_common_t::font_open(const char *name, int &height)
{
	return NULL;
}


//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_common_t::font_close(osd_font font)
{
}


//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_common_t::font_get_bitmap(osd_font font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs)
{
	return false;
}

//-------------------------------------------------
//  get_slider_list - allocate and populate a
//  list of OS-dependent slider values.
//-------------------------------------------------

void *osd_common_t::get_slider_list()
{
	return NULL;
}

//-------------------------------------------------
//  execute_command - execute a command not yet
//  handled by the core
//-------------------------------------------------

bool osd_common_t::execute_command(const char *command)
{
    if (strcmp(command, OSDCOMMAND_LIST_NETWORK_ADAPTERS) == 0)
    {
        network_init();
        osd_list_network_adapters();
        network_exit();
        return true;
    }
    else if (strcmp(command, OSDCOMMAND_LIST_MIDI_DEVICES) == 0)
    {
        osd_list_midi_devices();
        return true;
    }

    return false;

}

void osd_common_t::init_subsystems()
{
	if (!video_init())
	{
		video_exit();
		osd_printf_error("video_init: Initialization failed!\n\n\n");
		fflush(stderr);
		fflush(stdout);
		exit(-1);
	}

	sound_init();
	input_init();
	// we need pause callbacks
	machine().add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(FUNC(osd_common_t::input_pause), this));
	machine().add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(FUNC(osd_common_t::input_resume), this));

	output_init();
#ifdef USE_NETWORK
	network_init();
#endif
	midi_init();
}

bool osd_common_t::video_init()
{
	return true;
}

bool osd_common_t::window_init()
{
	return true;
}

bool osd_common_t::sound_init()
{
	osd_sound_type sound = m_sound_options.find(options().sound());
	if (sound==NULL)
	{
		osd_printf_warning("sound_init: option %s not found switching to auto\n",options().sound());
		sound = m_sound_options.find("auto");
	}
	if (sound != NULL)
	    m_sound = (*sound)(*this, machine());
	else
	    m_sound = NULL;
	return true;
}

bool osd_common_t::no_sound()
{
	return (strcmp(options().sound(),"none")==0) ? true : false;
}

void osd_common_t::video_register()
{
}

void osd_common_t::sound_register()
{
}

void osd_common_t::debugger_register()
{
}

bool osd_common_t::input_init()
{
	return true;
}

void osd_common_t::input_pause()
{
}

void osd_common_t::input_resume()
{
}

bool osd_common_t::output_init()
{
	return true;
}

bool osd_common_t::network_init()
{
	return true;
}

void osd_common_t::exit_subsystems()
{
	video_exit();
	sound_exit();
	input_exit();
	output_exit();
	#ifdef USE_NETWORK
	network_exit();
	#endif
	midi_exit();
	debugger_exit();
}

void osd_common_t::video_exit()
{
}

void osd_common_t::window_exit()
{
}

void osd_common_t::sound_exit()
{
    if (m_sound != NULL)
        global_free(m_sound);
}

void osd_common_t::input_exit()
{
}

void osd_common_t::output_exit()
{
}

void osd_common_t::network_exit()
{
}

void osd_common_t::osd_exit()
{
	exit_subsystems();
}

void osd_common_t::video_options_add(const char *name, void *type)
{
	//m_video_options.add(name, type, false);
	m_video_names.append(core_strdup(name));
}

void osd_common_t::sound_options_add(const char *name, osd_sound_type type)
{
	m_sound_options.add(name, type, false);
	m_sound_names.append(core_strdup(name));
}

void osd_common_t::debugger_options_add(const char *name, osd_debugger_type type)
{
	m_debugger_options.add(name, type, false);
	m_debugger_names.append(core_strdup(name));
}

bool osd_common_t::midi_init()
{
    // this should be done on the OS_level
    return osd_midi_init();
}

void osd_common_t::midi_exit()
{
    osd_midi_exit();
}

//-------------------------------------------------
//  osd_sound_interface - constructor
//-------------------------------------------------

osd_sound_interface::osd_sound_interface(const osd_interface &osd, running_machine &machine)
    : m_osd(osd), m_machine(machine)
{
}

//-------------------------------------------------
//  osd_sound_interface - destructor
//-------------------------------------------------

osd_sound_interface::~osd_sound_interface()
{
}

//-------------------------------------------------
//  osd_debugger_interface - constructor
//-------------------------------------------------

osd_debugger_interface::osd_debugger_interface(const osd_interface &osd)
    : m_osd(osd)
{
}

//-------------------------------------------------
//  osd_debugger_interface - destructor
//-------------------------------------------------

osd_debugger_interface::~osd_debugger_interface()
{
}

