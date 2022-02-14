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

#include "imagedev/cassette.h"

#include "debugger.h"
#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "inputdev.h"
#include "natkeyboard.h"
#include "screen.h"
#include "softlist.h"
#include "uiinput.h"

#include "corestr.h"
#include "notifier.h"

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

} // extern "C"


template <typename T>
struct lua_engine::devenum
{
	template <typename... U> devenum(device_t &d, U &&... args) : device(d), iter(d, std::forward<U>(args)...) { }

	device_t &device;
	T iter;
	int count = -1;
};


namespace {

struct image_interface_formats
{
	image_interface_formats(device_image_interface &i) : image(i) { }
	device_image_interface::formatlist_type const &items() { return image.formatlist(); }

	static image_device_format const &unwrap(device_image_interface::formatlist_type::const_iterator const &it) { return **it; }
	static int push_key(lua_State *L, device_image_interface::formatlist_type::const_iterator const &it, std::size_t ix) { return sol::stack::push_reference(L, (*it)->name()); }

	device_image_interface &image;
};


struct plugin_options_plugins
{
	plugin_options_plugins(plugin_options &o) : options(o) { }
	std::list<plugin_options::plugin> &items() { return options.plugins(); }

	static plugin_options::plugin const &unwrap(std::list<plugin_options::plugin>::const_iterator const &it) { return *it; }
	static int push_key(lua_State *L, std::list<plugin_options::plugin>::const_iterator const &it, std::size_t ix) { return sol::stack::push_reference(L, it->m_name); }

	plugin_options &options;
};

} // anonymous namespace


namespace sol
{

template <> struct is_container<image_interface_formats> : std::true_type { };
template <> struct is_container<plugin_options_plugins> : std::true_type { };


sol::buffer *sol_lua_get(sol::types<buffer *>, lua_State *L, int index, sol::stack::record &tracking)
{
	return new sol::buffer(stack::get<int>(L, index), L);
}

int sol_lua_push(sol::types<buffer *>, lua_State *L, buffer *value)
{
	delete value;
	return 1;
}


template <typename T>
struct usertype_container<lua_engine::devenum<T> > : lua_engine::immutable_collection_helper<lua_engine::devenum<T>, T>
{
private:
	using enumerator = lua_engine::devenum<T>;

	template <bool Indexed>
	static int next_pairs(lua_State *L)
	{
		typename usertype_container::indexed_iterator &i(stack::unqualified_get<user<typename usertype_container::indexed_iterator> >(L, 1));
		if (i.src.end() == i.it)
			return stack::push(L, lua_nil);
		int result;
		if constexpr (Indexed)
			result = stack::push(L, i.ix + 1);
		else
			result = stack::push(L, i.it->tag());
		result += stack::push_reference(L, *i.it);
		++i;
		return result;
	}

	template <bool Indexed>
	static int start_pairs(lua_State *L)
	{
		enumerator &self(usertype_container::get_self(L));
		stack::push(L, next_pairs<Indexed>);
		stack::push<user<typename usertype_container::indexed_iterator> >(L, self.iter, self.iter.begin());
		stack::push(L, lua_nil);
		return 3;
	}

public:
	static int at(lua_State *L)
	{
		enumerator &self(usertype_container::get_self(L));
		std::ptrdiff_t const index(stack::unqualified_get<std::ptrdiff_t>(L, 2));
		auto const dev(self.iter.byindex(index - 1));
		if (dev)
			return stack::push_reference(L, *dev);
		else
			return stack::push(L, lua_nil);
	}

	static int get(lua_State *L)
	{
		enumerator &self(usertype_container::get_self(L));
		char const *const tag(stack::unqualified_get<char const *>(L));
		device_t *const dev(self.device.subdevice(tag));
		if (dev)
		{
			auto *const check(T(*dev, 0).first());
			bool match;
			if constexpr (std::is_base_of_v<device_t, decltype(*check)>)
				match = check && (static_cast<device_t *>(check) == dev);
			else if constexpr (std::is_base_of_v<device_interface, decltype(*check)>)
				match = check && (&check->device() == dev);
			else
				match = check && (dynamic_cast<device_t *>(check) == dev);
			if (match)
				return stack::push_reference(L, *check);
		}
		return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}

	static int index_of(lua_State *L)
	{
		enumerator &self(usertype_container::get_self(L));
		auto &dev(stack::unqualified_get<decltype(*self.iter.first())>(L, 2));
		std::ptrdiff_t found(self.iter.indexof(dev));
		if (0 > found)
			return stack::push(L, lua_nil);
		else
			return stack::push(L, found + 1);
	}

	static int size(lua_State *L)
	{
		enumerator &self(usertype_container::get_self(L));
		if (0 > self.count)
			self.count = self.iter.count();
		return stack::push(L, self.count);
	}

	static int empty(lua_State *L)
	{
		enumerator &self(usertype_container::get_self(L));
		if (0 > self.count)
			self.count = self.iter.count();
		return stack::push(L, !self.count);
	}

	static int next(lua_State *L) { return stack::push(L, next_pairs<false>); }
	static int pairs(lua_State *L) { return start_pairs<false>(L); }
	static int ipairs(lua_State *L) { return start_pairs<true>(L); }
};


template <>
struct usertype_container<image_interface_formats> : lua_engine::immutable_sequence_helper<image_interface_formats, device_image_interface::formatlist_type const, device_image_interface::formatlist_type::const_iterator>
{
private:
	using format_list = device_image_interface::formatlist_type;

public:
	static int get(lua_State *L)
	{
		image_interface_formats &self(get_self(L));
		char const *const name(stack::unqualified_get<char const *>(L));
		auto const found(std::find_if(
					self.image.formatlist().begin(),
					self.image.formatlist().end(),
					[&name] (std::unique_ptr<image_device_format> const &v) { return v->name() == name; }));
		if (self.image.formatlist().end() != found)
			return stack::push_reference(L, **found);
		else
			return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}
};


template <>
struct usertype_container<plugin_options_plugins> : lua_engine::immutable_sequence_helper<plugin_options_plugins, std::list<plugin_options::plugin> >
{
private:
	using plugin_list = std::list<plugin_options::plugin>;

public:
	static int get(lua_State *L)
	{
		plugin_options_plugins &self(get_self(L));
		char const *const name(stack::unqualified_get<char const *>(L));
		auto const found(std::find_if(
					self.options.plugins().begin(),
					self.options.plugins().end(),
					[&name] (plugin_options::plugin const &p) { return p.m_name == name; }));
		if (self.options.plugins().end() != found)
			return stack::push_reference(L, *found);
		else
			return stack::push(L, lua_nil);
	}

	static int index_get(lua_State *L)
	{
		return get(L);
	}
};

} // namespace sol


int sol_lua_push(sol::types<std::error_condition>, lua_State *L, std::error_condition &&value)
{
	if (!value)
		return sol::stack::push(L, sol::lua_nil);
	else
		return sol::stack::push(L, value.message());
}


