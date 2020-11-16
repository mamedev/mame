// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine.cpp

    Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "luaengine.ipp"

#include "mame.h"
#include "pluginopts.h"
#include "ui/pluginopt.h"
#include "ui/ui.h"

#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/points.h"
#include "debug/textbuf.h"
#include "debugger.h"
#include "drivenum.h"
#include "emuopts.h"
#include "inputdev.h"
#include "natkeyboard.h"
#include "softlist.h"
#include "uiinput.h"

#include <cstring>
#include <thread>


//**************************************************************************
//  LUA ENGINE
//**************************************************************************

extern "C" {
	int luaopen_zlib(lua_State *L);
	int luaopen_lfs(lua_State *L);
	int luaopen_linenoise(lua_State *L);
	int luaopen_lsqlite3(lua_State *L);
}

namespace sol
{
	sol::buffer *sol_lua_get(sol::types<buffer *>, lua_State *L, int index, sol::stack::record &tracking)
	{
		return new sol::buffer(stack::get<int>(L, index), L);
	}
	int sol_lua_push(sol::types<buffer *>, lua_State *L, buffer *value)
	{
		delete value;
		return 1;
	}
}

int sol_lua_push(sol::types<osd_file::error>, lua_State *L, osd_file::error &&value)
{
	const char *strerror;
	switch(value)
	{
		case osd_file::error::NONE:
			return sol::stack::push(L, sol::lua_nil);
		case osd_file::error::FAILURE:
			strerror = "failure";
			break;
		case osd_file::error::OUT_OF_MEMORY:
			strerror = "out_of_memory";
			break;
		case osd_file::error::NOT_FOUND:
			strerror = "not_found";
			break;
		case osd_file::error::ACCESS_DENIED:
			strerror = "access_denied";
			break;
		case osd_file::error::ALREADY_OPEN:
			strerror = "already_open";
			break;
		case osd_file::error::TOO_MANY_FILES:
			strerror = "too_many_files";
			break;
		case osd_file::error::INVALID_DATA:
			strerror = "invalid_data";
			break;
		case osd_file::error::INVALID_ACCESS:
			strerror = "invalid_access";
			break;
		default:
			strerror = "unknown_error";
			break;
	}
	return sol::stack::push(L, strerror);
}

template <typename Handler>
bool sol_lua_check(sol::types<osd_file::error>, lua_State *L, int index, Handler &&handler, sol::stack::record &tracking)
{
	return sol::stack::check<int>(L, index, std::forward<Handler>(handler));
}

int sol_lua_push(sol::types<map_handler_type>, lua_State *L, map_handler_type &&value)
{
	const char *typestr;
	switch(value)
	{
		case AMH_NONE:
			typestr = "none";
			break;
		case AMH_RAM:
			typestr = "ram";
			break;
		case AMH_ROM:
			typestr = "rom";
			break;
		case AMH_NOP:
			typestr = "nop";
			break;
		case AMH_UNMAP:
			typestr = "unmap";
			break;
		case AMH_DEVICE_DELEGATE:
		case AMH_DEVICE_DELEGATE_M:
		case AMH_DEVICE_DELEGATE_S:
		case AMH_DEVICE_DELEGATE_SM:
		case AMH_DEVICE_DELEGATE_MO:
		case AMH_DEVICE_DELEGATE_SMO:
			typestr = "delegate";
			break;
		case AMH_PORT:
			typestr = "port";
			break;
		case AMH_BANK:
			typestr = "bank";
			break;
		case AMH_DEVICE_SUBMAP:
			typestr = "submap";
			break;
		default:
			typestr = "unknown";
			break;
	}
	return sol::stack::push(L, typestr);
}


//-------------------------------------------------
//  process_snapshot_filename - processes a snapshot
//  filename
//-------------------------------------------------

static std::string process_snapshot_filename(running_machine &machine, const char *s)
{
	std::string result(s);
	if (!osd_is_absolute_path(s))
	{
		strreplace(result, "/", PATH_SEPARATOR);
		strreplace(result, "%g", machine.basename());
	}
	return result;
}


//-------------------------------------------------
//  lua_engine - constructor
//-------------------------------------------------

lua_engine::lua_engine()
{
	m_machine = nullptr;
	m_lua_state = luaL_newstate();  /* create state */
	m_sol_state = std::make_unique<sol::state_view>(m_lua_state); // create sol view

	luaL_checkversion(m_lua_state);
	lua_gc(m_lua_state, LUA_GCSTOP, 0);  /* stop collector during initialization */
	sol().open_libraries();

	// Get package.preload so we can store builtins in it.
	sol()["package"]["preload"]["zlib"] = &luaopen_zlib;
	sol()["package"]["preload"]["lfs"] = &luaopen_lfs;
	sol()["package"]["preload"]["linenoise"] = &luaopen_linenoise;
	sol()["package"]["preload"]["lsqlite3"] = &luaopen_lsqlite3;

	lua_gc(m_lua_state, LUA_GCRESTART, 0);
}

//-------------------------------------------------
//  ~lua_engine - destructor
//-------------------------------------------------

lua_engine::~lua_engine()
{
	close();
}

sol::object lua_engine::call_plugin(const std::string &name, sol::object in)
{
	std::string field = "cb_" + name;
	sol::object obj = sol().registry()[field];
	if(obj.is<sol::protected_function>())
	{
		auto res = invoke(obj.as<sol::protected_function>(), in);
		if(!res.valid())
		{
			sol::error err = res;
			osd_printf_error("[LUA ERROR] in call_plugin: %s\n", err.what());
		}
		else
			return res.get<sol::object>();
	}
	return sol::make_object(sol(), sol::lua_nil);
}

void lua_engine::menu_populate(const std::string &menu, std::vector<std::tuple<std::string, std::string, std::string>> &menu_list)
{
	std::string field = "menu_pop_" + menu;
	sol::object obj = sol().registry()[field];
	if(obj.is<sol::protected_function>())
	{
		auto res = invoke(obj.as<sol::protected_function>());
		if(!res.valid())
		{
			sol::error err = res;
			osd_printf_error("[LUA ERROR] in menu_populate: %s\n", err.what());
		}
		else
		{
			sol::table table = res;
			for(auto &entry : table)
			{
				if(entry.second.is<sol::table>())
				{
					sol::table enttable = entry.second.as<sol::table>();
					menu_list.emplace_back(enttable.get<std::string, std::string, std::string>(1, 2, 3));
				}
			}
		}
	}
}

bool lua_engine::menu_callback(const std::string &menu, int index, const std::string &event)
{
	std::string field = "menu_cb_" + menu;
	bool ret = false;
	sol::object obj = sol().registry()[field];
	if(obj.is<sol::protected_function>())
	{
		auto res = invoke(obj.as<sol::protected_function>(), index, event);
		if(!res.valid())
		{
			sol::error err = res;
			osd_printf_error("[LUA ERROR] in menu_callback: %s\n", err.what());
		}
		else
			ret = res;
	}
	return ret;
}

void lua_engine::set_machine(running_machine *machine)
{
	if (!machine || (machine != m_machine))
		m_seq_poll.reset();
	m_machine = machine;
}

int lua_engine::enumerate_functions(const char *id, std::function<bool(const sol::protected_function &func)> &&callback)
{
	int count = 0;
	sol::object functable = sol().registry()[id];
	if (functable.is<sol::table>())
	{
		for (auto &func : functable.as<sol::table>())
		{
			if (func.second.is<sol::protected_function>())
			{
				bool cont = callback(func.second.as<sol::protected_function>());
				count++;
				if (!cont)
					break;
			}
		}
		return true;
	}
	return count;
}

bool lua_engine::execute_function(const char *id)
{
	int count = enumerate_functions(id, [this](const sol::protected_function &func)
	{
		auto ret = invoke(func);
		if(!ret.valid())
		{
			sol::error err = ret;
			osd_printf_error("[LUA ERROR] in execute_function: %s\n", err.what());
		}
		return true;
	});
	return count > 0;
}

void lua_engine::register_function(sol::function func, const char *id)
{
	sol::object functable = sol().registry()[id];
	if(functable.is<sol::table>())
		functable.as<sol::table>().add(func);
	else
		sol().registry().create_named(id, 1, func);
}

void lua_engine::on_machine_prestart()
{
	execute_function("LUA_ON_PRESTART");
}

void lua_engine::on_machine_start()
{
	execute_function("LUA_ON_START");
}

void lua_engine::on_machine_stop()
{
	execute_function("LUA_ON_STOP");
}

void lua_engine::on_machine_before_load_settings()
{
	execute_function("LUA_ON_BEFORE_LOAD_SETTINGS");
}

void lua_engine::on_machine_pause()
{
	execute_function("LUA_ON_PAUSE");
}

void lua_engine::on_machine_resume()
{
	execute_function("LUA_ON_RESUME");
}

void lua_engine::on_machine_frame()
{
	execute_function("LUA_ON_FRAME");
}

void lua_engine::on_frame_done()
{
	execute_function("LUA_ON_FRAME_DONE");
}

