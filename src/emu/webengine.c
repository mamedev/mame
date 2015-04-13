// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    webengine.c

    Handle MAME internal web server.

***************************************************************************/

#include "mongoose/mongoose.h"
#include "jsoncpp/include/json/json.h"
#include "emu.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "webengine.h"
#include "lua.hpp"

#include "osdepend.h"

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

void reg_string(struct lua_State *L, const char *name, const char *val) {
	lua_pushstring(L, name);
	lua_pushstring(L, val);
	lua_rawset(L, -3);
}

void reg_int(struct lua_State *L, const char *name, int val) {
	lua_pushstring(L, name);
	lua_pushinteger(L, val);
	lua_rawset(L, -3);
}

void reg_function(struct lua_State *L, const char *name,
							lua_CFunction func, struct mg_connection *conn) {
	lua_pushstring(L, name);
	lua_pushlightuserdata(L, conn);
	lua_pushcclosure(L, func, 1);
	lua_rawset(L, -3);
}

static int lua_write(lua_State *L) {
	int i, num_args;
	const char *str;
	size_t size;
	struct mg_connection *conn = (struct mg_connection *)
	lua_touserdata(L, lua_upvalueindex(1));

	num_args = lua_gettop(L);
	for (i = 1; i <= num_args; i++) {
	if (lua_isstring(L, i)) {
		str = lua_tolstring(L, i, &size);
		mg_send_data(conn, str, size);
	}
	}

	return 0;
}

static int lua_header(lua_State *L) {
	struct mg_connection *conn = (struct mg_connection *)
	lua_touserdata(L, lua_upvalueindex(1));

	const char *header = luaL_checkstring(L,1);
	const char *value  = luaL_checkstring(L,2);

	mg_send_header(conn, header, value);

	return 0;
}


static void prepare_lua_environment(struct mg_connection *ri, lua_State *L) {
	extern void luaL_openlibs(lua_State *);
	int i;

	luaL_openlibs(L);

	if (ri == NULL) return;

	// Register mg module
	lua_newtable(L);
	reg_function(L, "write", lua_write, ri);
	reg_function(L, "header", lua_header, ri);

	// Export request_info
	lua_pushstring(L, "request_info");
	lua_newtable(L);
	reg_string(L, "request_method", ri->request_method);
	reg_string(L, "uri", ri->uri);
	reg_string(L, "http_version", ri->http_version);
	reg_string(L, "query_string", ri->query_string);
	reg_string(L, "remote_ip", ri->remote_ip);
	reg_int(L, "remote_port", ri->remote_port);
	reg_string(L, "local_ip", ri->local_ip);
	reg_int(L, "local_port", ri->local_port);
	lua_pushstring(L, "content");
	lua_pushlstring(L, ri->content == NULL ? "" : ri->content, ri->content_len);
	lua_rawset(L, -3);
	reg_int(L, "num_headers", ri->num_headers);
	lua_pushstring(L, "http_headers");
	lua_newtable(L);
	for (i = 0; i < ri->num_headers; i++) {
	reg_string(L, ri->http_headers[i].name, ri->http_headers[i].value);
	}
	lua_rawset(L, -3);
	lua_rawset(L, -3);

	lua_setglobal(L, "mg");

}


static void lsp(struct mg_connection *conn, const char *p, int len, lua_State *L) {
	int i, j, pos = 0;
	for (i = 0; i < len; i++) {
	if (p[i] == '<' && p[i + 1] == '?') {
		for (j = i + 1; j < len ; j++) {
		if (p[j] == '?' && p[j + 1] == '>') {
			if (i-pos!=0) mg_send_data(conn, p + pos, i - pos);
			if (luaL_loadbuffer(L, p + (i + 2), j - (i + 2), "") == 0) {
			lua_pcall(L, 0, LUA_MULTRET, 0);
			}
			pos = j + 2;
			i = pos - 1;
			break;
		}
		}
	}
	}
	if (i > pos) {
	mg_send_data(conn, p + pos, i - pos);
	}
}