int sol_lua_push(sol::types<screen_type_enum>, lua_State *L, screen_type_enum &&value)
{
	switch (value)
	{
	case SCREEN_TYPE_INVALID:   return sol::stack::push(L, "invalid");
	case SCREEN_TYPE_RASTER:    return sol::stack::push(L, "raster");
	case SCREEN_TYPE_VECTOR:    return sol::stack::push(L, "vector");
	case SCREEN_TYPE_LCD:       return sol::stack::push(L, "svg");
	case SCREEN_TYPE_SVG:       return sol::stack::push(L, "none");
	}
	return sol::stack::push(L, "unknown");
}

int sol_lua_push(sol::types<image_init_result>, lua_State *L, image_init_result &&value)
{
	switch (value)
	{
	case image_init_result::PASS:   return sol::stack::push(L, "pass");
	case image_init_result::FAIL:   return sol::stack::push(L, "fail");
	}
	return sol::stack::push(L, "invalid");
}

int sol_lua_push(sol::types<image_verify_result>, lua_State *L, image_verify_result &&value)
{
	switch (value)
	{
	case image_verify_result::PASS: return sol::stack::push(L, "pass");
	case image_verify_result::FAIL: return sol::stack::push(L, "fail");
	}
	return sol::stack::push(L, "invalid");
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
	return sol::lua_nil;
}

std::optional<long> lua_engine::menu_populate(const std::string &menu, std::vector<std::tuple<std::string, std::string, std::string> > &menu_list, std::string &flags)
{
	std::string field = "menu_pop_" + menu;
	sol::object obj = sol().registry()[field];
	if (obj.is<sol::protected_function>())
	{
		auto res = invoke(obj.as<sol::protected_function>());
		if (!res.valid())
		{
			sol::error err = res;
			osd_printf_error("[LUA ERROR] in menu_populate: %s\n", err.what());
		}
		else
		{
			std::tuple<sol::table, std::optional<long>, std::string> table = res;
			for (auto &entry : std::get<0>(table))
			{
				if (entry.second.is<sol::table>())
				{
					sol::table enttable = entry.second.as<sol::table>();
					menu_list.emplace_back(enttable.get<std::string, std::string, std::string>(1, 2, 3));
				}
			}
			flags = std::get<2>(table);
			return std::get<1>(table);
		}
	}
	flags.clear();
	return std::nullopt;
}

std::pair<bool, std::optional<long> > lua_engine::menu_callback(const std::string &menu, int index, const std::string &event)
{
	std::string field = "menu_cb_" + menu;
	std::pair<bool, std::optional<long> > ret(false, std::nullopt);
	sol::object obj = sol().registry()[field];
	if (obj.is<sol::protected_function>())
	{
		auto res = invoke(obj.as<sol::protected_function>(), index, event);
		if (!res.valid())
		{
			sol::error err = res;
			osd_printf_error("[LUA ERROR] in menu_callback: %s\n", err.what());
		}
		else
		{
			ret = res;
		}
	}
	return ret;
}

