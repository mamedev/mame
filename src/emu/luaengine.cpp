// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine.c

    Controls execution of the core MAME system.

***************************************************************************/

#include <limits>
#include <thread>
#include <lua.hpp>
#include "luabridge/Source/LuaBridge/LuaBridge.h"
#include <signal.h>
#include "emu.h"
#include "cheat.h"
#include "drivenum.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "luaengine.h"
#include <mutex>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wshift-count-overflow"
#endif
//**************************************************************************
//  LUA ENGINE
//**************************************************************************

#if !defined(LUA_PROMPT)
#define LUA_PROMPT      "> "
#define LUA_PROMPT2     ">> "
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT        512
#endif

#define lua_readline(b,p) \
	(fputs(p, stdout), fflush(stdout),  /* show prompt */ \
	fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */

static lua_State *globalL = nullptr;

#define luai_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#define luai_writeline()    (luai_writestring("\n", 1), fflush(stdout))

const char *const lua_engine::tname_ioport = "lua.ioport";
lua_engine* lua_engine::luaThis = nullptr;

extern "C" {
	int luaopen_lsqlite3(lua_State *L);
	int luaopen_zlib(lua_State *L);
	int luaopen_lfs(lua_State *L);
}

static void lstop(lua_State *L, lua_Debug *ar)
{
	(void)ar;  /* unused arg. */
	lua_sethook(L, nullptr, 0, 0);
	luaL_error(L, "interrupted!");
}


static void laction(int i)
{
	signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
	lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

int lua_engine::report(int status) {
	if (status != LUA_OK && !lua_isnil(m_lua_state, -1))
	{
		const char *msg = lua_tostring(m_lua_state, -1);
		if (msg == nullptr) msg = "(error object is not a string)";
		lua_writestringerror("%s\n", msg);
		lua_pop(m_lua_state, 1);
		/* force a complete garbage collection in case of errors */
		lua_gc(m_lua_state, LUA_GCCOLLECT, 0);
	}
	return status;
}


static int traceback (lua_State *L)
{
	const char *msg = lua_tostring(L, 1);
	if (msg)
	luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1))
	{  /* is there an error object? */
	if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
		lua_pushliteral(L, "(no error message)");
	}
	return 1;
}


int lua_engine::docall(int narg, int nres)
{
	int status;
	int base = lua_gettop(m_lua_state) - narg;  /* function index */
	lua_pushcfunction(m_lua_state, traceback);  /* push traceback function */
	lua_insert(m_lua_state, base);  /* put it under chunk and args */
	globalL = m_lua_state;  /* to be available to 'laction' */
	signal(SIGINT, laction);
	status = lua_pcall(m_lua_state, narg, nres, base);
	signal(SIGINT, SIG_DFL);
	lua_remove(m_lua_state, base);  /* remove traceback function */
	return status;
}

namespace luabridge
{
template <>
struct Stack <osd_file::error>
{
	static void push(lua_State *L, osd_file::error error)
	{
		std::string strerror;
		switch(error)
		{
			case osd_file::error::NONE:
				lua_pushboolean(L, false);
				return;
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
		lua_pushstring(L, strerror.c_str());
	}
};
}

/* mark in error messages for incomplete statements */
#define EOFMARK     "<eof>"
#define marklen     (sizeof(EOFMARK)/sizeof(char) - 1)

int lua_engine::incomplete(int status)
{
	if (status == LUA_ERRSYNTAX)
	{
		size_t lmsg;
		const char *msg = lua_tolstring(m_lua_state, -1, &lmsg);
		if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0)
		{
			lua_pop(m_lua_state, 1);
			return 1;
		}
	}
	return 0;  /* else... */
}

lua_engine::hook::hook()
{
	L = nullptr;
	cb = -1;
}

void lua_engine::hook::set(lua_State *lua, int idx)
{
	if (L)
		luaL_unref(L, LUA_REGISTRYINDEX, cb);

	if (lua_isnil(lua, idx)) {
		L = nullptr;
		cb = -1;

	} else {
		L = lua;
		lua_pushvalue(lua, idx);
		cb = luaL_ref(lua, LUA_REGISTRYINDEX);
	}
}

lua_State *lua_engine::hook::precall()
{
	lua_State *T = lua_newthread(L);
	lua_rawgeti(T, LUA_REGISTRYINDEX, cb);
	return T;
}

void lua_engine::hook::call(lua_engine *engine, lua_State *T, int nparam)
{
	engine->resume(T, nparam, L);
}

void lua_engine::resume(lua_State *L, int nparam, lua_State *root)
{
	int s = lua_resume(L, nullptr, nparam);
	switch(s) {
	case LUA_OK:
		if(!root) {
			std::map<lua_State *, std::pair<lua_State *, int> >::iterator i = thread_registry.find(L);
			if(i != thread_registry.end()) {
				luaL_unref(i->second.first, LUA_REGISTRYINDEX, i->second.second);
				thread_registry.erase(i);
			}
		} else
			lua_pop(root, 1);
		break;

	case LUA_YIELD:
		if(root) {
			int id = luaL_ref(root, LUA_REGISTRYINDEX);
			thread_registry[L] = std::pair<lua_State *, int>(root, id);
		}
		break;

	default:
		osd_printf_error("[LUA ERROR] %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		break;
	}
}

void lua_engine::resume(void *lua, INT32 param)
{
	resume(static_cast<lua_State *>(lua));
}

int lua_engine::l_ioport_write(lua_State *L)
{
	ioport_field *field = static_cast<ioport_field *>(getparam(L, 1, tname_ioport));
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "value expected");
	field->set_value(lua_tointeger(L, 2));
	return 0;
}

//-------------------------------------------------
//  emu_app_name - return application name
//-------------------------------------------------

int lua_engine::l_emu_app_name(lua_State *L)
{
	lua_pushstring(L, emulator_info::get_appname_lower());
	return 1;
}

//-------------------------------------------------
//  emu_app_version - return application version
//-------------------------------------------------

int lua_engine::l_emu_app_version(lua_State *L)
{
	lua_pushstring(L, bare_build_version);
	return 1;
}


//-------------------------------------------------
//  emu_gamename - returns game full name
//-------------------------------------------------

int lua_engine::l_emu_gamename(lua_State *L)
{
	lua_pushstring(L, luaThis->machine().system().description);
	return 1;
}

//-------------------------------------------------
//  emu_romname - returns rom base name
//-------------------------------------------------

int lua_engine::l_emu_romname(lua_State *L)
{
	lua_pushstring(L, luaThis->machine().basename());
	return 1;
}

//-------------------------------------------------
//  emu_softname - returns softlist name
//-------------------------------------------------

int lua_engine::l_emu_softname(lua_State *L)
{
	lua_pushstring(L, luaThis->machine().options().software_name());
	return 1;
}

//-------------------------------------------------
//  emu_pause/emu_unpause - pause/unpause game
//-------------------------------------------------

int lua_engine::l_emu_pause(lua_State *L)
{
	luaThis->machine().pause();
	return 0;
}

int lua_engine::l_emu_unpause(lua_State *L)
{
	luaThis->machine().resume();
	return 0;
}

//-------------------------------------------------
//  emu_keypost - post keys to natural keyboard
//-------------------------------------------------

int lua_engine::l_emu_keypost(lua_State *L)
{
	const char *keys = luaL_checkstring(L,1);
	luaThis->machine().ioport().natkeyboard().post_utf8(keys);
	return 1;
}

int lua_engine::l_emu_time(lua_State *L)
{
	lua_pushnumber(L, luaThis->machine().time().as_double());
	return 1;
}

void lua_engine::emu_after_done(void *_h, INT32 param)
{
	hook *h = static_cast<hook *>(_h);
	h->call(this, h->precall(), 0);
	delete h;
}

int lua_engine::emu_after(lua_State *L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "waiting duration expected");
	struct hook *h = new hook;
	h->set(L, 2);
	machine().scheduler().timer_set(attotime::from_double(lua_tonumber(L, 1)), timer_expired_delegate(FUNC(lua_engine::emu_after_done), this), 0, h);
	return 0;
}

int lua_engine::l_emu_after(lua_State *L)
{
	return luaThis->emu_after(L);
}

int lua_engine::emu_wait(lua_State *L)
{
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "waiting duration expected");
	machine().scheduler().timer_set(attotime::from_double(lua_tonumber(L, 1)), timer_expired_delegate(FUNC(lua_engine::resume), this), 0, L);
	return lua_yieldk(L, 0, 0, nullptr);
}

int lua_engine::l_emu_wait(lua_State *L)
{
	return luaThis->emu_wait(L);
}

void lua_engine::output_notifier(const char *outname, INT32 value)
{
	if (hook_output_cb.active()) {
		lua_State *L = hook_output_cb.precall();
		lua_pushstring(L, outname);
		lua_pushnumber(L, value);
		hook_output_cb.call(this, L, 2);
	}
}

void lua_engine::s_output_notifier(const char *outname, INT32 value, void *param)
{
	static_cast<lua_engine *>(param)->output_notifier(outname, value);
}

void lua_engine::emu_hook_output(lua_State *L)
{
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "callback function expected");
	hook_output_cb.set(L, 1);

	if (!output_notifier_set) {
		machine().output().set_notifier(nullptr, s_output_notifier, this);
		output_notifier_set = true;
	}
}

int lua_engine::l_emu_hook_output(lua_State *L)
{
	luaThis->emu_hook_output(L);
	return 0;
}

int lua_engine::l_emu_set_hook(lua_State *L)
{
	luaThis->emu_set_hook(L);
	return 0;
}

