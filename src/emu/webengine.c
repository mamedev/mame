// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    webengine.c

    Handle MAME internal web server.

***************************************************************************/

#include "web/mongoose.h"
#include "web/json/json.h"
#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "webengine.h"


//**************************************************************************
//  WEB ENGINE
//**************************************************************************

char* websanitize_statefilename ( char* unsanitized )
{
	// It's important that we remove any dangerous characters from any filename
	// we receive from a web client. This can be a serious security hole.
	// As MAME/MESS policy is lowercase filenames, also lowercase it.

	char* sanitized = new char[64];
	int insertpoint =0;
	char charcompare;

	while (*unsanitized != 0)
	{
	charcompare = *unsanitized;
		// ASCII 48-57 are 0-9
		// ASCII 97-122 are lowercase A-Z

		if ((charcompare >= 48 && charcompare <= 57) || (charcompare >= 97 && charcompare <= 122))
		{
			sanitized[insertpoint] = charcompare;
			insertpoint++;
			sanitized[insertpoint] = '\0'; // Make sure we're null-terminated.
		}
		// ASCII 65-90 are uppercase A-Z. These need to be lowercased.
		if (charcompare >= 65 && charcompare <= 90)
		{
			sanitized[insertpoint] = tolower(charcompare); // Lowercase it
			insertpoint++;
			sanitized[insertpoint] = '\0'; // Make sure we're null-terminated.
		}
		unsanitized++;
	}
	return (sanitized);
}

int web_engine::json_game_handler(struct mg_connection *conn)
{
	Json::Value data;
	data["name"] = m_machine->system().name;
	data["description"] = m_machine->system().description;
	data["year"] = m_machine->system().year;
	data["manufacturer"] = m_machine->system().manufacturer;
	data["parent"] = m_machine->system().parent;
	data["source_file"] = m_machine->system().source_file;
	data["flags"] = m_machine->system().flags;
	data["ispaused"] = m_machine->paused();

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

	return MG_TRUE;
}

int web_engine::json_slider_handler(struct mg_connection *conn)
{
	const slider_state *curslider;
	astring tempstring;
	Json::Value array(Json::arrayValue);

	// add all sliders
	for (curslider = machine().ui().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		Json::Value data;
		data["description"] = curslider->description;
		data["minval"] = curslider->minval;
		data["maxval"] = curslider->maxval;
		data["defval"] = curslider->defval;
		data["incval"] = curslider->incval;
		data["curval"] = curval;
		array.append(data);
	}

	// add all sliders 
	for (curslider = (slider_state*)machine().osd().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		Json::Value data;
		data["description"] = curslider->description;
		data["minval"] = curslider->minval;
		data["maxval"] = curslider->maxval;
		data["defval"] = curslider->defval;
		data["incval"] = curslider->incval;
		data["curval"] = curval;
		array.append(data);
	}
	Json::FastWriter writer;
	const char *json = writer.write(array).c_str();
	// Send HTTP reply to the client
	mg_printf(conn,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Content-Length: %d\r\n"        // Always set Content-Length
			"\r\n"
			"%s",
			(int)strlen(json), json);

	return MG_TRUE;
}

