// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    machine.h

    Controls execution of the core MAME system.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MACHINE_H__
#define __MACHINE_H__

#include "vecstream.h"

#include <time.h>

// forward declaration instead of osdepend.h
class osd_interface;

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// machine phases
enum machine_phase
{
	MACHINE_PHASE_PREINIT,
	MACHINE_PHASE_INIT,
	MACHINE_PHASE_RESET,
	MACHINE_PHASE_RUNNING,
	MACHINE_PHASE_EXIT
};


// notification callback types
enum machine_notification
{
	MACHINE_NOTIFY_FRAME,
	MACHINE_NOTIFY_RESET,
	MACHINE_NOTIFY_PAUSE,
	MACHINE_NOTIFY_RESUME,
	MACHINE_NOTIFY_EXIT,
	MACHINE_NOTIFY_COUNT
};


// debug flags
const int DEBUG_FLAG_ENABLED        = 0x00000001;       // debugging is enabled
const int DEBUG_FLAG_CALL_HOOK      = 0x00000002;       // CPU cores must call instruction hook
const int DEBUG_FLAG_WPR_PROGRAM    = 0x00000010;       // watchpoints are enabled for PROGRAM memory reads
const int DEBUG_FLAG_WPR_DATA       = 0x00000020;       // watchpoints are enabled for DATA memory reads
const int DEBUG_FLAG_WPR_IO         = 0x00000040;       // watchpoints are enabled for IO memory reads
const int DEBUG_FLAG_WPW_PROGRAM    = 0x00000100;       // watchpoints are enabled for PROGRAM memory writes
const int DEBUG_FLAG_WPW_DATA       = 0x00000200;       // watchpoints are enabled for DATA memory writes
const int DEBUG_FLAG_WPW_IO         = 0x00000400;       // watchpoints are enabled for IO memory writes
const int DEBUG_FLAG_OSD_ENABLED    = 0x00001000;       // The OSD debugger is enabled



//**************************************************************************
//  MACROS
//**************************************************************************

// global allocation helpers
#define auto_alloc(m, t)                pool_alloc(static_cast<running_machine &>(m).respool(), t)
#define auto_alloc_clear(m, t)          pool_alloc_clear(static_cast<running_machine &>(m).respool(), t)
#define auto_alloc_array(m, t, c)       pool_alloc_array(static_cast<running_machine &>(m).respool(), t, c)
#define auto_alloc_array_clear(m, t, c) pool_alloc_array_clear(static_cast<running_machine &>(m).respool(), t, c)
#define auto_free(m, v)                 pool_free(static_cast<running_machine &>(m).respool(), v)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class cheat_manager;
class render_manager;
class sound_manager;
class video_manager;
class ui_manager;
class tilemap_manager;
class debug_view_manager;
class network_manager;
class bookkeeping_manager;
class configuration_manager;
class output_manager;
class ui_input_manager;
class crosshair_manager;
class image_manager;
class rom_load_manager;
class debugger_manager;
class osd_interface;
class datfile_manager;
enum class config_type;
class inifile_manager;
class favorite_manager;
struct debugcpu_private;


// ======================> system_time

// system time description, both local and UTC
class system_time
{
public:
	system_time();
	void set(time_t t);

	struct full_time
	{
		void set(struct tm &t);

		UINT8       second;     // seconds (0-59)
		UINT8       minute;     // minutes (0-59)
		UINT8       hour;       // hours (0-23)
		UINT8       mday;       // day of month (1-31)
		UINT8       month;      // month (0-11)
		INT32       year;       // year (1=1 AD)
		UINT8       weekday;    // day of week (0-6)
		UINT16      day;        // day of year (0-365)
		UINT8       is_dst;     // is this daylight savings?
	};

	INT64           time;       // number of seconds elapsed since midnight, January 1 1970 UTC
	full_time       local_time; // local time
	full_time       utc_time;   // UTC coordinated time
};



// ======================> running_machine

typedef delegate<void ()> machine_notify_delegate;

// description of the currently-running machine
class running_machine
{
	DISABLE_COPYING(running_machine);

	friend class sound_manager;

	typedef void (*logerror_callback)(const running_machine &machine, const char *string);

	// must be at top of member variables
	resource_pool           m_respool;              // pool of resources for this machine

public:
	// construction/destruction
	running_machine(const machine_config &config, machine_manager &manager);
	~running_machine();