void lua_engine::emu_set_hook(lua_State *L)
{
	luaL_argcheck(L, lua_isfunction(L, 1) || lua_isnil(L, 1), 1, "callback function expected");
	luaL_argcheck(L, lua_isstring(L, 2), 2, "message (string) expected");
	const char *hookname = luaL_checkstring(L,2);

	if (strcmp(hookname, "output") == 0) {
		hook_output_cb.set(L, 1);
		if (!output_notifier_set) {
			machine().output().set_notifier(nullptr, s_output_notifier, this);
			output_notifier_set = true;
		}
	} else if (strcmp(hookname, "frame") == 0) {
		hook_frame_cb.set(L, 1);
	} else {
		lua_writestringerror("%s", "Unknown hook name, aborting.\n");
	}
}

//-------------------------------------------------
//  options_entry - return table of option entries
//  -> manager:options().entries
//  -> manager:machine():options().entries
//  -> manager:machine():ui():options().entries
//-------------------------------------------------

template <typename T>
luabridge::LuaRef lua_engine::l_options_get_entries(const T *o)
{
	T *options = const_cast<T *>(o);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef entries_table = luabridge::LuaRef::newTable(L);

	int unadorned_index = 0;
	for (typename T::entry &curentry : *options)
	{
		const char *name = curentry.name();
		bool is_unadorned = false;
		// check if it's unadorned
		if (name && strlen(name) && !strcmp(name, options->unadorned(unadorned_index)))
		{
			unadorned_index++;
			is_unadorned = true;
		}
		if (!curentry.is_header() && !curentry.is_command() && !curentry.is_internal() && !is_unadorned)
			entries_table[name] = &curentry;
	}

	return entries_table;
}

//-------------------------------------------------
//  machine_get_screens - return table of available screens userdata
//  -> manager:machine().screens[":screen"]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_machine_get_screens(const running_machine *r)
{
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef screens_table = luabridge::LuaRef::newTable(L);

	for (device_t *dev = r->first_screen(); dev != nullptr; dev = dev->next()) {
		screen_device *sc = dynamic_cast<screen_device *>(dev);
		if (sc && sc->configured() && sc->started() && sc->type()) {
			screens_table[sc->tag()] = sc;
		}
	}

	return screens_table;
}

//-------------------------------------------------
//  machine_get_devices - return table of available devices userdata
//  -> manager:machine().devices[":maincpu"]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_machine_get_devices(const running_machine *r)
{
	running_machine *m = const_cast<running_machine *>(r);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef devs_table = luabridge::LuaRef::newTable(L);

	device_t *root = &(m->root_device());
	devs_table = devtree_dfs(root, devs_table);

	return devs_table;
}

//-------------------------------------------------
//  machine_get_images - return table of available image devices userdata
//  -> manager:machine().images["flop1"]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_machine_get_images(const running_machine *r)
{
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef image_table = luabridge::LuaRef::newTable(L);

	image_interface_iterator iter(r->root_device());
	for (device_image_interface *image = iter.first(); image != nullptr; image = iter.next()) {
		image_table[image->brief_instance_name()] = image;
		image_table[image->instance_name()] = image;
	}

	return image_table;
}

//-------------------------------------------------
//  memory_banks - return memory_banks
//  -> manager:machine():memory().banks["maincpu"]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_memory_get_banks(const memory_manager *m)
{
	memory_manager *mm = const_cast<memory_manager *>(m);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef table = luabridge::LuaRef::newTable(L);

	for (memory_bank &bank : mm->banks()) {
		table[bank.tag()] = &bank;
	}

	return table;
}

//-------------------------------------------------
//  memory_regions - return memory_regions
//  -> manager:machine():memory().region[":maincpu"]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_memory_get_regions(const memory_manager *m)
{
	memory_manager *mm = const_cast<memory_manager *>(m);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef table = luabridge::LuaRef::newTable(L);

	for (memory_region &region: mm->regions()) {
		table[region.name()] = &region;
	}

	return table;
}

//-------------------------------------------------
//  machine_cheat_entries - return cheat entries
//  -> manager:machine():cheat().entries[0]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_cheat_get_entries(const cheat_manager *c)
{
	cheat_manager *cm = const_cast<cheat_manager *>(c);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef entry_table = luabridge::LuaRef::newTable(L);

	int cheatnum = 0;
	for (cheat_entry &entry : cm->entries()) {
		entry_table[cheatnum++] = &entry;
	}

	return entry_table;
}

//-------------------------------------------------
//  cheat_entry_state - return cheat entry state
//  -> manager:machine():cheat().entries[0]:state()
//-------------------------------------------------

int lua_engine::lua_cheat_entry::l_get_state(lua_State *L)
{
	cheat_entry *ce = luabridge::Stack<cheat_entry *>::get(L, 1);

	switch (ce->state())
	{
		case SCRIPT_STATE_ON:     lua_pushliteral(L, "on"); break;
		case SCRIPT_STATE_RUN:    lua_pushliteral(L, "run"); break;
		case SCRIPT_STATE_CHANGE: lua_pushliteral(L, "change"); break;
		case SCRIPT_STATE_COUNT:  lua_pushliteral(L, "count"); break;
		default:                  lua_pushliteral(L, "off"); break;
	}

	return 1;
}

//-------------------------------------------------
//  machine_ioports - return table of ioports
//  -> manager:machine():ioport().ports[':P1']
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_ioport_get_ports(const ioport_manager *m)
{
	ioport_manager *im = const_cast<ioport_manager *>(m);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef port_table = luabridge::LuaRef::newTable(L);

	for (ioport_port &port : im->ports()) {
		port_table[port.tag()] = &port;
	}

	return port_table;
}

//-------------------------------------------------
//  ioport_fields - return table of ioport fields
//  -> manager:machine().ioport().ports[':P1'].fields[':']
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_ioports_port_get_fields(const ioport_port *i)
{
	ioport_port *p = const_cast<ioport_port *>(i);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef f_table = luabridge::LuaRef::newTable(L);

	for (ioport_field &field : p->fields()) {
		f_table[field.name()] = &field;
	}

	return f_table;
}

//-------------------------------------------------
//  render_get_targets - return table of render targets
//  -> manager:machine():render().targets[0]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_render_get_targets(const render_manager *r)
{
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef target_table = luabridge::LuaRef::newTable(L);

	int tc = 0;
	for (render_target &curr_rt : r->targets())
	{
		target_table[tc++] = &curr_rt;
	}

	return target_table;
}

// private helper for get_devices - DFS visit all devices in a running machine
luabridge::LuaRef lua_engine::devtree_dfs(device_t *root, luabridge::LuaRef devs_table)
{
	if (root) {
		for (device_t &dev : root->subdevices()) {
			if (dev.configured() && dev.started()) {
				devs_table[dev.tag()] = &dev;
				devtree_dfs(&dev, devs_table);
			}
		}
	}
	return devs_table;
}

//-------------------------------------------------
//  device_get_memspaces - return table of available address spaces userdata
//  -> manager:machine().devices[":maincpu"].spaces["program"]
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_dev_get_memspaces(const device_t *d)
{
	device_t *dev = const_cast<device_t *>(d);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef sp_table = luabridge::LuaRef::newTable(L);

	if(!&dev->memory())
		return sp_table;

	for (address_spacenum sp = AS_0; sp < ADDRESS_SPACES; ++sp) {
		if (dev->memory().has_space(sp)) {
			sp_table[dev->memory().space(sp).name()] = &(dev->memory().space(sp));
		}
	}

	return sp_table;
}

//-------------------------------------------------
//  device_get_state - return table of available state userdata
//  -> manager:machine().devices[":maincpu"].state
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_dev_get_states(const device_t *d)
{
	device_t *dev = const_cast<device_t *>(d);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef st_table = luabridge::LuaRef::newTable(L);

	if(!&dev->state())
		return st_table;

	for (device_state_entry &s : dev->state().state_entries())
	{
		// XXX: refrain from exporting non-visible entries?
		st_table[s.symbol()] = &s;
	}

	return st_table;
}

//-------------------------------------------------
//  device_get_item - return table of indexed items owned by this device
//  -> manager:machine().devices[":maincpu"].items
//-------------------------------------------------

luabridge::LuaRef lua_engine::l_dev_get_items(const device_t *d)
{
	device_t *dev = const_cast<device_t *>(d);
	lua_State *L = luaThis->m_lua_state;
	luabridge::LuaRef table = luabridge::LuaRef::newTable(L);
	std::string tag = dev->tag();

	// 10000 is enough?
	for(int i = 0; i < 10000; i++)
	{
		std::string name;
		const char *item;
		unsigned int size, count;
		void *base;
		item = dev->machine().save().indexed_item(i, base, size, count);
		if(!item)
			break;
		name = &(strchr(item, '/')[1]);
		if(name.substr(0, name.find("/")) == tag)
		{
			name = name.substr(name.find("/") + 1, std::string::npos);
			table[name] = i;
		}
	}
	return table;
}

lua_engine::lua_item::lua_item(int index)
{
	std::string name;
	const char *item;
	item = luaThis->machine().save().indexed_item(index, l_item_base, l_item_size, l_item_count);
	if(!item)
	{
		l_item_base = nullptr;
		l_item_size = 0;
		l_item_count= 0;
	}
}

int lua_engine::lua_item::l_item_read(lua_State *L)
{
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "offset (integer) expected");
	int offset = lua_tounsigned(L, 2);
	int ret = 0;
	if(!l_item_base || (offset > l_item_count))
	{
		lua_pushnil(L);
		return 1;
	}
	switch(l_item_size)
	{
		case 1:
		default:
			ret = ((UINT8 *)l_item_base)[offset];
			break;
		case 2:
			ret = ((UINT16 *)l_item_base)[offset];
			break;
		case 4:
			ret = ((UINT32 *)l_item_base)[offset];
			break;
		case 8:
			ret = ((UINT64 *)l_item_base)[offset];
			break;
	}
	lua_pushunsigned(L, ret);
	return 1;
}

