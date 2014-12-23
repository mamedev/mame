// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    luaengine.c

    Controls execution of the core MAME system.

***************************************************************************/

#include "lua/lua.hpp"
#include "lua/lib/lualibs.h"
#include "lua/bridge/LuaBridge.h"
#include <signal.h>
#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "drivenum.h"
#include "ui/ui.h"
#include "web/mongoose.h"

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

static lua_State *globalL = NULL;

#define luai_writestring(s,l)   fwrite((s), sizeof(char), (l), stdout)
#define luai_writeline()    (luai_writestring("\n", 1), fflush(stdout))

const char *const lua_engine::tname_ioport = "lua.ioport";
lua_engine* lua_engine::luaThis = NULL;


static void lstop(lua_State *L, lua_Debug *ar)
{
	(void)ar;  /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
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
		if (msg == NULL) msg = "(error object is not a string)";
		luai_writestringerror("%s\n", msg);
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
	L = NULL;
	cb = -1;
}

void lua_engine::hook::set(lua_State *_L, int idx)
{
	if (L)
		luaL_unref(L, LUA_REGISTRYINDEX, cb);

	if (lua_isnil(_L, idx)) {
		L = NULL;
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
	int s = lua_resume(L, NULL, nparam);
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
//  gui_screen_width - return current screen width in pixels
//-------------------------------------------------

int lua_engine::l_gui_screen_width(lua_State *L)
{
	lua_pushunsigned(L, luaThis->m_screen.width());
	return 1;
}

//-------------------------------------------------
//  gui_screen_height - return current screen height in pixels
//-------------------------------------------------

int lua_engine::l_gui_screen_height(lua_State *L)
{
	lua_pushunsigned(L, luaThis->m_screen.height());
	return 1;
}

//-------------------------------------------------
//  gui_draw_box - draw a box to HUD
//  -> gui.draw_box(x1, y1, x2, y2, fillcolor, outlinecolor)
//-------------------------------------------------

int lua_engine::l_gui_draw_box(lua_State *L)
{
	// ensure that we got 6 numerical parameters
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "x1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "y1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "x2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "y2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "fill color (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 6), 6, "outline color (integer) expected");

	// retrieve all parameters
	float x1, y1, x2, y2;
	x1 = MIN(lua_tounsigned(L, 1) / static_cast<float>(luaThis->m_screen.width()) , 1.0f);
	y1 = MIN(lua_tounsigned(L, 2) / static_cast<float>(luaThis->m_screen.height()), 1.0f);
	x2 = MIN(lua_tounsigned(L, 3) / static_cast<float>(luaThis->m_screen.width()) , 1.0f);
	y2 = MIN(lua_tounsigned(L, 4) / static_cast<float>(luaThis->m_screen.height()), 1.0f);
	UINT32 bgcolor = lua_tounsigned(L, 5);
	UINT32 fgcolor = lua_tounsigned(L, 6);

	// draw the box
	render_container &rc = luaThis->machine().first_screen()->container();
	ui_manager &ui = luaThis->machine().ui();
	ui.draw_outlined_box(&rc, x1, y1, x2, y2, fgcolor, bgcolor);

	return 0;
}

//-------------------------------------------------
//  gui_draw_line - draw a line to HUD
//  -> gui.draw_line(x1, y1, x2, y2, color)
//-------------------------------------------------

int lua_engine::l_gui_draw_line(lua_State *L)
{
	// ensure that we got 5 numerical parameters
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "x1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "y1 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 3), 3, "x2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 4), 4, "y2 (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 5), 5, "color (integer) expected");

	// retrieve all parameters
	float x1, y1, x2, y2;
	x1 = MIN(lua_tounsigned(L, 1) / static_cast<float>(luaThis->m_screen.width()) , 1.0f);
	y1 = MIN(lua_tounsigned(L, 2) / static_cast<float>(luaThis->m_screen.height()), 1.0f);
	x2 = MIN(lua_tounsigned(L, 3) / static_cast<float>(luaThis->m_screen.width()) , 1.0f);
	y2 = MIN(lua_tounsigned(L, 4) / static_cast<float>(luaThis->m_screen.height()), 1.0f);
	UINT32 color = lua_tounsigned(L, 5);

	// draw the line
	render_container &rc = luaThis->machine().first_screen()->container();
	rc.add_line(x1, y1, x2, y2, UI_LINE_WIDTH, rgb_t(color), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	return 0;
}

//-------------------------------------------------
//  gui_draw_text - draw text to HUD
//  -> gui.draw_text(int x, int y, string msg[, color="white", outline="black"])
//-------------------------------------------------

int lua_engine::l_gui_draw_text(lua_State *L)
{
	// ensure that we got proper parameters
	luaL_argcheck(L, lua_isnumber(L, 1), 1, "x (integer) expected");
	luaL_argcheck(L, lua_isnumber(L, 2), 2, "y (integer) expected");
	luaL_argcheck(L, lua_isstring(L, 3), 3, "message (string) expected");

	// retrieve all parameters
	float x = MIN(lua_tounsigned(L, 1) / static_cast<float>(luaThis->m_screen.width()) , 1.0f);
	float y = MIN(lua_tounsigned(L, 2) / static_cast<float>(luaThis->m_screen.height()), 1.0f);
	const char *msg = luaL_checkstring(L,3);
	// TODO: optional parameters

	// draw the text
	render_container &rc = luaThis->machine().first_screen()->container();
	ui_manager &ui = luaThis->machine().ui();
	ui.draw_text_full(&rc, msg, x, y , (1.0f - x),
						JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR,
						UI_TEXT_BG_COLOR, NULL, NULL);

	return 0;
}

//-------------------------------------------------
//  gui_show_fps - display/hide fps counter
//-------------------------------------------------

int lua_engine::l_gui_show_fps(lua_State *L)
{
	luaL_argcheck(L, lua_isboolean(L, 1), 1, "enabled (bool) expected");
	bool enabled = lua_toboolean(L, 1);
	render_container &rc = luaThis->machine().first_screen()->container();
	ui_manager &ui = luaThis->machine().ui();
	ui.set_show_fps(enabled);
	lua_pushboolean(L, ui.show_fps_counter());
	return 1;
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
	return lua_yieldk(L, 0, 0, 0);
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
		output_set_notifier(NULL, s_output_notifier, this);
		output_notifier_set = true;
	}
}

