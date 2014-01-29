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

static void get_qsvar(const struct mg_request_info *request_info,
						const char *name, char *dst, size_t dst_len) {
	const char *qs = request_info->query_string;
	mg_get_var(qs, strlen(qs == NULL ? "" : qs), name, dst, dst_len);
}

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
	return 1;
}

int web_engine::json_slider_handler(struct mg_connection *conn)
{
	const slider_state *curslider;
	astring tempstring;
	Json::Value array(Json::arrayValue);

	/* add all sliders */
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

	/* add all sliders */
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

	return 1;
}

// This function will be called by mongoose on every new request.
int web_engine::begin_request_handler(struct mg_connection *conn)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	if (!strncmp(request_info->uri, "/json/",6))
	{
		if (!strcmp(request_info->uri, "/json/game"))
		{
			return json_game_handler(conn);
		}
		if (!strcmp(request_info->uri, "/json/slider"))
		{
			return json_slider_handler(conn);
		}
	}
	else if (!strncmp(request_info->uri, "/cmd",4))
	{
		char cmd_name[64];
		get_qsvar(request_info, "name", cmd_name, sizeof(cmd_name));

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
			get_qsvar(request_info, "val", cmd_val, sizeof(cmd_val));
			char *filename = websanitize_statefilename(cmd_val);
			m_machine->schedule_save(filename);
		}
		else if(!strcmp(cmd_name,"loadstate"))
		{
			char cmd_val[64];
			get_qsvar(request_info, "val", cmd_val, sizeof(cmd_val));
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
		return 1;
	}
	else if (!strncmp(request_info->uri, "/slider",7))
	{
		char cmd_id[64];
		char cmd_val[64];
		get_qsvar(request_info, "id", cmd_id, sizeof(cmd_id));
		get_qsvar(request_info, "val", cmd_val, sizeof(cmd_val));
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
		return 1;
	}
	else if (!strncmp(request_info->uri, "/screenshot.png",15))
	{
		screen_device_iterator iter(m_machine->root_device());
		screen_device *screen = iter.first();

		if (screen == NULL)
		{
			return 0;
		}

		astring fname("screenshot.png");
		emu_file file(m_machine->options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		file_error filerr = file.open(fname);

		if (filerr != FILERR_NONE)
		{
			return 0;
		}

		m_machine->video().save_snapshot(screen, file);
		astring fullpath(file.fullpath());
		file.close();

		mg_send_file(conn,fullpath);
		return 1;
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
				int status = mg_websocket_write(curitem->object(), WEBSOCKET_OPCODE_PING, NULL, 0);
				if (status==0) m_websockets.detach(*curitem); // remove inactive clients
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
	web_engine *engine = static_cast<web_engine *>(request_info->user_data);
	engine->websocket_ready_handler(conn);
}

static int websocket_data_handler_static(struct mg_connection *conn, int flags,
									char *data, size_t data_len)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	web_engine *engine = static_cast<web_engine *>(request_info->user_data);
	return engine->websocket_data_handler(conn, flags, data, data_len);
}

static int begin_request_handler_static(struct mg_connection *conn)
{
	const struct mg_request_info *request_info = mg_get_request_info(conn);
	web_engine *engine = static_cast<web_engine *>(request_info->user_data);
	return engine->begin_request_handler(conn);
}

static int begin_http_error_handler_static(struct mg_connection *conn, int status)
{
	//const struct mg_request_info *request_info = mg_get_request_info(conn);
	if (status == 404) // 404 -- File Not Found
	{
		{
				mg_printf(conn,
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Type: text/plain\r\n"
					"Content-Length: 14\r\n"        // Always set Content-Length
					"\r\n"
					"Nothing to do.");
		}
	}
	// Returning non-zero tells mongoose that our function has replied to
	// the client, and mongoose should not send client any more data.
	return 1;
}

static void *websocket_keepalive_static(void *thread_func_param)
{
	web_engine *engine = static_cast<web_engine *>(thread_func_param);
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
	callbacks.http_error = begin_http_error_handler_static;

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
		int status = mg_websocket_write(curitem->object(), WEBSOCKET_OPCODE_TEXT, message, strlen(message));
		if (status==0) m_websockets.detach(*curitem); // remove inactive clients
	}
}