int lua_engine::lua_item::l_item_write(lua_State *L)
{
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "offset (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "value (integer) expected");
	int offset = lua_tounsigned(L, 2);
	UINT64 value = lua_tounsigned(L, 3);
	if(!l_item_base || (offset > l_item_count))
		return 1;
	switch(l_item_size)
	{
		case 1:
		default:
			((UINT8 *)l_item_base)[offset] = (UINT8)value;
			break;
		case 2:
			((UINT16 *)l_item_base)[offset] = (UINT16)value;
			break;
		case 4:
			((UINT32 *)l_item_base)[offset] = (UINT32)value;
			break;
		case 8:
			((UINT64 *)l_item_base)[offset] = (UINT64)value;
			break;
	}
	return 1;
}

int lua_engine::lua_item::l_item_read_block(lua_State *L)
{
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "offset (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "length (integer) expected");
	int offset = lua_tounsigned(L, 2);
	int len = lua_tonumber(L, 3);
	if(!l_item_base || ((offset + len) > (l_item_size * l_item_count)))
	{
		lua_pushnil(L);
		return 1;
	}
	luaL_Buffer buff;
	char *ptr = luaL_buffinitsize(L, &buff, len);
	memcpy(ptr, l_item_base, len);
	luaL_pushresultsize(&buff, len);
	return 1;
}

//-------------------------------------------------
//  state_get_value - return value of a device state entry
//  -> manager:machine().devices[":maincpu"].state["PC"].value
//-------------------------------------------------

UINT64 lua_engine::l_state_get_value(const device_state_entry *d)
{
	device_state_interface *state = d->parent_state();
	if(state) {
		luaThis->machine().save().dispatch_presave();
		return state->state_int(d->index());
	} else {
		return 0;
	}
}

//-------------------------------------------------
//  state_set_value - set value of a device state entry
//  -> manager:machine().devices[":maincpu"].state["D0"].value = 0x0c00
//-------------------------------------------------

void lua_engine::l_state_set_value(device_state_entry *d, UINT64 val)
{
	device_state_interface *state = d->parent_state();
	if(state) {
		state->set_state_int(d->index(), val);
		luaThis->machine().save().dispatch_presave();
	}
}

//-------------------------------------------------
//  mem_read - templated memory readers for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:read_i8(0xC000)
//-------------------------------------------------

template <typename T>
int lua_engine::lua_addr_space::l_mem_read(lua_State *L)
{
	address_space &sp = luabridge::Stack<address_space &>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "address (integer) expected");
	offs_t address = lua_tounsigned(L, 2);
	T mem_content = 0;
	switch(sizeof(mem_content) * 8) {
		case 8:
			mem_content = sp.read_byte(address);
			break;
		case 16:
			if (WORD_ALIGNED(address)) {
				mem_content = sp.read_word(address);
			} else {
				mem_content = sp.read_word_unaligned(address);
			}
			break;
		case 32:
			if (DWORD_ALIGNED(address)) {
				mem_content = sp.read_dword(address);
			} else {
				mem_content = sp.read_dword_unaligned(address);
			}
			break;
		case 64:
			if (QWORD_ALIGNED(address)) {
				mem_content = sp.read_qword(address);
			} else {
				mem_content = sp.read_qword_unaligned(address);
			}
			break;
		default:
			break;
	}

	if (std::numeric_limits<T>::is_signed) {
		lua_pushinteger(L, mem_content);
	} else {
		lua_pushunsigned(L, mem_content);
	}

	return 1;

}

//-------------------------------------------------
//  mem_write - templated memory writer for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:write_u16(0xC000, 0xF00D)
//-------------------------------------------------

template <typename T>
int lua_engine::lua_addr_space::l_mem_write(lua_State *L)
{
	address_space &sp = luabridge::Stack<address_space &>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "address (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "value (integer) expected");
	offs_t address = lua_tounsigned(L, 2);
	T val = lua_tounsigned(L, 3);

	switch(sizeof(val) * 8) {
		case 8:
			sp.write_byte(address, val);
			break;
		case 16:
			if (WORD_ALIGNED(address)) {
				sp.write_word(address, val);
			} else {
				sp.read_word_unaligned(address, val);
			}
			break;
		case 32:
			if (DWORD_ALIGNED(address)) {
				sp.write_dword(address, val);
			} else {
				sp.write_dword_unaligned(address, val);
			}
			break;
		case 64:
			if (QWORD_ALIGNED(address)) {
				sp.write_qword(address, val);
			} else {
				sp.write_qword_unaligned(address, val);
			}
			break;
		default:
			break;
	}

	return 0;
}

//-------------------------------------------------
//  mem_direct_read - templated direct memory readers for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:read_direct_i8(0xC000)
//-------------------------------------------------

UINT8 lua_engine::read_direct_byte(address_space &space, offs_t addr)
{
	UINT8 *base = (UINT8 *)space.get_read_ptr(addr);
	if(base)
		return base[addr];
	else
		return 0;
}

template <typename T>
int lua_engine::lua_addr_space::l_direct_mem_read(lua_State *L)
{
	address_space &sp = luabridge::Stack<address_space &>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "address (integer) expected");
	offs_t address = lua_tounsigned(L, 2);
	T mem_content = 0;
	for(int i = 0; i < sizeof(T); i++)
	{
		UINT8 byte;
		mem_content <<= 8;
		if(sp.endianness() == ENDIANNESS_BIG)
			byte = read_direct_byte(sp, address + sizeof(T) - i);
		else
			byte = read_direct_byte(sp, address + i);
		mem_content |= byte;
	}

	if (std::numeric_limits<T>::is_signed) {
		lua_pushinteger(L, mem_content);
	} else {
		lua_pushunsigned(L, mem_content);
	}

	return 1;
}

//-------------------------------------------------
//  mem_direct_write - templated memory writer for <sign>,<size>
//  -> manager:machine().devices[":maincpu"].spaces["program"]:write_direct_u16(0xC000, 0xF00D)
//-------------------------------------------------

void lua_engine::write_direct_byte(address_space &space, offs_t addr, UINT8 byte)
{
	UINT8 *base = (UINT8 *)space.get_read_ptr(addr);
	if(base)
		base[addr] = byte;
}

template <typename T>
int lua_engine::lua_addr_space::l_direct_mem_write(lua_State *L)
{
	address_space &sp = luabridge::Stack<address_space &>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "address (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "value (integer) expected");
	offs_t address = lua_tounsigned(L, 2);
	T val = lua_tounsigned(L, 3);
	for(int i = 0; i < sizeof(T); i++)
	{
		if(sp.endianness() == ENDIANNESS_BIG)
			write_direct_byte(sp, address + sizeof(T) - i, val & 0xff);
		else
			write_direct_byte(sp, address + i, val & 0xff);
		val >>= 8;
	}

	return 0;
}

//-------------------------------------------------
//  region_read - templated region readers for <sign>,<size>
//  -> manager:machine():memory().region[":maincpu"]:read_i8(0xC000)
//-------------------------------------------------

UINT8 lua_engine::read_region_byte(memory_region &region, offs_t addr)
{
	if(addr >= region.bytes())
		return 0;

	return region.u8(addr);
}

template <typename T>
int lua_engine::lua_memory_region::l_region_read(lua_State *L)
{
	memory_region &region = luabridge::Stack<memory_region &>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "address (integer) expected");
	offs_t address = lua_tounsigned(L, 2);
	T mem_content = 0;
	for(int i = 0; i < sizeof(T); i++)
	{
		UINT8 byte;
		mem_content <<= 8;
		if(region.endianness() == ENDIANNESS_BIG)
			byte = read_region_byte(region, address + sizeof(T) - i);
		else
			byte = read_region_byte(region, address + i);
		mem_content |= byte;
	}

	if (std::numeric_limits<T>::is_signed) {
		lua_pushinteger(L, mem_content);
	} else {
		lua_pushunsigned(L, mem_content);
	}

	return 1;
}

//-------------------------------------------------
//  region_write - templated region writer for <sign>,<size>
//  -> manager:machine():memory().region[":maincpu"]:write_u16(0xC000, 0xF00D)
//-------------------------------------------------

void lua_engine::write_region_byte(memory_region &region, offs_t addr, UINT8 byte)
{
	if(addr >= region.bytes())
		return;

	region.base()[addr] = byte;
}

template <typename T>
int lua_engine::lua_memory_region::l_region_write(lua_State *L)
{
	memory_region &region = luabridge::Stack<memory_region &>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "address (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "value (integer) expected");
	offs_t address = lua_tounsigned(L, 2);
	T val = lua_tounsigned(L, 3);
	for(int i = 0; i < sizeof(T); i++)
	{
		if(region.endianness() == ENDIANNESS_BIG)
			write_region_byte(region, address + sizeof(T) - i, val & 0xff);
		else
			write_region_byte(region, address + i, val & 0xff);
		val >>= 8;
	}

	return 0;
}

int lua_engine::lua_options_entry::l_entry_value(lua_State *L)
{
	core_options::entry *e = luabridge::Stack<core_options::entry *>::get(L, 1);
	if(!e) {
		return 0;
	}

	luaL_argcheck(L, !lua_isfunction(L, 2), 2, "optional argument: unsupported value");

	if (!lua_isnone(L, 2))
	{
		std::string error;
		// FIXME: not working with ui_options::entry
		// TODO: optional arg for priority
		luaThis->machine().options().set_value(e->name(),
				lua_isboolean(L, 2) ? (lua_toboolean(L, 2) ? "1" : "0") : lua_tostring(L, 2),
				OPTION_PRIORITY_CMDLINE, error);

		if (!error.empty())
		{
			luaL_error(L, "%s", error.c_str());
		}
	}

	switch (e->type())
	{
		case OPTION_BOOLEAN:
			lua_pushboolean(L, (atoi(e->value()) != 0));
			break;
		case OPTION_INTEGER:
			lua_pushnumber(L, atoi(e->value()));
			break;
		case OPTION_FLOAT:
			lua_pushnumber(L, atof(e->value()));
			break;
		default:
			lua_pushstring(L, e->value());
			break;
	}

	return 1;
}