void lua_engine::on_sound_update()
{
	execute_function("LUA_ON_SOUND_UPDATE");
}

void lua_engine::on_periodic()
{
	execute_function("LUA_ON_PERIODIC");
}

bool lua_engine::on_missing_mandatory_image(const std::string &instance_name)
{
	bool handled = false;
	enumerate_functions("LUA_ON_MANDATORY_FILE_MANAGER_OVERRIDE", [this, &instance_name, &handled](const sol::protected_function &func)
	{
		auto ret = invoke(func, instance_name);

		if(!ret.valid())
		{
			sol::error err = ret;
			osd_printf_error("[LUA ERROR] in on_missing_mandatory_image: %s\n", err.what());
		}
		else if (ret.get<bool>())
		{
			handled = true;
		}
		return !handled;
	});
	return handled;
}

void lua_engine::attach_notifiers()
{
	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&lua_engine::on_machine_prestart, this), true);
	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(&lua_engine::on_machine_start, this));
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(&lua_engine::on_machine_stop, this));
	machine().add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(&lua_engine::on_machine_pause, this));
	machine().add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(&lua_engine::on_machine_resume, this));
	machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&lua_engine::on_machine_frame, this));
}

//-------------------------------------------------
//  initialize - initialize lua hookup to emu engine
//-------------------------------------------------

void lua_engine::initialize()
{

	static const enum_parser<movie_recording::format, 2> s_movie_recording_format_parser =
	{
		{ "avi", movie_recording::format::AVI },
		{ "mng", movie_recording::format::MNG }
	};


	static const enum_parser<read_or_write, 4> s_read_or_write_parser =
	{
		{ "r", read_or_write::READ },
		{ "w", read_or_write::WRITE },
		{ "rw", read_or_write::READWRITE },
		{ "wr", read_or_write::READWRITE }
	};


	static const enum_parser<ui::text_layout::text_justify, 3> s_text_justify_parser =
	{
		{ "left", ui::text_layout::LEFT },
		{ "right", ui::text_layout::RIGHT },
		{ "center", ui::text_layout::CENTER },
	};


/*  emu library
 *
 * emu.app_name() - return application name
 * emu.app_version() - return application version
 * emu.gamename() - return game full name
 * emu.romname() - return game ROM name
 * emu.softname() - return softlist name
 * emu.time() - return emulation time
 * emu.pid() - return frontend process ID
 *
 * emu.driver_find(driver_name) - find and return game_driver for driver_name
 * emu.start(driver_name) - start given driver_name
 * emu.pause() - pause emulation
 * emu.unpause() - unpause emulation
 * emu.step() - advance one frame
 * emu.keypost(keys) - post keys to natural keyboard
 * emu.wait(len) - wait for len within coroutine
 * emu.lang_translate(str) - get translation for str if available
 * emu.subst_env(str) - substitute environment variables with values for str
 *
 * emu.register_prestart(callback) - register callback before reset
 * emu.register_start(callback) - register callback after reset
 * emu.register_stop(callback) - register callback after stopping
 * emu.register_pause(callback) - register callback at pause
 * emu.register_resume(callback) - register callback at resume
 * emu.register_frame(callback) - register callback at end of frame
 * emu.register_frame_done(callback) - register callback after frame is drawn to screen (for overlays)
 * emu.register_sound_update(callback) - register callback after sound update has generated new samples
 * emu.register_periodic(callback) - register periodic callback while program is running
 * emu.register_callback(callback, name) - register callback to be used by MAME via lua_engine::call_plugin()
 * emu.register_menu(event_callback, populate_callback, name) - register callbacks for plugin menu
 * emu.register_mandatory_file_manager_override(callback) - register callback invoked to override mandatory file manager
 * emu.register_before_load_settings(callback) - register callback to be run before settings are loaded
 * emu.show_menu(menu_name) - show menu by name and pause the machine
 *
 * emu.print_verbose(str) - output to stderr at verbose level
 * emu.print_error(str) - output to stderr at error level
 * emu.print_info(str) - output to stderr at info level
 * emu.print_debug(str) - output to stderr at debug level
 */

	sol::table emu = sol().create_named_table("emu");
	emu["app_name"] = &emulator_info::get_appname_lower;
	emu["app_version"] = &emulator_info::get_bare_build_version;
	emu["gamename"] = [this](){ return machine().system().type.fullname(); };
	emu["romname"] = [this](){ return machine().basename(); };
	emu["softname"] = [this]() { return machine().options().software_name(); };
	emu["keypost"] = [this](const char *keys){ machine().ioport().natkeyboard().post_utf8(keys); };
	emu["time"] = [this](){ return machine().time().as_double(); };
	emu["start"] = [this](const char *driver) {
			int i = driver_list::find(driver);
			if (i != -1)
			{
				mame_machine_manager::instance()->schedule_new_driver(driver_list::driver(i));
				machine().schedule_hard_reset();
			}
			return 1;
		};
	emu["pause"] = [this](){ return machine().pause(); };
	emu["unpause"] = [this](){ return machine().resume(); };
	emu["step"] = [this]() {
			mame_machine_manager::instance()->ui().set_single_step(true);
			machine().resume();
		};
	emu["register_prestart"] = [this](sol::function func){ register_function(func, "LUA_ON_PRESTART"); };
	emu["register_start"] = [this](sol::function func){ register_function(func, "LUA_ON_START"); };
	emu["register_stop"] = [this](sol::function func){ register_function(func, "LUA_ON_STOP"); };
	emu["register_pause"] = [this](sol::function func){ register_function(func, "LUA_ON_PAUSE"); };
	emu["register_resume"] = [this](sol::function func){ register_function(func, "LUA_ON_RESUME"); };
	emu["register_frame"] = [this](sol::function func){ register_function(func, "LUA_ON_FRAME"); };
	emu["register_frame_done"] = [this](sol::function func){ register_function(func, "LUA_ON_FRAME_DONE"); };
	emu["register_sound_update"] = [this](sol::function func){ register_function(func, "LUA_ON_SOUND_UPDATE"); };
	emu["register_periodic"] = [this](sol::function func){ register_function(func, "LUA_ON_PERIODIC"); };
	emu["register_mandatory_file_manager_override"] = [this](sol::function func) { register_function(func, "LUA_ON_MANDATORY_FILE_MANAGER_OVERRIDE"); };
	emu["register_before_load_settings"] = [this](sol::function func) { register_function(func, "LUA_ON_BEFORE_LOAD_SETTINGS"); };
	emu["register_menu"] = [this](sol::function cb, sol::function pop, const std::string &name) {
			std::string cbfield = "menu_cb_" + name;
			std::string popfield = "menu_pop_" + name;
			sol().registry()[cbfield] = cb;
			sol().registry()[popfield] = pop;
			m_menu.push_back(name);
		};
	emu["show_menu"] = [this](const char *name) {
			mame_ui_manager &mui = mame_machine_manager::instance()->ui();
			render_container &container = machine().render().ui_container();
			ui::menu_plugin::show_menu(mui, container, (char *)name);
		};
	emu["register_callback"] = [this](sol::function cb, const std::string &name) {
			std::string field = "cb_" + name;
			sol().registry()[field] = cb;
		};
	emu["print_verbose"] = [](const char *str) { osd_printf_verbose("%s\n", str); };
	emu["print_error"] = [](const char *str) { osd_printf_error("%s\n", str); };
	emu["print_info"] = [](const char *str) { osd_printf_info("%s\n", str); };
	emu["print_debug"] = [](const char *str) { osd_printf_debug("%s\n", str); };
	emu["driver_find"] = [this](const char *driver) -> sol::object {
			int i = driver_list::find(driver);
			if(i == -1)
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), driver_list::driver(i));
		};
	emu["wait"] = lua_CFunction([](lua_State *L) {
			lua_engine *engine = mame_machine_manager::instance()->lua();
			luaL_argcheck(L, lua_isnumber(L, 1), 1, "waiting duration expected");
			int ret = lua_pushthread(L);
			if(ret == 1)
				return luaL_error(L, "cannot wait from outside coroutine");
			int ref = luaL_ref(L, LUA_REGISTRYINDEX);
			engine->machine().scheduler().timer_set(attotime::from_double(lua_tonumber(L, 1)), timer_expired_delegate(FUNC(lua_engine::resume), engine), ref, nullptr);
			return lua_yield(L, 0);
		});
	emu["lang_translate"] = &lang_translate;
	emu["pid"] = &osd_getpid;
	emu["subst_env"] = [](const std::string &str) {
			std::string result;
			osd_subst_env(result, str);
			return result;
		};