// This function will be called by mongoose on every new request.
int web_engine::begin_request_handler(struct mg_connection *conn)
{
	if (!strncmp(conn->uri, "/json/",6))
	{
		if (!strcmp(conn->uri, "/json/game"))
		{
			return json_game_handler(conn);
		}
		if (!strcmp(conn->uri, "/json/slider"))
		{
			return json_slider_handler(conn);
		}
	}
	else if (!strncmp(conn->uri, "/cmd",4))
	{
		char cmd_name[64];
		mg_get_var(conn, "name", cmd_name, sizeof(cmd_name));

		if(!strcmp(cmd_name,"softreset"))
		{
			m_machine->schedule_soft_reset();
		}
		else if(!strcmp(cmd_name,"hardreset"))
		{
			m_machine->schedule_hard_reset();
		}
		else if(!strcmp(cmd_name,"exit"))
		{
			m_machine->schedule_exit();
		}
		else if(!strcmp(cmd_name,"togglepause"))
		{
			if (m_machine->paused())
				m_machine->resume();
		else
				m_machine->pause();
		}
		else if(!strcmp(cmd_name,"savestate"))
		{
			char cmd_val[64];
			mg_get_var(conn, "val", cmd_val, sizeof(cmd_val));
			char *filename = websanitize_statefilename(cmd_val);
			m_machine->schedule_save(filename);
		}
		else if(!strcmp(cmd_name,"loadstate"))
		{
			char cmd_val[64];
			mg_get_var(conn, "val", cmd_val, sizeof(cmd_val));
			char *filename = cmd_val;
			m_machine->schedule_load(filename);
		}
		else if(!strcmp(cmd_name,"loadauto"))
		{
			// This is here to just load the autosave and only the autosave.
			m_machine->schedule_load("auto");
		}

		// Send HTTP reply to the client
		mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 2\r\n"        // Always set Content-Length
				"\r\n"
				"OK");

		// Returning non-zero tells mongoose that our function has replied to
		// the client, and mongoose should not send client any more data.
		return MG_TRUE;
	}
	else if (!strncmp(conn->uri, "/slider",7))
	{
		char cmd_id[64];
		char cmd_val[64];
		mg_get_var(conn, "id", cmd_id, sizeof(cmd_id));
		mg_get_var(conn, "val", cmd_val, sizeof(cmd_val));
		int cnt = 0;
		int id = atoi(cmd_id);
		const slider_state *curslider;
		for (curslider = machine().ui().get_slider_list(); curslider != NULL; curslider = curslider->next)
		{
			if (cnt==id)
				(*curslider->update)(machine(), curslider->arg, NULL, atoi(cmd_val));
			cnt++;
		}
		for (curslider = (slider_state*)machine().osd().get_slider_list(); curslider != NULL; curslider = curslider->next)
		{
			if (cnt==id)
				(*curslider->update)(machine(), curslider->arg, NULL, atoi(cmd_val));
			cnt++;
		}

		// Send HTTP reply to the client
		mg_printf(conn,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: text/plain\r\n"
				"Content-Length: 2\r\n"        // Always set Content-Length
				"\r\n"
				"OK");

		// Returning non-zero tells mongoose that our function has replied to
		// the client, and mongoose should not send client any more data.
		return MG_TRUE;
	}
	else if (!strncmp(conn->uri, "/screenshot.png",15))
	{
		FILE *fp = (FILE *) conn->connection_param;
		char buf[200];
		size_t n = 0;
		if (fp == NULL) 
		{
			screen_device_iterator iter(m_machine->root_device());
			screen_device *screen = iter.first();

			if (screen == NULL)
			{
				return 0;
			}

			astring fname("screenshot.png");
			{
				emu_file file(m_machine->options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
				file_error filerr = file.open(fname);

				if (filerr != FILERR_NONE)
				{
					return 0;
				}

				m_machine->video().save_snapshot(screen, file);
				astring fullpath(file.fullpath());
				file.close();
			}
			
			{
				emu_file file(m_machine->options().snapshot_directory(), OPEN_FLAG_READ);
				file_error filerr = file.open(fname);

				if (filerr != FILERR_NONE)
				{
					return 0;
				}
							
				file.seek(0, SEEK_SET);
				mg_send_header(conn, "Content-Type", "image/png");
				mg_send_header(conn, "Cache-Control", "no-cache, no-store, must-revalidate");
				mg_send_header(conn, "Pragma", "no-cache");
				mg_send_header(conn, "Expires", "0");			 
				do 
				{
					n = file.read(buf, sizeof(buf));										
					mg_send_data(conn, buf, n);
				}
				while (n==sizeof(buf));
				file.close();				
			}
		}
		return MG_TRUE;
	}
	return 0;
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
  if (ev == MG_REQUEST) {
	if (conn->is_websocket) {
		// This handler is called for each incoming websocket frame, one or more
		// times for connection lifetime.
		// Echo websocket data back to the client.
		//const char *msg = "update_machine";
		//mg_websocket_write(conn, 1, msg, strlen(msg));
		return conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ? MG_FALSE : MG_TRUE;
    } else {
		web_engine *engine = static_cast<web_engine *>(conn->server_param);	
		return engine->begin_request_handler(conn);    
	}
  } else if (ev == MG_AUTH) {
    return MG_TRUE;
  } else {
    return MG_FALSE;
  }
}

static int iterate_callback(struct mg_connection *c, enum mg_event ev) {
  if (ev == MG_POLL && c->is_websocket) {
    char buf[20];
    int len = snprintf(buf, sizeof(buf), "%lu",
     (unsigned long) * (time_t *) c->callback_param);
    mg_websocket_write(c, 1, buf, len);
  }
  return MG_TRUE;
}

static void *serve(void *server) {
	time_t current_timer = 0, last_timer = time(NULL);
	for (;;) mg_poll_server((struct mg_server *) server, 1000);
	current_timer = time(NULL);
	if (current_timer - last_timer > 0) {
		last_timer = current_timer;
		mg_iterate_over_connections((struct mg_server *)server, iterate_callback, &current_timer);
	}  
	return NULL;
}

//-------------------------------------------------
//  web_engine - constructor
//-------------------------------------------------

web_engine::web_engine(emu_options &options)
	: m_options(options),
		m_machine(NULL),
		m_server(NULL),
		m_lastupdatetime(0),
		m_exiting_core(false)

{
	if (m_options.http()) {
		m_server = mg_create_server(this, ev_handler);
		
		mg_set_option(m_server, "listening_port", options.http_port());
		mg_set_option(m_server, "document_root",  options.http_path());
		
		mg_start_thread(serve, m_server);
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
	// Cleanup, and free server instance
	mg_destroy_server(&m_server);	
}

static int websocket_callback(struct mg_connection *c, enum mg_event ev) {
  if (c->is_websocket) {
    const char *message = (const char *)c->callback_param;
    mg_websocket_write(c, 1, message, strlen(message));
  }
  return MG_TRUE;
}

void web_engine::push_message(const char *message)
{
	if (m_server!=NULL)
		mg_iterate_over_connections(m_server, websocket_callback, (void*)message);
}