	// getters
	const machine_config &config() const { return m_config; }
	device_t &root_device() const { return m_config.root_device(); }
	const game_driver &system() const { return m_system; }
	osd_interface &osd() const;
	machine_manager &manager() const { return m_manager; }
	resource_pool &respool() { return m_respool; }
	device_scheduler &scheduler() { return m_scheduler; }
	save_manager &save() { return m_save; }
	memory_manager &memory() { return m_memory; }
	ioport_manager &ioport() { return m_ioport; }
	parameters_manager &parameters() { return m_parameters; }
	cheat_manager &cheat() const { assert(m_cheat != nullptr); return *m_cheat; }
	datfile_manager &datfile() const { assert(m_datfile != nullptr); return *m_datfile; }
	inifile_manager &inifile() const { assert(m_inifile != nullptr); return *m_inifile; }
	favorite_manager &favorite() const { assert(m_favorite != nullptr); return *m_favorite; }
	render_manager &render() const { assert(m_render != nullptr); return *m_render; }
	input_manager &input() const { assert(m_input != nullptr); return *m_input; }
	sound_manager &sound() const { assert(m_sound != nullptr); return *m_sound; }
	video_manager &video() const { assert(m_video != nullptr); return *m_video; }
	network_manager &network() const { assert(m_network != nullptr); return *m_network; }
	bookkeeping_manager &bookkeeping() const { assert(m_network != nullptr); return *m_bookkeeping; }
	configuration_manager  &configuration() const { assert(m_configuration != nullptr); return *m_configuration; }
	output_manager  &output() const { assert(m_output != nullptr); return *m_output; }
	ui_manager &ui() const { assert(m_ui != nullptr); return *m_ui; }
	ui_input_manager &ui_input() const { assert(m_ui_input != nullptr); return *m_ui_input; }
	crosshair_manager &crosshair() const { assert(m_crosshair != nullptr); return *m_crosshair; }
	image_manager &image() const { assert(m_image != nullptr); return *m_image; }
	rom_load_manager &rom_load() const { assert(m_rom_load != nullptr); return *m_rom_load; }
	tilemap_manager &tilemap() const { assert(m_tilemap != nullptr); return *m_tilemap; }
	debug_view_manager &debug_view() const { assert(m_debug_view != nullptr); return *m_debug_view; }
	debugger_manager &debugger() const { assert(m_debugger != nullptr); return *m_debugger; }
	driver_device *driver_data() const { return &downcast<driver_device &>(root_device()); }
	template<class _DriverClass> _DriverClass *driver_data() const { return &downcast<_DriverClass &>(root_device()); }
	machine_phase phase() const { return m_current_phase; }
	bool paused() const { return m_paused || (m_current_phase != MACHINE_PHASE_RUNNING); }
	bool exit_pending() const { return m_exit_pending; }
	bool ui_active() const { return m_ui_active; }
	const char *basename() const { return m_basename.c_str(); }
	int sample_rate() const { return m_sample_rate; }
	bool save_or_load_pending() const { return !m_saveload_pending_file.empty(); }
	screen_device *first_screen() const { return primary_screen; }

	// additional helpers
	emu_options &options() const { return m_config.options(); }
	attotime time() const { return m_scheduler.time(); }
	bool scheduled_event_pending() const { return m_exit_pending || m_hard_reset_pending; }

	// fetch items by name
	inline device_t *device(const char *tag) const { return root_device().subdevice(tag); }
	template<class _DeviceClass> inline _DeviceClass *device(const char *tag) { return downcast<_DeviceClass *>(device(tag)); }

	// immediate operations
	int run(bool firstrun);
	void pause();
	void resume();
	void toggle_pause();
	void add_notifier(machine_notification event, machine_notify_delegate callback, bool first = false);
	void call_notifiers(machine_notification which);
	void add_logerror_callback(logerror_callback callback);
	void set_ui_active(bool active) { m_ui_active = active; }

	// TODO: Do saves and loads still require scheduling?
	void immediate_save(const char *filename);
	void immediate_load(const char *filename);

	// scheduled operations
	void schedule_exit();
	void schedule_hard_reset();
	void schedule_soft_reset();
	void schedule_save(const char *filename);
	void schedule_load(const char *filename);

	// date & time
	void base_datetime(system_time &systime);
	void current_datetime(system_time &systime);

	// watchdog control
	void watchdog_reset();
	void watchdog_enable(bool enable = true);
	INT32 get_vblank_watchdog_counter() const { return m_watchdog_counter; }

	// misc
	void popmessage() const { popmessage(static_cast<char const *>(nullptr)); }
	template <typename Format, typename... Params> void popmessage(Format &&fmt, Params &&... args) const;
	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;
	UINT32 rand();
	const char *describe_context();

	// CPU information
	cpu_device *            firstcpu;           // first CPU

private:
	// video-related information
	screen_device *         primary_screen;     // the primary screen device, or NULL if screenless

public:
	// debugger-related information
	UINT32                  debug_flags;        // the current debug flags

	// internal core information
	debugcpu_private *      debugcpu_data;      // internal data from debugcpu.c

private:
	// internal helpers
	template <typename T> struct is_null { template <typename U> static bool value(U &&x) { return false; } };
	template <typename T> struct is_null<T *> { template <typename U> static bool value(U &&x) { return !x; } };
	void start();
	void set_saveload_filename(const char *filename);
	std::string get_statename(const char *statename_opt) const;
	void handle_saveload();
	void soft_reset(void *ptr = nullptr, INT32 param = 0);
	void watchdog_fired(void *ptr = nullptr, INT32 param = 0);
	void watchdog_vblank(screen_device &screen, bool vblank_state);
	std::string nvram_filename(device_t &device) const;
	void nvram_load();
	void nvram_save();

