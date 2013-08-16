/***************************************************************************

    webengine.h

    Handle MAME internal web server.

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
	void *websocket_keepalive();	
protected:
	// getters
	running_machine &machine() const { return *m_machine; }
	
	int json_game_handler(struct mg_connection *conn);
	int json_slider_handler(struct mg_connection *conn);
private:
	// internal state
	emu_options &		m_options;
	running_machine *   m_machine;	
	struct mg_context * m_ctx;
	osd_ticks_t 		m_lastupdatetime;
	bool 				m_exiting_core;
	simple_list<simple_list_wrapper<mg_connection> > m_websockets;
};

#endif  /* __web_engine_H__ */
