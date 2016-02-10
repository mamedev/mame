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

// None is typedef'd already in SDL/X11 libs
#ifdef None
#undef None
#endif

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

	void resume(lua_State *L, int nparam = 0, lua_State *root = nullptr);
	void set_machine(running_machine *machine) { m_machine = machine; update_machine(); }
private:
	struct hook {
		lua_State *L;
		int cb;

		hook();
		void set(lua_State *L, int idx);
		lua_State *precall();
		void call(lua_engine *engine, lua_State *T, int nparam);
		bool active() const { return L != nullptr; }
	};

	static const char *const tname_ioport;

	// internal state
	lua_State          *m_lua_state;
	running_machine *   m_machine;

	hook hook_output_cb;
	bool output_notifier_set;

	hook hook_frame_cb;

	static lua_engine*  luaThis;

	std::map<lua_State *, std::pair<lua_State *, int> > thread_registry;

	running_machine &machine() const { return *m_machine; }

	void update_machine();
	void output_notifier(const char *outname, INT32 value);
	static void s_output_notifier(const char *outname, INT32 value, void *param);

	void emu_after_done(void *_h, INT32 param);
	int emu_after(lua_State *L);
	int emu_wait(lua_State *L);
	void emu_hook_output(lua_State *L);
	void emu_set_hook(lua_State *L);

	static int l_ioport_write(lua_State *L);
	static int l_emu_after(lua_State *L);
	static int l_emu_app_name(lua_State *L);
	static int l_emu_app_version(lua_State *L);
	static int l_emu_wait(lua_State *L);
	static int l_emu_time(lua_State *L);
	static int l_emu_gamename(lua_State *L);
	static int l_emu_romname(lua_State *L);
	static int l_emu_keypost(lua_State *L);
	static int l_emu_hook_output(lua_State *L);
	static int l_emu_exit(lua_State *L);
	static int l_emu_start(lua_State *L);
	static int l_emu_pause(lua_State *L);
	static int l_emu_unpause(lua_State *L);
	static int l_emu_set_hook(lua_State *L);

	// "emu.machine" namespace
	static luabridge::LuaRef l_machine_get_devices(const running_machine *r);
	static luabridge::LuaRef devtree_dfs(device_t *root, luabridge::LuaRef dev_table);
	static luabridge::LuaRef l_dev_get_states(const device_t *d);
	static UINT64 l_state_get_value(const device_state_entry *d);
	static void l_state_set_value(device_state_entry *d, UINT64 v);
	static luabridge::LuaRef l_dev_get_memspaces(const device_t *d);
	struct lua_addr_space {
		template<typename T> int l_mem_read(lua_State *L);
		template<typename T> int l_mem_write(lua_State *L);
	};
	static luabridge::LuaRef l_machine_get_screens(const running_machine *r);
	struct lua_screen {
		int l_height(lua_State *L);
		int l_width(lua_State *L);
		int l_draw_box(lua_State *L);
		int l_draw_line(lua_State *L);
		int l_draw_text(lua_State *L);
	};

	void resume(void *L, INT32 param);
	void start();
	static int luaopen_ioport(lua_State *L);
	void close();

	static void *checkparam(lua_State *L, int idx, const char *tname);
	static void *getparam(lua_State *L, int idx, const char *tname);
	static void push(lua_State *L, void *p, const char *tname);

	int report(int status);
	int docall(int narg, int nres);
	int incomplete(int status) ;
};

#endif  /* __LUA_ENGINE_H__ */
