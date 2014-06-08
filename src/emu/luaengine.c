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

const char *lua_engine::get_prompt(int firstline) {
	const char *p;
	lua_getglobal(m_lua_state, firstline ? "_PROMPT" : "_PROMPT2");
	p = lua_tostring(m_lua_state, -1);
	if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
	return p;
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

int lua_engine::pushline(int firstline) 
{
	char buffer[LUA_MAXINPUT];
	char *b = buffer;
	size_t l;
	const char *prmt = get_prompt(firstline);
	int readstatus = lua_readline(b, prmt);
	lua_pop(m_lua_state, 1);  /* remove result from 'get_prompt' */
	if (readstatus == 0)
		return 0;  /* no input */
	l = strlen(b);
	if (l > 0 && b[l - 1] == '\n')  /* line ends with newline? */
		b[l - 1] = '\0';  /* remove it */
	if (firstline && b[0] == '=')  /* first line starts with `=' ? */
		lua_pushfstring(m_lua_state, "return %s", b + 1);  /* change it to `return' */
	else
		lua_pushstring(m_lua_state, b);
	return 1;
}


int lua_engine::loadline() 
{
	int status;
	lua_settop(m_lua_state, 0);
	if (!pushline(1))
		return -1;  /* no input */
	for (;;) {  /* repeat until gets a complete line */
		size_t l;
		const char *line = lua_tolstring(m_lua_state, 1, &l);
		status = luaL_loadbuffer(m_lua_state, line, l, "=stdin");
		if (!incomplete(status)) break;  /* cannot try to add lines? */
		if (!pushline(0))  /* no more input? */
			return -1;
		lua_pushliteral(m_lua_state, "\n");  /* add a new line... */
		lua_insert(m_lua_state, -2);  /* ...between the two lines */
		lua_concat(m_lua_state, 3);  /* join them */
	}
	lua_remove(m_lua_state, 1);  /* remove line */
	return status;
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

static const struct luaL_Reg emu_funcs [] =
{
	{ "gamename", emu_gamename },
	{ "keypost", emu_keypost },
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

void lua_engine::serve_lua()
{
	printf("%s v%s - %s\n%s\n%s\n\n", emulator_info::get_applongname(),build_version,emulator_info::get_fulllongname(),emulator_info::get_copyright_info(),LUA_COPYRIGHT);
	int status;
	while ((status = loadline()) != -1) {
		if (status == LUA_OK) status = docall(0, LUA_MULTRET);
		report(status);
		if (status == LUA_OK && lua_gettop(m_lua_state) > 0) {  /* any result to print? */
			luaL_checkstack(m_lua_state, LUA_MINSTACK, "too many results to print");
			lua_getglobal(m_lua_state, "print");
			lua_insert(m_lua_state, 1);
			if (lua_pcall(m_lua_state, lua_gettop(m_lua_state) - 1, 0, 0) != LUA_OK)
				luai_writestringerror("%s\n", lua_pushfstring(m_lua_state,
				"error calling " LUA_QL("print") " (%s)",
				lua_tostring(m_lua_state, -1)));
		}
	}
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
