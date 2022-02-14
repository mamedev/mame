// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    luaengine.h

    Controls execution of the core MAME system.

***************************************************************************/
#ifndef MAME_FRONTEND_MAME_LUAENGINE_H
#define MAME_FRONTEND_MAME_LUAENGINE_H

#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#define SOL_SAFE_USERTYPE 1
#include "sol/sol.hpp"

struct lua_State;

class lua_engine
{
public:
	// helper structures
	template <typename T> struct devenum;
	template <typename T> struct simple_list_wrapper;
	template <typename T> struct tag_object_ptr_map;
	template <typename T> using standard_tag_object_ptr_map = tag_object_ptr_map<std::unordered_map<std::string, std::unique_ptr<T> > >;
	template <typename T> struct immutable_container_helper;
	template <typename T, typename C, typename I = typename C::iterator> struct immutable_collection_helper;
	template <typename T, typename C, typename I = typename C::iterator> struct immutable_sequence_helper;

	// construction/destruction
	lua_engine();
	~lua_engine();

	void initialize();
	void load_script(const char *filename);
	void load_string(const char *value);

	bool frame_hook();

	std::optional<long> menu_populate(const std::string &menu, std::vector<std::tuple<std::string, std::string, std::string> > &menu_list, std::string &flags);
	std::pair<bool, std::optional<long> > menu_callback(const std::string &menu, int index, const std::string &event);

	void set_machine(running_machine *machine);
	std::vector<std::string> &get_menu() { return m_menu; }
	void attach_notifiers();
	void on_frame_done();
	void on_sound_update();
	void on_periodic();
	bool on_missing_mandatory_image(const std::string &instance_name);
	void on_machine_before_load_settings();

	template <typename T, typename U>
	bool call_plugin(const std::string &name, T &&in, U &out)
	{
		bool ret = false;
		sol::object outobj = call_plugin(name, sol::make_object(sol(), std::forward<T>(in)));
		if (outobj.is<U>())
		{
			out = outobj.as<U>();
			ret = true;
		}
		return ret;
	}

	template <typename T, typename U>
	bool call_plugin(const std::string &name, T &&in, std::vector<U> &out)
	{
		bool ret = false;
		sol::object outobj = call_plugin(name, sol::make_object(sol(), std::forward<T>(in)));
		if (outobj.is<sol::table>())
		{
			for (auto &entry : outobj.as<sol::table>())
			{
				if (entry.second.template is<U>())
				{
					out.push_back(entry.second.template as<U>());
					ret = true;
				}
			}
		}
		return ret;
	}

	// this can also check if a returned table contains type T
	template <typename T, typename U>
	bool call_plugin_check(const std::string &name, U &&in, bool table = false)
	{
		bool ret = false;
		sol::object outobj = call_plugin(name, sol::make_object(sol(), std::forward<U>(in)));
		if (outobj.is<T>() && !table)
			ret = true;
		else if (outobj.is<sol::table>() && table)
		{
			// check just one entry, checking the whole thing shouldn't be necessary as this only supports homogeneous tables
			if (outobj.as<sol::table>().begin().operator*().second.template is<T>())
				ret = true;
		}
		return ret;
	}

	template <typename T>
	void call_plugin_set(const std::string &name, T &&in)
	{
		call_plugin(name, sol::make_object(sol(), std::forward<T>(in)));
	}

	sol::state_view &sol() const { return *m_sol_state; }

private:
	template<typename T, size_t SIZE> class enum_parser;

	struct addr_space;
	class tap_helper;
	class addr_space_change_notif;

	struct save_item {
		void *base;
		unsigned int size;
		unsigned int count;
		unsigned int valcount;
		unsigned int blockcount;
		unsigned int stride;
	};

	struct context
	{
		context() { busy = false; yield = false; }
		std::string result;
		std::condition_variable sync;
		bool busy;
		bool yield;
	};

	// internal state
	lua_State *m_lua_state;
	std::unique_ptr<sol::state_view> m_sol_state;
	running_machine *m_machine;

	std::vector<std::string> m_menu;

	template <typename R, typename T, typename D>
	auto make_simple_callback_setter(void (T::*setter)(delegate<R ()> &&), D &&dflt, const char *name, const char *desc);

	running_machine &machine() const { return *m_machine; }

	void on_machine_prestart();
	void on_machine_start();
	void on_machine_stop();
	void on_machine_pause();
	void on_machine_resume();
	void on_machine_frame();

	void resume(int nparam);
	void register_function(sol::function func, const char *id);
	int enumerate_functions(const char *id, std::function<bool(const sol::protected_function &func)> &&callback);
	bool execute_function(const char *id);
	sol::object call_plugin(const std::string &name, sol::object in);

	void close();

	void run(sol::load_result res);

	template <typename TFunc, typename... TArgs>
	sol::protected_function_result invoke(TFunc &&func, TArgs&&... args);

	void initialize_debug(sol::table &emu);
	void initialize_input(sol::table &emu);
	void initialize_memory(sol::table &emu);
	void initialize_render(sol::table &emu);
};

#endif // MAME_FRONTEND_MAME_LUAENGINE_H