/*  emu_file library
 *
 * emu.file([opt] searchpath, flags) - flags can be as in osdcore "OPEN_FLAG_*" or lua style
 *                                     with 'rwc' with addtional c for create *and truncate*
 *                                     (be careful) support zipped files on the searchpath
 *
 * file:open(name) - open first file matching name in searchpath. supports read
 *                   and write sockets as "socket.127.0.0.1:1234"
 * file:open_next() - open next file matching name in searchpath
 * file:read(len) - only reads len bytes, doesn't do lua style formats
 * file:write(data) - write data to file
 * file:seek(offset, whence) - whence is as C "SEEK_*" int
 * file:seek([opt] whence, [opt] offset) - lua style "set"|"cur"|"end", returns cur offset
 * file:size() - file size in bytes
 * file:filename() - name of current file, container name if file is in zip
 * file:fullpath() -
*/

	auto file_type = emu.new_usertype<emu_file>("file", sol::call_constructor, sol::initializers(
				[](emu_file &file, u32 flags) { new (&file) emu_file(flags); },
				[](emu_file &file, const char *path, u32 flags) { new (&file) emu_file(path, flags); },
				[](emu_file &file, const char *mode) {
					int flags = 0;
					for(int i = 0; i < 3 && mode[i]; i++) // limit to three chars
					{
						switch(mode[i])
						{
							case 'r':
								flags |= OPEN_FLAG_READ;
								break;
							case 'w':
								flags |= OPEN_FLAG_WRITE;
								break;
							case 'c':
								flags |= OPEN_FLAG_CREATE;
								break;
						}
					}
					new (&file) emu_file(flags);
				},
				[](emu_file &file, const char *path, const char* mode) {
					int flags = 0;
					for(int i = 0; i < 3 && mode[i]; i++) // limit to three chars
					{
						switch(mode[i])
						{
							case 'r':
								flags |= OPEN_FLAG_READ;
								break;
							case 'w':
								flags |= OPEN_FLAG_WRITE;
								break;
							case 'c':
								flags |= OPEN_FLAG_CREATE;
								break;
						}
					}
					new (&file) emu_file(path, flags);
				}));
	file_type.set("read", [](emu_file &file, sol::buffer *buff) { buff->set_len(file.read(buff->get_ptr(), buff->get_len())); return buff; });
	file_type.set("write", [](emu_file &file, const std::string &data) { return file.write(data.data(), data.size()); });
	file_type.set("open", static_cast<osd_file::error (emu_file::*)(const std::string &)>(&emu_file::open));
	file_type.set("open_next", &emu_file::open_next);
	file_type.set("seek", sol::overload(
			[](emu_file &file) { return file.tell(); },
			[this](emu_file &file, s64 offset, int whence) -> sol::object {
				if(file.seek(offset, whence))
					return sol::make_object(sol(), sol::lua_nil);
				else
					return sol::make_object(sol(), file.tell());
			},
			[this](emu_file &file, const char* whence) -> sol::object {
				int wval = -1;
				const char *seekdirs[] = {"set", "cur", "end"};
				for(int i = 0; i < 3; i++)
				{
					if(!strncmp(whence, seekdirs[i], 3))
					{
						wval = i;
						break;
					}
				}
				if(wval < 0 || wval >= 3)
					return sol::make_object(sol(), sol::lua_nil);
				if(file.seek(0, wval))
					return sol::make_object(sol(), sol::lua_nil);
				return sol::make_object(sol(), file.tell());
			},
			[this](emu_file &file, const char* whence, s64 offset) -> sol::object {
				int wval = -1;
				const char *seekdirs[] = {"set", "cur", "end"};
				for(int i = 0; i < 3; i++)
				{
					if(!strncmp(whence, seekdirs[i], 3))
					{
						wval = i;
						break;
					}
				}
				if(wval < 0 || wval >= 3)
					return sol::make_object(sol(), sol::lua_nil);
				if(file.seek(offset, wval))
					return sol::make_object(sol(), sol::lua_nil);
				return sol::make_object(sol(), file.tell());
			}));
	file_type.set("size", &emu_file::size);
	file_type.set("filename", &emu_file::filename);
	file_type.set("fullpath", &emu_file::fullpath);


/*  thread library
 *
 * emu.thread()
 *
 * thread:start(scr) - run scr (lua code as string) in a separate thread
 *                     in a new empty (other than modules) lua context.
 *                     thread runs until yield() and/or terminates on return.
 * thread:continue(val) - resume thread that has yielded and pass val to it
 *
 * thread.result - get result of a terminated thread as string
 * thread.busy - check if thread is running
 * thread.yield - check if thread is yielded
 */

	auto thread_type = emu.new_usertype<context>("thread", sol::call_constructor, sol::constructors<sol::types<>>());
	thread_type.set("start", [](context &ctx, const char *scr) {
			std::string script(scr);
			if(ctx.busy)
				return false;
			std::thread th([&ctx, script]() {
					sol::state thstate;
					thstate.open_libraries();
					thstate["package"]["preload"]["zlib"] = &luaopen_zlib;
					thstate["package"]["preload"]["lfs"] = &luaopen_lfs;
					thstate["package"]["preload"]["linenoise"] = &luaopen_linenoise;
					sol::load_result res = thstate.load(script);
					if(res.valid())
					{
						sol::protected_function func = res.get<sol::protected_function>();
						thstate["yield"] = [&ctx, &thstate]() {
								std::mutex m;
								std::unique_lock<std::mutex> lock(m);
								ctx.result = thstate["status"];
								ctx.yield = true;
								ctx.sync.wait(lock);
								ctx.yield = false;
								thstate["status"] = ctx.result;
							};
						auto ret = func();
						if (ret.valid()) {
							const char *tmp = ret.get<const char *>();
							if (tmp != nullptr)
								ctx.result = tmp;
							else
								exit(0);
						}
					}
					ctx.busy = false;
				});
			ctx.busy = true;
			ctx.yield = false;
			th.detach();
			return true;
		});
	thread_type.set("continue", [](context &ctx, const char *val) {
			if(!ctx.yield)
				return;
			ctx.result = val;
			ctx.sync.notify_all();
		});
	thread_type.set("result", sol::property([](context &ctx) -> std::string {
			if(ctx.busy && !ctx.yield)
				return "";
			return ctx.result;
		}));
	thread_type.set("busy", sol::readonly(&context::busy));
	thread_type.set("yield", sol::readonly(&context::yield));


/*  save_item library
 *
 * emu.item(item_index)
 *
 * item.size - size of the raw data type
 * item.count - number of entries
 *
 * item:read(offset) - read entry value by index
 * item:read_block(offset, count) - read a block of entry values as a string (byte addressing)
 * item:write(offset, value) - write entry value by index
 */

	auto item_type = emu.new_usertype<save_item>("item", sol::call_constructor, sol::initializers([this](save_item &item, int index) {
					if(machine().save().indexed_item(index, item.base, item.size, item.valcount, item.blockcount, item.stride))
					{
						item.count = item.valcount * item.blockcount;
					}
					else
					{
						item.base = nullptr;
						item.size = 0;
						item.count = 0;
						item.valcount = 0;
						item.blockcount = 0;
						item.stride = 0;
					}
				}));
	item_type.set("size", sol::readonly(&save_item::size));
	item_type.set("count", sol::readonly(&save_item::count));
	item_type.set("read", [this](save_item &item, int offset) -> sol::object {
			if(!item.base || (offset >= item.count))
				return sol::make_object(sol(), sol::lua_nil);
			const void *const data = reinterpret_cast<const uint8_t *>(item.base) + (item.stride * (offset / item.valcount));
			uint64_t ret = 0;
			switch(item.size)
			{
				case 1:
				default:
					ret = reinterpret_cast<const uint8_t *>(data)[offset % item.valcount];
					break;
				case 2:
					ret = reinterpret_cast<const uint16_t *>(data)[offset % item.valcount];
					break;
				case 4:
					ret = reinterpret_cast<const uint32_t *>(data)[offset % item.valcount];
					break;
				case 8:
					ret = reinterpret_cast<const uint64_t *>(data)[offset % item.valcount];
					break;
			}
			return sol::make_object(sol(), ret);
		});
	item_type.set("read_block", [](save_item &item, int offset, sol::buffer *buff) {
			if(!item.base || ((offset + buff->get_len()) > (item.size * item.count)))
			{
				buff->set_len(0);
			}
			else
			{
				const uint32_t blocksize = item.size * item.valcount;
				uint32_t remaining = buff->get_len();
				uint8_t *dest = reinterpret_cast<uint8_t *>(buff->get_ptr());
				while(remaining)
				{
					const uint32_t blockno = offset / blocksize;
					const uint32_t available = blocksize - (offset % blocksize);
					const uint32_t chunk = (available < remaining) ? available : remaining;
					const void *const source = reinterpret_cast<const uint8_t *>(item.base) + (blockno * item.stride) + (offset % blocksize);
					std::memcpy(dest, source, chunk);
					offset += chunk;
					remaining -= chunk;
					dest += chunk;
				}
			}
			return buff;
		});
	item_type.set("write", [](save_item &item, int offset, uint64_t value) {
			if(!item.base || (offset >= item.count))
				return;
			void *const data = reinterpret_cast<uint8_t *>(item.base) + (item.stride * (offset / item.valcount));
			switch(item.size)
			{
				case 1:
				default:
					reinterpret_cast<uint8_t *>(data)[offset % item.valcount] = uint8_t(value);
					break;
				case 2:
					reinterpret_cast<uint16_t *>(data)[offset % item.valcount] = uint16_t(value);
					break;
				case 4:
					reinterpret_cast<uint32_t *>(data)[offset % item.valcount] = uint32_t(value);
					break;
				case 8:
					reinterpret_cast<uint64_t *>(data)[offset % item.valcount] = uint64_t(value);
					break;
			}
		});


