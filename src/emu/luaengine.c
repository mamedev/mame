/***************************************************************************

    luaengine.c

    Controls execution of the core MAME system.

****************************************************************************

    Copyright Miodrag Milanovic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY MIODRAG MILANOVIC ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL MIODRAG MILANOVIC BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "osdepend.h"
#include "lua/lua.hpp"


lua_engine* lua_engine::luaThis = NULL;

//**************************************************************************
//  LUA ENGINE
//**************************************************************************

//-------------------------------------------------
//  emu_gamename - returns game full name
//-------------------------------------------------

int lua_engine::emu_gamename(lua_State *L) 
{
	lua_pushstring(L, luaThis->machine().system().description);
	return 1;
}

//-------------------------------------------------
//  emu_keypost - post keys to natural keyboard
//-------------------------------------------------

int lua_engine::emu_keypost(lua_State *L) 
{
	const char *keys = luaL_checkstring(L,1);
	luaThis->machine().ioport().natkeyboard().post_utf8(keys);
	return 1;
}

static const struct luaL_Reg emu_funcs [] =
{
  { "gamename", lua_engine::emu_gamename },
  { "keypost", lua_engine::emu_keypost },
  { NULL, NULL }  /* sentinel */
};

//-------------------------------------------------
//  luaopen_emu - connect emu section lib
//-------------------------------------------------

int luaopen_emu ( lua_State * L )
{
  luaL_newlib(L, emu_funcs);
  return 1;
}

//-------------------------------------------------
//  hook - lua hook to make slice execution of
//  script possible
//-------------------------------------------------

void hook(lua_State* l, lua_Debug* ar)
{
    lua_yield(l, 0);
}

//-------------------------------------------------
//  lua_engine - constructor
//-------------------------------------------------

lua_engine::lua_engine(running_machine &machine)
	: m_machine(machine)
{
	luaThis = this;
	m_lua_state = NULL;
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
	machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(FUNC(lua_engine::lua_execute), this));
}

//-------------------------------------------------
//  close - close and cleanup of lua engine
//-------------------------------------------------

void lua_engine::close()
{
	if (m_lua_state) {
		// close the Lua state    
		lua_close(m_lua_state);
		mame_printf_verbose("[LUA] End executing script\n");		
		m_lua_state = NULL;
	}
}

//-------------------------------------------------
//  report_errors - report lua error in execution
//-------------------------------------------------

void lua_engine::report_errors(int status)
{
  if ( status!=0 ) {
    mame_printf_error("[LUA ERROR] %s\n",lua_tostring(m_lua_state, -1));
    lua_pop(m_lua_state, 1); // remove error message
	
	close(); // close in case of error
  }
}

//-------------------------------------------------
//  createvm - setup lua VM and load script
//-------------------------------------------------

void lua_engine::createvm()
{
	close();
	
	// create new Lua state    
	m_lua_state = luaL_newstate();    	
	luaL_openlibs(m_lua_state);
	luaL_requiref(m_lua_state, "emu", luaopen_emu, 1);	
	lua_sethook(m_lua_state, hook, LUA_MASKLINE, 0);		
}

//-------------------------------------------------
//  execute - load and execute script
//-------------------------------------------------

void lua_engine::execute(const char *filename)
{
	createvm();	
	
	int s = luaL_loadfile(m_lua_state, filename);     	
	report_errors(s);

	mame_printf_verbose("[LUA] Start executing script\n");
}

//-------------------------------------------------
//  execute_string - execute script from string
//-------------------------------------------------

void lua_engine::execute_string(const char *value)
{
	createvm();	
	
	int s = luaL_loadstring(m_lua_state, value);     	
	report_errors(s);

	mame_printf_verbose("[LUA] Start executing script\n");
}
//-------------------------------------------------
//  lua_execute - execute slice of lua script
//  this callback is hooked to frame notification
//-------------------------------------------------

void lua_engine::lua_execute()
{
	if (m_lua_state==NULL) return;
	
	int s = lua_resume(m_lua_state, m_lua_state, 0);
	
	if (s != LUA_YIELD) {
		report_errors(s);	
		close();
	}
}