	// internal callbacks
	static void logfile_callback(const running_machine &machine, const char *buffer);

	// internal device helpers
	void start_all_devices();
	void reset_all_devices();
	void stop_all_devices();
	void presave_all_devices();
	void postload_all_devices();

	TIMER_CALLBACK_MEMBER(autoboot_callback);

	// internal state
	const machine_config &  m_config;               // reference to the constructed machine_config
	const game_driver &     m_system;               // reference to the definition of the game machine
	machine_manager &       m_manager;              // reference to machine manager system
	// managers
	std::unique_ptr<cheat_manager> m_cheat;            // internal data from cheat.cpp
	std::unique_ptr<render_manager> m_render;          // internal data from render.cpp
	std::unique_ptr<input_manager> m_input;            // internal data from input.cpp
	std::unique_ptr<sound_manager> m_sound;            // internal data from sound.cpp
	std::unique_ptr<video_manager> m_video;            // internal data from video.cpp
	std::unique_ptr<ui_manager> m_ui;                  // internal data from ui.cpp
	std::unique_ptr<ui_input_manager> m_ui_input;      // internal data from uiinput.cpp
	std::unique_ptr<tilemap_manager> m_tilemap;        // internal data from tilemap.cpp
	std::unique_ptr<debug_view_manager> m_debug_view;  // internal data from debugvw.cpp
	std::unique_ptr<network_manager> m_network;        // internal data from network.cpp
	std::unique_ptr<bookkeeping_manager> m_bookkeeping;// internal data from bookkeeping.cpp
	std::unique_ptr<configuration_manager> m_configuration; // internal data from config.cpp
	std::unique_ptr<output_manager> m_output;          // internal data from output.cpp
	std::unique_ptr<crosshair_manager> m_crosshair;    // internal data from crsshair.cpp
	std::unique_ptr<image_manager> m_image;            // internal data from image.cpp
	std::unique_ptr<rom_load_manager> m_rom_load;      // internal data from romload.cpp
	std::unique_ptr<debugger_manager> m_debugger;      // internal data from debugger.cpp

	// system state
	machine_phase           m_current_phase;        // current execution phase
	bool                    m_paused;               // paused?
	bool                    m_hard_reset_pending;   // is a hard reset pending?
	bool                    m_exit_pending;         // is an exit pending?
	emu_timer *             m_soft_reset_timer;     // timer used to schedule a soft reset

	// watchdog state
	bool                    m_watchdog_enabled;     // is the watchdog enabled?
	INT32                   m_watchdog_counter;     // counter for watchdog tracking
	emu_timer *             m_watchdog_timer;       // timer for watchdog tracking

	// misc state
	UINT32                  m_rand_seed;            // current random number seed
	bool                    m_ui_active;            // ui active or not (useful for games / systems with keyboard inputs)
	time_t                  m_base_time;            // real time at initial emulation time
	std::string             m_basename;             // basename used for game-related paths
	std::string             m_context;              // context string buffer
	int                     m_sample_rate;          // the digital audio sample rate
	std::unique_ptr<emu_file>  m_logfile;              // pointer to the active log file

	// load/save management
	enum saveload_schedule
	{
		SLS_NONE,
		SLS_SAVE,
		SLS_LOAD
	};
	saveload_schedule       m_saveload_schedule;
	attotime                m_saveload_schedule_time;
	std::string             m_saveload_pending_file;
	const char *            m_saveload_searchpath;

	// notifier callbacks
	struct notifier_callback_item
	{
		// construction/destruction
		notifier_callback_item(machine_notify_delegate func);

		// state
		machine_notify_delegate     m_func;
	};
	std::list<std::unique_ptr<notifier_callback_item>> m_notifier_list[MACHINE_NOTIFY_COUNT];

	// logerror callbacks
	class logerror_callback_item
	{
	public:
		// construction/destruction
		logerror_callback_item(logerror_callback func);

		// state
		logerror_callback           m_func;
	};
	std::list<std::unique_ptr<logerror_callback_item>> m_logerror_list;

	// embedded managers and objects
	save_manager            m_save;                 // save manager
	memory_manager          m_memory;               // memory manager
	ioport_manager          m_ioport;               // I/O port manager
	parameters_manager      m_parameters;           // parameters manager
	device_scheduler        m_scheduler;            // scheduler object
	emu_timer               *m_autoboot_timer;      // autoboot timer

	std::unique_ptr<datfile_manager>   m_datfile;      // internal data from datfile.c
	std::unique_ptr<inifile_manager>   m_inifile;      // internal data from inifile.c for INIs
	std::unique_ptr<favorite_manager>  m_favorite;     // internal data from inifile.c for favorites

	// string formatting buffer
	mutable util::ovectorstream m_string_buffer;
};

#endif  /* __MACHINE_H__ */
