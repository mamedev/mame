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
	lua_engine();
	~lua_engine();

	void initialize();
	void execute(const char *filename);
	void execute_string(const char *value);
	void close();

	void serve_lua();
private:	
	int report(int status);
	int docall(int narg, int nres);
	const char *get_prompt(int firstline);
	int incomplete(int status) ;
	int pushline(int firstline);
	int loadline();
private:
	// internal state
	lua_State*          m_lua_state;
};

#endif  /* __LUA_ENGINE_H__ */
