// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    webengine_retro.c

    Handle MAME internal web server (stub for libretro).

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "webengine.h"


//**************************************************************************
//  WEB ENGINE
//**************************************************************************

char* websanitize_statefilename ( char* unsanitized )
{
   return unsanitized;
}

int web_engine::json_game_handler(struct mg_connection *conn)
{
	return 1;
}

int web_engine::json_slider_handler(struct mg_connection *conn)
{
	return 1;
}

// This function will be called by mongoose on every new request.
int web_engine::begin_request_handler(struct mg_connection *conn)
{
	return 0;
}

//-------------------------------------------------
//  web_engine - constructor
//-------------------------------------------------

web_engine::web_engine(emu_options &options)
	: m_options(options),
		m_machine(NULL),
		m_server(NULL),
		//m_lastupdatetime(0),
		m_exiting_core(false)

{
}

//-------------------------------------------------
//  ~web_engine - destructor
//-------------------------------------------------

web_engine::~web_engine()
{
}

//-------------------------------------------------
//  close - close and cleanup of lua engine
//-------------------------------------------------

void web_engine::close()
{
}

void web_engine::serve()
{
}


void web_engine::push_message(const char *message)
{
}
