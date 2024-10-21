// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    osdobj_common.h

    OS-dependent code interface.

***************************************************************************/

#include "osdobj_common.h"

#include "modules/osdwindow.h"
#include "modules/debugger/debug_module.h"
#include "modules/font/font_module.h"
#include "modules/input/input_module.h"
#include "modules/midi/midi_module.h"
#include "modules/monitor/monitor_module.h"
#include "modules/netdev/netdev_module.h"
#include "modules/render/render_module.h"
#include "modules/sound/sound_module.h"

#include "osdnet.h"
#include "watchdog.h"

#include "emu.h"
#include "../frontend/mame/ui/menuitem.h"

#include <iostream>


const options_entry osd_options::s_option_entries[] =
{
	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD INPUT MAPPING OPTIONS" },
#if defined(SDLMAME_MACOSX) || defined(OSD_MAC)
	{ OSDOPTION_UIMODEKEY,                       "DEL",            core_options::option_type::STRING,    "key to enable/disable MAME controls when emulated system has keyboard inputs" },
#else
	{ OSDOPTION_UIMODEKEY,                       "auto",           core_options::option_type::STRING,    "key to enable/disable MAME controls when emulated system has keyboard inputs" },
#endif  // SDLMAME_MACOSX
	{ OSDOPTION_CONTROLLER_MAP_FILE ";ctrlmap",  OSDOPTVAL_NONE,   core_options::option_type::PATH,      "game controller mapping file" },
	{ OSDOPTION_BACKGROUND_INPUT,                "0",              core_options::option_type::BOOLEAN,   "don't ignore input when losing UI focus" },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD FONT OPTIONS" },
	{ OSD_FONT_PROVIDER,                         OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "provider for UI font: " },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD OUTPUT OPTIONS" },
	{ OSD_OUTPUT_PROVIDER,                       OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "provider for output notifications: " },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD INPUT OPTIONS" },
	{ OSD_KEYBOARDINPUT_PROVIDER,                OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "provider for keyboard input: " },
	{ OSD_MOUSEINPUT_PROVIDER,                   OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "provider for mouse input: " },
	{ OSD_LIGHTGUNINPUT_PROVIDER,                OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "provider for lightgun input: " },
	{ OSD_JOYSTICKINPUT_PROVIDER,                OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "provider for joystick input: " },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD CLI OPTIONS" },
	{ OSDCOMMAND_LIST_MIDI_DEVICES ";mlist",     "0",              core_options::option_type::COMMAND,   "list available MIDI I/O devices" },
	{ OSDCOMMAND_LIST_NETWORK_ADAPTERS ";nlist", "0",              core_options::option_type::COMMAND,   "list available network adapters" },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD DEBUGGING OPTIONS" },
	{ OSDOPTION_DEBUGGER,                        OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "debugger used: " },
	{ OSDOPTION_DEBUGGER_HOST,                   "localhost",      core_options::option_type::STRING,    "address to bind to for gdbstub debugger" },
	{ OSDOPTION_DEBUGGER_PORT,                   "23946",          core_options::option_type::INTEGER,   "port to use for gdbstub debugger" },
	{ OSDOPTION_DEBUGGER_FONT ";dfont",          OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "font to use for debugger views" },
	{ OSDOPTION_DEBUGGER_FONT_SIZE ";dfontsize", "0",              core_options::option_type::FLOAT,     "font size to use for debugger views" },
	{ OSDOPTION_WATCHDOG ";wdog",                "0",              core_options::option_type::INTEGER,   "force the program to terminate if no updates within specified number of seconds" },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD PERFORMANCE OPTIONS" },
	{ OSDOPTION_NUMPROCESSORS ";np",             OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "number of processors; this overrides the number the system reports" },
	{ OSDOPTION_BENCH,                           "0",              core_options::option_type::INTEGER,   "benchmark for the given number of emulated seconds; implies -video none -sound none -nothrottle" },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD VIDEO OPTIONS" },
	{ OSDOPTION_VIDEO,                           OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "video output method: " },
	{ OSDOPTION_NUMSCREENS "(1-4)",              "1",              core_options::option_type::INTEGER,   "number of output screens/windows to create; usually, you want just one" },
	{ OSDOPTION_WINDOW ";w",                     "0",              core_options::option_type::BOOLEAN,   "enable window mode; otherwise, full screen mode is assumed" },
	{ OSDOPTION_MAXIMIZE ";max",                 "1",              core_options::option_type::BOOLEAN,   "default to maximized windows" },
	{ OSDOPTION_WAITVSYNC ";vs",                 "0",              core_options::option_type::BOOLEAN,   "enable waiting for the start of VBLANK before flipping screens (reduces tearing effects)" },
	{ OSDOPTION_SYNCREFRESH ";srf",              "0",              core_options::option_type::BOOLEAN,   "enable using the start of VBLANK for throttling instead of the game time" },
	{ OSD_MONITOR_PROVIDER,                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "monitor discovery method: " },

	// per-window options
	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD PER-WINDOW VIDEO OPTIONS" },
	{ OSDOPTION_SCREEN,                          OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT ";screen_aspect",         OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION ";r",                 OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW,                            OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred view for all screens" },

	{ OSDOPTION_SCREEN "0",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "0",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "0;r0",               OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "0",                        OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred view for the first screen" },

	{ OSDOPTION_SCREEN "1",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "1",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "1;r1",               OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "1",                        OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred view for the second screen" },

	{ OSDOPTION_SCREEN "2",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "2",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "2;r2",               OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "2",                        OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred view for the third screen" },

	{ OSDOPTION_SCREEN "3",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_ASPECT "3",                      OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
	{ OSDOPTION_RESOLUTION "3;r3",               OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
	{ OSDOPTION_VIEW "3",                        OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "preferred view for the fourth screen" },

	// full screen options
	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD FULL SCREEN OPTIONS" },
	{ OSDOPTION_SWITCHRES,                       "0",              core_options::option_type::BOOLEAN,   "enable resolution switching" },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD ACCELERATED VIDEO OPTIONS" },
	{ OSDOPTION_FILTER ";glfilter;flt",          "1",              core_options::option_type::BOOLEAN,   "use bilinear filtering when scaling emulated video" },
	{ OSDOPTION_PRESCALE "(1-20)",               "1",              core_options::option_type::INTEGER,   "scale emulated video by this factor before applying filters/shaders" },

#if USE_OPENGL
	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OpenGL-SPECIFIC OPTIONS" },
	{ OSDOPTION_GL_FORCEPOW2TEXTURE,             "0",              core_options::option_type::BOOLEAN,   "force power-of-two texture sizes (default no)" },
	{ OSDOPTION_GL_NOTEXTURERECT,                "0",              core_options::option_type::BOOLEAN,   "don't use OpenGL GL_ARB_texture_rectangle (default on)" },
	{ OSDOPTION_GL_VBO,                          "1",              core_options::option_type::BOOLEAN,   "enable OpenGL VBO if available (default on)" },
	{ OSDOPTION_GL_PBO,                          "1",              core_options::option_type::BOOLEAN,   "enable OpenGL PBO if available (default on)" },
	{ OSDOPTION_GL_GLSL,                         "0",              core_options::option_type::BOOLEAN,   "enable OpenGL GLSL if available (default off)" },
	{ OSDOPTION_GLSL_FILTER,                     "1",              core_options::option_type::STRING,    "enable OpenGL GLSL filtering instead of FF filtering 0-plain, 1-bilinear (default), 2-bicubic" },
	{ OSDOPTION_SHADER_MAME "0",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 0" },
	{ OSDOPTION_SHADER_MAME "1",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 1" },
	{ OSDOPTION_SHADER_MAME "2",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 2" },
	{ OSDOPTION_SHADER_MAME "3",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 3" },
	{ OSDOPTION_SHADER_MAME "4",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 4" },
	{ OSDOPTION_SHADER_MAME "5",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 5" },
	{ OSDOPTION_SHADER_MAME "6",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 6" },
	{ OSDOPTION_SHADER_MAME "7",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 7" },
	{ OSDOPTION_SHADER_MAME "8",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 8" },
	{ OSDOPTION_SHADER_MAME "9",                 OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader set mame bitmap 9" },
	{ OSDOPTION_SHADER_SCREEN "0",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 0" },
	{ OSDOPTION_SHADER_SCREEN "1",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 1" },
	{ OSDOPTION_SHADER_SCREEN "2",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 2" },
	{ OSDOPTION_SHADER_SCREEN "3",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 3" },
	{ OSDOPTION_SHADER_SCREEN "4",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 4" },
	{ OSDOPTION_SHADER_SCREEN "5",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 5" },
	{ OSDOPTION_SHADER_SCREEN "6",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 6" },
	{ OSDOPTION_SHADER_SCREEN "7",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 7" },
	{ OSDOPTION_SHADER_SCREEN "8",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 8" },
	{ OSDOPTION_SHADER_SCREEN "9",               OSDOPTVAL_NONE,   core_options::option_type::STRING,    "custom OpenGL GLSL shader screen bitmap 9" },
#endif

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD SOUND OPTIONS" },
	{ OSDOPTION_SOUND,                           OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "sound output method: " },
	{ OSDOPTION_AUDIO_LATENCY "(0-5)",           "2",              core_options::option_type::INTEGER,   "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

#ifndef NO_USE_PORTAUDIO
	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "PORTAUDIO OPTIONS" },
	{ OSDOPTION_PA_API,                          OSDOPTVAL_NONE,   core_options::option_type::STRING,    "PortAudio API" },
	{ OSDOPTION_PA_DEVICE,                       OSDOPTVAL_NONE,   core_options::option_type::STRING,    "PortAudio device" },
	{ OSDOPTION_PA_LATENCY "(0-0.25)",           "0",              core_options::option_type::FLOAT,     "suggested latency in seconds, 0 for default" },
#endif

#ifdef SDLMAME_MACOSX
	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "CoreAudio-SPECIFIC OPTIONS" },
	{ OSDOPTION_AUDIO_OUTPUT,                    OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "audio output device" },
	{ OSDOPTION_AUDIO_EFFECT "0",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 0" },
	{ OSDOPTION_AUDIO_EFFECT "1",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 1" },
	{ OSDOPTION_AUDIO_EFFECT "2",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 2" },
	{ OSDOPTION_AUDIO_EFFECT "3",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 3" },
	{ OSDOPTION_AUDIO_EFFECT "4",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 4" },
	{ OSDOPTION_AUDIO_EFFECT "5",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 5" },
	{ OSDOPTION_AUDIO_EFFECT "6",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 6" },
	{ OSDOPTION_AUDIO_EFFECT "7",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 7" },
	{ OSDOPTION_AUDIO_EFFECT "8",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 8" },
	{ OSDOPTION_AUDIO_EFFECT "9",                OSDOPTVAL_NONE,   core_options::option_type::STRING,    "AudioUnit effect 9" },
#endif

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD MIDI OPTIONS" },
	{ OSDOPTION_MIDI_PROVIDER,                   OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "MIDI I/O method: " },

	{ nullptr,                                   nullptr,          core_options::option_type::HEADER,    "OSD EMULATED NETWORKING OPTIONS" },
	{ OSDOPTION_NETWORK_PROVIDER,                OSDOPTVAL_AUTO,   core_options::option_type::STRING,    "Emulated networking provider: " },

	{ nullptr,                                   nullptr,           core_options::option_type::HEADER,   "BGFX POST-PROCESSING OPTIONS" },
	{ OSDOPTION_BGFX_PATH,                       "bgfx",            core_options::option_type::PATH,     "path to BGFX-related files" },
	{ OSDOPTION_BGFX_BACKEND,                    "auto",            core_options::option_type::STRING,   "BGFX backend to use (d3d9, d3d11, d3d12, metal, opengl, gles, vulkan)" },
	{ OSDOPTION_BGFX_DEBUG,                      "0",               core_options::option_type::BOOLEAN,  "enable BGFX debugging statistics" },
	{ OSDOPTION_BGFX_SCREEN_CHAINS,              "",                core_options::option_type::STRING,   "comma-delimited list of screen chain JSON names, colon-delimited per-window" },
	{ OSDOPTION_BGFX_SHADOW_MASK,                "slot-mask.png",   core_options::option_type::STRING,   "shadow mask texture name" },
	{ OSDOPTION_BGFX_LUT,                        "lut-default.png", core_options::option_type::STRING,   "LUT texture name" },
	{ OSDOPTION_BGFX_AVI_NAME,                   OSDOPTVAL_AUTO,    core_options::option_type::PATH,     "filename for BGFX output logging" },

	// End of list
	{ nullptr }
};

osd_options::osd_options()
	: emu_options()
{
	add_entries(osd_options::s_option_entries);
}

// Window list
std::list<std::unique_ptr<osd_window> > osd_common_t::s_window_list;

//-------------------------------------------------
//  osd_interface - constructor
//-------------------------------------------------

osd_common_t::osd_common_t(osd_options &options)
	: osd_output(), m_machine(nullptr)
	, m_options(options)
	, m_print_verbose(false)
	, m_font_module(nullptr)
	, m_sound(nullptr)
	, m_debugger(nullptr)
	, m_midi(nullptr)
	, m_keyboard_input(nullptr)
	, m_mouse_input(nullptr)
	, m_lightgun_input(nullptr)
	, m_joystick_input(nullptr)
	, m_output(nullptr)
	, m_monitor_module(nullptr)
	, m_watchdog(nullptr)
{
	osd_output::push(this);
}

//-------------------------------------------------
//  osd_interface - destructor
//-------------------------------------------------

osd_common_t::~osd_common_t()
{
	//m_video_options,reset();
	osd_output::pop(this);
}

#define REGISTER_MODULE(O, X) { extern const module_type X; O.register_module(X); }

void osd_common_t::register_options()
{
	REGISTER_MODULE(m_mod_man, FONT_OSX);
	REGISTER_MODULE(m_mod_man, FONT_WINDOWS);
	REGISTER_MODULE(m_mod_man, FONT_DWRITE);
	REGISTER_MODULE(m_mod_man, FONT_SDL);
	REGISTER_MODULE(m_mod_man, FONT_NONE);

#if defined(SDLMAME_EMSCRIPTEN)
	REGISTER_MODULE(m_mod_man, RENDERER_SDL1); // don't bother trying to use video acceleration in browsers
#endif
#if defined(OSD_WINDOWS)
	REGISTER_MODULE(m_mod_man, RENDERER_D3D); // this is only built for OSD=windows, there's no dummy stub
#endif
#if defined(OSD_WINDOWS) || defined(SDLMAME_WIN32)
	REGISTER_MODULE(m_mod_man, RENDERER_BGFX); // try BGFX before GDI on windows to get DirectX 10/11 acceleration
#endif
	REGISTER_MODULE(m_mod_man, RENDERER_GDI); // GDI ahead of OpenGL as there's a chance Windows has no OpenGL
	REGISTER_MODULE(m_mod_man, RENDERER_OPENGL);
#if !defined(OSD_WINDOWS) && !defined(SDLMAME_WIN32)
	REGISTER_MODULE(m_mod_man, RENDERER_BGFX); // try BGFX after OpenGL on other operating systems for now
#endif
	REGISTER_MODULE(m_mod_man, RENDERER_SDL2);
#if !defined(SDLMAME_EMSCRIPTEN)
	REGISTER_MODULE(m_mod_man, RENDERER_SDL1);
#endif
	REGISTER_MODULE(m_mod_man, RENDERER_NONE);

	REGISTER_MODULE(m_mod_man, SOUND_DSOUND);
	REGISTER_MODULE(m_mod_man, SOUND_XAUDIO2);
	REGISTER_MODULE(m_mod_man, SOUND_COREAUDIO);
	REGISTER_MODULE(m_mod_man, SOUND_JS);
	REGISTER_MODULE(m_mod_man, SOUND_SDL);
#ifndef NO_USE_PORTAUDIO
	REGISTER_MODULE(m_mod_man, SOUND_PORTAUDIO);
#endif
#ifndef NO_USE_PULSEAUDIO
	REGISTER_MODULE(m_mod_man, SOUND_PULSEAUDIO);
#endif
	REGISTER_MODULE(m_mod_man, SOUND_NONE);

	REGISTER_MODULE(m_mod_man, MONITOR_SDL);
	REGISTER_MODULE(m_mod_man, MONITOR_WIN32);
	REGISTER_MODULE(m_mod_man, MONITOR_DXGI);
	REGISTER_MODULE(m_mod_man, MONITOR_MAC);

#ifdef SDLMAME_MACOSX
	REGISTER_MODULE(m_mod_man, DEBUG_OSX);
#endif
#ifndef OSD_MINI
	REGISTER_MODULE(m_mod_man, DEBUG_WINDOWS);
	REGISTER_MODULE(m_mod_man, DEBUG_QT);
	REGISTER_MODULE(m_mod_man, DEBUG_IMGUI);
	REGISTER_MODULE(m_mod_man, DEBUG_GDBSTUB);
	REGISTER_MODULE(m_mod_man, DEBUG_NONE);
#endif

	REGISTER_MODULE(m_mod_man, NETDEV_VMNET);
	REGISTER_MODULE(m_mod_man, NETDEV_VMNET_HELPER);
	REGISTER_MODULE(m_mod_man, NETDEV_TAPTUN);
	REGISTER_MODULE(m_mod_man, NETDEV_PCAP);
	REGISTER_MODULE(m_mod_man, NETDEV_NONE);

#ifndef NO_USE_MIDI
	REGISTER_MODULE(m_mod_man, MIDI_PM);
#endif
	REGISTER_MODULE(m_mod_man, MIDI_NONE);

	REGISTER_MODULE(m_mod_man, KEYBOARDINPUT_SDL);
	REGISTER_MODULE(m_mod_man, KEYBOARDINPUT_RAWINPUT);
	REGISTER_MODULE(m_mod_man, KEYBOARDINPUT_DINPUT);
	REGISTER_MODULE(m_mod_man, KEYBOARDINPUT_WIN32);
	REGISTER_MODULE(m_mod_man, KEYBOARD_NONE);

	REGISTER_MODULE(m_mod_man, MOUSEINPUT_SDL);
	REGISTER_MODULE(m_mod_man, MOUSEINPUT_RAWINPUT);
	REGISTER_MODULE(m_mod_man, MOUSEINPUT_DINPUT);
	REGISTER_MODULE(m_mod_man, MOUSEINPUT_WIN32);
	REGISTER_MODULE(m_mod_man, MOUSE_NONE);

	REGISTER_MODULE(m_mod_man, LIGHTGUNINPUT_SDL);
	REGISTER_MODULE(m_mod_man, LIGHTGUN_X11);
	REGISTER_MODULE(m_mod_man, LIGHTGUNINPUT_RAWINPUT);
	REGISTER_MODULE(m_mod_man, LIGHTGUNINPUT_WIN32);
	REGISTER_MODULE(m_mod_man, LIGHTGUN_NONE);

	REGISTER_MODULE(m_mod_man, JOYSTICKINPUT_SDLGAME);
	REGISTER_MODULE(m_mod_man, JOYSTICKINPUT_SDLJOY);
	REGISTER_MODULE(m_mod_man, JOYSTICKINPUT_WINHYBRID);
	REGISTER_MODULE(m_mod_man, JOYSTICKINPUT_DINPUT);
	REGISTER_MODULE(m_mod_man, JOYSTICKINPUT_XINPUT);
	REGISTER_MODULE(m_mod_man, JOYSTICK_NONE);

	REGISTER_MODULE(m_mod_man, OUTPUT_NONE);
	REGISTER_MODULE(m_mod_man, OUTPUT_CONSOLE);
	REGISTER_MODULE(m_mod_man, OUTPUT_NETWORK);
	REGISTER_MODULE(m_mod_man, OUTPUT_WIN32);


	// after initialization we know which modules are supported
	update_option(OSD_FONT_PROVIDER, m_mod_man.get_module_names(OSD_FONT_PROVIDER));
	update_option(OSD_RENDERER_PROVIDER, m_mod_man.get_module_names(OSD_RENDERER_PROVIDER));
	update_option(OSD_SOUND_PROVIDER, m_mod_man.get_module_names(OSD_SOUND_PROVIDER));
	update_option(OSD_MONITOR_PROVIDER, m_mod_man.get_module_names(OSD_MONITOR_PROVIDER));
	update_option(OSD_KEYBOARDINPUT_PROVIDER, m_mod_man.get_module_names(OSD_KEYBOARDINPUT_PROVIDER));
	update_option(OSD_MOUSEINPUT_PROVIDER, m_mod_man.get_module_names(OSD_MOUSEINPUT_PROVIDER));
	update_option(OSD_LIGHTGUNINPUT_PROVIDER, m_mod_man.get_module_names(OSD_LIGHTGUNINPUT_PROVIDER));
	update_option(OSD_JOYSTICKINPUT_PROVIDER, m_mod_man.get_module_names(OSD_JOYSTICKINPUT_PROVIDER));
	update_option(OSD_MIDI_PROVIDER, m_mod_man.get_module_names(OSD_MIDI_PROVIDER));
	update_option(OSD_NETDEV_PROVIDER, m_mod_man.get_module_names(OSD_NETDEV_PROVIDER));
	update_option(OSD_DEBUG_PROVIDER, m_mod_man.get_module_names(OSD_DEBUG_PROVIDER));
	update_option(OSD_OUTPUT_PROVIDER, m_mod_man.get_module_names(OSD_OUTPUT_PROVIDER));
}

void osd_common_t::update_option(const std::string &key, std::vector<std::string_view> const &values)
{
	std::string current_value(m_options.description(key.c_str()));
	std::string new_option_value("");
	for (unsigned int index = 0; index < values.size(); index++)
	{
		if (new_option_value.length() > 0)
		{
			if (index != (values.size() - 1))
				new_option_value.append(", ");
			else
				new_option_value.append(" or ");
		}
		new_option_value.append(values[index]);
	}

	m_option_descs[key] = current_value + new_option_value;
	m_options.set_description(key.c_str(), m_option_descs[key].c_str());
}


//-------------------------------------------------
//  output_callback  - callback for osd_printf_...
//-------------------------------------------------
void osd_common_t::output_callback(osd_output_channel channel, const util::format_argument_pack<char> &args)
{
	switch (channel)
	{
	case OSD_OUTPUT_CHANNEL_ERROR:
	case OSD_OUTPUT_CHANNEL_WARNING:
		util::stream_format(std::cerr, args);
		break;
	case OSD_OUTPUT_CHANNEL_INFO:
	case OSD_OUTPUT_CHANNEL_LOG:
		util::stream_format(std::cout, args);
		break;
	case OSD_OUTPUT_CHANNEL_VERBOSE:
		if (verbose()) util::stream_format(std::cout, args);
		break;
	case OSD_OUTPUT_CHANNEL_DEBUG:
#ifdef MAME_DEBUG
		util::stream_format(std::cout, args);
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

	m_machine = &machine;

	auto &options = downcast<osd_options &>(machine.options());
	// extract the verbose printing option
	if (options.verbose())
		set_verbose(true);

	// ensure we get called on the way out
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&osd_common_t::osd_exit, this));


	/* now setup watchdog */
	int watchdog_timeout = options.watchdog();

	if (watchdog_timeout != 0)
	{
		m_watchdog = std::make_unique<osd_watchdog>();
		m_watchdog->setTimeout(watchdog_timeout);
	}
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
	if (m_watchdog != nullptr)
		m_watchdog->reset();
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

void osd_common_t::update_audio_stream(const int16_t *buffer, int samples_this_frame)
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
	if (m_sound != nullptr)
		m_sound->set_mastervolume(attenuation);
}


//-------------------------------------------------
//  customize_input_type_list - provide OSD
//  additions/modifications to the input list
//-------------------------------------------------

void osd_common_t::customize_input_type_list(std::vector<input_type_entry> &typelist)
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
//  get_slider_list - allocate and populate a
//  list of OS-dependent slider values.
//-------------------------------------------------

std::vector<ui::menu_item> osd_common_t::get_slider_list()
{
	// check if any window has dirty sliders
	bool dirty = false;
	for (const auto &window : window_list())
	{
		if (window->has_renderer() && window->renderer().sliders_dirty())
		{
			dirty = true;
			break;
		}
	}

	if (dirty)
	{
		m_sliders.clear();

		for (const auto &window : osd_common_t::window_list())
		{
			if (window->has_renderer())
			{
				std::vector<ui::menu_item> window_sliders = window->renderer().get_slider_list();
				m_sliders.insert(m_sliders.end(), window_sliders.begin(), window_sliders.end());
			}
		}
	}

	return m_sliders;
}


//-------------------------------------------------
//  add_audio_to_recording - append audio samples
//  to an AVI recording if one is active
//-------------------------------------------------

void osd_common_t::add_audio_to_recording(const int16_t *buffer, int samples_this_frame)
{
	// Do nothing
}


//-------------------------------------------------
//  execute_command - execute a command not yet
//  handled by the core
//-------------------------------------------------

bool osd_common_t::execute_command(const char *command)
{
	if (strcmp(command, OSDCOMMAND_LIST_NETWORK_ADAPTERS) == 0)
	{
		osd_module &om = select_module_options<osd_module>(OSD_NETDEV_PROVIDER);
		auto const &interfaces = get_netdev_list();
		if (interfaces.empty())
		{
			printf("No supported network interfaces were found\n");
		}
		else
		{
			printf("Available network interfaces:\n");
			for (auto &entry : interfaces)
			{
				printf("    %s\n", entry->description);
			}
		}
		om.exit();

		return true;
	}
	else if (strcmp(command, OSDCOMMAND_LIST_MIDI_DEVICES) == 0)
	{
		osd_module &om = select_module_options<osd_module>(OSD_MIDI_PROVIDER);
		auto const ports = dynamic_cast<midi_module &>(om).list_midi_ports();
		if (ports.empty())
		{
			printf("No MIDI ports were found\n");
		}
		else
		{
			printf("MIDI input ports:\n");
			for (auto const &port : ports)
			{
				if (port.input)
					printf(port.default_input ? "%s (default)\n" : "%s\n", port.name.c_str());
			}

			printf("\nMIDI output ports:\n");
			for (auto const &port : ports)
			{
				if (port.output)
					printf(port.default_output ? "%s (default)\n" : "%s\n", port.name.c_str());
			}
		}
		om.exit();

		return true;
	}

	return false;

}

static void output_notifier_callback(const char *outname, int32_t value, void *param)
{
	static_cast<osd_common_t*>(param)->notify(outname, value);
}

void osd_common_t::init_subsystems()
{
	// monitors have to be initialized before video init
	m_monitor_module = &select_module_options<monitor_module>(OSD_MONITOR_PROVIDER);

	// various modules depend on having a window handle
	m_render = &select_module_options<render_module>(OSD_RENDERER_PROVIDER);
	if (!video_init())
	{
		video_exit();
		osd_printf_error("video_init: Initialization failed!\n\n\n");
		fflush(stderr);
		fflush(stdout);
		exit(-1);
	}

	m_keyboard_input = &select_module_options<input_module>(OSD_KEYBOARDINPUT_PROVIDER);
	m_mouse_input = &select_module_options<input_module>(OSD_MOUSEINPUT_PROVIDER);
	m_lightgun_input = &select_module_options<input_module>(OSD_LIGHTGUNINPUT_PROVIDER);
	m_joystick_input = &select_module_options<input_module>(OSD_JOYSTICKINPUT_PROVIDER);

	m_font_module = &select_module_options<font_module>(OSD_FONT_PROVIDER);
	m_sound = &select_module_options<sound_module>(OSD_SOUND_PROVIDER);

	m_debugger = &select_module_options<debug_module>(OSD_DEBUG_PROVIDER);

	select_module_options<netdev_module>(OSD_NETDEV_PROVIDER);

	m_midi = &select_module_options<midi_module>(OSD_MIDI_PROVIDER);

	m_output = &select_module_options<output_module>(OSD_OUTPUT_PROVIDER);
	machine().output().set_global_notifier(output_notifier_callback, this);

	input_init();
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

bool osd_common_t::input_init()
{
	m_keyboard_input->input_init(machine());
	m_mouse_input->input_init(machine());
	m_lightgun_input->input_init(machine());
	m_joystick_input->input_init(machine());
	return true;
}

void osd_common_t::poll_input_modules(bool relative_reset)
{
	m_keyboard_input->poll_if_necessary(relative_reset);
	m_mouse_input->poll_if_necessary(relative_reset);
	m_lightgun_input->poll_if_necessary(relative_reset);
	m_joystick_input->poll_if_necessary(relative_reset);
}

void osd_common_t::exit_subsystems()
{
	video_exit();
}

void osd_common_t::video_exit()
{
}

void osd_common_t::window_exit()
{
}

void osd_common_t::osd_exit()
{
	// destroy the renderers before shutting down their parent module
	for (auto it = window_list().rbegin(); window_list().rend() != it; ++it)
		(*it)->renderer_reset();

	m_mod_man.exit();

	exit_subsystems();
}

osd_font::ptr osd_common_t::font_alloc()
{
	return m_font_module->font_alloc();
}

bool osd_common_t::get_font_families(std::string const &font_path, std::vector<std::pair<std::string, std::string> > &result)
{
	return m_font_module->get_font_families(font_path, result);
}

std::unique_ptr<osd::midi_input_port> osd_common_t::create_midi_input(std::string_view name)
{
	return m_midi->create_input(name);
}

std::unique_ptr<osd::midi_output_port> osd_common_t::create_midi_output(std::string_view name)
{
	return m_midi->create_output(name);
}