/*  core_options library
 *
 * manager:options()
 * manager:machine():options()
 * manager:ui():options()
 * manager:plugins()
 *
 * options:help() - get help for options
 * options:command(command) - return output for command
 *
 * options.entries[] - get table of option entries (k=name, v=core_options::entry)
 */

	auto core_options_type = sol().registry().new_usertype<core_options>("core_options", "new", sol::no_constructor);
	core_options_type.set("help", &core_options::output_help);
	core_options_type.set("command", &core_options::command);
	core_options_type.set("entries", sol::property([this](core_options &options) {
			sol::table table = sol().create_table();
			int unadorned_index = 0;
			for (auto &curentry : options.entries())
			{
				const char *name = curentry->names().size() > 0
					? curentry->name().c_str()
					: nullptr;
				bool is_unadorned = false;
				// check if it's unadorned
				if (name && strlen(name) && !strcmp(name, options.unadorned(unadorned_index)))
				{
					unadorned_index++;
					is_unadorned = true;
				}
				if (curentry->type() != core_options::option_type::HEADER && curentry->type() != core_options::option_type::COMMAND && !is_unadorned)
					table[name] = &*curentry;
			}
			return table;
		}));


/*  core_options::entry library
 *
 * options.entries[entry_name]
 *
 * entry:value() - get value of entry
 * entry:value(val) - set entry to val
 * entry:description() - get info about entry
 * entry:default_value() - get default for entry
 * entry:minimum() - get min value for entry
 * entry:maximum() - get max value for entry
 * entry:has_range() - are min and max valid for entry
 */

	auto core_options_entry_type = sol().registry().new_usertype<core_options::entry>("core_options_entry", "new", sol::no_constructor);
	core_options_entry_type.set("value", sol::overload(
		[this](core_options::entry &e, bool val) {
			if(e.type() != OPTION_BOOLEAN)
				luaL_error(m_lua_state, "Cannot set option to wrong type");
			else
				e.set_value(val ? "1" : "0", OPTION_PRIORITY_CMDLINE);
		},
		[this](core_options::entry &e, float val) {
			if(e.type() != OPTION_FLOAT)
				luaL_error(m_lua_state, "Cannot set option to wrong type");
			else
				e.set_value(string_format("%f", val), OPTION_PRIORITY_CMDLINE);
		},
		[this](core_options::entry &e, int val) {
			if(e.type() != OPTION_INTEGER)
				luaL_error(m_lua_state, "Cannot set option to wrong type");
			else
				e.set_value(string_format("%d", val), OPTION_PRIORITY_CMDLINE);
		},
		[this](core_options::entry &e, const char *val) {
			if(e.type() != OPTION_STRING)
				luaL_error(m_lua_state, "Cannot set option to wrong type");
			else
				e.set_value(val, OPTION_PRIORITY_CMDLINE);
		},
		[this](core_options::entry &e) -> sol::object {
			if (e.type() == core_options::option_type::INVALID)
				return sol::make_object(sol(), sol::lua_nil);
			switch(e.type())
			{
				case core_options::option_type::BOOLEAN:
					return sol::make_object(sol(), atoi(e.value()) != 0);
				case core_options::option_type::INTEGER:
					return sol::make_object(sol(), atoi(e.value()));
				case core_options::option_type::FLOAT:
					return sol::make_object(sol(), atof(e.value()));
				default:
					return sol::make_object(sol(), e.value());
			}
		}));
	core_options_entry_type.set("description", &core_options::entry::description);
	core_options_entry_type.set("default_value", &core_options::entry::default_value);
	core_options_entry_type.set("minimum", &core_options::entry::minimum);
	core_options_entry_type.set("maximum", &core_options::entry::maximum);
	core_options_entry_type.set("has_range", &core_options::entry::has_range);


/*  running_machine library
 *
 * manager:machine()
 *
 * machine:exit() - close program
 * machine:hard_reset() - hard reset emulation
 * machine:soft_reset() - soft reset emulation
 * machine:save(filename) - save state to filename
 * machine:load(filename) - load state from filename
 * machine:buffer_save() - return save state buffer as binary string
 * machine:buffer_load(str) - load state from binary string buffer. returns true on success, otherwise nil
 * machine:popmessage(str) - print str as popup
 * machine:popmessage() - clear displayed popup message
 * machine:logerror(str) - print str to log
 * machine:system() - get game_driver for running driver
 * machine:video() - get video_manager
 * machine:sound() - get sound_manager
 * machine:render() - get render_manager
 * machine:ioport() - get ioport_manager
 * machine:parameters() - get parameter_manager
 * machine:memory() - get memory_manager
 * machine:options() - get machine core_options
 * machine:outputs() - get output_manager
 * machine:input() - get input_manager
 * machine:uiinput() - get ui_input_manager
 * machine:debugger() - get debugger_manager
 *
 * machine.paused - get paused state
 * machine.samplerate - get audio sample rate
 * machine.exit_pending
 * machine.hard_reset_pending
 *
 * machine.devices[] - get device table (k=tag, v=device_t)
 * machine.screens[] - get screens table (k=tag, v=screen_device)
 * machine.images[] - get available image devices table (k=type, v=device_image_interface)
 */

	auto machine_type = sol().registry().new_usertype<running_machine>("machine", "new", sol::no_constructor);
	machine_type.set("exit", &running_machine::schedule_exit);
	machine_type.set("hard_reset", &running_machine::schedule_hard_reset);
	machine_type.set("soft_reset", &running_machine::schedule_soft_reset);
	machine_type.set("save", &running_machine::schedule_save);
	machine_type.set("load", &running_machine::schedule_load);
	machine_type.set("buffer_save", [](running_machine &m, sol::this_state s) {
			lua_State *L = s;
			luaL_Buffer buff;
			int size = ram_state::get_size(m.save());
			u8 *ptr = (u8 *)luaL_buffinitsize(L, &buff, size);
			save_error error = m.save().write_buffer(ptr, size);
			if (error == STATERR_NONE)
			{
				luaL_pushresultsize(&buff, size);
				return sol::make_reference(L, sol::stack_reference(L, -1));
			}
			luaL_error(L, "State save error.");
			return sol::make_reference(L, nullptr);
		});
	machine_type.set("buffer_load", [](running_machine &m, sol::this_state s, std::string str) {
			lua_State *L = s;
			save_error error = m.save().read_buffer((u8 *)str.data(), str.size());
			if (error == STATERR_NONE)
				return true;
			else
			{
				luaL_error(L,"State load error.");
				return false;
			}
		});
	machine_type.set("system", &running_machine::system);
	machine_type.set("video", &running_machine::video);
	machine_type.set("sound", &running_machine::sound);
	machine_type.set("render", &running_machine::render);
	machine_type.set("ioport", &running_machine::ioport);
	machine_type.set("parameters", &running_machine::parameters);
	machine_type.set("memory", &running_machine::memory);
	machine_type.set("options", [](running_machine &m) { return static_cast<core_options *>(&m.options()); });
	machine_type.set("outputs", &running_machine::output);
	machine_type.set("input", &running_machine::input);
	machine_type.set("uiinput", &running_machine::ui_input);
	machine_type.set("debugger", [this](running_machine &m) -> sol::object {
			if(!(m.debug_flags & DEBUG_FLAG_ENABLED))
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), &m.debugger());
		});
	machine_type.set("paused", sol::property(&running_machine::paused));
	machine_type.set("samplerate", sol::property(&running_machine::sample_rate));
	machine_type.set("exit_pending", sol::property(&running_machine::exit_pending));
	machine_type.set("hard_reset_pending", sol::property(&running_machine::hard_reset_pending));
	machine_type.set("devices", sol::property([this](running_machine &m) {
			std::function<void(device_t &, sol::table)> tree;
			sol::table table = sol().create_table();
			tree = [&tree](device_t &root, sol::table table) {
				for(device_t &dev : root.subdevices())
				{
					if(dev.configured() && dev.started())
					{
						table[dev.tag()] = &dev;
						tree(dev, table);
					}
				}
			};
			tree(m.root_device(), table);
			return table;
		}));
	machine_type.set("screens", sol::property([this](running_machine &r) {
			sol::table table = sol().create_table();
			for (screen_device &sc : screen_device_iterator(r.root_device()))
			{
				if (sc.configured() && sc.started() && sc.type())
					table[sc.tag()] = &sc;
			}
			return table;
		}));
	machine_type.set("images", sol::property([this](running_machine &r) {
			sol::table image_table = sol().create_table();
			for(device_image_interface &image : image_interface_iterator(r.root_device()))
			{
				image_table[image.brief_instance_name()] = &image;
				image_table[image.instance_name()] = &image;
			}
			return image_table;
		}));
	machine_type.set("popmessage", sol::overload(
			[](running_machine &m, const char *str) { m.popmessage("%s", str); },
			[](running_machine &m) { m.popmessage(); }));
	machine_type.set("logerror", [](running_machine &m, const char *str) { m.logerror("[luaengine] %s\n", str); } );


