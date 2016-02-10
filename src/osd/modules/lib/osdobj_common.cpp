// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdepend.c

    OS-dependent code interface.

***************************************************************************/


#include "emu.h"
#include "osdepend.h"
#include "modules/lib/osdobj_common.h"

const options_entry osd_options::s_option_entries[] =
{
	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD KEYBOARD MAPPING OPTIONS" },
#ifdef SDLMAME_MACOSX
	{ OSDOPTION_UIMODEKEY,                   "DEL",             OPTION_STRING,    "Key to toggle keyboard mode" },
#else
	{ OSDOPTION_UIMODEKEY,                   "SCRLOCK",         OPTION_STRING,    "Key to toggle keyboard mode" },
#endif  // SDLMAME_MACOSX

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD FONT OPTIONS" },
	{ OSD_FONT_PROVIDER,                      OSDOPTVAL_AUTO,   OPTION_STRING,    "provider for ui font: " },

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD CLI OPTIONS" },
	{ OSDCOMMAND_LIST_MIDI_DEVICES ";mlist",  "0",              OPTION_COMMAND,   "list available MIDI I/O devices" },
	{ OSDCOMMAND_LIST_NETWORK_ADAPTERS ";nlist", "0",           OPTION_COMMAND,   "list available network adapters" },

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD DEBUGGING OPTIONS" },
	{ OSDOPTION_DEBUGGER,                     OSDOPTVAL_AUTO,   OPTION_STRING,    "debugger used: " },
	{ OSDOPTION_DEBUGGER_FONT ";dfont",       OSDOPTVAL_AUTO,   OPTION_STRING,    "specifies the font to use for debugging" },
	{ OSDOPTION_DEBUGGER_FONT_SIZE ";dfontsize", "0",           OPTION_FLOAT,     "specifies the font size to use for debugging" },
	{ OSDOPTION_WATCHDOG ";wdog",             "0",              OPTION_INTEGER,   "force the program to terminate if no updates within specified number of seconds" },

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD PERFORMANCE OPTIONS" },
	{ OSDOPTION_MULTITHREADING ";mt",         "0",              OPTION_BOOLEAN,   "enable multithreading; this enables rendering and blitting on a separate thread" },
	{ OSDOPTION_NUMPROCESSORS ";np",          OSDOPTVAL_AUTO,   OPTION_STRING,    "number of processors; this overrides the number the system reports" },
	{ OSDOPTION_BENCH,                        "0",              OPTION_INTEGER,   "benchmark for the given number of emulated seconds; implies -video none -sound none -nothrottle" },

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD VIDEO OPTIONS" },
// OS X can be trusted to have working hardware OpenGL, so default to it on for the best user experience
	{ OSDOPTION_VIDEO,                        OSDOPTVAL_AUTO,   OPTION_STRING,    "video output method: " },
	{ OSDOPTION_NUMSCREENS "(1-4)",           "1",              OPTION_INTEGER,   "number of screens to create; usually, you want just one" },
	{ OSDOPTION_WINDOW ";w",                  "0",              OPTION_BOOLEAN,   "enable window mode; otherwise, full screen mode is assumed" },
	{ OSDOPTION_MAXIMIZE ";max",              "1",              OPTION_BOOLEAN,   "default to maximized windows; otherwise, windows will be minimized" },
	{ OSDOPTION_KEEPASPECT ";ka",             "1",              OPTION_BOOLEAN,   "constrain to the proper aspect ratio" },
	{ OSDOPTION_UNEVENSTRETCH ";ues",         "1",              OPTION_BOOLEAN,   "allow non-integer stretch factors" },
	{ OSDOPTION_WAITVSYNC ";vs",              "0",              OPTION_BOOLEAN,   "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
	{ OSDOPTION_SYNCREFRESH ";srf",           "0",              OPTION_BOOLEAN,   "enable using the start of VBLANK for throttling instead of the game time" },

	// per-window options
	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD PER-WINDOW VIDEO OPTIONS" },
	{ OSDOPTION_SCREEN,                       OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT ";screen_aspect",      OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION ";r",              OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW,                         OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for all screens" },

	{ OSDOPTION_SCREEN "0",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "0",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "0;r0",            OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "0",                     OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the first screen" },

	{ OSDOPTION_SCREEN "1",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "1",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "1;r1",            OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "1",                     OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the second screen" },

	{ OSDOPTION_SCREEN "2",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "2",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "2;r2",            OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "2",                     OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the third screen" },

	{ OSDOPTION_SCREEN "3",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "3",                   OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "3;r3",            OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "3",                     OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the fourth screen" },

	// full screen options
	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD FULL SCREEN OPTIONS" },
	{ OSDOPTION_SWITCHRES,                    "0",              OPTION_BOOLEAN,   "enable resolution switching" },

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD ACCELERATED VIDEO OPTIONS" },
	{ OSDOPTION_FILTER ";glfilter;flt",       "1",              OPTION_BOOLEAN,   "enable bilinear filtering on screen output" },
	{ OSDOPTION_PRESCALE,                     "1",              OPTION_INTEGER,   "scale screen rendering by this amount in software" },

#if USE_OPENGL
	{ NULL,                                   NULL,             OPTION_HEADER,    "OpenGL-SPECIFIC OPTIONS" },
	{ OSDOPTION_GL_FORCEPOW2TEXTURE,          "0",              OPTION_BOOLEAN,   "force power of two textures  (default no)" },
	{ OSDOPTION_GL_NOTEXTURERECT,             "0",              OPTION_BOOLEAN,   "don't use OpenGL GL_ARB_texture_rectangle (default on)" },
	{ OSDOPTION_GL_VBO,                       "1",              OPTION_BOOLEAN,   "enable OpenGL VBO,  if available (default on)" },
	{ OSDOPTION_GL_PBO,                       "1",              OPTION_BOOLEAN,   "enable OpenGL PBO,  if available (default on)" },
	{ OSDOPTION_GL_GLSL,                      "0",              OPTION_BOOLEAN,   "enable OpenGL GLSL, if available (default off)" },
	{ OSDOPTION_GLSL_FILTER,                  "1",              OPTION_STRING,    "enable OpenGL GLSL filtering instead of FF filtering 0-plain, 1-bilinear (default)" },
	{ OSDOPTION_SHADER_MAME "0",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 0" },
	{ OSDOPTION_SHADER_MAME "1",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 1" },
	{ OSDOPTION_SHADER_MAME "2",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 2" },
	{ OSDOPTION_SHADER_MAME "3",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 3" },
	{ OSDOPTION_SHADER_MAME "4",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 4" },
	{ OSDOPTION_SHADER_MAME "5",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 5" },
	{ OSDOPTION_SHADER_MAME "6",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 6" },
	{ OSDOPTION_SHADER_MAME "7",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 7" },
	{ OSDOPTION_SHADER_MAME "8",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 8" },
	{ OSDOPTION_SHADER_MAME "9",              OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader set mame bitmap 9" },
	{ OSDOPTION_SHADER_SCREEN "0",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 0" },
	{ OSDOPTION_SHADER_SCREEN "1",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 1" },
	{ OSDOPTION_SHADER_SCREEN "2",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 2" },
	{ OSDOPTION_SHADER_SCREEN "3",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 3" },
	{ OSDOPTION_SHADER_SCREEN "4",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 4" },
	{ OSDOPTION_SHADER_SCREEN "5",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 5" },
	{ OSDOPTION_SHADER_SCREEN "6",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 6" },
	{ OSDOPTION_SHADER_SCREEN "7",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 7" },
	{ OSDOPTION_SHADER_SCREEN "8",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 8" },
	{ OSDOPTION_SHADER_SCREEN "9",            OSDOPTVAL_NONE,   OPTION_STRING,    "custom OpenGL GLSL shader screen bitmap 9" },
#endif

	{ NULL,                                   NULL,             OPTION_HEADER,    "OSD SOUND OPTIONS" },
	{ OSDOPTION_SOUND,                        OSDOPTVAL_AUTO,   OPTION_STRING,    "sound output method: " },
	{ OSDOPTION_AUDIO_LATENCY "(1-5)",        "2",              OPTION_INTEGER,   "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

#ifdef SDLMAME_MACOSX
	{ NULL,                                   NULL,             OPTION_HEADER,    "CoreAudio-SPECIFIC OPTIONS" },
	{ OSDOPTION_AUDIO_OUTPUT,                 OSDOPTVAL_AUTO,   OPTION_STRING,    "Audio output device" },
	{ OSDOPTION_AUDIO_EFFECT "0",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 0" },
	{ OSDOPTION_AUDIO_EFFECT "1",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 1" },
	{ OSDOPTION_AUDIO_EFFECT "2",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 2" },
	{ OSDOPTION_AUDIO_EFFECT "3",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 3" },
	{ OSDOPTION_AUDIO_EFFECT "4",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 4" },
	{ OSDOPTION_AUDIO_EFFECT "5",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 5" },
	{ OSDOPTION_AUDIO_EFFECT "6",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 6" },
	{ OSDOPTION_AUDIO_EFFECT "7",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 7" },
	{ OSDOPTION_AUDIO_EFFECT "8",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 8" },
	{ OSDOPTION_AUDIO_EFFECT "9",             OSDOPTVAL_NONE,   OPTION_STRING,    "AudioUnit effect 9" },
#endif

	// End of list
	{ NULL }
};

osd_options::osd_options()
: cli_options()
{
	add_entries(osd_options::s_option_entries);
}


//-------------------------------------------------
//  osd_interface - constructor
//-------------------------------------------------

osd_common_t::osd_common_t(osd_options &options)
	: osd_output(), m_machine(NULL),
		m_options(options),
		m_print_verbose(false),
		m_sound(NULL),
		m_debugger(NULL)
{
	osd_output::push(this);
}

//-------------------------------------------------
//  osd_interface - destructor
//-------------------------------------------------

osd_common_t::~osd_common_t()
{
	for(unsigned int i= 0; i < m_video_names.size(); ++i)
		osd_free(const_cast<char*>(m_video_names[i]));
	//m_video_options,reset();
	osd_output::pop(this);
}

#define REGISTER_MODULE(_O, _X ) { extern const module_type _X; _O . register_module( _X ); }

void osd_common_t::register_options()
{
	REGISTER_MODULE(m_mod_man, FONT_OSX);
	REGISTER_MODULE(m_mod_man, FONT_WINDOWS);
	REGISTER_MODULE(m_mod_man, FONT_SDL);
	REGISTER_MODULE(m_mod_man, FONT_NONE);

	REGISTER_MODULE(m_mod_man, SOUND_DSOUND);
	REGISTER_MODULE(m_mod_man, SOUND_COREAUDIO);
	REGISTER_MODULE(m_mod_man, SOUND_JS);
	REGISTER_MODULE(m_mod_man, SOUND_SDL);
	REGISTER_MODULE(m_mod_man, SOUND_XAUDIO2);
	REGISTER_MODULE(m_mod_man, SOUND_NONE);

#ifdef SDLMAME_MACOSX
	REGISTER_MODULE(m_mod_man, DEBUG_OSX);
#endif
#ifndef OSD_MINI
	REGISTER_MODULE(m_mod_man, DEBUG_WINDOWS);
	REGISTER_MODULE(m_mod_man, DEBUG_QT);
	REGISTER_MODULE(m_mod_man, DEBUG_INTERNAL);
	REGISTER_MODULE(m_mod_man, DEBUG_NONE);
#endif

	REGISTER_MODULE(m_mod_man, NETDEV_TAPTUN);
	REGISTER_MODULE(m_mod_man, NETDEV_PCAP);
	REGISTER_MODULE(m_mod_man, NETDEV_NONE);

#ifndef NO_USE_MIDI
	REGISTER_MODULE(m_mod_man, MIDI_PM);
#endif
	REGISTER_MODULE(m_mod_man, MIDI_NONE);

	// after initialization we know which modules are supported

	const char *names[20];
	int num;
	m_mod_man.get_module_names(OSD_FONT_PROVIDER, 20, &num, names);
	std::vector<const char *> dnames;
	for (int i = 0; i < num; i++)
		dnames.push_back(names[i]);
	update_option(OSD_FONT_PROVIDER, dnames);

	m_mod_man.get_module_names(OSD_SOUND_PROVIDER, 20, &num, names);
	dnames.clear();
	for (int i = 0; i < num; i++)
		dnames.push_back(names[i]);
	update_option(OSD_SOUND_PROVIDER, dnames);

#if 0
	// Register midi options and update options
	m_mod_man.get_module_names(OSD_MIDI_PROVIDER, 20, &num, names);
	dnames.clear();
	for (int i = 0; i < num; i++)
		dnames.push_back(names[i]);
	update_option(OSD_MIDI_PROVIDER, dnames);
#endif

	// Register debugger options and update options
	m_mod_man.get_module_names(OSD_DEBUG_PROVIDER, 20, &num, names);
	dnames.clear();
	for (int i = 0; i < num; i++)
		dnames.push_back(names[i]);
	update_option(OSD_DEBUG_PROVIDER, dnames);

	// Register video options and update options
	video_options_add("none", NULL);
	video_register();
	update_option(OSDOPTION_VIDEO, m_video_names);
}

void osd_common_t::update_option(const char * key, std::vector<const char *> &values)
{
	std::string current_value(m_options.description(key));
	std::string new_option_value("");
	for (unsigned int index = 0; index < values.size(); index++)
	{
		std::string t(values[index]);
		if (new_option_value.length() > 0)
		{
			if( index != (values.size()-1))
				new_option_value.append(", ");
			else
				new_option_value.append(" or ");
		}
		new_option_value.append(t);
	}
	// TODO: core_strdup() is leaked
	m_options.set_description(key, core_strdup(current_value.append(new_option_value).c_str()));
}


//-------------------------------------------------
//  output_callback  - callback for osd_printf_...
//-------------------------------------------------
void osd_common_t::output_callback(osd_output_channel channel, const char *msg, va_list args)
{
	switch (channel)
	{
		case OSD_OUTPUT_CHANNEL_ERROR:
		case OSD_OUTPUT_CHANNEL_WARNING:
			vfprintf(stderr, msg, args);
			break;
		case OSD_OUTPUT_CHANNEL_INFO:
		case OSD_OUTPUT_CHANNEL_LOG:
			vfprintf(stdout, msg, args);
			break;
		case OSD_OUTPUT_CHANNEL_VERBOSE:
			if (verbose()) vfprintf(stdout, msg, args);
			break;
		case OSD_OUTPUT_CHANNEL_DEBUG:
#ifdef MAME_DEBUG
			vfprintf(stdout, msg, args);
#endif
			break;
		default:
			break;
	}
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
		set_verbose(true);

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
	m_sound->update_audio_stream(m_machine->video().throttled(), buffer,samples_this_frame);
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

osd_font *osd_common_t::font_open(const char *name, int &height)
{
	return NULL;
}


//-------------------------------------------------
//  font_close - release resources associated with
//  a given OSD font
//-------------------------------------------------

void osd_common_t::font_close(osd_font *font)
{
}


//-------------------------------------------------
//  font_get_bitmap - allocate and populate a
//  BITMAP_FORMAT_ARGB32 bitmap containing the
//  pixel values rgb_t(0xff,0xff,0xff,0xff)
//  or rgb_t(0x00,0xff,0xff,0xff) for each
//  pixel of a black & white font
//-------------------------------------------------

bool osd_common_t::font_get_bitmap(osd_font *font, unicode_char chnum, bitmap_argb32 &bitmap, INT32 &width, INT32 &xoffs, INT32 &yoffs)
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
		osd_module *om = select_module_options(options(), OSD_NETDEV_PROVIDER);

		if (om->probe())
		{
			om->init(options());
			osd_list_network_adapters();
			om->exit();
		}

		return true;
	}
	else if (strcmp(command, OSDCOMMAND_LIST_MIDI_DEVICES) == 0)
	{
		osd_module *om = select_module_options(options(), OSD_MIDI_PROVIDER);
		midi_module *pm = select_module_options<midi_module *>(options(), OSD_MIDI_PROVIDER);

		if (om->probe())
		{
			om->init(options());
			pm->list_midi_devices();
			om->exit();
		}
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

	input_init();
	// we need pause callbacks
	machine().add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(FUNC(osd_common_t::input_pause), this));
	machine().add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(FUNC(osd_common_t::input_resume), this));

	output_init();

	m_font_module = select_module_options<font_module *>(options(), OSD_FONT_PROVIDER);

	m_sound = select_module_options<sound_module *>(options(), OSD_SOUND_PROVIDER);
	m_sound->m_sample_rate = options().sample_rate();
	m_sound->m_audio_latency = options().audio_latency();

	m_debugger = select_module_options<debug_module *>(options(), OSD_DEBUG_PROVIDER);

	select_module_options<netdev_module *>(options(), OSD_NETDEV_PROVIDER);

	m_midi = select_module_options<midi_module *>(options(), OSD_MIDI_PROVIDER);

	m_mod_man.init(options());

}

bool osd_common_t::video_init()
{
	return true;
}

bool osd_common_t::window_init()
{
	return true;
}

bool osd_common_t::no_sound()
{
	return (strcmp(options().sound(),"none")==0) ? true : false;
}

void osd_common_t::video_register()
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

void osd_common_t::exit_subsystems()
{
	video_exit();
	input_exit();
	output_exit();
}

void osd_common_t::video_exit()
{
}

void osd_common_t::window_exit()
{
}

void osd_common_t::input_exit()
{
}

void osd_common_t::output_exit()
{
}

void osd_common_t::osd_exit()
{
	m_mod_man.exit();

	exit_subsystems();
}

void osd_common_t::video_options_add(const char *name, void *type)
{
	//m_video_options.add(name, type, false);
	m_video_names.push_back(core_strdup(name));
}
