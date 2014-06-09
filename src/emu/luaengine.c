// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    luaengine.c

    Controls execution of the core MAME system.

***************************************************************************/

#include <signal.h>
#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "drivenum.h"
#include "lua/lua.hpp"
#include "lua/lib/lualibs.h"
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

int emu_gamename(lua_State *L)
{
	lua_pushstring(L, machine_manager::instance()->machine()->system().description);
	return 1;
}

//-------------------------------------------------
//  emu_keypost - post keys to natural keyboard
//-------------------------------------------------

int emu_keypost(lua_State *L)
{
	const char *keys = luaL_checkstring(L,1);
	machine_manager::instance()->machine()->ioport().natkeyboard().post_utf8(keys);
	return 1;
}

int emu_exit(lua_State *L)
{
	machine_manager::instance()->machine()->schedule_exit();
	return 1;
}

int emu_start(lua_State *L)
{
	const char *system_name = luaL_checkstring(L,1);
	
	int index = driver_list::find(system_name);
	if (index != -1) {
		machine_manager::instance()->schedule_new_driver(driver_list::driver(index));	
		machine_manager::instance()->machine()->schedule_hard_reset();
	}
	return 1;
}

static const struct luaL_Reg emu_funcs [] =
{
	{ "gamename", emu_gamename },
	{ "keypost", emu_keypost },
	{ "exit", emu_exit },
	{ "start", emu_start },
	{ NULL, NULL }  /* sentinel */
};

//-------------------------------------------------
//  luaopen_emu - connect emu section lib
//-------------------------------------------------

static int luaopen_emu ( lua_State * L )
{
	luaL_newlib(L, emu_funcs);
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

lua_engine::lua_engine()
{
	m_lua_state = luaL_newstate();  /* create state */
	luaL_checkversion(m_lua_state);
	lua_gc(m_lua_state, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(m_lua_state);  /* open libraries */
	
	luaopen_lsqlite3(m_lua_state);
	luaL_requiref(m_lua_state, "emu", luaopen_emu, 1);
		
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

//-------------------------------------------------
//  initialize - initialize lua hookup to emu engine
//-------------------------------------------------

void lua_engine::initialize()
{
	mg_start_thread(::serve_lua, this);
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

void lua_engine::execute(const char *filename)
{
	int s = luaL_loadfile(m_lua_state, filename);
	s = docall(0, 0);	
	report(s);	
	lua_settop(m_lua_state, 0);
}

//-------------------------------------------------
//  execute_string - execute script from string
//-------------------------------------------------

void lua_engine::execute_string(const char *value)
{
	int s = luaL_loadstring(m_lua_state, value);
	s = docall(0, 0);
	report(s);	
	lua_settop(m_lua_state, 0);
}
