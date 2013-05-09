/***************************************************************************

    luaengine.h

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

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __LUA_ENGINE_H__
#define __LUA_ENGINE_H__

struct lua_State;

class lua_engine
{
public:
	// construction/destruction
	lua_engine(running_machine &machine);
	~lua_engine();

	// getters
	running_machine &machine() const { return m_machine; }

	void initialize();
	void lua_execute();
	void report_errors(int status);
	
	void createvm();
	void execute(const char *filename);	
	void execute_string(const char *value);
	void close();
	
	//static 
	static int emu_gamename(lua_State *L);
	static int emu_keypost(lua_State *L);
private:	
	// internal state
	running_machine &   m_machine;                          // reference to our machine
	lua_State* 			m_lua_state;
	
	static lua_engine*  luaThis;
};

#endif  /* __LUA_ENGINE_H__ */