/*  game_driver library
 *
 * emu.driver_find(driver_name)
 *
 * driver.source_file - relative path to the source file
 * driver.parent
 * driver.name
 * driver.description
 * driver.year
 * driver.manufacturer
 * driver.compatible_with
 * driver.default_layout
 * driver.orientation - screen rotation degree (rot0/90/180/270)
 * driver.type - machine type (arcade/console/computer/other)
 * driver.not_working - not considered working
 * driver.supports_save - supports save states
 * driver.no_cocktail - screen flip support is missing
 * driver.is_bios_root - this driver entry is a BIOS root
 * driver.requires_artwork - requires external artwork for key game elements
 * driver.clickable_artwork - artwork is clickable and requires mouse cursor
 * driver.unofficial - unofficial hardware modification
 * driver.no_sound_hw - system has no sound output
 * driver.mechanical - contains mechanical parts (pinball, redemption games, ...)
 * driver.is_incomplete - official system with blatantly incomplete hardware/software
 */

	auto game_driver_type = sol().registry().new_usertype<game_driver>("game_driver", "new", sol::no_constructor);
	game_driver_type.set("source_file", sol::property([] (game_driver const &driver) { return &driver.type.source()[0]; }));
	game_driver_type.set("parent", sol::readonly(&game_driver::parent));
	game_driver_type.set("name", sol::property([] (game_driver const &driver) { return &driver.name[0]; }));
	game_driver_type.set("description", sol::property([] (game_driver const &driver) { return &driver.type.fullname()[0]; }));
	game_driver_type.set("year", sol::readonly(&game_driver::year));
	game_driver_type.set("manufacturer", sol::readonly(&game_driver::manufacturer));
	game_driver_type.set("compatible_with", sol::readonly(&game_driver::compatible_with));
	game_driver_type.set("default_layout", sol::readonly(&game_driver::default_layout));
	game_driver_type.set("orientation", sol::property([](game_driver const &driver) {
			std::string rot;
			switch (driver.flags & machine_flags::MASK_ORIENTATION)
			{
			case machine_flags::ROT0:
				rot = "rot0";
				break;
			case machine_flags::ROT90:
				rot = "rot90";
				break;
			case machine_flags::ROT180:
				rot = "rot180";
				break;
			case machine_flags::ROT270:
				rot = "rot270";
				break;
			default:
				rot = "undefined";
				break;
			}
			return rot;
		}));
	game_driver_type.set("type", sol::property([](game_driver const &driver) {
			std::string type;
			switch (driver.flags & machine_flags::MASK_TYPE)
			{
			case machine_flags::TYPE_ARCADE:
				type = "arcade";
				break;
			case machine_flags::TYPE_CONSOLE:
				type = "console";
				break;
			case machine_flags::TYPE_COMPUTER:
				type = "computer";
				break;
			default:
				type = "other";
				break;
			}
			return type;
		}));
	game_driver_type.set("not_working", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::NOT_WORKING) > 0; }));
	game_driver_type.set("supports_save", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::SUPPORTS_SAVE) > 0; }));
	game_driver_type.set("no_cocktail", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::NO_COCKTAIL) > 0; }));
	game_driver_type.set("is_bios_root", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::IS_BIOS_ROOT) > 0; }));
	game_driver_type.set("requires_artwork", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::REQUIRES_ARTWORK) > 0; }));
	game_driver_type.set("clickable_artwork", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::CLICKABLE_ARTWORK) > 0; }));
	game_driver_type.set("unofficial", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::UNOFFICIAL) > 0; }));
	game_driver_type.set("no_sound_hw", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::NO_SOUND_HW) > 0; }));
	game_driver_type.set("mechanical", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::MECHANICAL) > 0; }));
	game_driver_type.set("is_incomplete", sol::property([](game_driver const &driver) { return (driver.flags & machine_flags::IS_INCOMPLETE) > 0; } ));


/*  debugger_manager library (requires debugger to be active)
 *
 * manager:machine():debugger()
 *
 * debugger:command(command_string) - run command_string in debugger console
 *
 * debugger.consolelog[] - get consolelog text buffer (wrap_textbuf)
 * debugger.errorlog[] - get errorlog text buffer (wrap_textbuf)
 * debugger.visible_cpu - accessor for debugger active cpu for commands, affects debug views
 * debugger.execution_state - accessor for active cpu run state
 */

	struct wrap_textbuf { wrap_textbuf(const text_buffer &buf) : textbuf(buf) { } std::reference_wrapper<const text_buffer> textbuf; };

	auto debugger_type = sol().registry().new_usertype<debugger_manager>("debugger", "new", sol::no_constructor);
	debugger_type.set("command", [](debugger_manager &debug, const std::string &cmd) { debug.console().execute_command(cmd, false); });
	debugger_type.set("consolelog", sol::property([](debugger_manager &debug) { return wrap_textbuf(debug.console().get_console_textbuf()); }));
	debugger_type.set("errorlog", sol::property([](debugger_manager &debug) { return wrap_textbuf(debug.console().get_errorlog_textbuf()); }));
	debugger_type.set("visible_cpu", sol::property(
		[](debugger_manager &debug) { debug.console().get_visible_cpu(); },
		[](debugger_manager &debug, device_t &dev) { debug.console().set_visible_cpu(&dev); }));
	debugger_type.set("execution_state", sol::property(
		[](debugger_manager &debug) {
			return debug.cpu().is_stopped() ? "stop" : "run";
		},
		[](debugger_manager &debug, const std::string &state) {
			if(state == "stop")
				debug.cpu().set_execution_stopped();
			else
				debug.cpu().set_execution_running();
		}));


/*  wrap_textbuf library (requires debugger to be active)
 *
 * manager:machine():debugger().consolelog
 * manager:machine():debugger().errorlog
 *
 * log[index] - get log entry
 * #log - entry count
 */

	sol().registry().new_usertype<wrap_textbuf>("text_buffer", "new", sol::no_constructor,
			"__metatable", [](){},
			"__newindex", [](){},
			"__index", [](wrap_textbuf &buf, int index) { return text_buffer_get_seqnum_line(buf.textbuf, index - 1); },
			"__len", [](wrap_textbuf &buf) { return text_buffer_num_lines(buf.textbuf) + text_buffer_line_index_to_seqnum(buf.textbuf, 0) - 1; });

/*  device_debug library (requires debugger to be active)
 *
 * manager:machine().devices[device_tag]:debug()
 *
 * debug:step([opt] steps) - run cpu steps, default 1
 * debug:go() - run cpu
 * debug:bpset(addr, [opt] cond, [opt] act) - set breakpoint on addr, cond and act are debugger
 *                                            expressions. returns breakpoint index
 * debug:bpclr(idx) - clear break
 * debug:bplist()[] - table of breakpoints (k=index, v=debug_breakpoint)
 * debug:wpset(space, type, addr, len, [opt] cond, [opt] act) - set watchpoint, cond and act
 *                                                              are debugger expressions.
 *                                                              returns watchpoint index
 * debug:wpclr(idx) - clear watch
 * debug:wplist(space)[] - table of watchpoints (k=index, v=watchpoint)
 */

	auto device_debug_type = sol().registry().new_usertype<device_debug>("device_debug", "new", sol::no_constructor);
	device_debug_type.set("step", [](device_debug &dev, sol::object num) {
			int steps = 1;
			if(num.is<int>())
				steps = num.as<int>();
			dev.single_step(steps);
		});
	device_debug_type.set("go", &device_debug::go);
	device_debug_type.set("bpset", [](device_debug &dev, offs_t addr, const char *cond, const char *act) { return dev.breakpoint_set(addr, cond, act); });
	device_debug_type.set("bpclr", &device_debug::breakpoint_clear);
	device_debug_type.set("bplist", [this](device_debug &dev) {
			sol::table table = sol().create_table();
			for(const auto &bpp : dev.breakpoint_list())
			{
				const debug_breakpoint &bpt = *bpp.second;
				sol::table bp = sol().create_table();
				bp["enabled"] = bpt.enabled();
				bp["address"] = bpt.address();
				bp["condition"] = bpt.condition();
				bp["action"] = bpt.action();
				table[bpt.index()] = bp;
			}
			return table;
		});
	device_debug_type.set("wpset", [](device_debug &dev, addr_space &sp, const std::string &type, offs_t addr, offs_t len, const char *cond, const char *act) {
			read_or_write wptype = s_read_or_write_parser(type);
			return dev.watchpoint_set(sp.space, wptype, addr, len, cond, act);
		});
	device_debug_type.set("wpclr", &device_debug::watchpoint_clear);
	device_debug_type.set("wplist", [this](device_debug &dev, addr_space &sp) {
			sol::table table = sol().create_table();
			for(auto &wpp : dev.watchpoint_vector(sp.space.spacenum()))
			{
				sol::table wp = sol().create_table();
				wp["enabled"] = wpp->enabled();
				wp["address"] = wpp->address();
				wp["length"] = wpp->length();
				switch(wpp->type())
				{
					case read_or_write::READ:
						wp["type"] = "r";
						break;
					case read_or_write::WRITE:
						wp["type"] = "w";
						break;
					case read_or_write::READWRITE:
						wp["type"] = "rw";
						break;
					default: // huh?
						wp["type"] = "";
						break;
				}
				wp["condition"] = wpp->condition();
				wp["action"] = wpp->action();
				table[wpp->index()] = wp;
			}
			return table;
		});


