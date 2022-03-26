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

#ifndef MAME_EMU_MACHINE_H
#define MAME_EMU_MACHINE_H

#include <functional>

#include <ctime>

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// machine phases
enum class machine_phase
{
	PREINIT,
	INIT,
	RESET,
	RUNNING,
	EXIT
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
constexpr int DEBUG_FLAG_ENABLED        = 0x00000001;       // debugging is enabled
constexpr int DEBUG_FLAG_CALL_HOOK      = 0x00000002;       // CPU cores must call instruction hook
constexpr int DEBUG_FLAG_OSD_ENABLED    = 0x00001000;       // The OSD debugger is enabled



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> system_time

// system time description, both local and UTC
class system_time
{
public:
	system_time();
	explicit system_time(time_t t);
	void set(time_t t);

	struct full_time
	{
		void set(struct tm &t);

		u8          second;     // seconds (0-59)
		u8          minute;     // minutes (0-59)
		u8          hour;       // hours (0-23)
		u8          mday;       // day of month (1-31)
		u8          month;      // month (0-11)
		s32         year;       // year (1=1 AD)
		u8          weekday;    // day of week (0-6)
		u16         day;        // day of year (0-365)
		u8          is_dst;     // is this daylight savings?
	};

	s64           time;       // number of seconds elapsed since midnight, January 1 1970 UTC
	full_time       local_time; // local time
	full_time       utc_time;   // UTC coordinated time
};



// ======================> running_machine

typedef delegate<void ()> machine_notify_delegate;

// description of the currently-running machine
class running_machine
{
	DISABLE_COPYING(running_machine);

	class side_effects_disabler;

	friend class sound_manager;
	friend class memory_manager;

	typedef std::function<void (const char*)> logerror_callback;

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
	device_scheduler &scheduler() { return m_scheduler; }
	save_manager &save() { return m_save; }
	memory_manager &memory() { return m_memory; }
	ioport_manager &ioport() { return m_ioport; }
	parameters_manager &parameters() { return m_parameters; }
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
	natural_keyboard &natkeyboard() noexcept { assert(m_natkeyboard != nullptr); return *m_natkeyboard; }
	template <class DriverClass> DriverClass *driver_data() const { return &downcast<DriverClass &>(root_device()); }
	machine_phase phase() const { return m_current_phase; }
	bool paused() const { return m_paused || (m_current_phase != machine_phase::RUNNING); }
	bool exit_pending() const { return m_exit_pending; }
	bool hard_reset_pending() const { return m_hard_reset_pending; }
	bool ui_active() const { return m_ui_active; }
	const std::string &basename() const { return m_basename; }
	int sample_rate() const { return m_sample_rate; }
	bool save_or_load_pending() const { return !m_saveload_pending_file.empty(); }

	// RAII-based side effect disable
	// NOP-ed when passed false, to make it more easily conditional
	side_effects_disabler disable_side_effects(bool disable_se = true) { return side_effects_disabler(this, disable_se); }
	bool side_effects_disabled() const { return m_side_effects_disabled != 0; }

	// additional helpers
	emu_options &options() const { return m_config.options(); }
	attotime time() const noexcept { return m_scheduler.time(); }
	bool scheduled_event_pending() const { return m_exit_pending || m_hard_reset_pending; }
	bool allow_logging() const { return !m_logerror_list.empty(); }

	// fetch items by name
	template <class DeviceClass> [[deprecated("absolute tag lookup; use subdevice or finder instead")]] inline DeviceClass *device(const char *tag) { return downcast<DeviceClass *>(root_device().subdevice(tag)); }

	// immediate operations
	int run(bool quiet);
	void pause();
	void resume();
	void toggle_pause();
	void add_notifier(machine_notification event, machine_notify_delegate callback, bool first = false);
	void call_notifiers(machine_notification which);
	void add_logerror_callback(logerror_callback callback);
	void set_ui_active(bool active) { m_ui_active = active; }
	void debug_break();
	void export_http_api();

	// TODO: Do saves and loads still require scheduling?
	void immediate_save(std::string_view filename);
	void immediate_load(std::string_view filename);

	// rewind operations
	bool rewind_capture();
	bool rewind_step();
	void rewind_invalidate();

	// scheduled operations
	void schedule_exit();
	void schedule_hard_reset();
	void schedule_soft_reset();
	void schedule_save(std::string &&filename);
	void schedule_load(std::string &&filename);

	// date & time
	void base_datetime(system_time &systime);
	void current_datetime(system_time &systime);
	void set_rtc_datetime(const system_time &systime);

	// misc
	void popmessage() const { popmessage(static_cast<char const *>(nullptr)); }
	template <typename Format, typename... Params> void popmessage(Format &&fmt, Params &&... args) const;
	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;
	void strlog(const char *str) const;
	u32 rand();
	std::string describe_context() const;
	std::string compose_saveload_filename(std::string &&base_filename, const char **searchpath = nullptr);
	std::string get_statename(const char *statename_opt) const;

private:
	// side effect disable counter
	u32                     m_side_effects_disabled;

public:
	// debugger-related information
	u32                     debug_flags;        // the current debug flags
	bool debug_enabled() { return (debug_flags & DEBUG_FLAG_ENABLED) != 0; }