int lua_engine::l_emu_hook_output(lua_State *L)
{
	luaThis->emu_hook_output(L);
	return 0;
}

//-------------------------------------------------
//  emu_hook_frame - register callback to draw a HUD from LUA on top of each frame
//  -> emu.hook_frame(function cb)
//-------------------------------------------------

int lua_engine::l_emu_hook_frame(lua_State *L)
{
	luaThis->emu_hook_frame(L);
	return 0;
}

void lua_engine::emu_hook_frame(lua_State *L)
{
	luaL_argcheck(L, lua_isfunction(L, 1), 1, "callback function expected");
	frame_hook_cb.set(L, 1);
}

void *lua_engine::checkparam(lua_State *L, int idx, const char *tname)
{
	const char *name;

	if(!lua_getmetatable(L, idx))
	return 0;

	lua_rawget(L, LUA_REGISTRYINDEX);
	name = lua_tostring(L, -1);
	if(!name || strcmp(name, tname)) {
	lua_pop(L, 1);
	return 0;
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
		{ NULL, NULL }  /* sentinel */
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
	astring text;
	int ready;
	astring response;
	int status;
	int done;
} msg;

osd_lock *lock;

void lua_engine::serve_lua()
{
	osd_sleep(osd_ticks_per_second() / 1000 * 50);
	printf("%s v%s - %s\n%s\n%s\n\n", emulator_info::get_applongname(),build_version,emulator_info::get_fulllongname(),emulator_info::get_copyright_info(),LUA_COPYRIGHT);
	fflush(stdout);
	char buff[LUA_MAXINPUT];
	astring oldbuff;

	const char *b = LUA_PROMPT;

	do {
		// Wait for input
		fputs(b, stdout); fflush(stdout);  /* show prompt */
		fgets(buff, LUA_MAXINPUT, stdin);

		// Create message
		osd_lock_acquire(lock);
		if (msg.ready == 0) {
			msg.text = oldbuff;
			if (oldbuff.len()!=0) msg.text.cat("\n");
			msg.text.cat(buff);
			msg.ready = 1;
			msg.done = 0;
		}
		osd_lock_release(lock);

		// Wait for response
		int done = 0;
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

static void *serve_lua(void *param)
{
	lua_engine *engine = (lua_engine *)param;
	engine->serve_lua();
	return NULL;
}


//-------------------------------------------------
//  lua_engine - constructor
//-------------------------------------------------

lua_engine::lua_engine() :  m_screen()
{
	m_machine = NULL;
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
	if (m_machine!=NULL)
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
		// Register a screen-tracking bitmap
		m_machine->first_screen()->register_screen_bitmap(m_screen);
	}
	lua_setglobal(m_lua_state, "ioport");
}

//-------------------------------------------------
//  initialize - initialize lua hookup to emu engine
//-------------------------------------------------

void lua_engine::initialize()
{
	// "emu" namespace
	luabridge::getGlobalNamespace (m_lua_state)
		.beginNamespace ("emu")
			.addCFunction ("app_name",    l_emu_app_name )
			.addCFunction ("app_version", l_emu_app_version )
			.addCFunction ("gamename",    l_emu_gamename )
			.addCFunction ("romname",     l_emu_romname )
			.addCFunction ("keypost",     l_emu_keypost )
			.addCFunction ("hook_output", l_emu_hook_output )
			.addCFunction ("hook_frame",  l_emu_hook_frame )
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
				.addFunction ("system", &running_machine::system)
			.endClass ()
			.beginClass <game_driver> ("game_driver")
				.addData ("name", &game_driver::name)
				.addData ("description", &game_driver::description)
				.addData ("year", &game_driver::year)
				.addData ("manufacturer", &game_driver::manufacturer)
			.endClass ()
		.endNamespace ();

	// "gui" namespace
	luabridge::getGlobalNamespace (m_lua_state)
		.beginNamespace ("gui")
			.addCFunction ("screen_width",    l_gui_screen_width )
			.addCFunction ("screen_height",   l_gui_screen_height )
			.addCFunction ("draw_box",        l_gui_draw_box )
			.addCFunction ("draw_line",       l_gui_draw_line )
			.addCFunction ("draw_text",       l_gui_draw_text )
			.addCFunction ("show_fps",        l_gui_show_fps )
		.endNamespace ();

	luabridge::push (m_lua_state, machine_manager::instance());
	lua_setglobal(m_lua_state, "manager");
}

void lua_engine::start_console()
{
	mg_start_thread(::serve_lua, this);
}

//-------------------------------------------------
//  frame_hook - called at each frame refresh, used to draw a HUD
//-------------------------------------------------
bool lua_engine::frame_hook()
{
	bool is_cb_hooked = false;
	if (m_machine != NULL)
	{
		// invoke registered callback (if any)
		is_cb_hooked = frame_hook_cb.active();
		if (is_cb_hooked)
		{
			lua_State *L = frame_hook_cb.precall();
			frame_hook_cb.call(this, L, 0);
		}
	}
	return is_cb_hooked;
}

void lua_engine::periodic_check()
{
	osd_lock_acquire(lock);
	if (msg.ready == 1) {
	lua_settop(m_lua_state, 0);
	int status = luaL_loadbuffer(m_lua_state, msg.text.cstr(), strlen(msg.text.cstr()), "=stdin");
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
				luai_writestringerror("%s\n", lua_pushfstring(m_lua_state,
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