/*  device_t library
 *
 * manager:machine().devices[device_tag]
 *
 * device:name() - device long name
 * device:shortname() - device short name
 * device:tag() - device tree tag
 * device:owner() - device parent tag
 * device:debug() - debug interface, cpus only
 *
 * device.spaces[] - device address spaces table (k=name, v=addr_space)
 * device.state[] - device state entries table (k=name, v=device_state_entry)
 * device.items[] - device save state items table (k=name, v=index)
 * device.roms[] - device rom entry table (k=name, v=rom_entry)
 */

	auto device_type = sol().registry().new_usertype<device_t>("device", "new", sol::no_constructor);
	device_type.set("name", &device_t::name);
	device_type.set("shortname", &device_t::shortname);
	device_type.set("tag", &device_t::tag);
	device_type.set("owner", &device_t::owner);
	device_type.set("debug", [this](device_t &dev) -> sol::object {
			if(!(dev.machine().debug_flags & DEBUG_FLAG_ENABLED) || !dynamic_cast<cpu_device *>(&dev)) // debugger not enabled or not cpu
				return sol::make_object(sol(), sol::lua_nil);
			return sol::make_object(sol(), dev.debug());
		});
	device_type.set("spaces", sol::property([this](device_t &dev) {
			device_memory_interface *memdev = dynamic_cast<device_memory_interface *>(&dev);
			sol::table sp_table = sol().create_table();
			if(!memdev)
				return sp_table;
			for(int sp = 0; sp < memdev->max_space_count(); ++sp)
			{
				if(memdev->has_space(sp))
					sp_table[memdev->space(sp).name()] = addr_space(memdev->space(sp), *memdev);
			}
			return sp_table;
		}));
	device_type.set("state", sol::property([this](device_t &dev) {
			sol::table st_table = sol().create_table();
			if(!dynamic_cast<device_state_interface *>(&dev))
				return st_table;
			// XXX: refrain from exporting non-visible entries?
			for(auto &s : dev.state().state_entries())
				st_table[s->symbol()] = s.get();
			return st_table;
		}));
	device_type.set("items", sol::property([this](device_t &dev) {
			sol::table table = sol().create_table();
			std::string tag = dev.tag();
			// 10000 is enough?
			for(int i = 0; i < 10000; i++)
			{
				std::string name;
				const char *item;
				void *base;
				uint32_t size, valcount, blockcount, stride;
				item = dev.machine().save().indexed_item(i, base, size, valcount, blockcount, stride);
				if(!item)
					break;
				name = &(strchr(item, '/')[1]);
				if(name.substr(0, name.find('/')) == tag)
				{
					name = name.substr(name.find('/') + 1, std::string::npos);
					table[name] = i;
				}
			}
			return table;
		}));
	device_type.set("roms", sol::property([this](device_t &dev) {
			sol::table table = sol().create_table();
			for(auto rom : dev.rom_region_vector())
				if(!rom.name().empty())
					table[rom.name()] = rom;
			return table;
		}));


/*  parameters_manager library
 *
 * manager:machine():parameters()
 *
 * parameters:add(tag, val) - add tag = val parameter
 * parameters:lookup(tag) - get val for tag
 */

	sol().registry().new_usertype<parameters_manager>("parameters", "new", sol::no_constructor,
			"add", &parameters_manager::add,
			"lookup", &parameters_manager::lookup);


/*  video_manager library
 *
 * manager:machine():video()
 *
 * video:begin_recording([opt] filename, [opt] format) - start AVI recording to filename if given or default
 * video:end_recording() - stop AVI recording
 * video:is_recording() - get recording status
 * video:snapshot() - save shot of all screens
 * video:skip_this_frame() - is current frame going to be skipped
 * video:speed_factor() - get speed factor
 * video:speed_percent() - get percent from realtime
 * video:frame_update() - render a frame
 * video:size() - get width and height of snapshot bitmap in pixels
 * video:pixels() - get binary bitmap of all screens as string
 *
 * video.frameskip - current frameskip
 * video.throttled - throttle state
 * video.throttle_rate - throttle rate
 */

	auto video_type = sol().registry().new_usertype<video_manager>("video", "new", sol::no_constructor);
	video_type.set("begin_recording", sol::overload(
		[this](video_manager &vm, const char *filename, const char *format_string) {
			std::string fn = process_snapshot_filename(machine(), filename);
			movie_recording::format format = s_movie_recording_format_parser(format_string);
			vm.begin_recording(fn.c_str(), format);
		},
		[this](video_manager &vm, const char *filename) {
			std::string fn = process_snapshot_filename(machine(), filename);
			vm.begin_recording(fn.c_str(), movie_recording::format::AVI);
		},
		[](video_manager &vm) {
			vm.begin_recording(nullptr, movie_recording::format::AVI);
		}));
	video_type.set("end_recording", [this](video_manager &vm) {
			if(!vm.is_recording())
			{
				machine().logerror("[luaengine] No active recording to stop\n");
				return;
			}
			vm.end_recording();
		});
	video_type.set("snapshot", &video_manager::save_active_screen_snapshots);
	video_type.set("is_recording", &video_manager::is_recording);
	video_type.set("skip_this_frame", &video_manager::skip_this_frame);
	video_type.set("speed_factor", &video_manager::speed_factor);
	video_type.set("speed_percent", &video_manager::speed_percent);
	video_type.set("effective_frameskip", &video_manager::effective_frameskip);
	video_type.set("frame_update", &video_manager::frame_update);
	video_type.set("size", [](video_manager &vm) {
			s32 width, height;
			vm.compute_snapshot_size(width, height);
			return std::tuple<s32, s32>(width, height);
		});
	video_type.set("pixels", [](video_manager &vm, sol::this_state s) {
			lua_State *L = s;
			luaL_Buffer buff;
			s32 width, height;
			vm.compute_snapshot_size(width, height);
			int size = width * height * 4;
			u32 *ptr = (u32 *)luaL_buffinitsize(L, &buff, size);
			vm.pixels(ptr);
			luaL_pushresultsize(&buff, size);
			return sol::make_reference(L, sol::stack_reference(L, -1));
		});
	video_type.set("frameskip", sol::property(&video_manager::frameskip, &video_manager::set_frameskip));
	video_type.set("throttled", sol::property(&video_manager::throttled, &video_manager::set_throttled));
	video_type.set("throttle_rate", sol::property(&video_manager::throttle_rate, &video_manager::set_throttle_rate));


/*  sound_manager library
 *
 * manager:machine():sound()
 *
 * sound:start_recording() - begin audio recording
 * sound:stop_recording() - end audio recording
 * sound:ui_mute(turn_off) - turns on/off UI sound
 * sound:system_mute() - turns on/off system sound
 * sound:samples() - get current audio buffer contents in binary form as string (updates 50 times per second)
 *
 * sound.attenuation - sound attenuation
 */

	auto sound_type = sol().registry().new_usertype<sound_manager>("sound", "new", sol::no_constructor);
	sound_type.set("start_recording", &sound_manager::start_recording);
	sound_type.set("stop_recording", &sound_manager::stop_recording);
	sound_type.set("ui_mute", &sound_manager::ui_mute);
	sound_type.set("debugger_mute", &sound_manager::debugger_mute);
	sound_type.set("system_mute", &sound_manager::system_mute);
	sound_type.set("samples", [](sound_manager &sm, sol::this_state s) {
			lua_State *L = s;
			luaL_Buffer buff;
			s32 count = sm.sample_count() * 2 * 2; // 2 channels, 2 bytes per sample
			s16 *ptr = (s16 *)luaL_buffinitsize(L, &buff, count);
			sm.samples(ptr);
			luaL_pushresultsize(&buff, count);
			return sol::make_reference(L, sol::stack_reference(L, -1));
		});
	sound_type.set("attenuation", sol::property(&sound_manager::attenuation, &sound_manager::set_attenuation));


