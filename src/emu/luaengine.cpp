// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic,Luca Bruno
/***************************************************************************

    luaengine.c

    Controls execution of the core MAME system.

***************************************************************************/

#include <limits>
#include "lua.hpp"
#include "luabridge/Source/LuaBridge/LuaBridge.h"
#include <signal.h>
#include "emu.h"
#include "drivenum.h"
#include "ui/ui.h"
#include "luaengine.h"

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

#if (defined(__sun__) && defined(__svr4__)) || defined(__ANDROID__) || defined(__OpenBSD__)
#undef _L
#endif

void lua_engine::hook::set(lua_State *_L, int idx)
{
	if (L)
		luaL_unref(L, LUA_REGISTRYINDEX, cb);

	if (lua_isnil(_L, idx)) {
		L = nullptr;
		cb = -1;

	} else {
		L = _L;
		lua_pushvalue(_L, idx);
		cb = luaL_ref(_L, LUA_REGISTRYINDEX);
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

void lua_engine::resume(void *_L, INT32 param)
{
	resume(static_cast<lua_State *>(_L));
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
		output_set_notifier(nullptr, s_output_notifier, this);
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
			output_set_notifier(nullptr, s_output_notifier, this);
			output_notifier_set = true;
		}
	} else if (strcmp(hookname, "frame") == 0) {
		hook_frame_cb.set(L, 1);
	} else {
		lua_writestringerror("%s", "Unknown hook name, aborting.\n");
	}
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

// private helper for get_devices - DFS visit all devices in a running machine
luabridge::LuaRef lua_engine::devtree_dfs(device_t *root, luabridge::LuaRef devs_table)
{
	if (root) {
		for (device_t *dev = root->first_subdevice(); dev != nullptr; dev = dev->next()) {
			if (dev && dev->configured() && dev->started()) {
				devs_table[dev->tag()] = dev;
				devtree_dfs(dev, devs_table);
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
	for (const device_state_entry *s = dev->state().state_first(); s != nullptr; s = s->next()) {
		// XXX: refrain from exporting non-visible entries?
		if (s) {
			st_table[s->symbol()] = const_cast<device_state_entry *>(s);
		}
	}

	return st_table;
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
			if ((address & 1) == 0) {
				mem_content = sp.read_word(address);
			} else {
				mem_content = sp.read_word_unaligned(address);
			}
			break;
		case 32:
			if ((address & 3) == 0) {
				mem_content = sp.read_dword(address);
			} else {
				mem_content = sp.read_dword_unaligned(address);
			}
			break;
		case 64:
			if ((address & 7) == 0) {
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
			if ((address & 1) == 0) {
				sp.write_word(address, val);
			} else {
				sp.read_word_unaligned(address, val);
			}
			break;
		case 32:
			if ((address & 3) == 0) {
				sp.write_dword(address, val);
			} else {
				sp.write_dword_unaligned(address, val);
			}
			break;
		case 64:
			if ((address & 7) == 0) {
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
//  -> manager:machine().screens[":screen"]:draw_text(x, y, message)
//-------------------------------------------------

int lua_engine::lua_screen::l_draw_text(lua_State *L)
{
	screen_device *sc = luabridge::Stack<screen_device *>::get(L, 1);
	if(!sc) {
		return 0;
	}

	// ensure that we got proper parameters
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "x (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "y (integer) expected");
	luaL_argcheck(L, lua_isstring(L, 4), 4, "message (string) expected");
	luaL_argcheck(L, lua_isinteger(L, 5) || lua_isnone(L, 5), 5, "optional argument: text color, integer expected (default: 0xffffffff)");

	// retrieve all parameters
	int sc_width = sc->visible_area().width();
	int sc_height = sc->visible_area().height();
	float x = MIN(MAX(0, (float) lua_tonumber(L, 2)), sc_width-1) / static_cast<float>(sc_width);
	float y = MIN(MAX(0, (float) lua_tonumber(L, 3)), sc_height-1) / static_cast<float>(sc_height);
	const char *msg = luaL_checkstring(L,4);
	rgb_t textcolor = UI_TEXT_COLOR;
	if (!lua_isnone(L, 5)) {
		textcolor = rgb_t(lua_tounsigned(L, 5));
	}

	// draw the text
	render_container &rc = sc->container();
	ui_manager &ui = sc->machine().ui();
	ui.draw_text_full(&rc, msg, x, y , (1.0f - x),
						JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, textcolor,
						UI_TEXT_BG_COLOR, nullptr, nullptr);

	return 0;
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

osd_lock *lock;

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
		osd_lock_acquire(lock);
		if (msg.ready == 0) {
			msg.text = oldbuff;
			if (oldbuff.length()!=0) msg.text.append("\n");
			msg.text.append(buff);
			msg.ready = 1;
			msg.done = 0;
		}
		osd_lock_release(lock);

		// Wait for response
		int done;
		do {
			osd_sleep(osd_ticks_per_second() / 1000);
			osd_lock_acquire(lock);
			done = msg.done;
			osd_lock_release(lock);
		} while (done==0);

		// Do action on client side
		osd_lock_acquire(lock);
		if (msg.status == -1){
			b = LUA_PROMPT2;
			oldbuff = msg.response;
		}
		else {
			b = LUA_PROMPT;
			oldbuff = "";
		}
		msg.done = 0;
		osd_lock_release(lock);

	} while (1);
}

/*
static void *serve_lua(void *param)
{
    lua_engine *engine = (lua_engine *)param;
    engine->serve_lua();
    return NULL;
}
*/

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

	luaopen_lsqlite3(m_lua_state);

	luaopen_ioport(m_lua_state);

	lua_gc(m_lua_state, LUA_GCRESTART, 0);
	msg.ready = 0;
	msg.status = 0;
	msg.done = 0;
	lock = osd_lock_alloc();
}

//-------------------------------------------------
//  ~lua_engine - destructor
//-------------------------------------------------

lua_engine::~lua_engine()
{
	close();
}


void lua_engine::update_machine()
{
	lua_newtable(m_lua_state);
	if (m_machine!=nullptr)
	{
		// Create the ioport array
		ioport_port *port = machine().ioport().first_port();
		while(port) {
			ioport_field *field = port->first_field();
			while(field) {
				if(field->name()) {
					push(m_lua_state, field, tname_ioport);
					lua_setfield(m_lua_state, -2, field->name());
				}
				field = field->next();
			}
			port = port->next();
		}
	}
	lua_setglobal(m_lua_state, "ioport");
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
			.beginClass <machine_manager> ("manager")
				.addFunction ("machine", &machine_manager::machine)
				.addFunction ("options", &machine_manager::options)
			.endClass ()
			.beginClass <running_machine> ("machine")
				.addFunction ("exit", &running_machine::schedule_exit)
				.addFunction ("hard_reset", &running_machine::schedule_hard_reset)
				.addFunction ("soft_reset", &running_machine::schedule_soft_reset)
				.addFunction ("save", &running_machine::schedule_save)
				.addFunction ("load", &running_machine::schedule_load)
				.addFunction ("system", &running_machine::system)
				.addProperty <luabridge::LuaRef, void> ("devices", &lua_engine::l_machine_get_devices)
				.addProperty <luabridge::LuaRef, void> ("screens", &lua_engine::l_machine_get_screens)
			.endClass ()
			.beginClass <game_driver> ("game_driver")
				.addData ("name", &game_driver::name)
				.addData ("description", &game_driver::description)
				.addData ("year", &game_driver::year)
				.addData ("manufacturer", &game_driver::manufacturer)
			.endClass ()
			.beginClass <device_t> ("device")
				.addFunction ("name", &device_t::name)
				.addFunction ("shortname", &device_t::shortname)
				.addFunction ("tag", &device_t::tag)
				.addProperty <luabridge::LuaRef, void> ("spaces", &lua_engine::l_dev_get_memspaces)
				.addProperty <luabridge::LuaRef, void> ("state", &lua_engine::l_dev_get_states)
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
			.endClass()
			.deriveClass <address_space, lua_addr_space> ("addr_space")
				.addFunction("name", &address_space::name)
			.endClass()
			.beginClass <lua_screen> ("lua_screen_dev")
				.addCFunction ("draw_box",  &lua_screen::l_draw_box)
				.addCFunction ("draw_line", &lua_screen::l_draw_line)
				.addCFunction ("draw_text", &lua_screen::l_draw_text)
				.addCFunction ("height", &lua_screen::l_height)
				.addCFunction ("width", &lua_screen::l_width)
			.endClass()
			.deriveClass <screen_device, lua_screen> ("screen_dev")
				.addFunction ("frame_number", &screen_device::frame_number)
				.addFunction ("name", &screen_device::name)
				.addFunction ("shortname", &screen_device::shortname)
				.addFunction ("tag", &screen_device::tag)
			.endClass()
			.beginClass <device_state_entry> ("dev_space")
				.addFunction ("name", &device_state_entry::symbol)
				.addProperty <UINT64, UINT64> ("value", &lua_engine::l_state_get_value, &lua_engine::l_state_set_value)
				.addFunction ("is_visible", &device_state_entry::visible)
				.addFunction ("is_divider", &device_state_entry::divider)
			.endClass()
		.endNamespace();

	luabridge::push (m_lua_state, machine_manager::instance());
	lua_setglobal(m_lua_state, "manager");
}

void lua_engine::start_console()
{
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
	osd_lock_acquire(lock);
	if (msg.ready == 1) {
	lua_settop(m_lua_state, 0);
	int status = luaL_loadbuffer(m_lua_state, msg.text.c_str(), strlen(msg.text.c_str()), "=stdin");
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
	osd_lock_release(lock);
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
	struct Stack <UINT64> {
		static inline void push (lua_State* L, UINT64 value) {
			lua_pushunsigned(L, static_cast <lua_Unsigned> (value));
		}

		static inline UINT64 get (lua_State* L, int index) {
			return static_cast <UINT64> (luaL_checkunsigned (L, index));
		}
	};
}