//-------------------------------------------------
//  begin_recording - start avi
//  -> manager:machine():video():begin_recording()
//-------------------------------------------------

int lua_engine::lua_video::l_begin_recording(lua_State *L)
{
	video_manager *vm = luabridge::Stack<video_manager *>::get(L, 1);
	if (!vm) {
		return 0;
	}

	luaL_argcheck(L, lua_isstring(L, 2) || lua_isnone(L, 2), 2, "optional argument: filename, string expected");

	const char *filename = lua_tostring(L, 2);
	if (!lua_isnone(L, 2)) {
		std::string vidname(filename);
		strreplace(vidname, "/", PATH_SEPARATOR);
		strreplace(vidname, "%g", luaThis->machine().basename());
		filename = vidname.c_str();
	} else {
		filename = nullptr;
	}
	vm->begin_recording(filename, video_manager::MF_AVI);

	return 1;
}

//-------------------------------------------------
//  end_recording - start saving avi
//  -> manager:machine():video():end_recording()
//-------------------------------------------------

int lua_engine::lua_video::l_end_recording(lua_State *L)
{
	video_manager *vm = luabridge::Stack<video_manager *>::get(L, 1);
	if (!vm) {
		return 0;
	}

	if (!vm->is_recording()) {
		lua_writestringerror("%s", "No active recording to stop");
		return 0;
	}

	vm->end_recording(video_manager::MF_AVI);
	return 1;
}

//-------------------------------------------------
//  screen_height - return screen visible height
//  -> manager:machine().screens[":screen"]:height()
//-------------------------------------------------

int lua_engine::lua_screen::l_height(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	lua_pushunsigned(L, sc->visible_area().height());
	return 1;
}

//-------------------------------------------------
//  screen_width - return screen visible width
//  -> manager:machine().screens[":screen"]:width()
//-------------------------------------------------

int lua_engine::lua_screen::l_width(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	lua_pushunsigned(L, sc->visible_area().width());
	return 1;
}


//-------------------------------------------------
//  screen_orientation - return screen orientation
//  -> manager:machine().screens[":screen"]:orientation()
//     -> rotation_angle (0, 90, 180, 270)
//     -> flipx (true, false)
//     -> flipy (true, false)
//-------------------------------------------------

int lua_engine::lua_screen::l_orientation(lua_State *L)
{
	UINT32 flags = (luaThis->machine().system().flags & ORIENTATION_MASK);

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

	lua_createtable(L, 2, 2);
	lua_pushliteral(L, "rotation_angle");
	lua_pushinteger(L, rotation_angle);

	lua_settable(L, -3);
	lua_pushliteral(L, "flipx");
	lua_pushboolean(L, (flags & ORIENTATION_FLIP_X));

	lua_settable(L, -3);
	lua_pushliteral(L, "flipy");
	lua_pushboolean(L, (flags & ORIENTATION_FLIP_Y));
	lua_settable(L, -3);
	return 1;
}

//-------------------------------------------------
//  screen_refresh - return screen refresh rate
//  -> manager:machine().screens[":screen"]:refresh()
//-------------------------------------------------

int lua_engine::lua_screen::l_refresh(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	lua_pushnumber(L, ATTOSECONDS_TO_HZ(sc->refresh_attoseconds()));
	return 1;
}

//-------------------------------------------------
//  screen_snapshot - save png bitmap of screen to snapshots folder
//  -> manager:machine().screens[":screen"]:snapshot("filename.png")
//-------------------------------------------------

int lua_engine::lua_screen::l_snapshot(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc || !sc->machine().render().is_live(*sc))
	{
		return 0;
	}

	luaL_argcheck(L, lua_isstring(L, 2) || lua_isnone(L, 2), 2, "optional argument: filename, string expected");

	emu_file file(sc->machine().options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	osd_file::error filerr;

	if (!lua_isnone(L, 2)) {
		const char *filename = lua_tostring(L, 2);
		std::string snapstr(filename);
		strreplace(snapstr, "/", PATH_SEPARATOR);
		strreplace(snapstr, "%g", sc->machine().basename());
		filerr = file.open(snapstr.c_str());
	}
	else
	{
		filerr = sc->machine().video().open_next(file, "png");
	}

	if (filerr != osd_file::error::NONE)
	{
		luaL_error(L, "osd_file::error=%d", filerr);
		return 0;
	}

	sc->machine().video().save_snapshot(sc, file);
	lua_writestringerror("saved %s", file.fullpath());
	file.close();
	return 1;
}

//-------------------------------------------------
//  screen_type - return human readable screen type
//  -> manager:machine().screens[":screen"]:type()
//-------------------------------------------------

int lua_engine::lua_screen::l_type(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	switch (sc->screen_type())
	{
		case SCREEN_TYPE_RASTER:  lua_pushliteral(L, "raster"); break;
		case SCREEN_TYPE_VECTOR:  lua_pushliteral(L, "vector"); break;
		case SCREEN_TYPE_LCD:     lua_pushliteral(L, "lcd"); break;
		case SCREEN_TYPE_SVG:     lua_pushliteral(L, "svg"); break;
		default:                  lua_pushliteral(L, "unknown"); break;
	}

	return 1;
}

//-------------------------------------------------
//  draw_box - draw a box on a screen container
//  -> manager:machine().screens[":screen"]:draw_box(x1, y1, x2, y2, bgcolor, linecolor)
//-------------------------------------------------

int lua_engine::lua_screen::l_draw_box(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	// ensure that we got 6 numerical parameters
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "x1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "y1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "x2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "y2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "background color (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 7), 7, "outline color (integer) expected");

	// retrieve all parameters
	int sc_width = sc->visible_area().width();
	int sc_height = sc->visible_area().height();
	float x1, y1, x2, y2;
	x1 = MIN(MAX(0, (float) lua_tonumber(L, 2)), sc_width-1) / static_cast<float>(sc_width);
	y1 = MIN(MAX(0, (float) lua_tonumber(L, 3)), sc_height-1) / static_cast<float>(sc_height);
	x2 = MIN(MAX(0, (float) lua_tonumber(L, 4)), sc_width-1) / static_cast<float>(sc_width);
	y2 = MIN(MAX(0, (float) lua_tonumber(L, 5)), sc_height-1) / static_cast<float>(sc_height);
	UINT32 bgcolor = lua_tounsigned(L, 6);
	UINT32 fgcolor = lua_tounsigned(L, 7);

	// draw the box
	render_container &rc = sc->container();
	ui_manager &ui = sc->machine().ui();
	ui.draw_outlined_box(&rc, x1, y1, x2, y2, fgcolor, bgcolor);

	return 0;
}

//-------------------------------------------------
//  draw_line - draw a line on a screen container
//  -> manager:machine().screens[":screen"]:draw_line(x1, y1, x2, y2, linecolor)
//-------------------------------------------------

