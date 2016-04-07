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
	void execute_function(const char *id);

	struct menu_item {
		std::string text;
		std::string subtext;
		int flags;
	};
	std::vector<menu_item> &menu_populate(std::string &menu);
	bool menu_callback(std::string &menu, int index, std::string event);

	void resume(lua_State *L, int nparam = 0, lua_State *root = nullptr);
	void set_machine(running_machine *machine) { m_machine = machine; update_machine(); }
	std::vector<std::string> &get_menu() { return m_menu; }
	void attach_notifiers();
	void on_frame_done();

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

	std::vector<std::string> m_menu;

	hook hook_output_cb;
	bool output_notifier_set;

	hook hook_frame_cb;

	static lua_engine*  luaThis;

	std::map<lua_State *, std::pair<lua_State *, int> > thread_registry;

	running_machine &machine() const { return *m_machine; }

	void update_machine();

	void on_machine_prestart();
	void on_machine_start();
	void on_machine_stop();
	void on_machine_pause();
	void on_machine_resume();
	void on_machine_frame();

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
	static int l_emu_softname(lua_State *L);
	static int l_emu_keypost(lua_State *L);
	static int l_emu_hook_output(lua_State *L);
	static int l_emu_exit(lua_State *L);
	static int l_emu_start(lua_State *L);
	static int l_emu_pause(lua_State *L);
	static int l_emu_unpause(lua_State *L);
	static int l_emu_set_hook(lua_State *L);
	static int l_emu_register_prestart(lua_State *L);
	static int l_emu_register_start(lua_State *L);
	static int l_emu_register_stop(lua_State *L);
	static int l_emu_register_pause(lua_State *L);
	static int l_emu_register_resume(lua_State *L);
	static int l_emu_register_frame(lua_State *L);
	static int l_emu_register_frame_done(lua_State *L);
	static int l_emu_register_menu(lua_State *L);
	static int register_function(lua_State *L, const char *id);

	// "emu.machine" namespace
	static luabridge::LuaRef l_machine_get_devices(const running_machine *r);
	static luabridge::LuaRef l_machine_get_images(const running_machine *r);
	static luabridge::LuaRef l_ioport_get_ports(const ioport_manager *i);
	static luabridge::LuaRef l_render_get_targets(const render_manager *r);
	static luabridge::LuaRef l_ioports_port_get_fields(const ioport_port *i);
	static luabridge::LuaRef devtree_dfs(device_t *root, luabridge::LuaRef dev_table);
	static luabridge::LuaRef l_dev_get_states(const device_t *d);
	static UINT64 l_state_get_value(const device_state_entry *d);
	static void l_state_set_value(device_state_entry *d, UINT64 v);
	static luabridge::LuaRef l_dev_get_memspaces(const device_t *d);
	struct lua_machine {
		int l_popmessage(lua_State *L);
		int l_logerror(lua_State *L);
	};
	static UINT8 read_direct_byte(address_space &space, offs_t addr);
	static void write_direct_byte(address_space &space, offs_t addr, UINT8 byte);
	struct lua_addr_space {
		template<typename T> int l_mem_read(lua_State *L);
		template<typename T> int l_mem_write(lua_State *L);
		template<typename T> int l_direct_mem_read(lua_State *L);
		template<typename T> int l_direct_mem_write(lua_State *L);
	};
	static luabridge::LuaRef l_machine_get_screens(const running_machine *r);
	struct lua_screen {
		int l_height(lua_State *L);
		int l_width(lua_State *L);
		int l_orientation(lua_State *L);
		int l_refresh(lua_State *L);
		int l_type(lua_State *L);
		int l_snapshot(lua_State *L);
		int l_draw_box(lua_State *L);
		int l_draw_line(lua_State *L);
		int l_draw_text(lua_State *L);
	};
	static luabridge::LuaRef l_dev_get_items(const device_t *d);

	struct lua_video {
		int l_begin_recording(lua_State *L);
		int l_end_recording(lua_State *L);
	};

	static luabridge::LuaRef l_cheat_get_entries(const cheat_manager *c);
	struct lua_cheat_entry {
		int l_get_state(lua_State *L);
	};

	template<typename T> static luabridge::LuaRef l_options_get_entries(const T *o);
	struct lua_options_entry {
		int l_entry_value(lua_State *L);
	};

	static luabridge::LuaRef l_memory_get_banks(const memory_manager *m);
	static luabridge::LuaRef l_memory_get_regions(const memory_manager *m);
	static UINT8 read_region_byte(memory_region &region, offs_t addr);
	static void write_region_byte(memory_region &region, offs_t addr, UINT8 byte);
	struct lua_memory_region {
		template<typename T> int l_region_read(lua_State *L);
		template<typename T> int l_region_write(lua_State *L);
	};

	struct lua_emu_file {
		int l_emu_file_read(lua_State *L);
	};

	struct lua_item {
		lua_item(int index);
		void *l_item_base;
		unsigned int l_item_size;
		unsigned int l_item_count;
		int l_item_read(lua_State *L);
		int l_item_read_block(lua_State *L);
		int l_item_write(lua_State *L);
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
