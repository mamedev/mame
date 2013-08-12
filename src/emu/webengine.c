/***************************************************************************

    webengine.c

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

#include "emu.h"
#include "emuopts.h"
#include "webengine.h"
#include "web/mongoose.h"
#include "web/json/json.h"

//**************************************************************************
//  WEB ENGINE
//**************************************************************************

void web_engine::websocket_ready_handler(struct mg_connection *conn) {	
	static const char *message = "update_machine";
	mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, message, strlen(message));
	m_websockets.append(*global_alloc(simple_list_wrapper<mg_connection>(conn)));
} 

// Arguments:
//   flags: first byte of websocket frame, see websocket RFC,
//          http://tools.ietf.org/html/rfc6455, section 5.2
//   data, data_len: payload data. Mask, if any, is already applied.
int web_engine::websocket_data_handler(struct mg_connection *conn, int flags,
                                  char *data, size_t data_len) 
{
	// just Echo example for now
	if ((flags & 0x0f) == WEBSOCKET_OPCODE_TEXT)
		mg_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, data, data_len);

	// Returning zero means stoping websocket conversation.
	// Close the conversation if client has sent us "exit" string.
	return memcmp(data, "exit", 4);
} 

// This function will be called by mongoose on every new request.
int web_engine::begin_request_handler(struct mg_connection *conn) 
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	if (!strncmp(request_info->uri, "/json/",6)) 
	{
		if (!strcmp(request_info->uri, "/json/game")) 
		{
			Json::Value data;
			data["name"] = m_machine->system().name;
			data["description"] = m_machine->system().description;
			data["year"] = m_machine->system().year;
			data["manufacturer"] = m_machine->system().manufacturer;
			data["parent"] = m_machine->system().parent;
			data["source_file"] = m_machine->system().source_file;
			data["flags"] = m_machine->system().flags;
		
			Json::FastWriter writer;
			const char *json = writer.write(data).c_str();
			// Send HTTP reply to the client
			mg_printf(conn,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/json\r\n"
					"Content-Length: %d\r\n"        // Always set Content-Length
					"\r\n"
					"%s",
					(int)strlen(json), json);

			// Returning non-zero tells mongoose that our function has replied to
			// the client, and mongoose should not send client any more data.
			mg_close_connection(conn);
			return 1;
		}		
	}
	return 0;
} 


void *web_engine::websocket_keepalive() 
{
	while(!m_exiting_core) 
	{
		osd_ticks_t curtime = osd_ticks();	
		if ((curtime - m_lastupdatetime) > osd_ticks_per_second() * 5)
		{
			m_lastupdatetime = curtime;
			for (simple_list_wrapper<mg_connection> *curitem = m_websockets.first(); curitem != NULL; curitem = curitem->next())
			{
				mg_websocket_write(curitem->object(), WEBSOCKET_OPCODE_PING, NULL, 0);		
			}
		}
		osd_sleep(osd_ticks_per_second()/5);
	}
	return NULL;
}

//-------------------------------------------------
//  static callbacks
//-------------------------------------------------
static void websocket_ready_handler_static(struct mg_connection *conn)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
    web_engine *engine = downcast<web_engine *>(request_info->user_data);
	engine->websocket_ready_handler(conn);
}

static int websocket_data_handler_static(struct mg_connection *conn, int flags,
                                  char *data, size_t data_len) 
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
    web_engine *engine = downcast<web_engine *>(request_info->user_data);
	return engine->websocket_data_handler(conn, flags, data, data_len);
}

static int begin_request_handler_static(struct mg_connection *conn) 
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);	
    web_engine *engine = downcast<web_engine *>(request_info->user_data);
	return engine->begin_request_handler(conn);
}

static void *websocket_keepalive_static(void *thread_func_param) 
{
	web_engine *engine = downcast<web_engine *>(thread_func_param);
	return engine->websocket_keepalive();
}

//-------------------------------------------------
//  web_engine - constructor
//-------------------------------------------------

web_engine::web_engine(emu_options &options)
	: m_options(options),
	  m_machine(NULL),
	  m_ctx(NULL),
	  m_lastupdatetime(0),
	  m_exiting_core(false)
	
{
	
	struct mg_callbacks callbacks;

	// List of options. Last element must be NULL.
	const char *web_options[] = {
		"listening_ports", options.http_port(), 
		"document_root", options.http_path(),
		NULL
	};

	// Prepare callbacks structure. 
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.begin_request = begin_request_handler_static;
    callbacks.websocket_ready = websocket_ready_handler_static;
    callbacks.websocket_data = websocket_data_handler_static;	

	// Start the web server.
	if (m_options.http()) {
		m_ctx = mg_start(&callbacks, this, web_options);
		
		mg_start_thread(websocket_keepalive_static, this);
	}
	
}

//-------------------------------------------------
//  ~web_engine - destructor
//-------------------------------------------------

web_engine::~web_engine()
{
	if (m_options.http())
		close();
}

//-------------------------------------------------
//  close - close and cleanup of lua engine
//-------------------------------------------------

void web_engine::close()
{
	m_exiting_core = 1;
	osd_sleep(osd_ticks_per_second()/5);
	for (simple_list_wrapper<mg_connection> *curitem = m_websockets.first(); curitem != NULL; curitem = curitem->next())
	{
		mg_websocket_write(curitem->object(), WEBSOCKET_OPCODE_CONNECTION_CLOSE, NULL, 0);
	}
	// Stop the server.	
	mg_stop(m_ctx);
}


void web_engine::push_message(const char *message)
{
	for (simple_list_wrapper<mg_connection> *curitem = m_websockets.first(); curitem != NULL; curitem = curitem->next())
	{		
		mg_websocket_write(curitem->object(), WEBSOCKET_OPCODE_TEXT, message, strlen(message));
	}
}