int lua_engine::lua_screen::l_draw_line(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	// ensure that we got 5 numerical parameters
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "x1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "y1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "x2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "y2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "color (integer) expected");

	// retrieve all parameters
	int sc_width = sc->visible_area().width();
	int sc_height = sc->visible_area().height();
	float x1, y1, x2, y2;
	x1 = MIN(MAX(0, (float) lua_tonumber(L, 2)), sc_width-1) / static_cast<float>(sc_width);
	y1 = MIN(MAX(0, (float) lua_tonumber(L, 3)), sc_height-1) / static_cast<float>(sc_height);
	x2 = MIN(MAX(0, (float) lua_tonumber(L, 4)), sc_width-1) / static_cast<float>(sc_width);
	y2 = MIN(MAX(0, (float) lua_tonumber(L, 5)), sc_height-1) / static_cast<float>(sc_height);
	UINT32 color = lua_tounsigned(L, 6);

	// draw the line
	sc->container().add_line(x1, y1, x2, y2, UI_LINE_WIDTH, rgb_t(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	return 0;
}

//-------------------------------------------------
//  draw_text - draw text on a screen container
//	if x is a position, then y is a pixel position, otherwise x and y are screen size relative
//  -> manager:machine().screens[":screen"]:draw_text(x, y, message)
//-------------------------------------------------

int lua_engine::lua_screen::l_draw_text(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	// ensure that we got proper parameters
	luaL_argcheck(L, lua_isnumber(L, 2) || lua_isstring(L, 2), 2, "x (integer or string) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "y (integer) expected");
	luaL_argcheck(L, lua_isstring(L, 4), 4, "message (string) expected");
	luaL_argcheck(L, lua_isinteger(L, 5) || lua_isnone(L, 5), 5, "optional argument: text color, integer expected (default: 0xffffffff)");

	// retrieve all parameters
	int sc_width = sc->visible_area().width();
	int sc_height = sc->visible_area().height();
	int justify = JUSTIFY_LEFT;
	float y, x = 0;
	if(lua_isnumber(L, 2))
	{
		x = MIN(MAX(0, (float) lua_tonumber(L, 2)), sc_width-1) / static_cast<float>(sc_width);
		y = MIN(MAX(0, (float) lua_tonumber(L, 3)), sc_height-1) / static_cast<float>(sc_height);
	}
	else
	{
		std::string just_str = lua_tostring(L, 2);
		if(just_str == "right")
			justify = JUSTIFY_RIGHT;
		else if(just_str == "center")
			justify = JUSTIFY_CENTER;
		y = lua_tonumber(L, 3);
	}
	const char *msg = luaL_checkstring(L,4);
	rgb_t textcolor = UI_TEXT_COLOR;
	if (!lua_isnone(L, 5)) {
		textcolor = rgb_t(lua_tounsigned(L, 5));
	}

	// draw the text
	render_container &rc = sc->container();
	ui_manager &ui = sc->machine().ui();
	ui.draw_text_full(&rc, msg, x, y, (1.0f - x),
						justify, WRAP_WORD, DRAW_NORMAL, textcolor,
						UI_TEXT_BG_COLOR, nullptr, nullptr);

	return 0;
}

int lua_engine::lua_emu_file::l_emu_file_read(lua_State *L)
{
	emu_file *file = luabridge::Stack<emu_file *>::get(L, 1);
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "length (integer) expected");
	int ret, len = lua_tonumber(L, 2);
	luaL_Buffer buff;
	char *ptr = luaL_buffinitsize(L, &buff, len);
	ret = file->read(ptr, len);
	luaL_pushresultsize(&buff, ret);
	return 1;
}

void *lua_engine::checkparam(lua_State *L, int idx, const char *tname)
{
	const char *name;

	if(!lua_getmetatable(L, idx))
	return nullptr;

	lua_rawget(L, LUA_REGISTRYINDEX);
	name = lua_tostring(L, -1);
	if(!name || strcmp(name, tname)) {
	lua_pop(L, 1);
	return nullptr;
	}
	lua_pop(L, 1);

	return *static_cast<void **>(lua_touserdata(L, idx));
}

void *lua_engine::getparam(lua_State *L, int idx, const char *tname)
{
	void *p = checkparam(L, idx, tname);
	char msg[256];
	sprintf(msg, "%s expected", tname);
	luaL_argcheck(L, p, idx, msg);
	return p;
}

void lua_engine::push(lua_State *L, void *p, const char *tname)
{
	void **pp = static_cast<void **>(lua_newuserdata(L, sizeof(void *)));
	*pp = p;
	luaL_getmetatable(L, tname);
	lua_setmetatable(L, -2);
}

int lua_engine::l_emu_exit(lua_State *L)
{
	luaThis->machine().schedule_exit();
	return 1;
}

int lua_engine::l_emu_start(lua_State *L)
{
	const char *system_name = luaL_checkstring(L,1);

	int index = driver_list::find(system_name);
	if (index != -1) {
		machine_manager::instance()->schedule_new_driver(driver_list::driver(index));
		luaThis->machine().schedule_hard_reset();
	}
	return 1;
}

int lua_engine::luaopen_ioport(lua_State *L)
{
	static const struct luaL_Reg ioport_funcs [] = {
		{ "write",       l_ioport_write },
		{ nullptr, nullptr }  /* sentinel */
	};

	luaL_newmetatable(L, tname_ioport);
	lua_pushvalue(L, -1);
	lua_pushstring(L, tname_ioport);
	lua_rawset(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);
	luaL_setfuncs(L, ioport_funcs, 0);
	return 1;
}

struct msg {
	std::string text;
	int ready;
	std::string response;
	int status;
	int done;
} msg;

static std::mutex g_mutex;

void lua_engine::serve_lua()
{
	osd_sleep(osd_ticks_per_second() / 1000 * 50);
	printf("%s v%s\n%s\n%s\n\n", emulator_info::get_appname(),build_version,emulator_info::get_copyright_info(),LUA_COPYRIGHT);
	fflush(stdout);
	char buff[LUA_MAXINPUT];
	std::string oldbuff;

	const char *b = LUA_PROMPT;

	do {
		// Wait for input
		fputs(b, stdout); fflush(stdout);  /* show prompt */
		fgets(buff, LUA_MAXINPUT, stdin);

		// Create message
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			if (msg.ready == 0) {
				msg.text = oldbuff;
				if (oldbuff.length() != 0) msg.text.append("\n");
				msg.text.append(buff);
				msg.ready = 1;
				msg.done = 0;
			}
		}

		// Wait for response
		int done;
		do {
			osd_sleep(osd_ticks_per_second() / 1000);
			std::lock_guard<std::mutex> lock(g_mutex);
			done = msg.done;
		} while (done==0);

		// Do action on client side
		{
			std::lock_guard<std::mutex> lock(g_mutex);

			if (msg.status == -1) {
				b = LUA_PROMPT2;
				oldbuff = msg.response;
			}
			else {
				b = LUA_PROMPT;
				oldbuff = "";
			}
			msg.done = 0;
		}

	} while (1);
}

static void *serve_lua(void *param)
{
	lua_engine *engine = (lua_engine *)param;
	engine->serve_lua();
	return NULL;
}

//-------------------------------------------------
//  lua_engine - constructor
//-------------------------------------------------

lua_engine::lua_engine()
{
	m_machine = nullptr;
	luaThis = this;
	m_lua_state = luaL_newstate();  /* create state */
	output_notifier_set = false;

	luaL_checkversion(m_lua_state);
	lua_gc(m_lua_state, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(m_lua_state);  /* open libraries */

		// Get package.preload so we can store builtins in it.
	lua_getglobal(m_lua_state, "package");
	lua_getfield(m_lua_state, -1, "preload");
	lua_remove(m_lua_state, -2); // Remove package

	lua_pushcfunction(m_lua_state, luaopen_zlib);
	lua_setfield(m_lua_state, -2, "zlib");

	lua_pushcfunction(m_lua_state, luaopen_lsqlite3);
	lua_setfield(m_lua_state, -2, "lsqlite3");

	lua_pushcfunction(m_lua_state, luaopen_lfs);
	lua_setfield(m_lua_state, -2, "lfs");

	luaopen_ioport(m_lua_state);

	lua_gc(m_lua_state, LUA_GCRESTART, 0);
	msg.ready = 0;
	msg.status = 0;
	msg.done = 0;
}

//-------------------------------------------------
//  ~lua_engine - destructor
//-------------------------------------------------

lua_engine::~lua_engine()
{
	close();
}

std::vector<lua_engine::menu_item> &lua_engine::menu_populate(std::string &menu)
{
	std::vector<menu_item> &menu_list = *global_alloc(std::vector<menu_item>);
	std::string field = "menu_pop_" + menu;
	lua_settop(m_lua_state, 0);
	lua_getfield(m_lua_state, LUA_REGISTRYINDEX, field.c_str());

	if(!lua_isfunction(m_lua_state, -1))
	{
		lua_pop(m_lua_state, 1);
		return menu_list;
	}
	lua_pcall(m_lua_state, 0, 1, 0);
	if(!lua_istable(m_lua_state, -1))
	{
		lua_pop(m_lua_state, 1);
		return menu_list;
	}

	lua_pushnil(m_lua_state);
	while(lua_next(m_lua_state, -2))
	{
		if(lua_istable(m_lua_state, -1))
		{
			menu_item item;
			lua_rawgeti(m_lua_state, -1, 1);
			item.text = lua_tostring(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			lua_rawgeti(m_lua_state, -1, 2);
			item.subtext = lua_tostring(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			lua_rawgeti(m_lua_state, -1, 3);
			item.flags = lua_tointeger(m_lua_state, -1);
			lua_pop(m_lua_state, 1);
			menu_list.push_back(item);
		}
		lua_pop(m_lua_state, 1);
	}
	lua_pop(m_lua_state, 1);
	return menu_list;
}

bool lua_engine::menu_callback(std::string &menu, int index, std::string event)
{
	std::string field = "menu_cb_" + menu;
	bool ret = false;
	lua_settop(m_lua_state, 0);
	lua_getfield(m_lua_state, LUA_REGISTRYINDEX, field.c_str());

	if(lua_isfunction(m_lua_state, -1))
	{
		lua_pushinteger(m_lua_state, index);
		lua_pushstring(m_lua_state, event.c_str());
		if(int error = lua_pcall(m_lua_state, 2, 1, 0))
		{
			if(error == 2)
				printf("%s\n", lua_tostring(m_lua_state, -1));
			lua_pop(m_lua_state, 1);
			return false;
		}
		ret = lua_toboolean(m_lua_state, -1);
		lua_pop(m_lua_state, 1);
	}
	return ret;
}

int lua_engine::l_emu_register_menu(lua_State *L)
{
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "callback function expected");
	luaL_argcheck(L, lua_isfunction(L, 2), 2, "callback function expected");
	luaL_argcheck(L, lua_isstring(L, 3), 3, "message (string) expected");
	std::string name = luaL_checkstring(L, 3);
	std::string cbfield = "menu_cb_" + name;
	std::string popfield = "menu_pop_" + name;
	luaThis->m_menu.push_back(std::string(name));
	lua_pushvalue(L, 1);
	lua_setfield(L, LUA_REGISTRYINDEX, cbfield.c_str());
	lua_pushvalue(L, 2);
	lua_setfield(L, LUA_REGISTRYINDEX, popfield.c_str());
	return 1;
}

void lua_engine::execute_function(const char *id)
{
	lua_settop(m_lua_state, 0);
	lua_getfield(m_lua_state, LUA_REGISTRYINDEX, id);

	if (lua_istable(m_lua_state, -1))
	{
		lua_pushnil(m_lua_state);
		while (lua_next(m_lua_state, -2) != 0)
		{
			if (lua_isfunction(m_lua_state, -1))
			{
				if(int error = lua_pcall(m_lua_state, 0, 0, 0))
				{
					if(error == 2)
						printf("%s\n", lua_tostring(m_lua_state, -1));
					lua_pop(m_lua_state, 1);
				}
			}
			else
			{
				lua_pop(m_lua_state, 1);
			}
		}
	}
}

int lua_engine::register_function(lua_State *L, const char *id)
{
	if (!lua_isnil(L, 1))
		luaL_checktype(L, 1, LUA_TFUNCTION);
	lua_settop(L, 1);
	lua_getfield(L, LUA_REGISTRYINDEX, id);
	if (lua_isnil(L, -1))
	{
		lua_newtable(L);
	}
	luaL_checktype(L, -1, LUA_TTABLE);
	int len = lua_rawlen(L, -1);
	lua_pushnumber(L, len + 1);
	lua_pushvalue(L, 1);
	lua_rawset(L, -3);      /* Stores the pair in the table */

	lua_pushvalue(L, -1);
	lua_setfield(L, LUA_REGISTRYINDEX, id);
	return 1;
}

int lua_engine::l_emu_register_prestart(lua_State *L)
{
	return register_function(L, "LUA_ON_PRESTART");
}

int lua_engine::l_emu_register_start(lua_State *L)
{
	return register_function(L, "LUA_ON_START");
}

int lua_engine::l_emu_register_stop(lua_State *L)
{
	return register_function(L, "LUA_ON_STOP");
}

int lua_engine::l_emu_register_pause(lua_State *L)
{
	return register_function(L, "LUA_ON_PAUSE");
}

int lua_engine::l_emu_register_resume(lua_State *L)
{
	return register_function(L, "LUA_ON_RESUME");
}

int lua_engine::l_emu_register_frame(lua_State *L)
{
	return register_function(L, "LUA_ON_FRAME");
}

int lua_engine::l_emu_register_frame_done(lua_State *L)
{
	return register_function(L, "LUA_ON_FRAME_DONE");
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

void lua_engine::update_machine()
{
	lua_newtable(m_lua_state);
	if (m_machine!=nullptr)
	{
		// Create the ioport array
		for (ioport_port &port : machine().ioport().ports())
		{
			for (ioport_field &field : port.fields())
			{
				if (field.name())
				{
					push(m_lua_state, &field, tname_ioport);
					lua_setfield(m_lua_state, -2, field.name());
				}
			}
		}
	}
	lua_setglobal(m_lua_state, "ioport");
}

void lua_engine::attach_notifiers()
{
	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(lua_engine::on_machine_prestart), this), true);
	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(lua_engine::on_machine_start), this));
	machine().add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(lua_engine::on_machine_stop), this));
	machine().add_notifier(MACHINE_NOTIFY_PAUSE, machine_notify_delegate(FUNC(lua_engine::on_machine_pause), this));
	machine().add_notifier(MACHINE_NOTIFY_RESUME, machine_notify_delegate(FUNC(lua_engine::on_machine_resume), this));
	machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(FUNC(lua_engine::on_machine_frame), this));
}