static int filename_endswith(const char *str, const char *suffix)
{
	if (!str || !suffix)
		return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix >  lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// This function will be called by mongoose on every new request.
int web_engine::begin_request_handler(struct mg_connection *conn)
{
	astring file_path = astring(mg_get_option(m_server, "document_root")).cat(PATH_SEPARATOR).cat(conn->uri);
	if (filename_endswith(file_path.c_str(), ".lp"))
	{
		FILE *fp = NULL;
		if ((fp = fopen(file_path.c_str(), "rb")) != NULL) {
		fseek (fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek (fp, 0, SEEK_SET);
		char *data = (char*)mg_mmap(fp,size);

		lua_State *L = luaL_newstate();
		prepare_lua_environment(conn, L);
		lsp(conn, data, (int) size, L);
		if (L != NULL) lua_close(L);
		mg_munmap(data,size);
		fclose(fp);
		return MG_TRUE;
		} else {
		return MG_FALSE;
		}
	}
	else if (!strncmp(conn->uri, "/json/",6))
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
	else if (!strncmp(conn->uri, "/keypost",8))
	{
		// Is there any sane way to determine the length of the buffer before getting it?
		// A request for a way was previously filed with the mongoose devs,
		// but it looks like it was never implemented.

		// For now, we'll allow a paste buffer of 32k.
		// To-do: Send an error if the paste is too big?
		char cmd_val[32768];

		int pastelength = mg_get_var(conn, "val", cmd_val, sizeof(cmd_val));
		if (pastelength > 0) {
			machine().ioport().natkeyboard().post_utf8(cmd_val);
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
	else if (!strncmp(conn->uri, "/keyupload",8))
	{
		char *upload_data;
		int data_length, ofs = 0;
		char var_name[100], file_name[255];
		while ((ofs = mg_parse_multipart(conn->content + ofs, conn->content_len - ofs, var_name, sizeof(var_name), file_name, sizeof(file_name), (const char **)&upload_data, &data_length)) > 0) {
				mg_printf_data(conn, "File: %s, size: %d bytes", file_name, data_length);
		}

		// That upload_data contains more than we need. It also has the headers.
		// We'll need to strip it down to just what we want.

		if ((&data_length > 0) && (sizeof(file_name) > 0))
		{
			// MSVC doesn't yet support variable-length arrays, so chop the string the old-fashioned way
			upload_data[data_length] = '\0';

			// Now paste the stripped down paste_data..
			machine().ioport().natkeyboard().post_utf8(upload_data);
		}
		return MG_TRUE;
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
		screen_device_iterator iter(m_machine->root_device());
		screen_device *screen = iter.first();

		if (screen == NULL)
		{
			return 0;
		}

		astring fname("screenshot.png");
		emu_file file(m_machine->options().snapshot_directory(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
		file_error filerr = file.open(fname.c_str());

		if (filerr != FILERR_NONE)
		{
			return 0;
		}

		m_machine->video().save_snapshot(screen, file);
		astring fullpath(file.fullpath());
		file.close();
		mg_send_header(conn, "Cache-Control", "no-cache, no-store, must-revalidate");
		mg_send_header(conn, "Pragma", "no-cache");
		mg_send_header(conn, "Expires", "0");
		mg_send_file(conn, fullpath.c_str(), NULL);
		return MG_MORE; // It is important to return MG_MORE after mg_send_file!
	}
	return 0;
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
	if (ev == MG_REQUEST) {
	if (conn->is_websocket) {
		// This handler is called for each incoming websocket frame, one or more
		// times for connection lifetime.
		// Echo websocket data back to the client.
		return conn->content_len == 4 && !memcmp(conn->content, "exit", 4) ? MG_FALSE : MG_TRUE;
	} else {
		web_engine *engine = static_cast<web_engine *>(conn->server_param);
		return engine->begin_request_handler(conn);
	}
	} else if (ev== MG_WS_CONNECT) {
	// New websocket connection. Send connection ID back to the client.
	mg_websocket_printf(conn, WEBSOCKET_OPCODE_TEXT, "update_machine");
	return MG_FALSE;
	} else if (ev == MG_AUTH) {
	return MG_TRUE;
	} else {
	return MG_FALSE;
	}
}

//-------------------------------------------------
//  web_engine - constructor
//-------------------------------------------------

web_engine::web_engine(emu_options &options)
	: m_options(options),
		m_machine(NULL),
		m_server(NULL),
		//m_lastupdatetime(0),
		m_exiting_core(false),
		m_http(m_options.http())

{
	if (m_http) {
		m_server = mg_create_server(this, ev_handler);

		mg_set_option(m_server, "listening_port", options.http_port());
		mg_set_option(m_server, "document_root",  options.http_path());
	}

}

//-------------------------------------------------
//  ~web_engine - destructor
//-------------------------------------------------

web_engine::~web_engine()
{
	if (m_http)
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

void web_engine::serve()
{
	if (m_http) mg_poll_server(m_server, 0);
}

void web_engine::push_message(const char *message)
{
	struct mg_connection *c;
	if (m_server!=NULL) {
		// Iterate over all connections, and push current time message to websocket ones.
		for (c = mg_next(m_server, NULL); c != NULL; c = mg_next(m_server, c)) {
			if (c->is_websocket) {
				mg_websocket_write(c, 1, message, strlen(message));
			}
		}
	}
}
