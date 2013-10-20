// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    webengine.h

    Handle MAME internal web server.

***************************************************************************/

#pragma once

#ifndef __WEB_ENGINE_H__
#define __WEB_ENGINE_H__

struct mg_context;     // Handle for the HTTP service itself
struct mg_connection;  // Handle for the individual connection

class web_engine
{
public:
	// construction/destruction
	web_engine(emu_options &options);
	~web_engine();

	void push_message(const char *message);
	void close();

	void set_machine(running_machine &machine) { m_machine = &machine; }

	void websocket_ready_handler(struct mg_connection *conn);
	int websocket_data_handler(struct mg_connection *conn, int flags, char *data, size_t data_len);
	int begin_request_handler(struct mg_connection *conn);
	int begin_http_error_handler(struct mg_connection *conn, int status);
	void *websocket_keepalive();
protected:
	// getters
	running_machine &machine() const { return *m_machine; }

	int json_game_handler(struct mg_connection *conn);
	int json_slider_handler(struct mg_connection *conn);
private:
	// internal state
	emu_options &       m_options;
	running_machine *   m_machine;
	struct mg_context * m_ctx;
	osd_ticks_t         m_lastupdatetime;
	bool                m_exiting_core;
	simple_list<simple_list_wrapper<mg_connection> > m_websockets;
};

#endif  /* __web_engine_H__ */
