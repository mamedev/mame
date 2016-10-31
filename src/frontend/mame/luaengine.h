// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    luaengine.h

    Controls execution of the core MAME system.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __LUA_ENGINE_H__
#define __LUA_ENGINE_H__

#include <map>
#include "sol2/sol.hpp"

// None is typedef'd already in SDL/X11 libs
#ifdef None
#undef None
#endif

class cheat_manager;

struct lua_State;
namespace luabridge
{
	class LuaRef;
}

class lua_engine
{
public:
	// construction/destruction
	lua_engine();
	~lua_engine();

	void initialize();
	void start_console();
	void load_script(const char *filename);
	void load_string(const char *value);

	void serve_lua();
	void periodic_check();
	bool frame_hook();

	void menu_populate(const std::string &menu, std::vector<std::tuple<std::string, std::string, std::string>> &menu_list);
	bool menu_callback(const std::string &menu, int index, const std::string &event);

	void set_machine(running_machine *machine) { m_machine = machine; }
	std::vector<std::string> &get_menu() { return m_menu; }
	void attach_notifiers();
	void on_frame_done();
	const char *call_plugin(const char *data, const char *name);

private:
	// internal state
	lua_State *m_lua_state;
	sol::state_view *m_sol_state;
	sol::state_view &sol() const { return *m_sol_state; }
	running_machine *m_machine;

	std::vector<std::string> m_menu;

	std::map<lua_State *, std::pair<lua_State *, int> > thread_registry;

	running_machine &machine() const { return *m_machine; }

	void update_machine();

	void on_machine_prestart();
	void on_machine_start();
	void on_machine_stop();
	void on_machine_pause();
	void on_machine_resume();
	void on_machine_frame();

	void register_function(sol::function func, const char *id);
	bool execute_function(const char *id);

	struct addr_space {
		addr_space(address_space &space, device_memory_interface &dev) :
			space(space), dev(dev) {}
		template<typename T> T mem_read(offs_t address, sol::object shift);
		template<typename T> void mem_write(offs_t address, T val, sol::object shift);
		template<typename T> T log_mem_read(offs_t address);
		template<typename T> void log_mem_write(offs_t address, T val);
		template<typename T> T direct_mem_read(offs_t address);
		template<typename T> void direct_mem_write(offs_t address, T val);
		const char *name() const { return space.name(); }

		address_space &space;
		device_memory_interface &dev;
	};

	template<typename T> static T share_read(memory_share &share, offs_t address);
	template<typename T> static void share_write(memory_share &share, offs_t address, T val);
	template<typename T> static T region_read(memory_region &region, offs_t address);
	template<typename T> static void region_write(memory_region &region, offs_t address, T val);

	struct save_item {
		void *base;
		unsigned int size;
		unsigned int count;
	};

	void close();

	int report(int status);
	int incomplete(int status) ;
	int docall(int narg, int nres);
	void run(sol::load_result res);
};

#endif  /* __LUA_ENGINE_H__ */
