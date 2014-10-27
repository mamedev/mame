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

#include <time.h>



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

#define auto_bitmap_alloc(m, w, h, f)   auto_alloc(m, bitmap_t(w, h, f))
#define auto_bitmap_ind8_alloc(m, w, h) auto_alloc(m, bitmap_ind8(w, h))
#define auto_bitmap_ind16_alloc(m, w, h)    auto_alloc(m, bitmap_ind16(w, h))
#define auto_bitmap_ind32_alloc(m, w, h)    auto_alloc(m, bitmap_ind32(w, h))
#define auto_bitmap_rgb32_alloc(m, w, h)    auto_alloc(m, bitmap_rgb32(w, h))



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
class osd_interface;

struct romload_private;
struct ui_input_private;
struct debugcpu_private;
struct generic_machine_private;


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

	friend void debugger_init(running_machine &machine);
	friend class sound_manager;

	typedef void (*logerror_callback)(running_machine &machine, const char *string);

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
	osd_interface &osd() const { return m_manager.osd(); }
	machine_manager &manager() const { return m_manager; }
	resource_pool &respool() { return m_respool; }
	device_scheduler &scheduler() { return m_scheduler; }
	save_manager &save() { return m_save; }
	memory_manager &memory() { return m_memory; }
	ioport_manager &ioport() { return m_ioport; }
	cheat_manager &cheat() const { assert(m_cheat != NULL); return *m_cheat; }
	render_manager &render() const { assert(m_render != NULL); return *m_render; }
	input_manager &input() const { assert(m_input != NULL); return *m_input; }
	sound_manager &sound() const { assert(m_sound != NULL); return *m_sound; }
	video_manager &video() const { assert(m_video != NULL); return *m_video; }
	ui_manager &ui() const { assert(m_ui != NULL); return *m_ui; }
	tilemap_manager &tilemap() const { assert(m_tilemap != NULL); return *m_tilemap; }
	debug_view_manager &debug_view() const { assert(m_debug_view != NULL); return *m_debug_view; }
	driver_device *driver_data() const { return &downcast<driver_device &>(root_device()); }
	template<class _DriverClass> _DriverClass *driver_data() const { return &downcast<_DriverClass &>(root_device()); }
	machine_phase phase() const { return m_current_phase; }
	bool paused() const { return m_paused || (m_current_phase != MACHINE_PHASE_RUNNING); }
	bool exit_pending() const { return m_exit_pending; }
	bool ui_active() const { return m_ui_active; }
	const char *basename() const { return m_basename; }
	int sample_rate() const { return m_sample_rate; }
	bool save_or_load_pending() const { return m_saveload_pending_file; }
	screen_device *first_screen() const { return primary_screen; }

	// additional helpers
	emu_options &options() const { return m_config.options(); }
	attotime time() const { return m_scheduler.time(); }
	bool scheduled_event_pending() const { return m_exit_pending || m_hard_reset_pending; }

	// fetch items by name
	inline device_t *device(const char *tag) { return root_device().subdevice(tag); }
	template<class _DeviceClass> inline _DeviceClass *device(const char *tag) { return downcast<_DeviceClass *>(device(tag)); }

	// configuration helpers
	device_t &add_dynamic_device(device_t &owner, device_type type, const char *tag, UINT32 clock);

	// immediate operations
	int run(bool firstrun);
	void pause();
	void resume();
	void toggle_pause();
	void add_notifier(machine_notification event, machine_notify_delegate callback);
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
	void schedule_new_driver(const game_driver &driver);
	void schedule_save(const char *filename);
	void schedule_load(const char *filename);

	// date & time
	void base_datetime(system_time &systime);
	void current_datetime(system_time &systime);

	// watchdog control
	void watchdog_reset();
	void watchdog_enable(bool enable = true);

	// misc
	void CLIB_DECL vlogerror(const char *format, va_list args);
	UINT32 rand();
	const char *describe_context();

	// CPU information
	cpu_device *            firstcpu;           // first CPU
	device_t *		        cpu[8];             // MKChamp--CPU for hiscore support 

private:
	// video-related information
	screen_device *         primary_screen;     // the primary screen device, or NULL if screenless

public:
	// debugger-related information
	UINT32                  debug_flags;        // the current debug flags

	// internal core information
	romload_private *       romload_data;       // internal data from romload.c
	ui_input_private *      ui_input_data;      // internal data from uiinput.c
	debugcpu_private *      debugcpu_data;      // internal data from debugcpu.c
	generic_machine_private *generic_machine_data; // internal data from machine/generic.c

private:
	// internal helpers
	void start();
	void set_saveload_filename(const char *filename);
	astring get_statename(const char *statename_opt);
	void fill_systime(system_time &systime, time_t t);
	void handle_saveload();
	void soft_reset(void *ptr = NULL, INT32 param = 0);
	void watchdog_fired(void *ptr = NULL, INT32 param = 0);
	void watchdog_vblank(screen_device &screen, bool vblank_state);
	const char *image_parent_basename(device_t *device);
	astring &nvram_filename(astring &result, device_t &device);
	void nvram_load();
	void nvram_save();

	// internal callbacks
	static void logfile_callback(running_machine &machine, const char *buffer);

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
	auto_pointer<cheat_manager> m_cheat;            // internal data from cheat.c
	auto_pointer<render_manager> m_render;          // internal data from render.c
	auto_pointer<input_manager> m_input;            // internal data from input.c
	auto_pointer<sound_manager> m_sound;            // internal data from sound.c
	auto_pointer<video_manager> m_video;            // internal data from video.c
	auto_pointer<ui_manager> m_ui;                  // internal data from ui.c
	auto_pointer<tilemap_manager> m_tilemap;        // internal data from tilemap.c
	auto_pointer<debug_view_manager> m_debug_view;  // internal data from debugvw.c

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
	astring                 m_basename;             // basename used for game-related paths
	astring                 m_context;              // context string buffer
	int                     m_sample_rate;          // the digital audio sample rate
	auto_pointer<emu_file>  m_logfile;              // pointer to the active log file

	// load/save management
	enum saveload_schedule
	{
		SLS_NONE,
		SLS_SAVE,
		SLS_LOAD
	};
	saveload_schedule       m_saveload_schedule;
	attotime                m_saveload_schedule_time;
	astring                 m_saveload_pending_file;
	const char *            m_saveload_searchpath;

	// notifier callbacks
	struct notifier_callback_item
	{
		// construction/destruction
		notifier_callback_item(machine_notify_delegate func);

		// getters
		notifier_callback_item *next() const { return m_next; }

		// state
		notifier_callback_item *    m_next;
		machine_notify_delegate     m_func;
	};
	simple_list<notifier_callback_item> m_notifier_list[MACHINE_NOTIFY_COUNT];

	// logerror callbacks
	class logerror_callback_item
	{
	public:
		// construction/destruction
		logerror_callback_item(logerror_callback func);

		// getters
		logerror_callback_item *next() const { return m_next; }

		// state
		logerror_callback_item *    m_next;
		logerror_callback           m_func;
	};
	simple_list<logerror_callback_item> m_logerror_list;

	// embedded managers and objects
	save_manager            m_save;                 // save manager
	memory_manager          m_memory;               // memory manager
	ioport_manager          m_ioport;               // I/O port manager
	device_scheduler        m_scheduler;            // scheduler object
	emu_timer               *m_autoboot_timer;      // autoboot timer
};


#endif  /* __MACHINE_H__ */