int lua_engine::lua_machine::l_popmessage(lua_State *L)
{
	running_machine *m = luabridge::Stack<running_machine *>::get(L, 1);
	luaL_argcheck(L, lua_isstring(L, 2), 2, "message (string) expected");
	m->popmessage("%s", luaL_checkstring(L, 2));
	return 0;
}

int lua_engine::lua_machine::l_logerror(lua_State *L)
{
	running_machine *m = luabridge::Stack<running_machine *>::get(L, 1);
	luaL_argcheck(L, lua_isstring(L, 2), 2, "message (string) expected");
	m->logerror("[luaengine] %s\n", luaL_checkstring(L, 2));
	return 0;
}

//-------------------------------------------------
//  initialize - initialize lua hookup to emu engine
//-------------------------------------------------

void lua_engine::initialize()
{
	luabridge::getGlobalNamespace (m_lua_state)
		.beginNamespace ("emu")
			.addCFunction ("app_name",    l_emu_app_name )
			.addCFunction ("app_version", l_emu_app_version )
			.addCFunction ("gamename",    l_emu_gamename )
			.addCFunction ("romname",     l_emu_romname )
			.addCFunction ("softname",    l_emu_softname )
			.addCFunction ("keypost",     l_emu_keypost )
			.addCFunction ("hook_output", l_emu_hook_output )
			.addCFunction ("sethook",     l_emu_set_hook )
			.addCFunction ("time",        l_emu_time )
			.addCFunction ("wait",        l_emu_wait )
			.addCFunction ("after",       l_emu_after )
			.addCFunction ("exit",        l_emu_exit )
			.addCFunction ("start",       l_emu_start )
			.addCFunction ("pause",       l_emu_pause )
			.addCFunction ("unpause",     l_emu_unpause )
			.addCFunction ("register_prestart", l_emu_register_prestart )
			.addCFunction ("register_start", l_emu_register_start )
			.addCFunction ("register_stop",  l_emu_register_stop )
			.addCFunction ("register_pause", l_emu_register_pause )
			.addCFunction ("register_resume",l_emu_register_resume )
			.addCFunction ("register_frame", l_emu_register_frame )
			.addCFunction ("register_frame_done", l_emu_register_frame_done )
			.addCFunction ("register_menu",  l_emu_register_menu )
			.beginClass <machine_manager> ("manager")
				.addFunction ("machine", &machine_manager::machine)
				.addFunction ("options", &machine_manager::options)
				.addFunction ("plugins", &machine_manager::plugins)
			.endClass ()
			.beginClass <lua_machine> ("lua_machine")
				.addCFunction ("popmessage", &lua_machine::l_popmessage)
				.addCFunction ("logerror", &lua_machine::l_logerror)
			.endClass ()
			.deriveClass <running_machine, lua_machine> ("machine")
				.addFunction ("exit", &running_machine::schedule_exit)
				.addFunction ("hard_reset", &running_machine::schedule_hard_reset)
				.addFunction ("soft_reset", &running_machine::schedule_soft_reset)
				.addFunction ("save", &running_machine::schedule_save)
				.addFunction ("load", &running_machine::schedule_load)
				.addFunction ("system", &running_machine::system)
				.addFunction ("video", &running_machine::video)
				.addFunction ("ui", &running_machine::ui)
				.addFunction ("render", &running_machine::render)
				.addFunction ("ioport", &running_machine::ioport)
				.addFunction ("parameters", &running_machine::parameters)
				.addFunction ("cheat", &running_machine::cheat)
				.addFunction ("memory", &running_machine::memory)
				.addFunction ("options", &running_machine::options)
				.addProperty <luabridge::LuaRef, void> ("devices", &lua_engine::l_machine_get_devices)
				.addProperty <luabridge::LuaRef, void> ("screens", &lua_engine::l_machine_get_screens)
				.addProperty <luabridge::LuaRef, void> ("images", &lua_engine::l_machine_get_images)
			.endClass ()
			.beginClass <game_driver> ("game_driver")
				.addData ("source_file", &game_driver::source_file)
				.addData ("parent", &game_driver::parent)
				.addData ("name", &game_driver::name)
				.addData ("description", &game_driver::description)
				.addData ("year", &game_driver::year)
				.addData ("manufacturer", &game_driver::manufacturer)
				.addData ("compatible_with", &game_driver::compatible_with)
				.addData ("default_layout", &game_driver::default_layout)
			.endClass ()
			.beginClass <device_t> ("device")
				.addFunction ("name", &device_t::name)
				.addFunction ("shortname", &device_t::shortname)
				.addFunction ("tag", &device_t::tag)
				.addFunction ("owner", &device_t::owner)
				.addProperty <luabridge::LuaRef, void> ("spaces", &lua_engine::l_dev_get_memspaces)
				.addProperty <luabridge::LuaRef, void> ("state", &lua_engine::l_dev_get_states)
				.addProperty <luabridge::LuaRef, void> ("items", &lua_engine::l_dev_get_items)
			.endClass()
			.beginClass <cheat_manager> ("cheat")
				.addProperty <bool, bool> ("enabled", &cheat_manager::enabled, &cheat_manager::set_enable)
				.addFunction ("reload", &cheat_manager::reload)
				.addFunction ("save_all", &cheat_manager::save_all)
				.addProperty <luabridge::LuaRef, void> ("entries", &lua_engine::l_cheat_get_entries)
			.endClass()
			.beginClass <lua_cheat_entry> ("lua_cheat_entry")
				.addCFunction ("state", &lua_cheat_entry::l_get_state)
			.endClass()
			.deriveClass <cheat_entry, lua_cheat_entry> ("cheat_entry")
				.addFunction ("description", &cheat_entry::description)
				.addFunction ("comment", &cheat_entry::comment)
				.addFunction ("has_run_script", &cheat_entry::has_run_script)
				.addFunction ("has_on_script", &cheat_entry::has_on_script)
				.addFunction ("has_off_script", &cheat_entry::has_off_script)
				.addFunction ("has_change_script", &cheat_entry::has_change_script)
				.addFunction ("execute_off_script", &cheat_entry::execute_off_script)
				.addFunction ("execute_on_script", &cheat_entry::execute_on_script)
				.addFunction ("execute_run_script", &cheat_entry::execute_run_script)
				.addFunction ("execute_change_script", &cheat_entry::execute_change_script)
				.addFunction ("is_text_only", &cheat_entry::is_text_only)
				.addFunction ("is_oneshot", &cheat_entry::is_oneshot)
				.addFunction ("is_onoff", &cheat_entry::is_onoff)
				.addFunction ("is_value_parameter", &cheat_entry::is_value_parameter)
				.addFunction ("is_itemlist_parameter", &cheat_entry::is_itemlist_parameter)
				.addFunction ("is_oneshot_parameter", &cheat_entry::is_oneshot_parameter)
				.addFunction ("activate", &cheat_entry::activate)
				.addFunction ("select_default_state", &cheat_entry::select_default_state)
				.addFunction ("select_previous_state", &cheat_entry::select_previous_state)
				.addFunction ("select_next_state", &cheat_entry::select_next_state)
			.endClass()
			.beginClass <ioport_manager> ("ioport")
				.addFunction ("has_configs", &ioport_manager::has_configs)
				.addFunction ("has_analog", &ioport_manager::has_analog)
				.addFunction ("has_dips", &ioport_manager::has_dips)
				.addFunction ("has_bioses", &ioport_manager::has_bioses)
				.addFunction ("has_keyboard", &ioport_manager::has_keyboard)
				.addFunction ("count_players", &ioport_manager::count_players)
				.addProperty <luabridge::LuaRef, void> ("ports", &lua_engine::l_ioport_get_ports)
			.endClass()
			.beginClass <ioport_port> ("ioport_port")
				.addFunction ("tag", &ioport_port::tag)
				.addFunction ("active", &ioport_port::active)
				.addFunction ("live", &ioport_port::live)
				.addProperty <luabridge::LuaRef, void> ("fields", &lua_engine::l_ioports_port_get_fields)
			.endClass()
			.beginClass <ioport_field> ("ioport_field")
				.addFunction ("set_value", &ioport_field::set_value)
				.addProperty ("device", &ioport_field::device)
				.addProperty ("name", &ioport_field::name)
				.addProperty <UINT8, UINT8> ("player", &ioport_field::player, &ioport_field::set_player)
				.addProperty ("mask", &ioport_field::mask)
				.addProperty ("defvalue", &ioport_field::defvalue)
				.addProperty ("sensitivity", &ioport_field::sensitivity)
				.addProperty ("way", &ioport_field::way)
				.addProperty ("is_analog", &ioport_field::is_analog)
				.addProperty ("is_digitial_joystick", &ioport_field::is_digital_joystick)
				.addProperty ("enabled", &ioport_field::enabled)
				.addProperty ("unused", &ioport_field::unused)
				.addProperty ("cocktail", &ioport_field::cocktail)
				.addProperty ("toggle", &ioport_field::toggle)
				.addProperty ("rotated", &ioport_field::rotated)
				.addProperty ("analog_reverse", &ioport_field::analog_reverse)
				.addProperty ("analog_reset", &ioport_field::analog_reset)
				.addProperty ("analog_wraps", &ioport_field::analog_wraps)
				.addProperty ("analog_invert", &ioport_field::analog_invert)
				.addProperty ("impulse", &ioport_field::impulse)
				.addProperty <double, double> ("crosshair_scale", &ioport_field::crosshair_scale, &ioport_field::set_crosshair_scale)
				.addProperty <double, double> ("crosshair_offset", &ioport_field::crosshair_offset, &ioport_field::set_crosshair_offset)
			.endClass()
			.beginClass <core_options> ("core_options")
				.addFunction ("help", &core_options::output_help)
				.addFunction ("command", &core_options::command)
				.addProperty <luabridge::LuaRef, void> ("entries", &lua_engine::l_options_get_entries)
			.endClass()
			.beginClass <lua_options_entry> ("lua_options_entry")
				.addCFunction ("value", &lua_options_entry::l_entry_value)
			.endClass()
			.deriveClass <core_options::entry, lua_options_entry> ("core_options_entry")
				.addFunction ("description", &core_options::entry::description)
				.addFunction ("default_value", &core_options::entry::default_value)
				.addFunction ("minimum", &core_options::entry::minimum)
				.addFunction ("maximum", &core_options::entry::maximum)
				.addFunction ("has_range", &core_options::entry::has_range)
			.endClass()
			.deriveClass <emu_options, core_options> ("emu_options")
			.endClass()
			.deriveClass <ui_options, core_options> ("ui_options")
			.endClass()
			.deriveClass <plugin_options, core_options> ("plugin_options")
			.endClass()
			.beginClass <parameters_manager> ("parameters")
				.addFunction ("add", &parameters_manager::add)
				.addFunction ("lookup", &parameters_manager::lookup)
			.endClass()
			.beginClass <lua_video> ("lua_video_manager")
				.addCFunction ("begin_recording", &lua_video::l_begin_recording)
				.addCFunction ("end_recording", &lua_video::l_end_recording)
			.endClass()
			.deriveClass <video_manager, lua_video> ("video")
				.addFunction ("snapshot", &video_manager::save_active_screen_snapshots)
				.addFunction ("is_recording", &video_manager::is_recording)
				.addFunction ("skip_this_frame", &video_manager::skip_this_frame)
				.addFunction ("speed_factor", &video_manager::speed_factor)
				.addFunction ("speed_percent", &video_manager::speed_percent)
				.addProperty <int, int> ("frameskip", &video_manager::frameskip, &video_manager::set_frameskip)
				.addProperty <bool, bool> ("throttled", &video_manager::throttled, &video_manager::set_throttled)
				.addProperty <float, float> ("throttle_rate", &video_manager::throttle_rate, &video_manager::set_throttle_rate)
			.endClass()
			.beginClass <lua_addr_space> ("lua_addr_space")
				.addCFunction ("read_i8", &lua_addr_space::l_mem_read<INT8>)
				.addCFunction ("read_u8", &lua_addr_space::l_mem_read<UINT8>)
				.addCFunction ("read_i16", &lua_addr_space::l_mem_read<INT16>)
				.addCFunction ("read_u16", &lua_addr_space::l_mem_read<UINT16>)
				.addCFunction ("read_i32", &lua_addr_space::l_mem_read<INT32>)
				.addCFunction ("read_u32", &lua_addr_space::l_mem_read<UINT32>)
				.addCFunction ("read_i64", &lua_addr_space::l_mem_read<INT64>)
				.addCFunction ("read_u64", &lua_addr_space::l_mem_read<UINT64>)
				.addCFunction ("write_i8", &lua_addr_space::l_mem_write<INT8>)
				.addCFunction ("write_u8", &lua_addr_space::l_mem_write<UINT8>)
				.addCFunction ("write_i16", &lua_addr_space::l_mem_write<INT16>)
				.addCFunction ("write_u16", &lua_addr_space::l_mem_write<UINT16>)
				.addCFunction ("write_i32", &lua_addr_space::l_mem_write<INT32>)
				.addCFunction ("write_u32", &lua_addr_space::l_mem_write<UINT32>)
				.addCFunction ("write_i64", &lua_addr_space::l_mem_write<INT64>)
				.addCFunction ("write_u64", &lua_addr_space::l_mem_write<UINT64>)
				.addCFunction ("read_direct_i8", &lua_addr_space::l_direct_mem_read<INT8>)
				.addCFunction ("read_direct_u8", &lua_addr_space::l_direct_mem_read<UINT8>)
				.addCFunction ("read_direct_i16", &lua_addr_space::l_direct_mem_read<INT16>)
				.addCFunction ("read_direct_u16", &lua_addr_space::l_direct_mem_read<UINT16>)
				.addCFunction ("read_direct_i32", &lua_addr_space::l_direct_mem_read<INT32>)
				.addCFunction ("read_direct_u32", &lua_addr_space::l_direct_mem_read<UINT32>)
				.addCFunction ("read_direct_i64", &lua_addr_space::l_direct_mem_read<INT64>)
				.addCFunction ("read_direct_u64", &lua_addr_space::l_direct_mem_read<UINT64>)
				.addCFunction ("write_direct_i8", &lua_addr_space::l_direct_mem_write<INT8>)
				.addCFunction ("write_direct_u8", &lua_addr_space::l_direct_mem_write<UINT8>)
				.addCFunction ("write_direct_i16", &lua_addr_space::l_direct_mem_write<INT16>)
				.addCFunction ("write_direct_u16", &lua_addr_space::l_direct_mem_write<UINT16>)
				.addCFunction ("write_direct_i32", &lua_addr_space::l_direct_mem_write<INT32>)
				.addCFunction ("write_direct_u32", &lua_addr_space::l_direct_mem_write<UINT32>)
				.addCFunction ("write_direct_i64", &lua_addr_space::l_direct_mem_write<INT64>)
				.addCFunction ("write_direct_u64", &lua_addr_space::l_direct_mem_write<UINT64>)
			.endClass()
			.deriveClass <address_space, lua_addr_space> ("addr_space")
				.addFunction("name", &address_space::name)
			.endClass()
			.beginClass <render_target> ("target")
				.addFunction ("width", &render_target::width)
				.addFunction ("height", &render_target::height)
				.addFunction ("pixel_aspect", &render_target::pixel_aspect)
				.addFunction ("hidden", &render_target::hidden)
				.addFunction ("is_ui_target", &render_target::is_ui_target)
				.addFunction ("index", &render_target::index)
				.addProperty <float, float> ("max_update_rate", &render_target::max_update_rate, &render_target::set_max_update_rate)
				.addProperty <int, int> ("view", &render_target::view, &render_target::set_view)
				.addProperty <int, int> ("orientation", &render_target::orientation, &render_target::set_orientation)
				.addProperty <bool, bool> ("backdrops", &render_target::backdrops_enabled, &render_target::set_backdrops_enabled)
				.addProperty <bool, bool> ("overlays", &render_target::overlays_enabled, &render_target::set_overlays_enabled)
				.addProperty <bool, bool> ("bezels", &render_target::bezels_enabled, &render_target::set_bezels_enabled)
				.addProperty <bool, bool> ("marquees", &render_target::marquees_enabled, &render_target::set_marquees_enabled)
				.addProperty <bool, bool> ("screen_overlay", &render_target::screen_overlay_enabled, &render_target::set_screen_overlay_enabled)
				.addProperty <bool, bool> ("zoom", &render_target::zoom_to_screen, &render_target::set_zoom_to_screen)
			.endClass()
			.beginClass <render_container> ("render_container")
				.addFunction ("orientation", &render_container::orientation)
				.addFunction ("xscale", &render_container::xscale)
				.addFunction ("yscale", &render_container::yscale)
				.addFunction ("xoffset", &render_container::xoffset)
				.addFunction ("yoffset", &render_container::yoffset)
				.addFunction ("is_empty", &render_container::is_empty)
			.endClass()
			.beginClass <render_manager> ("render")
				.addFunction ("max_update_rate", &render_manager::max_update_rate)
				.addFunction ("ui_target", &render_manager::ui_target)
				.addFunction ("ui_container", &render_manager::ui_container)
				.addProperty <luabridge::LuaRef, void> ("targets", &lua_engine::l_render_get_targets)
			.endClass()
			.beginClass <ui_manager> ("ui")
				.addFunction ("is_menu_active", &ui_manager::is_menu_active)
				.addFunction ("options", &ui_manager::options)
				.addProperty <bool, bool> ("show_fps", &ui_manager::show_fps, &ui_manager::set_show_fps)
				.addProperty <bool, bool> ("show_profiler", &ui_manager::show_profiler, &ui_manager::set_show_profiler)
				.addProperty <bool, bool> ("single_step", &ui_manager::single_step, &ui_manager::set_single_step)
				.addFunction ("get_line_height", &ui_manager::get_line_height)
			.endClass()
			.beginClass <lua_screen> ("lua_screen_dev")
				.addCFunction ("draw_box",  &lua_screen::l_draw_box)
				.addCFunction ("draw_line", &lua_screen::l_draw_line)
				.addCFunction ("draw_text", &lua_screen::l_draw_text)
				.addCFunction ("height", &lua_screen::l_height)
				.addCFunction ("width", &lua_screen::l_width)
				.addCFunction ("orientation", &lua_screen::l_orientation)
				.addCFunction ("refresh", &lua_screen::l_refresh)
				.addCFunction ("snapshot", &lua_screen::l_snapshot)
				.addCFunction ("type", &lua_screen::l_type)
			.endClass()
			.deriveClass <screen_device, lua_screen> ("screen_dev")
				.addFunction ("frame_number", &screen_device::frame_number)
				.addFunction ("name", &screen_device::name)
				.addFunction ("shortname", &screen_device::shortname)
				.addFunction ("tag", &screen_device::tag)
				.addFunction ("xscale", &screen_device::xscale)
				.addFunction ("yscale", &screen_device::yscale)
			.endClass()
			.beginClass <device_state_entry> ("dev_space")
				.addFunction ("name", &device_state_entry::symbol)
				.addProperty <UINT64, UINT64> ("value", &lua_engine::l_state_get_value, &lua_engine::l_state_set_value)
				.addFunction ("is_visible", &device_state_entry::visible)
				.addFunction ("is_divider", &device_state_entry::divider)
			.endClass()
			.beginClass <memory_manager> ("memory")
				.addProperty <luabridge::LuaRef, void> ("banks", &lua_engine::l_memory_get_banks)
				.addProperty <luabridge::LuaRef, void> ("regions", &lua_engine::l_memory_get_regions)
			.endClass()
			.beginClass <lua_memory_region> ("lua_region")
				.addCFunction ("read_i8", &lua_memory_region::l_region_read<INT8>)
				.addCFunction ("read_u8", &lua_memory_region::l_region_read<UINT8>)
				.addCFunction ("read_i16", &lua_memory_region::l_region_read<INT16>)
				.addCFunction ("read_u16", &lua_memory_region::l_region_read<UINT16>)
				.addCFunction ("read_i32", &lua_memory_region::l_region_read<INT32>)
				.addCFunction ("read_u32", &lua_memory_region::l_region_read<UINT32>)
				.addCFunction ("read_i64", &lua_memory_region::l_region_read<INT64>)
				.addCFunction ("read_u64", &lua_memory_region::l_region_read<UINT64>)
				.addCFunction ("write_i8", &lua_memory_region::l_region_write<INT8>)
				.addCFunction ("write_u8", &lua_memory_region::l_region_write<UINT8>)
				.addCFunction ("write_i16", &lua_memory_region::l_region_write<INT16>)
				.addCFunction ("write_u16", &lua_memory_region::l_region_write<UINT16>)
				.addCFunction ("write_i32", &lua_memory_region::l_region_write<INT32>)
				.addCFunction ("write_u32", &lua_memory_region::l_region_write<UINT32>)
				.addCFunction ("write_i64", &lua_memory_region::l_region_write<INT64>)
				.addCFunction ("write_u64", &lua_memory_region::l_region_write<UINT64>)
			.endClass()
			.deriveClass <memory_region, lua_memory_region> ("region")
				.addProperty <UINT32> ("size", &memory_region::bytes)
			.endClass()
			.beginClass <device_image_interface> ("image")
				.addFunction ("exists", &device_image_interface::exists)
				.addFunction ("filename", &device_image_interface::filename)
				.addFunction ("longname", &device_image_interface::longname)
				.addFunction ("manufacturer", &device_image_interface::manufacturer)
				.addFunction ("year", &device_image_interface::year)
				.addFunction ("software_list_name", &device_image_interface::software_list_name)
				.addFunction ("image_type_name", &device_image_interface::image_type_name)
				.addFunction ("load", &device_image_interface::load)
				.addFunction ("unload", &device_image_interface::unload)
				.addFunction ("crc", &device_image_interface::crc)
				.addProperty <const device_t &> ("device", static_cast<const device_t &(device_image_interface::*)() const>(&device_image_interface::device))
				.addProperty <bool> ("is_readable", &device_image_interface::is_readable)
				.addProperty <bool> ("is_writeable", &device_image_interface::is_writeable)
				.addProperty <bool> ("is_creatable", &device_image_interface::is_creatable)
				.addProperty <bool> ("is_reset_on_load", &device_image_interface::is_reset_on_load)
			.endClass()
			.beginClass <lua_emu_file> ("lua_file")
				.addCFunction ("read", &lua_emu_file::l_emu_file_read)
			.endClass()
			.deriveClass <emu_file, lua_emu_file> ("file")
				.addConstructor <void (*)(const char *, UINT32)> ()
				.addFunction ("open", static_cast<osd_file::error (emu_file::*)(const char *)>(&emu_file::open))
				.addFunction ("seek", &emu_file::seek)
				.addFunction ("size", &emu_file::size)
			.endClass()
			.beginClass <lua_item> ("item")
				.addConstructor <void (*)(int)> ()
				.addData ("size", &lua_item::l_item_size, false)
				.addData ("count", &lua_item::l_item_count, false)
				.addCFunction ("read", &lua_item::l_item_read)
				.addCFunction ("read_block", &lua_item::l_item_read_block)
				.addCFunction ("write", &lua_item::l_item_write)
			.endClass()
		.endNamespace();

	luabridge::push (m_lua_state, machine_manager::instance());
	lua_setglobal(m_lua_state, "manager");
}