void lua_engine::set_machine(running_machine *machine)
{
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


	static const enum_parser<int, 3> s_seek_parser =
	{
		{ "set", SEEK_SET },
		{ "cur", SEEK_CUR },
		{ "end", SEEK_END }
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
 * emu.subst_env(str) - substitute environment variables with values for str (semantics are OS-specific)
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
 *
 * emu.device_enumerator(dev) - get device enumerator starting at arbitrary point in tree
 * emu.screen_enumerator(dev) - get screen device enumerator starting at arbitrary point in tree
 * emu.image_enumerator(dev) - get image interface enumerator starting at arbitrary point in tree
 * emu.image_enumerator(dev) - get image interface enumerator starting at arbitrary point in tree
 */

	sol::table emu = sol().create_named_table("emu");
	emu["app_name"] = &emulator_info::get_appname_lower;
	emu["app_version"] = &emulator_info::get_bare_build_version;
	emu["gamename"] = [this] () { return machine().system().type.fullname(); };
	emu["romname"] = [this] () { return machine().basename(); };
	emu["softname"] = [this] () { return machine().options().software_name(); };
	emu["keypost"] = [this] (const char *keys) { machine().natkeyboard().post_utf8(keys); };
	emu["time"] = [this] () { return machine().time().as_double(); };
	emu["start"] =
		[this](const char *driver)
		{
			int i = driver_list::find(driver);
			if (i != -1)
			{
				mame_machine_manager::instance()->schedule_new_driver(driver_list::driver(i));
				machine().schedule_hard_reset();
			}
			return 1;
		};
	emu["pause"] = [this] () { return machine().pause(); };
	emu["unpause"] = [this] () { return machine().resume(); };
	emu["step"] =
		[this] ()
		{
			mame_machine_manager::instance()->ui().set_single_step(true);
			machine().resume();
		};
	emu["register_prestart"] = [this] (sol::function func) { register_function(func, "LUA_ON_PRESTART"); };
	emu["register_start"] = [this] (sol::function func) { register_function(func, "LUA_ON_START"); };
	emu["register_stop"] = [this] (sol::function func) { register_function(func, "LUA_ON_STOP"); };
	emu["register_pause"] = [this] (sol::function func) { register_function(func, "LUA_ON_PAUSE"); };
	emu["register_resume"] = [this] (sol::function func) { register_function(func, "LUA_ON_RESUME"); };
	emu["register_frame"] = [this] (sol::function func) { register_function(func, "LUA_ON_FRAME"); };
	emu["register_frame_done"] = [this] (sol::function func) { register_function(func, "LUA_ON_FRAME_DONE"); };
	emu["register_sound_update"] = [this] (sol::function func) { register_function(func, "LUA_ON_SOUND_UPDATE"); };
	emu["register_periodic"] = [this] (sol::function func) { register_function(func, "LUA_ON_PERIODIC"); };
	emu["register_mandatory_file_manager_override"] = [this] (sol::function func) { register_function(func, "LUA_ON_MANDATORY_FILE_MANAGER_OVERRIDE"); };
	emu["register_before_load_settings"] = [this](sol::function func) { register_function(func, "LUA_ON_BEFORE_LOAD_SETTINGS"); };
	emu["register_menu"] =
		[this] (sol::function cb, sol::function pop, const std::string &name)
		{
			std::string cbfield = "menu_cb_" + name;
			std::string popfield = "menu_pop_" + name;
			sol().registry()[cbfield] = cb;
			sol().registry()[popfield] = pop;
			m_menu.push_back(name);
		};
	emu["show_menu"] =
		[this](const char *name)
		{
			mame_ui_manager &mui = mame_machine_manager::instance()->ui();
			render_container &container = machine().render().ui_container();
			ui::menu_plugin::show_menu(mui, container, (char *)name);
		};
	emu["register_callback"] =
		[this] (sol::function cb, const std::string &name)
		{
			std::string field = "cb_" + name;
			sol().registry()[field] = cb;
		};
	emu["print_verbose"] = [] (const char *str) { osd_printf_verbose("%s\n", str); };
	emu["print_error"] = [] (const char *str) { osd_printf_error("%s\n", str); };
	emu["print_info"] = [] (const char *str) { osd_printf_info("%s\n", str); };
	emu["print_debug"] = [] (const char *str) { osd_printf_debug("%s\n", str); };
	emu["osd_ticks"] = &osd_ticks;
	emu["osd_ticks_per_second"] = &osd_ticks_per_second;
	emu["driver_find"] =
		[] (sol::this_state s, const char *driver) -> sol::object
		{
			const int i = driver_list::find(driver);
			if (i < 0)
				return sol::lua_nil;
			return sol::make_object(s, driver_list::driver(i));
		};
	emu["wait"] = lua_CFunction(
			[] (lua_State *L)
			{
				lua_engine *engine = mame_machine_manager::instance()->lua();
				luaL_argcheck(L, lua_isnumber(L, 1), 1, "waiting duration expected");
				int ret = lua_pushthread(L);
				if (ret == 1)
					return luaL_error(L, "cannot wait from outside coroutine");
				int ref = luaL_ref(L, LUA_REGISTRYINDEX);
				engine->machine().scheduler().timer_set(attotime::from_double(lua_tonumber(L, 1)), timer_expired_delegate(FUNC(lua_engine::resume), engine), ref);
				return lua_yield(L, 0);
			});
	emu["lang_translate"] = sol::overload(
			static_cast<char const *(*)(char const *)>(&lang_translate),
			static_cast<char const *(*)(char const *, char const *)>(&lang_translate));
	emu["pid"] = &osd_getpid;
	emu["subst_env"] =
		[] (const std::string &str)
		{
			std::string result;
			osd_subst_env(result, str);
			return result;
		};
	emu["device_enumerator"] = sol::overload(
			[] (device_t &dev) { return devenum<device_enumerator>(dev); },
			[] (device_t &dev, int maxdepth) { return devenum<device_enumerator>(dev, maxdepth); });
	emu["screen_enumerator"] = sol::overload(
			[] (device_t &dev) { return devenum<screen_device_enumerator>(dev); },
			[] (device_t &dev, int maxdepth) { return devenum<screen_device_enumerator>(dev, maxdepth); });
	emu["cassette_enumerator"] = sol::overload(
			[] (device_t &dev) { return devenum<cassette_device_enumerator>(dev); },
			[] (device_t &dev, int maxdepth) { return devenum<cassette_device_enumerator>(dev, maxdepth); });
	emu["image_enumerator"] = sol::overload(
			[] (device_t &dev) { return devenum<image_interface_enumerator>(dev); },
			[] (device_t &dev, int maxdepth) { return devenum<image_interface_enumerator>(dev, maxdepth); });
	emu["slot_enumerator"] = sol::overload(
			[] (device_t &dev) { return devenum<slot_interface_enumerator>(dev); },
			[] (device_t &dev, int maxdepth) { return devenum<slot_interface_enumerator>(dev, maxdepth); });


	auto notifier_subscription_type = sol().registry().new_usertype<util::notifier_subscription>("notifier_subscription", sol::no_constructor);
	notifier_subscription_type["unsubscribe"] = &util::notifier_subscription::reset;
	notifier_subscription_type["is_active"] = sol::property(&util::notifier_subscription::operator bool);

	auto attotime_type = emu.new_usertype<attotime>(
			"attotime",
			sol::call_constructor, sol::constructors<attotime(), attotime(seconds_t, attoseconds_t), attotime(attotime const &)>());
	attotime_type["from_double"] = &attotime::from_double;
	attotime_type["from_ticks"] = static_cast<attotime (*)(u64, u32)>(&attotime::from_ticks);
	attotime_type["from_seconds"] = &attotime::from_seconds;
	attotime_type["from_msec"] = &attotime::from_msec;
	attotime_type["from_usec"] = &attotime::from_usec;
	attotime_type["from_nsec"] = &attotime::from_nsec;
	attotime_type["as_double"] = &attotime::as_double;
	attotime_type["as_hz"] = &attotime::as_hz;
	attotime_type["as_khz"] = &attotime::as_khz;
	attotime_type["as_mhz"] = &attotime::as_mhz;
	attotime_type["as_ticks"] = static_cast<u64 (attotime::*)(u32) const>(&attotime::as_ticks);
	attotime_type["is_zero"] = sol::property(&attotime::is_zero);
	attotime_type["is_never"] = sol::property(&attotime::is_never);
	attotime_type["attoseconds"] = sol::property(&attotime::attoseconds);
	attotime_type["seconds"] = sol::property(&attotime::seconds);
	attotime_type["msec"] = sol::property([] (attotime const &t) { return t.attoseconds() / ATTOSECONDS_PER_MILLISECOND; });
	attotime_type["usec"] = sol::property([] (attotime const &t) { return t.attoseconds() / ATTOSECONDS_PER_MICROSECOND; });
	attotime_type["nsec"] = sol::property([] (attotime const &t) { return t.attoseconds() / ATTOSECONDS_PER_NANOSECOND; });
	attotime_type[sol::meta_function::to_string] = &attotime::to_string;
	attotime_type[sol::meta_function::addition] = static_cast<attotime (*)(attotime const &, attotime const &)>(&operator+);
	attotime_type[sol::meta_function::subtraction] = static_cast<attotime (*)(attotime const &, attotime const &)>(&operator-);
	attotime_type[sol::meta_function::multiplication] = static_cast<attotime (*)(attotime const &, u32)>(&operator*);
	attotime_type[sol::meta_function::division] = static_cast<attotime (*)(attotime const &, u32)>(&operator/);


/* emu_file library
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
	file_type.set("puts", &emu_file::puts);
	file_type.set("open", static_cast<std::error_condition (emu_file::*)(std::string_view)>(&emu_file::open));
	file_type.set("open_next", &emu_file::open_next);
	file_type.set("close", &emu_file::close);
	file_type.set("seek", sol::overload(
			[](emu_file &file) { return file.tell(); },
			[this] (emu_file &file, s64 offset, int whence) -> sol::object {
				if(file.seek(offset, whence))
					return sol::lua_nil;
				else
					return sol::make_object(sol(), file.tell());
			},
			[this](emu_file &file, const char* whence) -> sol::object {
				int wval = s_seek_parser(whence);
				if(wval < 0 || wval >= 3)
					return sol::lua_nil;
				if(file.seek(0, wval))
					return sol::lua_nil;
				return sol::make_object(sol(), file.tell());
			},
			[this](emu_file &file, const char* whence, s64 offset) -> sol::object {
				int wval = s_seek_parser(whence);
				if(wval < 0 || wval >= 3)
					return sol::lua_nil;
				if(file.seek(offset, wval))
					return sol::lua_nil;
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
			if (ctx.busy)
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
						if (ret.valid())
						{
							const char *tmp = ret.get<const char *>();
							if (tmp != nullptr)
								ctx.result = tmp;
							else
								osd_printf_error("[LUA ERROR] in thread: return value must be string\n");
						}
						else
						{
							sol::error err = ret;
							osd_printf_error("[LUA ERROR] in thread: %s\n", err.what());
						}
					}
					else
					{
						sol::error err = res;
						osd_printf_error("[LUA ERROR] when loading script for thread: %s\n", err.what());
					}
					ctx.busy = false;
				});
			ctx.busy = true;
			ctx.yield = false;
			th.detach();
			return true;
		});
	thread_type.set("continue", [](context &ctx, const char *val) {
			if (!ctx.yield)
				return;
			ctx.result = val;
			ctx.sync.notify_all();
		});
	thread_type.set("result", sol::property([](context &ctx) -> std::string {
			if (ctx.busy && !ctx.yield)
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
				return sol::lua_nil;
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


/* core_options library
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


/* emu_options library
 *
 * manager:options()
 * manager:machine():options()
 *
 * options:slot_option(tag) - retrieves a specific slot option
 */

	auto emu_options_type = sol().registry().new_usertype<emu_options>("emu_options", sol::no_constructor, sol::base_classes, sol::bases<core_options>());
	emu_options_type["slot_option"] = [] (emu_options &opts, std::string const &name) { return opts.find_slot_option(name); };


/* slot_option library
 *
 * manager:options():slot_option("name")
 * manager:machine():options():slot_option("name")
 *
 * slot_option:specify(card, bios) - specifies the value of the slot, potentially causing a recalculation
 *
 * slot_option.value - the actual value of the option, after being interpreted
 * slot_option.specified_value - the value of the option, as specified from outside
 * slot_option.bios - the bios, if any, associated with the slot
 * slot_option.default_card_software - the software list item that is associated with this option, by default
 */

	auto slot_option_type = sol().registry().new_usertype<slot_option>("slot_option", sol::no_constructor);
	slot_option_type["specify"] =
		[] (slot_option &opt, std::string &&text, char const *bios)
		{
			opt.specify(std::move(text));
			if (bios)
				opt.set_bios(bios);
		};
	slot_option_type["value"] = sol::property(&slot_option::value);
	slot_option_type["specified_value"] = sol::property(&slot_option::specified_value);
	slot_option_type["bios"] = sol::property(&slot_option::bios);
	slot_option_type["default_card_software"] = sol::property(&slot_option::default_card_software);


/* core_options::entry library
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
				return sol::lua_nil;
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


	auto machine_type = sol().registry().new_usertype<running_machine>("machine", sol::no_constructor);
	machine_type["exit"] = &running_machine::schedule_exit;
	machine_type["hard_reset"] = &running_machine::schedule_hard_reset;
	machine_type["soft_reset"] = &running_machine::schedule_soft_reset;
	machine_type["save"] = &running_machine::schedule_save; // TODO: some kind of completion notification?
	machine_type["load"] = &running_machine::schedule_load; // TODO: some kind of completion notification?
	machine_type["buffer_save"] =
		[] (running_machine &m, sol::this_state s)
		{
			// FIXME: this needs to schedule saving to a buffer and return asynchronously somehow
			// right now it's broken by anonymous timers, synchronize, etc.
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
		};
	machine_type["buffer_load"] =
		[] (running_machine &m, sol::this_state s, std::string str)
		{
			// FIXME: this needs to schedule loading from the buffer and return asynchronously somehow
			// right now it's broken by anonymous timers, synchronize, etc.
			save_error error = m.save().read_buffer((u8 *)str.data(), str.size());
			if (error == STATERR_NONE)
			{
				return true;
			}
			else
			{
				luaL_error(s,"State load error.");
				return false;
			}
		};
	machine_type["popmessage"] =
		[] (running_machine &m, const char *str)
		{
			if (str)
				m.popmessage("%s", str);
			else
				m.popmessage();
		};
	machine_type["logerror"]  = [] (running_machine &m, std::string const *str) { m.logerror("[luaengine] %s\n", str); };
	machine_type["time"] = sol::property(&running_machine::time);
	machine_type["system"] = sol::property(&running_machine::system);
	machine_type["parameters"] = sol::property(&running_machine::parameters);
	machine_type["video"] = sol::property(&running_machine::video);
	machine_type["sound"] = sol::property(&running_machine::sound);
	machine_type["output"] = sol::property(&running_machine::output);
	machine_type["memory"] = sol::property(&running_machine::memory);
	machine_type["ioport"] = sol::property(&running_machine::ioport);
	machine_type["input"] = sol::property(&running_machine::input);
	machine_type["natkeyboard"] = sol::property(&running_machine::natkeyboard);
	machine_type["uiinput"] = sol::property(&running_machine::ui_input);
	machine_type["render"] = sol::property(&running_machine::render);
	machine_type["debugger"] = sol::property(
			[] (running_machine &m, sol::this_state s) -> sol::object
			{
				if (m.debug_flags & DEBUG_FLAG_ENABLED)
					return sol::make_object(s, &m.debugger());
				else
					return sol::lua_nil;
			});
	machine_type["options"] = sol::property(&running_machine::options);
	machine_type["samplerate"] = sol::property(&running_machine::sample_rate);
	machine_type["paused"] = sol::property(&running_machine::paused);
	machine_type["exit_pending"] = sol::property(&running_machine::exit_pending);
	machine_type["hard_reset_pending"] = sol::property(&running_machine::hard_reset_pending);
	machine_type["devices"] = sol::property([] (running_machine &m) { return devenum<device_enumerator>(m.root_device()); });
	machine_type["screens"] = sol::property([] (running_machine &m) { return devenum<screen_device_enumerator>(m.root_device()); });
	machine_type["cassettes"] = sol::property([] (running_machine &m) { return devenum<cassette_device_enumerator>(m.root_device()); });
	machine_type["images"] = sol::property([] (running_machine &m) { return devenum<image_interface_enumerator>(m.root_device()); });
	machine_type["slots"] = sol::property([](running_machine &m) { return devenum<slot_interface_enumerator>(m.root_device()); });


	auto game_driver_type = sol().registry().new_usertype<game_driver>("game_driver", sol::no_constructor);
	game_driver_type["name"] = sol::property([] (game_driver const &driver) { return &driver.name[0]; });
	game_driver_type["description"] = sol::property([] (game_driver const &driver) { return &driver.type.fullname()[0]; });
	game_driver_type["year"] = sol::readonly(&game_driver::year);
	game_driver_type["manufacturer"] = sol::readonly(&game_driver::manufacturer);
	game_driver_type["parent"] = sol::readonly(&game_driver::parent);
	game_driver_type["compatible_with"] = sol::property([] (game_driver const &driver) { return strcmp(driver.compatible_with, "0") ? driver.compatible_with : nullptr; });
	game_driver_type["source_file"] = sol::property([] (game_driver const &driver) { return &driver.type.source()[0]; });
	game_driver_type["orientation"] = sol::property(
			[] (game_driver const &driver)
			{
				// FIXME: this works differently to the screen orientation function and the render target orientation property
				// it should probably be made consistent with one of them
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
			});
	game_driver_type["type"] = sol::property(
			[] (game_driver const &driver)
			{
				// FIXME: this shouldn't be called type - there's potendial for confusion with the device type
				// also, this should eventually go away in favour of richer flags
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
			});
	game_driver_type["not_working"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::NOT_WORKING) != 0; });
	game_driver_type["supports_save"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::SUPPORTS_SAVE) != 0; });
	game_driver_type["no_cocktail"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::NO_COCKTAIL) != 0; });
	game_driver_type["is_bios_root"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::IS_BIOS_ROOT) != 0; });
	game_driver_type["requires_artwork"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::REQUIRES_ARTWORK) != 0; });
	game_driver_type["clickable_artwork"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::CLICKABLE_ARTWORK) != 0; });
	game_driver_type["unofficial"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::UNOFFICIAL) != 0; });
	game_driver_type["no_sound_hw"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::NO_SOUND_HW) != 0; });
	game_driver_type["mechanical"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::MECHANICAL) != 0; });
	game_driver_type["is_incomplete"] = sol::property([] (game_driver const &driver) { return (driver.flags & machine_flags::IS_INCOMPLETE) != 0; });


	auto device_type = sol().registry().new_usertype<device_t>("device", sol::no_constructor);
	device_type["subtag"] = &device_t::subtag;
	device_type["siblingtag"] = &device_t::siblingtag;
	device_type["memregion"] = &device_t::memregion;
	device_type["memshare"] = &device_t::memshare;
	device_type["membank"] = &device_t::membank;
	device_type["ioport"] = &device_t::ioport;
	device_type["subdevice"] = static_cast<device_t *(device_t::*)(std::string_view) const>(&device_t::subdevice);
	device_type["siblingdevice"] = static_cast<device_t *(device_t::*)(std::string_view) const>(&device_t::siblingdevice);
	device_type["parameter"] = &device_t::parameter;
	device_type["tag"] = sol::property(&device_t::tag);
	device_type["basetag"] = sol::property(&device_t::basetag);
	device_type["name"] = sol::property(&device_t::name);
	device_type["shortname"] = sol::property(&device_t::shortname);
	device_type["owner"] = sol::property(&device_t::owner);
	device_type["configured"] = sol::property(&device_t::configured);
	device_type["started"] = sol::property(&device_t::started);
	device_type["debug"] = sol::property(
			[] (device_t &dev, sol::this_state s) -> sol::object
			{
				if (!(dev.machine().debug_flags & DEBUG_FLAG_ENABLED) || !dynamic_cast<cpu_device *>(&dev)) // debugger not enabled or not CPU
					return sol::lua_nil;
				return sol::make_object(s, dev.debug());
			});
	device_type["spaces"] = sol::property(
			[this] (device_t &dev)
			{
				device_memory_interface *const memdev = dynamic_cast<device_memory_interface *>(&dev);
				sol::table sp_table = sol().create_table();
				if (!memdev)
					return sp_table;
				for (int sp = 0; sp < memdev->max_space_count(); ++sp)
				{
					if (memdev->has_space(sp))
						sp_table[memdev->space(sp).name()] = addr_space(memdev->space(sp), *memdev);
				}
				return sp_table;
			});
	// FIXME: improve this
	device_type["state"] = sol::property(
			[this] (device_t &dev)
			{
				sol::table st_table = sol().create_table();
				const device_state_interface *state;
				if(!dev.interface(state))
					return st_table;
				// XXX: refrain from exporting non-visible entries?
				for(auto &s : state->state_entries())
					st_table[s->symbol()] = s.get();
				return st_table;
			});
	// FIXME: turn into a wrapper - it's stupid slow to walk on every property access
	// also, this mixes up things like RAM areas with stuff saved by the device itself, so there's potential for key conflicts
	device_type["items"] = sol::property(
			[this] (device_t &dev)
			{
				sol::table table = sol().create_table();
				std::string const tag = dev.tag();
				for (int i = 0; ; i++)
				{
					char const *item;
					void *base;
					uint32_t size, valcount, blockcount, stride;
					item = dev.machine().save().indexed_item(i, base, size, valcount, blockcount, stride);
					if (!item)
						break;

					char const *name = &strchr(item, '/')[1];
					if (!strncmp(tag.c_str(), name, tag.length()) && (name[tag.length()] == '/'))
						table[name + tag.length() + 1] = i;
				}
				return table;
			});
	// FIXME: this is useless in its current form
	device_type["roms"] = sol::property(
			[this] (device_t &dev)
			{
				sol::table table = sol().create_table();
				for (auto rom : dev.rom_region_vector())
					if (!rom.name().empty())
						table[rom.name()] = rom;
				return table;
			});


	auto screen_dev_type = sol().registry().new_usertype<screen_device>(
			"screen_dev",
			sol::no_constructor,
			sol::base_classes, sol::bases<device_t>());
	screen_dev_type["draw_box"] =
			[] (screen_device &sdev, float x1, float y1, float x2, float y2, std::optional<uint32_t> fgcolor, std::optional<uint32_t> bgcolor)
			{
				float const sc_width(sdev.visible_area().width());
				float const sc_height(sdev.visible_area().height());
				x1 = std::clamp(x1, 0.0f, sc_width) / sc_width;
				y1 = std::clamp(y1, 0.0f, sc_height) / sc_height;
				x2 = std::clamp(x2, 0.0f, sc_width) / sc_width;
				y2 = std::clamp(y2, 0.0f, sc_height) / sc_height;
				mame_ui_manager &ui(mame_machine_manager::instance()->ui());
				if (!fgcolor)
					fgcolor = ui.colors().text_color();
				if (!bgcolor)
					bgcolor = ui.colors().background_color();
				ui.draw_outlined_box(sdev.container(), x1, y1, x2, y2, *fgcolor, *bgcolor);
			};
	screen_dev_type["draw_line"] =
			[] (screen_device &sdev, float x1, float y1, float x2, float y2, std::optional<uint32_t> color)
			{
				float const sc_width(sdev.visible_area().width());
				float const sc_height(sdev.visible_area().height());
				x1 = std::clamp(x1, 0.0f, sc_width) / sc_width;
				y1 = std::clamp(y1, 0.0f, sc_height) / sc_height;
				x2 = std::clamp(x2, 0.0f, sc_width) / sc_width;
				y2 = std::clamp(y2, 0.0f, sc_height) / sc_height;
				if (!color)
					color = mame_machine_manager::instance()->ui().colors().text_color();
				sdev.container().add_line(x1, y1, x2, y2, UI_LINE_WIDTH, rgb_t(*color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			};
	screen_dev_type["draw_text"] =
			[this] (screen_device &sdev, sol::object xobj, float y, char const *msg, std::optional<uint32_t> fgcolor, std::optional<uint32_t> bgcolor)
			{
				float const sc_width(sdev.visible_area().width());
				float const sc_height(sdev.visible_area().height());
				auto justify = ui::text_layout::text_justify::LEFT;
				float x = 0;
				if (xobj.is<float>())
				{
					x = std::clamp(xobj.as<float>(), 0.0f, sc_width) / sc_width;
				}
				else if (xobj.is<char const *>())
				{
					char const *const justifystr(xobj.as<char const *>());
					if (!strcmp(justifystr, "left"))
						justify = ui::text_layout::text_justify::LEFT;
					else if (!strcmp(justifystr, "right"))
						justify = ui::text_layout::text_justify::RIGHT;
					else if (!strcmp(justifystr, "center"))
						justify = ui::text_layout::text_justify::CENTER;
				}
				else
				{
					luaL_error(m_lua_state, "Error in param 1 to draw_text");
					return;
				}
				y = std::clamp(y, 0.0f, sc_height) / sc_height;
				mame_ui_manager &ui(mame_machine_manager::instance()->ui());
				if (!fgcolor)
					fgcolor = ui.colors().text_color();
				if (!bgcolor)
					bgcolor = 0;
				ui.draw_text_full(
						sdev.container(),
						msg,
						x, y, (1.0f - x),
						justify, ui::text_layout::word_wrapping::WORD,
						mame_ui_manager::OPAQUE_, *fgcolor, *bgcolor);
			};
	screen_dev_type["orientation"] =
		[] (screen_device &sdev)
		{
			uint32_t flags = sdev.orientation();
			int rotation_angle = 0;
			switch (flags)
			{
			case ORIENTATION_SWAP_XY:
			case ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X:
				rotation_angle = 90;
				flags ^= ORIENTATION_FLIP_X;
				break;
			case ORIENTATION_FLIP_Y:
			case ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y:
				rotation_angle = 180;
				flags ^= ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y;
				break;
			case ORIENTATION_SWAP_XY | ORIENTATION_FLIP_Y:
			case ORIENTATION_SWAP_XY | ORIENTATION_FLIP_X | ORIENTATION_FLIP_Y:
				rotation_angle = 270;
				flags ^= ORIENTATION_FLIP_Y;
				break;
			}
			return std::tuple<int, bool, bool>(rotation_angle, flags & ORIENTATION_FLIP_X, flags & ORIENTATION_FLIP_Y);
		};
	screen_dev_type["time_until_pos"] = sol::overload(
			[] (screen_device &sdev, int vpos) { return sdev.time_until_pos(vpos).as_double(); },
			[] (screen_device &sdev, int vpos, int hpos) { return sdev.time_until_pos(vpos, hpos).as_double(); });
	screen_dev_type["time_until_vblank_start"] = &screen_device::time_until_vblank_start;
	screen_dev_type["time_until_vblank_end"] = &screen_device::time_until_vblank_end;
	screen_dev_type["snapshot"] =
		[this] (screen_device &sdev, char const *filename) -> sol::object
		{
			// FIXME: this shouldn't be a member of the screen device
			// the screen is only used as a hint when configured for native snapshots and may be ignored
			std::string snapstr;
			bool is_absolute_path = false;
			if (filename)
			{
				// a filename was specified; if it isn't absolute post-process it
				snapstr = process_snapshot_filename(machine(), filename);
				is_absolute_path = osd_is_absolute_path(snapstr);
			}

			// open the file
			emu_file file(is_absolute_path ? "" : machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
			std::error_condition filerr;
			if (!snapstr.empty())
				filerr = file.open(snapstr);
			else
				filerr = machine().video().open_next(file, "png");
			if (filerr)
				return sol::make_object(sol(), filerr);

			// and save the snapshot
			machine().video().save_snapshot(&sdev, file);
			return sol::lua_nil;
		};
	screen_dev_type["pixel"] = [] (screen_device &sdev, s32 x, s32 y) { return sdev.pixel(x, y); };
	screen_dev_type["pixels"] =
		[] (screen_device &sdev, sol::this_state s)
		{
			// TODO: would be better if this could return a tuple of (buffer, width, height)
			const rectangle &visarea = sdev.visible_area();
			luaL_Buffer buff;
			int size = visarea.height() * visarea.width() * 4;
			u32 *ptr = (u32 *)luaL_buffinitsize(s, &buff, size);
			sdev.pixels(ptr);
			luaL_pushresultsize(&buff, size);
			return sol::make_reference(s, sol::stack_reference(s, -1));
		};
	screen_dev_type["screen_type"] = sol::property(&screen_device::screen_type);
	screen_dev_type["width"] = sol::property([] (screen_device &sdev) { return sdev.visible_area().width(); });
	screen_dev_type["height"] = sol::property([] (screen_device &sdev) { return sdev.visible_area().height(); });
	screen_dev_type["refresh"] = sol::property([] (screen_device &sdev) { return ATTOSECONDS_TO_HZ(sdev.refresh_attoseconds()); });
	screen_dev_type["refresh_attoseconds"] = sol::property([] (screen_device &sdev) { return sdev.refresh_attoseconds(); });
	screen_dev_type["xofffset"] = sol::property(&screen_device::xoffset);
	screen_dev_type["yofffset"] = sol::property(&screen_device::yoffset);
	screen_dev_type["xscale"] = sol::property(&screen_device::xscale);
	screen_dev_type["yscale"] = sol::property(&screen_device::yscale);
	screen_dev_type["pixel_period"] = sol::property([] (screen_device &sdev) { return sdev.pixel_period().as_double(); });
	screen_dev_type["scan_period"] = sol::property([] (screen_device &sdev) { return sdev.scan_period().as_double(); });
	screen_dev_type["frame_period"] = sol::property([] (screen_device &sdev) { return sdev.frame_period().as_double(); });
	screen_dev_type["frame_number"] = &screen_device::frame_number;
	screen_dev_type["container"] = sol::property(&screen_device::container);


	auto cass_type = sol().registry().new_usertype<cassette_image_device>(
			"cassette",
			sol::no_constructor,
			sol::base_classes, sol::bases<device_t, device_image_interface>());
	cass_type["stop"] = [] (cassette_image_device &c) { c.change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE); };
	cass_type["play"] = [] (cassette_image_device &c) { c.change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE); };
	cass_type["record"] = [] (cassette_image_device &c) { c.change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE); };
	cass_type["forward"] = &cassette_image_device::go_forward;
	cass_type["reverse"] = &cassette_image_device::go_reverse;
	cass_type["seek"] = [] (cassette_image_device &c, double time, const char* origin) { if (c.exists()) c.seek(time, s_seek_parser(origin)); };
	cass_type["is_stopped"] = sol::property(&cassette_image_device::is_stopped);
	cass_type["is_playing"] = sol::property(&cassette_image_device::is_playing);
	cass_type["is_recording"] = sol::property(&cassette_image_device::is_recording);
	cass_type["motor_state"] = sol::property(&cassette_image_device::motor_on, &cassette_image_device::set_motor);
	cass_type["speaker_state"] = sol::property(&cassette_image_device::speaker_on, &cassette_image_device::set_speaker);
	cass_type["position"] = sol::property(&cassette_image_device::get_position);
	cass_type["length"] = sol::property([] (cassette_image_device &c) { return c.exists() ? c.get_length() : 0.0; });


	auto image_type = sol().registry().new_usertype<device_image_interface>("image", sol::no_constructor);
	image_type["load"] = &device_image_interface::load;
	image_type["load_software"] = static_cast<image_init_result (device_image_interface::*)(std::string_view)>(&device_image_interface::load_software);
	image_type["unload"] = &device_image_interface::unload;
	image_type["create"] = static_cast<image_init_result (device_image_interface::*)(std::string_view)>(&device_image_interface::create);
	image_type["display"] = &device_image_interface::call_display;
	image_type["is_readable"] = sol::property(&device_image_interface::is_readable);
	image_type["is_writeable"] = sol::property(&device_image_interface::is_writeable);
	image_type["is_creatable"] = sol::property(&device_image_interface::is_creatable);
	image_type["must_be_loaded"] = sol::property(&device_image_interface::must_be_loaded);
	image_type["is_reset_on_load"] = sol::property(&device_image_interface::is_reset_on_load);
	image_type["image_type_name"] = sol::property(&device_image_interface::image_type_name);
	image_type["instance_name"] = sol::property(&device_image_interface::instance_name);
	image_type["brief_instance_name"] = sol::property(&device_image_interface::brief_instance_name);
	image_type["formatlist"] = sol::property([] (device_image_interface &image) { return image_interface_formats(image); });
	image_type["exists"] = sol::property(&device_image_interface::exists);
	image_type["readonly"] = sol::property(&device_image_interface::is_readonly);
	image_type["filename"] = sol::property(&device_image_interface::filename);
	image_type["crc"] = sol::property(&device_image_interface::crc);
	image_type["loaded_through_softlist"] = sol::property(&device_image_interface::loaded_through_softlist);
	image_type["software_list_name"] = sol::property(&device_image_interface::software_list_name);
	image_type["software_longname"] = sol::property(
			[] (device_image_interface &di)
			{
				software_info const *const si(di.software_entry());
				return si ? si->longname().c_str() : nullptr;
			});
	image_type["software_publisher"] = sol::property(
			[] (device_image_interface &di)
			{
				software_info const *const si(di.software_entry());
				return si ? si->publisher().c_str() : nullptr;
			});
	image_type["software_year"] = sol::property(
			[] (device_image_interface &di)
			{
				software_info const *const si(di.software_entry());
				return si ? si->year().c_str() : nullptr;
			});
	image_type["software_parent"] = sol::property(
			[] (device_image_interface &di)
			{
				software_info const *const si(di.software_entry());
				return si ? si->parentname().c_str() : nullptr;
			});
	image_type["device"] = sol::property(static_cast<device_t & (device_image_interface::*)()>(&device_image_interface::device));


	auto format_type = sol().registry().new_usertype<image_device_format>("image_format", sol::no_constructor);
	format_type["name"] = sol::property(&image_device_format::name);
	format_type["description"] = sol::property(&image_device_format::description);
	format_type["extensions"] = sol::property(
			[this] (image_device_format const &format)
			{
				int index = 1;
				sol::table option_table = sol().create_table();
				for (std::string const &ext : format.extensions())
					option_table[index++] = ext;
				return option_table;
			});
	format_type["option_spec"] = sol::property(&image_device_format::optspec);


	auto slot_type = sol().registry().new_usertype<device_slot_interface>("slot", sol::no_constructor);
	slot_type["fixed"] = sol::property(&device_slot_interface::fixed);
	slot_type["has_selectable_options"] = sol::property(&device_slot_interface::has_selectable_options);
	slot_type["default_option"] = sol::property(&device_slot_interface::default_option);
	slot_type["options"] = sol::property([] (device_slot_interface const &slot) { return standard_tag_object_ptr_map<device_slot_interface::slot_option>(slot.option_list()); });
	slot_type["device"] = sol::property(static_cast<device_t & (device_slot_interface::*)()>(&device_slot_interface::device));


	auto dislot_option_type = sol().registry().new_usertype<device_slot_interface::slot_option>("dislot_option", sol::no_constructor);
	dislot_option_type["name"] = sol::property(&device_slot_interface::slot_option::name);
	dislot_option_type["device_fullname"] = sol::property([] (device_slot_interface::slot_option &opt) { return opt.devtype().fullname(); });
	dislot_option_type["device_shortname"] = sol::property([] (device_slot_interface::slot_option &opt) { return opt.devtype().shortname(); });
	dislot_option_type["selectable"] = sol::property(&device_slot_interface::slot_option::selectable);
	dislot_option_type["default_bios"] = sol::property(static_cast<char const * (device_slot_interface::slot_option::*)() const>(&device_slot_interface::slot_option::default_bios));
	dislot_option_type["clock"] = sol::property(static_cast<u32 (device_slot_interface::slot_option::*)() const>(&device_slot_interface::slot_option::clock));


	auto parameters_type = sol().registry().new_usertype<parameters_manager>("parameters", sol::no_constructor);
	parameters_type["add"] = &parameters_manager::add;
	parameters_type["lookup"] = &parameters_manager::lookup;


	auto video_type = sol().registry().new_usertype<video_manager>("video", sol::no_constructor);
	video_type["frame_update"] = [] (video_manager &vm) { vm.frame_update(true); };
	video_type["snapshot"] = &video_manager::save_active_screen_snapshots;
	video_type["begin_recording"] =
		[this] (video_manager &vm, const char *filename, const char *format_string)
		{
			// FIXME: the filename substitution shouldn't be done here
			std::string fn;
			movie_recording::format format = movie_recording::format::AVI;
			if (filename)
				fn = process_snapshot_filename(machine(), filename);
			if (format_string)
				format = s_movie_recording_format_parser(format_string);
			vm.begin_recording(filename ? fn.c_str() : nullptr, format);
		};
	video_type["end_recording"] = &video_manager::end_recording;
	video_type["snapshot_size"] =
		[] (video_manager &vm)
		{
			s32 width, height;
			vm.compute_snapshot_size(width, height);
			return std::make_tuple(width, height);
		};
	video_type["snapshot_pixels"] =
		[] (video_manager &vm, sol::this_state s)
		{
			// TODO: would be better if this could return a tuple of (buffer, width, height)
			s32 width, height;
			vm.compute_snapshot_size(width, height);
			int size = width * height * 4;
			luaL_Buffer buff;
			u32 *ptr = (u32 *)luaL_buffinitsize(s, &buff, size);
			vm.pixels(ptr);
			luaL_pushresultsize(&buff, size);
			return sol::make_reference(s, sol::stack_reference(s, -1));
		};
	video_type["speed_factor"] = sol::property(&video_manager::speed_factor);
	video_type["throttled"] = sol::property(&video_manager::throttled, &video_manager::set_throttled);
	video_type["throttle_rate"] = sol::property(&video_manager::throttle_rate, &video_manager::set_throttle_rate);
	video_type["frameskip"] = sol::property(&video_manager::frameskip, &video_manager::set_frameskip);
	video_type["speed_percent"] = sol::property(&video_manager::speed_percent);
	video_type["effective_frameskip"] = sol::property(&video_manager::effective_frameskip);
	video_type["skip_this_frame"] = sol::property(&video_manager::skip_this_frame);
	video_type["snap_native"] = sol::property(&video_manager::snap_native);
	video_type["is_recording"] = sol::property(&video_manager::is_recording);
	video_type["snapshot_target"] = sol::property(&video_manager::snapshot_target);


	auto sound_type = sol().registry().new_usertype<sound_manager>("sound", sol::no_constructor);
	sound_type["start_recording"] =
		[] (sound_manager &sm, char const *filename)
		{
			return filename ? sm.start_recording(filename) : sm.start_recording();
		};
	sound_type["stop_recording"] = &sound_manager::stop_recording;
	sound_type["get_samples"] =
		[] (sound_manager &sm, sol::this_state s)
		{
			luaL_Buffer buff;
			s32 const count = sm.sample_count() * 2 * 2; // 2 channels, 2 bytes per sample
			s16 *const ptr = (s16 *)luaL_buffinitsize(s, &buff, count);
			sm.samples(ptr);
			luaL_pushresultsize(&buff, count);
			return sol::make_reference(s, sol::stack_reference(s, -1));
		};
	sound_type["muted"] = sol::property(&sound_manager::muted);
	sound_type["ui_mute"] = sol::property(
			static_cast<bool (sound_manager::*)() const>(&sound_manager::ui_mute),
			static_cast<void (sound_manager::*)(bool)>(&sound_manager::ui_mute));
	sound_type["debugger_mute"] = sol::property(
			static_cast<bool (sound_manager::*)() const>(&sound_manager::debugger_mute),
			static_cast<void (sound_manager::*)(bool)>(&sound_manager::debugger_mute));
	sound_type["system_mute"] = sol::property(
			static_cast<bool (sound_manager::*)() const>(&sound_manager::system_mute),
			static_cast<void (sound_manager::*)(bool)>(&sound_manager::system_mute));
	sound_type["attenuation"] = sol::property(
			&sound_manager::attenuation,
			&sound_manager::set_attenuation);
	sound_type["recording"] = sol::property(&sound_manager::is_recording);


	auto ui_type = sol().registry().new_usertype<mame_ui_manager>("ui", sol::no_constructor);
	// sol converts char32_t to a string
	ui_type["get_char_width"] = [] (mame_ui_manager &m, uint32_t utf8char) { return m.get_char_width(utf8char); };
	ui_type["get_string_width"] = &mame_ui_manager::get_string_width;
	ui_type["set_aggressive_input_focus"] = [](mame_ui_manager &m, bool aggressive_focus) { osd_set_aggressive_input_focus(aggressive_focus); };
	ui_type["get_general_input_setting"] = sol::overload(
			// TODO: overload with sequence type string - parser isn't available here
			[] (mame_ui_manager &ui, ioport_type type, int player) { return ui.get_general_input_setting(type, player, SEQ_TYPE_STANDARD); },
			[] (mame_ui_manager &ui, ioport_type type) { return ui.get_general_input_setting(type, 0, SEQ_TYPE_STANDARD); });
	ui_type["options"] = sol::property([] (mame_ui_manager &m) { return static_cast<core_options *>(&m.options()); });
	ui_type["line_height"] = sol::property(&mame_ui_manager::get_line_height);
	ui_type["menu_active"] = sol::property(&mame_ui_manager::is_menu_active);
	ui_type["single_step"] = sol::property(&mame_ui_manager::single_step, &mame_ui_manager::set_single_step);
	ui_type["show_fps"] = sol::property(&mame_ui_manager::show_fps, &mame_ui_manager::set_show_fps);
	ui_type["show_profiler"] = sol::property(&mame_ui_manager::show_profiler, &mame_ui_manager::set_show_profiler);


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


/* rom_entry library
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


	auto output_type = sol().registry().new_usertype<output_manager>("output", sol::no_constructor);
	output_type["set_value"] = &output_manager::set_value;
	output_type["set_indexed_value"] =
		[] (output_manager &o, char const *basename, int index, int value)
		{
			o.set_value(util::string_format("%s%d", basename, index), value);
		};
	output_type["get_value"] = &output_manager::get_value;
	output_type["get_indexed_value"] =
		[] (output_manager &o, char const *basename, int index)
		{
			return o.get_value(util::string_format("%s%d", basename, index));
		};
	output_type["name_to_id"] = &output_manager::name_to_id;
	output_type["id_to_name"] = &output_manager::id_to_name;


	auto mame_manager_type = sol().registry().new_usertype<mame_machine_manager>("manager", sol::no_constructor);
	mame_manager_type["machine"] = sol::property(&mame_machine_manager::machine);
	mame_manager_type["ui"] = sol::property(&mame_machine_manager::ui);
	mame_manager_type["options"] = sol::property(&mame_machine_manager::options);
	mame_manager_type["plugins"] = sol::property([] (mame_machine_manager &m) { return plugin_options_plugins(m.plugins()); });
	sol()["manager"] = std::ref(*mame_machine_manager::instance());
	sol()["mame_manager"] = std::ref(*mame_machine_manager::instance());


	auto plugin_type = sol().registry().new_usertype<plugin_options::plugin>("plugin", sol::no_constructor);
	plugin_type["name"] = sol::readonly(&plugin_options::plugin::m_name);
	plugin_type["description"] = sol::readonly(&plugin_options::plugin::m_description);
	plugin_type["type"] = sol::readonly(&plugin_options::plugin::m_type);
	plugin_type["directory"] = sol::readonly(&plugin_options::plugin::m_directory);
	plugin_type["start"] = sol::readonly(&plugin_options::plugin::m_start);


	// set up other user types
	initialize_debug(emu);
	initialize_input(emu);
	initialize_memory(emu);
	initialize_render(emu);
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

void lua_engine::resume(int nparam)
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
