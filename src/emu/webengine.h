// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    webengine.h

    Handle MAME internal web server.

***************************************************************************/

#pragma once

#ifndef __WEB_ENGINE_H__
#define __WEB_ENGINE_H__

struct mg_server;      // Handle for the HTTP server itself
struct mg_connection;  // Handle for the individual connection

class web_engine
{
public:
	// construction/destruction
	web_engine(emu_options &options);
	~web_engine();

	void push_message(const char *message);
	void close();

	void set_machine(running_machine *machine) { m_machine = machine; }
	int begin_request_handler(struct mg_connection *conn);
protected:
	// getters
	running_machine &machine() const { return *m_machine; }

	int json_game_handler(struct mg_connection *conn);
	int json_slider_handler(struct mg_connection *conn);
private:
	// internal state
	emu_options &       m_options;
	running_machine *   m_machine;
	struct mg_server *  m_server;
	//osd_ticks_t         m_lastupdatetime;
	bool                m_exiting_core;
};

#endif  /* __web_engine_H__ */