/* screen_device library
 *
 * manager:machine().screens[screen_tag]
 *
 * screen:draw_box(x1, y1, x2, y2, fillcol, linecol) - draw box from (x1, y1)-(x2, y2) colored linecol
 *                                                     filled with fillcol, color is 32bit argb
 * screen:draw_line(x1, y1, x2, y2, linecol) - draw line from (x1, y1)-(x2, y2) colored linecol
 * screen:draw_text(x || justify, y, message, [opt] fgcolor, [opt] bgcolor) - draw message at (x, y) or at line y
 *                                                                            with left/right/center justification
 * screen:height() - screen height
 * screen:width() - screen width
 * screen:orientation() - screen angle, flipx, flipy
 * screen:refresh() - screen refresh rate in Hz
 * screen:refresh_attoseconds() - screen refresh rate in attoseconds
 * screen:snapshot([opt] filename) - save snap shot
 * screen:type() - screen drawing type
 * screen:frame_number() - screen frame count
 * screen:name() - screen device full name
 * screen:shortname() - screen device short name
 * screen:tag() - screen device tag
 * screen:xscale() - screen x scale factor
 * screen:yscale() - screen y scale factor
 * screen:pixel(x, y) - get pixel at (x, y) as packed RGB in a u32
 * screen:pixels() - get whole screen binary bitmap as string
 * screen:time_until_pos(vpos, hpos) - get the time until this screen pos is reached
 */

	auto screen_dev_type = sol().registry().new_usertype<screen_device>("screen_dev", "new", sol::no_constructor);
	screen_dev_type.set("draw_box", [](screen_device &sdev, float x1, float y1, float x2, float y2, uint32_t bgcolor, uint32_t fgcolor) {
			int sc_width = sdev.visible_area().width();
			int sc_height = sdev.visible_area().height();
			x1 = std::min(std::max(0.0f, x1), float(sc_width-1)) / float(sc_width);
			y1 = std::min(std::max(0.0f, y1), float(sc_height-1)) / float(sc_height);
			x2 = std::min(std::max(0.0f, x2), float(sc_width-1)) / float(sc_width);
			y2 = std::min(std::max(0.0f, y2), float(sc_height-1)) / float(sc_height);
			mame_machine_manager::instance()->ui().draw_outlined_box(sdev.container(), x1, y1, x2, y2, fgcolor, bgcolor);
		});
	screen_dev_type.set("draw_line", [](screen_device &sdev, float x1, float y1, float x2, float y2, uint32_t color) {
			int sc_width = sdev.visible_area().width();
			int sc_height = sdev.visible_area().height();
			x1 = std::min(std::max(0.0f, x1), float(sc_width-1)) / float(sc_width);
			y1 = std::min(std::max(0.0f, y1), float(sc_height-1)) / float(sc_height);
			x2 = std::min(std::max(0.0f, x2), float(sc_width-1)) / float(sc_width);
			y2 = std::min(std::max(0.0f, y2), float(sc_height-1)) / float(sc_height);
			sdev.container().add_line(x1, y1, x2, y2, UI_LINE_WIDTH, rgb_t(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		});
	screen_dev_type.set("draw_text", [this](screen_device &sdev, sol::object xobj, float y, const char *msg, sol::object color, sol::object bcolor) {
			int sc_width = sdev.visible_area().width();
			int sc_height = sdev.visible_area().height();
			auto justify = ui::text_layout::LEFT;
			float x = 0;
			if(xobj.is<float>())
			{
				x = std::min(std::max(0.0f, xobj.as<float>()), float(sc_width-1)) / float(sc_width);
				y = std::min(std::max(0.0f, y), float(sc_height-1)) / float(sc_height);
			}
			else if(xobj.is<const char *>())
			{
				justify = s_text_justify_parser(xobj.as<const char *>());
			}
			else
			{
				luaL_error(m_lua_state, "Error in param 1 to draw_text");
				return;
			}
			rgb_t textcolor = mame_machine_manager::instance()->ui().colors().text_color();
			rgb_t bgcolor = 0;
			if(color.is<uint32_t>())
				textcolor = rgb_t(color.as<uint32_t>());
			if(bcolor.is<uint32_t>())
				bgcolor = rgb_t(bcolor.as<uint32_t>());
			mame_machine_manager::instance()->ui().draw_text_full(sdev.container(), msg, x, y, (1.0f - x),
								justify, ui::text_layout::WORD, mame_ui_manager::OPAQUE_, textcolor, bgcolor);
		});
	screen_dev_type.set("height", [](screen_device &sdev) { return sdev.visible_area().height(); });
	screen_dev_type.set("width", [](screen_device &sdev) { return sdev.visible_area().width(); });
	screen_dev_type.set("orientation", [](screen_device &sdev) {
			uint32_t flags = sdev.orientation();
			int rotation_angle = 0;
			switch (flags)
			{
				case ORIENTATION_FLIP_X:
					rotation_angle = 0;
					break;
				case ORIENTATION_SWAP_XY:
				case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X:
					rotation_angle = 90;
					break;
				case ORIENTATION_FLIP_Y:
				case ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
					rotation_angle = 180;
					break;
				case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_Y:
				case ORIENTATION_SWAP_XY|ORIENTATION_FLIP_X|ORIENTATION_FLIP_Y:
					rotation_angle = 270;
					break;
			}
			return std::tuple<int, bool, bool>(rotation_angle, flags & ORIENTATION_FLIP_X, flags & ORIENTATION_FLIP_Y);
		});
	screen_dev_type.set("refresh", [](screen_device &sdev) { return ATTOSECONDS_TO_HZ(sdev.refresh_attoseconds()); });
	screen_dev_type.set("refresh_attoseconds", [](screen_device &sdev) { return sdev.refresh_attoseconds(); });
	screen_dev_type.set("snapshot", [this](screen_device &sdev, sol::object filename) -> sol::object {
			std::string snapstr;
			bool is_absolute_path = false;
			if (filename.is<const char *>())
			{
				// a filename was specified; if it isn't absolute postprocess it
				snapstr = process_snapshot_filename(machine(), filename.as<const char *>());
				is_absolute_path = osd_is_absolute_path(snapstr);
			}

			// open the file
			emu_file file(is_absolute_path ? "" : machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			osd_file::error filerr;
			if (!snapstr.empty())
				filerr = file.open(snapstr);
			else
				filerr = machine().video().open_next(file, "png");
			if (filerr != osd_file::error::NONE)
				return sol::make_object(sol(), filerr);

			// and save the snapshot
			machine().video().save_snapshot(&sdev, file);
			return sol::make_object(sol(), sol::lua_nil);
		});
	screen_dev_type.set("type", [](screen_device &sdev) {
			switch (sdev.screen_type())
			{
				case SCREEN_TYPE_RASTER:  return "raster"; break;
				case SCREEN_TYPE_VECTOR:  return "vector"; break;
				case SCREEN_TYPE_LCD:     return "lcd"; break;
				case SCREEN_TYPE_SVG:     return "svg"; break;
				default: break;
			}
			return "unknown";
		});
	screen_dev_type.set("frame_number", &screen_device::frame_number);
	screen_dev_type.set("name", &screen_device::name);
	screen_dev_type.set("shortname", &screen_device::shortname);
	screen_dev_type.set("tag", &screen_device::tag);
	screen_dev_type.set("xscale", &screen_device::xscale);
	screen_dev_type.set("yscale", &screen_device::yscale);
	screen_dev_type.set("pixel", [](screen_device &sdev, float x, float y) { return sdev.pixel((s32)x, (s32)y); });
	screen_dev_type.set("pixels", [](screen_device &sdev, sol::this_state s) {
			lua_State *L = s;
			const rectangle &visarea = sdev.visible_area();
			luaL_Buffer buff;
			int size = visarea.height() * visarea.width() * 4;
			u32 *ptr = (u32 *)luaL_buffinitsize(L, &buff, size);
			sdev.pixels(ptr);
			luaL_pushresultsize(&buff, size);
			return sol::make_reference(L, sol::stack_reference(L, -1));
		});
	screen_dev_type.set("time_until_pos", [](screen_device &sdev, int vpos, int hpos) { return sdev.time_until_pos(vpos, hpos).as_double(); });


/*  mame_ui_manager library
 *
 * manager:ui()
 *
 * ui:is_menu_active() - ui menu state
 * ui:options() - ui core_options
 * ui:get_line_height() - current ui font height
 * ui:get_string_width(str, scale) - get str width with ui font at scale factor of current font size
 * ui:get_char_width(char) - get width of utf8 glyph char with ui font
 * ui:set_aggressive_input_focus(bool)
 *
 * ui.single_step
 * ui.show_fps - fps display enabled
 * ui.show_profiler - profiler display enabled
 */

	auto ui_type = sol().registry().new_usertype<mame_ui_manager>("ui", "new", sol::no_constructor);
	ui_type.set("is_menu_active", &mame_ui_manager::is_menu_active);
	ui_type.set("options", [](mame_ui_manager &m) { return static_cast<core_options *>(&m.options()); });
	ui_type.set("show_fps", sol::property(&mame_ui_manager::show_fps, &mame_ui_manager::set_show_fps));
	ui_type.set("show_profiler", sol::property(&mame_ui_manager::show_profiler, &mame_ui_manager::set_show_profiler));
	ui_type.set("single_step", sol::property(&mame_ui_manager::single_step, &mame_ui_manager::set_single_step));
	ui_type.set("get_line_height", &mame_ui_manager::get_line_height);
	ui_type.set("get_string_width", &mame_ui_manager::get_string_width);
	// sol converts char32_t to a string
	ui_type.set("get_char_width", [](mame_ui_manager &m, uint32_t utf8char) { return m.get_char_width(utf8char); });
	ui_type.set("set_aggressive_input_focus", [](mame_ui_manager &m, bool aggressive_focus) { osd_set_aggressive_input_focus(aggressive_focus); });


/*  device_state_entry library
 *
 * manager:machine().devices[device_tag].state[state_name]
 *
 * state:name() - get device state name
 * state:is_visible() - is state visible in debugger
 * state:is_divider() - is state a divider
 *
 * state.value - get device state value
 */

	auto dev_state_type = sol().registry().new_usertype<device_state_entry>("dev_state", "new", sol::no_constructor);
	dev_state_type.set("name", &device_state_entry::symbol);
	dev_state_type.set("value", sol::property(
		[this](device_state_entry &entry) -> uint64_t {
			device_state_interface *state = entry.parent_state();
			if(state)
			{
				machine().save().dispatch_presave();
				return state->state_int(entry.index());
			}
			return 0;
		},
		[this](device_state_entry &entry, uint64_t val) {
			device_state_interface *state = entry.parent_state();
			if(state)
			{
				state->set_state_int(entry.index(), val);
				machine().save().dispatch_presave();
			}
		}));
	dev_state_type.set("is_visible", &device_state_entry::visible);
	dev_state_type.set("is_divider", &device_state_entry::divider);


/*  rom_entry library
 *
 * manager:machine().devices[device_tag].roms[rom]
 *
 * rom:name()
 * rom:hashdata() - see hash.h
 * rom:offset()
 * rom:length()
 * rom:flags() - see romentry.h
 */

	auto rom_entry_type = sol().registry().new_usertype<rom_entry>("rom_entry", "new", sol::no_constructor);
	rom_entry_type.set("name", &rom_entry::name);
	rom_entry_type.set("hashdata", &rom_entry::hashdata);
	rom_entry_type.set("offset", &rom_entry::get_offset);
	rom_entry_type.set("length", &rom_entry::get_length);
	rom_entry_type.set("flags", &rom_entry::get_flags);


/*  output_manager library
 *
 * manager:machine():outputs()
 *
 * outputs:set_value(name, val) - set output name to val
 * outputs:set_indexed_value(index, val) - set output index to val
 * outputs:get_value(name) - get output name value
 * outputs:get_indexed_value(index) - get output index value
 * outputs:name_to_id(name) - get index for name
 * outputs:id_to_name(index) - get name for index
 */

	auto output_type = sol().registry().new_usertype<output_manager>("output", "new", sol::no_constructor);
	output_type.set("set_value", &output_manager::set_value);
	output_type.set("set_indexed_value", [](output_manager &o, char const *basename, int index, int value) {
			o.set_value(util::string_format("%s%d", basename, index).c_str(), value);
		});
	output_type.set("get_value", &output_manager::get_value);
	output_type.set("get_indexed_value", [](output_manager &o, char const *basename, int index) {
			return o.get_value(util::string_format("%s%d", basename, index).c_str());
		});
	output_type.set("name_to_id", &output_manager::name_to_id);
	output_type.set("id_to_name", &output_manager::id_to_name);


/*  device_image_interface library
 *
 * manager:machine().images[image_type]
 *
 * image:exists()
 * image:filename() - full path to the image file
 * image:longname()
 * image:manufacturer()
 * image:year()
 * image:software_list_name()
 * image:image_type_name() - floppy/cart/cdrom/tape/hdd etc
 * image:load(filename)
 * image:load_software(softlist_name)
 * image:unload()
 * image:create()
 * image:crc()
 * image:display()
 *
 * image.device - get associated device_t
 * image.instance_name
 * image.brief_instance_name
 * image.software_parent
 * image.is_readable
 * image.is_writeable
 * image.is_creatable
 * image.is_reset_on_load
 * image.must_be_loaded
 */

	auto image_type = sol().registry().new_usertype<device_image_interface>("image", "new", sol::no_constructor);
	image_type.set("exists", &device_image_interface::exists);
	image_type.set("filename", &device_image_interface::filename);
	image_type.set("longname", &device_image_interface::longname);
	image_type.set("manufacturer", &device_image_interface::manufacturer);
	image_type.set("year", &device_image_interface::year);
	image_type.set("software_list_name", &device_image_interface::software_list_name);
	image_type.set("software_parent", sol::property([](device_image_interface &di) {
			const software_info *si = di.software_entry();
			return si ? si->parentname() : "";
		}));
	image_type.set("image_type_name", &device_image_interface::image_type_name);
	image_type.set("load", &device_image_interface::load);
	image_type.set("load_software", static_cast<image_init_result (device_image_interface::*)(const std::string &)>(&device_image_interface::load_software));
	image_type.set("unload", &device_image_interface::unload);
	image_type.set("create", [](device_image_interface &di, const std::string &filename) { return di.create(filename); });
	image_type.set("crc", &device_image_interface::crc);
	image_type.set("display", [](device_image_interface &di) { return di.call_display(); });
	image_type.set("device", sol::property(static_cast<device_t & (device_image_interface::*)()>(&device_image_interface::device)));
	image_type.set("instance_name", sol::property(&device_image_interface::instance_name));
	image_type.set("brief_instance_name", sol::property(&device_image_interface::brief_instance_name));
	image_type.set("is_readable", sol::property(&device_image_interface::is_readable));
	image_type.set("is_writeable", sol::property(&device_image_interface::is_writeable));
	image_type.set("is_creatable", sol::property(&device_image_interface::is_creatable));
	image_type.set("is_reset_on_load", sol::property(&device_image_interface::is_reset_on_load));
	image_type.set("must_be_loaded", sol::property(&device_image_interface::must_be_loaded));


/*  mame_machine_manager library
 *
 * manager
 * mame_manager - alias of manager
 *
 * manager:machine() - running machine
 * manager:options() - core options
 * manager:plugins() - plugin options
 * manager:ui() - mame ui manager
 */

	sol().registry().new_usertype<mame_machine_manager>("manager", "new", sol::no_constructor,
			"machine", &machine_manager::machine,
			"options", [](mame_machine_manager &m) { return static_cast<core_options *>(&m.options()); },
			"plugins", [this](mame_machine_manager &m) {
				sol::table table = sol().create_table();
				for (auto &curentry : m.plugins().plugins())
				{
					sol::table plugin_table = sol().create_table();
					plugin_table["name"] = curentry.m_name;
					plugin_table["description"] = curentry.m_description;
					plugin_table["type"] = curentry.m_type;
					plugin_table["directory"] = curentry.m_directory;
					plugin_table["start"] = curentry.m_start;
					table[curentry.m_name] = plugin_table;
				}
				return table;
			},
			"ui", &mame_machine_manager::ui);
	sol()["manager"] = std::ref(*mame_machine_manager::instance());
	sol()["mame_manager"] = std::ref(*mame_machine_manager::instance());


	// set up other user types
	initialize_input();
	initialize_memory();
	initialize_render();
}

//-------------------------------------------------
//  frame_hook - called at each frame refresh, used to draw a HUD
//-------------------------------------------------
bool lua_engine::frame_hook()
{
	return execute_function("LUA_ON_FRAME_DONE");
}

//-------------------------------------------------
//  close - close and cleanup of lua engine
//-------------------------------------------------

void lua_engine::close()
{
	m_sol_state.reset();
	if (m_lua_state)
	{
		lua_settop(m_lua_state, 0);  /* clear stack */
		lua_close(m_lua_state);
		m_lua_state = nullptr;
	}
}

void lua_engine::resume(void *ptr, int nparam)
{
	lua_rawgeti(m_lua_state, LUA_REGISTRYINDEX, nparam);
	lua_State *L = lua_tothread(m_lua_state, -1);
	lua_pop(m_lua_state, 1);
	int stat = lua_resume(L, nullptr, 0);
	if((stat != LUA_OK) && (stat != LUA_YIELD))
	{
		osd_printf_error("[LUA ERROR] in resume: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	luaL_unref(m_lua_state, LUA_REGISTRYINDEX, nparam);
}

void lua_engine::run(sol::load_result res)
{
	if(res.valid())
	{
		auto ret = invoke(res.get<sol::protected_function>());
		if(!ret.valid())
		{
			sol::error err = ret;
			osd_printf_error("[LUA ERROR] in run: %s\n", err.what());
		}
	}
	else
		osd_printf_error("[LUA ERROR] %d loading Lua script\n", (int)res.status());
}

//-------------------------------------------------
//  execute - load and execute script
//-------------------------------------------------

void lua_engine::load_script(const char *filename)
{
	run(sol().load_file(filename));
}

//-------------------------------------------------
//  execute_string - execute script from string
//-------------------------------------------------

void lua_engine::load_string(const char *value)
{
	run(sol().load(value));
}

//-------------------------------------------------
//  invoke - invokes a function, wrapping profiler
//-------------------------------------------------

template<typename TFunc, typename... TArgs>
sol::protected_function_result lua_engine::invoke(TFunc &&func, TArgs&&... args)
{
	g_profiler.start(PROFILER_LUA);
	sol::protected_function_result result = func(std::forward<TArgs>(args)...);
	g_profiler.stop();
	return result;
}