void lua_engine::start_console()
{
	std::thread th(::serve_lua, this);
	th.detach();
}

//-------------------------------------------------
//  frame_hook - called at each frame refresh, used to draw a HUD
//-------------------------------------------------
bool lua_engine::frame_hook()
{
	bool is_cb_hooked = false;
	if (m_machine != nullptr) {
		// invoke registered callback (if any)
		is_cb_hooked = hook_frame_cb.active();
		if (is_cb_hooked) {
			lua_State *L = hook_frame_cb.precall();
			hook_frame_cb.call(this, L, 0);
		}
	}
	return is_cb_hooked;
}

void lua_engine::periodic_check()
{
	std::lock_guard<std::mutex> lock(g_mutex);
	if (msg.ready == 1) {
		lua_settop(m_lua_state, 0);
		int status = luaL_loadbuffer(m_lua_state, msg.text.c_str(), msg.text.length(), "=stdin");
		if (incomplete(status)==0)  /* cannot try to add lines? */
		{
			if (status == LUA_OK) status = docall(0, LUA_MULTRET);
			report(status);
			if (status == LUA_OK && lua_gettop(m_lua_state) > 0)   /* any result to print? */
			{
				luaL_checkstack(m_lua_state, LUA_MINSTACK, "too many results to print");
				lua_getglobal(m_lua_state, "print");
				lua_insert(m_lua_state, 1);
				if (lua_pcall(m_lua_state, lua_gettop(m_lua_state) - 1, 0, 0) != LUA_OK)
					lua_writestringerror("%s\n", lua_pushfstring(m_lua_state,
					"error calling " LUA_QL("print") " (%s)",
					lua_tostring(m_lua_state, -1)));
			}
		}
		else
		{
			status = -1;
		}
		msg.status = status;
		msg.response = msg.text;
		msg.text = "";
		msg.ready = 0;
		msg.done = 1;
	}
}

