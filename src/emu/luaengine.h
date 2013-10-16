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
	lua_State*          m_lua_state;

	static lua_engine*  luaThis;
};

#endif  /* __LUA_ENGINE_H__ */