	// used by debug_console to take ownership of the debug.log file
	std::unique_ptr<emu_file> steal_debuglogfile();

private:
	class side_effects_disabler {
		running_machine *m_machine;
		bool m_disable_se;

	public:
		side_effects_disabler(running_machine *m, bool disable_se) : m_machine(m), m_disable_se(disable_se) {
			if(m_disable_se)
				m_machine->disable_side_effects_count();
		}

		~side_effects_disabler() {
			if(m_disable_se)
				m_machine->enable_side_effects_count();
		}

		side_effects_disabler(const side_effects_disabler &) = delete;
		side_effects_disabler(side_effects_disabler &&) = default;
	};

	void disable_side_effects_count() { m_side_effects_disabled++; }
	void enable_side_effects_count()  { m_side_effects_disabled--; }

	// internal helpers
	template <typename T> struct is_null { template <typename U> static bool value(U &&x) { return false; } };
	template <typename T> struct is_null<T *> { template <typename U> static bool value(U &&x) { return !x; } };
	void start();
	void set_saveload_filename(std::string &&filename);
	void handle_saveload();
	void soft_reset(s32 param = 0);
	std::string nvram_filename(device_t &device) const;
	void nvram_load();
	void nvram_save();
	void popup_clear() const;
	void popup_message(util::format_argument_pack<std::ostream> const &args) const;

	// internal callbacks
	void logfile_callback(const char *buffer);

	// internal device helpers
	void start_all_devices();
	void reset_all_devices();
	void stop_all_devices();
	void presave_all_devices();
	void postload_all_devices();

	// internal state
	const machine_config &  m_config;               // reference to the constructed machine_config
	const game_driver &     m_system;               // reference to the definition of the game machine
	machine_manager &       m_manager;              // reference to machine manager system
	// managers
	std::unique_ptr<render_manager> m_render;          // internal data from render.cpp
	std::unique_ptr<input_manager> m_input;            // internal data from input.cpp
	std::unique_ptr<sound_manager> m_sound;            // internal data from sound.cpp
	std::unique_ptr<video_manager> m_video;            // internal data from video.cpp
	ui_manager *m_ui;                                  // internal data from ui.cpp
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
	std::unique_ptr<natural_keyboard> m_natkeyboard;   // internal data from natkeyboard.cpp

	// system state
	machine_phase           m_current_phase;        // current execution phase
	bool                    m_paused;               // paused?
	bool                    m_hard_reset_pending;   // is a hard reset pending?
	bool                    m_exit_pending;         // is an exit pending?
	emu_timer *             m_soft_reset_timer;     // timer used to schedule a soft reset

	// misc state
	u32                     m_rand_seed;            // current random number seed
	bool                    m_ui_active;            // ui active or not (useful for games / systems with keyboard inputs)
	time_t                  m_base_time;            // real time at initial emulation time
	std::string             m_basename;             // basename used for game-related paths
	int                     m_sample_rate;          // the digital audio sample rate
	std::unique_ptr<emu_file>  m_logfile;           // pointer to the active error.log file
	std::unique_ptr<emu_file>  m_debuglogfile;      // pointer to the active debug.log file

	// load/save management
	enum class saveload_schedule
	{
		NONE,
		SAVE,
		LOAD
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

	// string formatting buffer
	mutable util::ovectorstream m_string_buffer;

#if defined(__EMSCRIPTEN__)
private:
	static running_machine *emscripten_running_machine;
	static void emscripten_main_loop();
public:
	static void emscripten_set_running_machine(running_machine *machine);
	static running_machine * emscripten_get_running_machine();
	static ui_manager * emscripten_get_ui();
	static sound_manager * emscripten_get_sound();

	static void emscripten_exit();
	static void emscripten_hard_reset();
	static void emscripten_soft_reset();
	static void emscripten_save(const char *name);
	static void emscripten_load(const char *name);
#endif
};



//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

/*-------------------------------------------------
    popmessage - pop up a user-visible message
-------------------------------------------------*/

template <typename Format, typename... Params>
inline void running_machine::popmessage(Format &&fmt, Params &&... args) const
{
	// if the format is nullptr, it is a signal to clear the popmessage
	// otherwise, generate the buffer and call the UI to display the message
	if (is_null<Format>::value(fmt))
		popup_clear();
	else
		popup_message(util::make_format_argument_pack(std::forward<Format>(fmt), std::forward<Params>(args)...));
}


/*-------------------------------------------------
    logerror - log to the debugger and any other
    OSD-defined output streams
-------------------------------------------------*/

template <typename Format, typename... Params>
inline void running_machine::logerror(Format &&fmt, Params &&... args) const
{
	// process only if there is a target
	if (allow_logging())
	{
		g_profiler.start(PROFILER_LOGERROR);

		// dump to the buffer
		m_string_buffer.clear();
		m_string_buffer.seekp(0);
		util::stream_format(m_string_buffer, std::forward<Format>(fmt), std::forward<Params>(args)...);
		m_string_buffer.put('\0');

		strlog(&m_string_buffer.vec()[0]);

		g_profiler.stop();
	}
}

#endif  /* MAME_EMU_MACHINE_H */