//-------------------------------------------------
//  close - close and cleanup of lua engine
//-------------------------------------------------

void lua_engine::close()
{
	lua_settop(m_lua_state, 0);  /* clear stack */
	lua_close(m_lua_state);
}

//-------------------------------------------------
//  execute - load and execute script
//-------------------------------------------------

void lua_engine::load_script(const char *filename)
{
	int s = luaL_loadfile(m_lua_state, filename);
	report(s);
	update_machine();
	start();
}

//-------------------------------------------------
//  execute_string - execute script from string
//-------------------------------------------------

void lua_engine::load_string(const char *value)
{
	int s = luaL_loadstring(m_lua_state, value);
	report(s);
	update_machine();
	start();
}

//-------------------------------------------------
//  start - execute the loaded script
//-------------------------------------------------

void lua_engine::start()
{
	resume(m_lua_state);
}


//**************************************************************************
//  LuaBridge Stack specializations
//**************************************************************************

namespace luabridge {
	template <>
	struct Stack <unsigned long long> {
		static inline void push (lua_State* L, unsigned long long value) {
			lua_pushunsigned(L, static_cast <lua_Unsigned> (value));
		}

		static inline unsigned long long get (lua_State* L, int index) {
			return static_cast <unsigned long long> (luaL_checkunsigned (L, index));
		}
	};
}
