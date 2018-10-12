// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

http.cpp

HTTP server handling

***************************************************************************/

#include "emu.h"
#include "server_ws_impl.hpp"
#include "server_http_impl.hpp"
#include <fstream>

#include <inttypes.h>
#include <stdint.h>


const static struct mapping
{
	const char* extension;
	const char* mime_type;
} mappings[] =
{
	{ "aac",     "audio/aac" },
	{ "aat",     "application/font-sfnt" },
	{ "aif",     "audio/x-aif" },
	{ "arj",     "application/x-arj-compressed" },
	{ "asf",     "video/x-ms-asf" },
	{ "avi",     "video/x-msvideo" },
	{ "bmp",     "image/bmp" },
	{ "cff",     "application/font-sfnt" },
	{ "css",     "text/css" },
	{ "csv",     "text/csv" },
	{ "doc",     "application/msword" },
	{ "eps",     "application/postscript" },
	{ "exe",     "application/octet-stream" },
	{ "gif",     "image/gif" },
	{ "gz",      "application/x-gunzip" },
	{ "htm",     "text/html" },
	{ "html",    "text/html" },
	{ "ico",     "image/x-icon" },
	{ "ief",     "image/ief" },
	{ "jpeg",    "image/jpeg" },
	{ "jpg",     "image/jpeg" },
	{ "jpm",     "image/jpm" },
	{ "jpx",     "image/jpx" },
	{ "js",      "application/javascript" },
	{ "json",    "application/json" },
	{ "lay",     "text/xml" },
	{ "m3u",     "audio/x-mpegurl" },
	{ "m4v",     "video/x-m4v" },
	{ "mid",     "audio/x-midi" },
	{ "mov",     "video/quicktime" },
	{ "mp3",     "audio/mpeg" },
	{ "mp4",     "video/mp4" },
	{ "mpeg",    "video/mpeg" },
	{ "mpg",     "video/mpeg" },
	{ "oga",     "audio/ogg" },
	{ "ogg",     "audio/ogg" },
	{ "ogv",     "video/ogg" },
	{ "otf",     "application/font-sfnt" },
	{ "pct",     "image/x-pct" },
	{ "pdf",     "application/pdf" },
	{ "pfr",     "application/font-tdpfr" },
	{ "pict",    "image/pict" },
	{ "png",     "image/png" },
	{ "ppt",     "application/x-mspowerpoint" },
	{ "ps",      "application/postscript" },
	{ "qt",      "video/quicktime" },
	{ "ra",      "audio/x-pn-realaudio" },
	{ "ram",     "audio/x-pn-realaudio" },
	{ "rar",     "application/x-arj-compressed" },
	{ "rgb",     "image/x-rgb" },
	{ "rtf",     "application/rtf" },
	{ "sgm",     "text/sgml" },
	{ "shtm",    "text/html" },
	{ "shtml",   "text/html" },
	{ "sil",     "application/font-sfnt" },
	{ "svg",     "image/svg+xml" },
	{ "swf",     "application/x-shockwave-flash" },
	{ "tar",     "application/x-tar" },
	{ "tgz",     "application/x-tar-gz" },
	{ "tif",     "image/tiff" },
	{ "tiff",    "image/tiff" },
	{ "torrent", "application/x-bittorrent" },
	{ "ttf",     "application/font-sfnt" },
	{ "txt",     "text/plain" },
	{ "wav",     "audio/x-wav" },
	{ "webm",    "video/webm" },
	{ "woff",    "application/font-woff" },
	{ "wrl",     "model/vrml" },
	{ "xhtml",   "application/xhtml+xml" },
	{ "xls",     "application/x-msexcel" },
	{ "xml",     "text/xml" },
	{ "xsl",     "application/xml" },
	{ "xslt",    "application/xml" },
	{ "zip",     "application/x-zip-compressed" }
};

static std::string extension_to_type(const std::string& extension)
{
	for (mapping m : mappings)
	{
		if (m.extension == extension)
		{
			return m.mime_type;
		}
	}

	return "text/plain";
}

/** An HTTP Request. */
struct http_request_impl : public http_manager::http_request
{
public:
	std::shared_ptr<webpp::Request> m_request;
	std::size_t m_query;
	std::size_t m_fragment;
	std::size_t m_path_end;
	std::size_t m_query_end;

	http_request_impl(std::shared_ptr<webpp::Request> request) : m_request(request) {
		std::size_t len = m_request->path.length();

		m_fragment = m_request->path.find('#');
		m_query_end = m_fragment == std::string::npos ? len : m_fragment;

		m_query = m_request->path.find('?');
		m_path_end = m_query == std::string::npos ? m_query_end : m_query;
	}

	virtual ~http_request_impl() = default;

	/** Retrieves the requested resource. */
	virtual const std::string get_resource() {
		// The entire resource: path, query and fragment.
		return m_request->path;
	}

	/** Returns the path part of the requested resource. */
	virtual const std::string get_path() {
		return m_request->path.substr(0, m_path_end);
	}

	/** Returns the query part of the requested resource. */
	virtual const std::string get_query() {
		return m_query == std::string::npos ? "" : m_request->path.substr(m_query, m_query_end);
	}

	/** Returns the fragment part of the requested resource. */
	virtual const std::string get_fragment() {
		return m_fragment == std::string::npos ? "" : m_request->path.substr(m_fragment);
	}

	/** Retrieves a header from the HTTP request. */
	virtual const std::string get_header(const std::string &header_name) {
		auto i = m_request->header.find(header_name);
		if (i != m_request->header.end()) {
			return (*i).second;
		} else {
			return "";
		}
	}

	/** Retrieves a header from the HTTP request. */
	virtual const std::list<std::string> get_headers(const std::string &header_name) {
		std::list<std::string> result;
		auto range = m_request->header.equal_range(header_name);
		for (auto i = range.first; i != range.second; i++) {
			result.push_back((*i).second);
		}
		return result;
	}

	/** Returns the body that was submitted with the HTTP request. */
	virtual const std::string get_body() {
		// TODO(cbrunschen): What to return here - http_server::Request has a 'content' feld that is never filled in!
		return "";
	}
};

/** An HTTP response. */
struct http_response_impl : public http_manager::http_response {
	std::shared_ptr<webpp::Response> m_response;
	int m_status;
	std::string m_content_type;
	std::stringstream m_headers;
	std::stringstream m_body;

	http_response_impl(std::shared_ptr<webpp::Response> response) : m_response(response) { }

	virtual ~http_response_impl() = default;

	/** Sets the HTTP status to be returned to the client. */
	virtual void set_status(int status) {
		m_status = status;
	}

	/** Sets the HTTP content type to be returned to the client. */
	virtual void set_content_type(const std::string &content_type) {
		m_content_type = content_type;
	}

	/** Sets the body to be sent to the client. */
	virtual void set_body(const std::string &body) {
		m_body.str("");
		append_body(body);
	}

	/** Appends something to the body to be sent to the client. */
	virtual void append_body(const std::string &body) {
		m_body << body;
	}

	/** Sends the response to the client. */
	void send() {
		m_response->type(m_content_type);
		m_response->status(m_status);
		m_response->send(m_body.str());
	}
};

struct websocket_endpoint_impl : public http_manager::websocket_endpoint {
	/** The underlying edpoint. */
	webpp::ws_server::Endpoint *m_endpoint;

	websocket_endpoint_impl(webpp::ws_server::Endpoint *endpoint,
		http_manager::websocket_open_handler on_open,
		http_manager::websocket_message_handler on_message,
		http_manager::websocket_close_handler on_close,
		http_manager::websocket_error_handler on_error)
	: m_endpoint(endpoint) {
		this->on_open = on_open;
		this->on_message = on_message;
		this->on_close = on_close;
		this->on_error = on_error;
	}
};

struct websocket_connection_impl : public http_manager::websocket_connection {
	/** The server */
	webpp::ws_server *m_wsserver;
	/* The underlying Commection. */
	std::weak_ptr<webpp::Connection> m_connection;
	websocket_connection_impl(webpp::ws_server *server, std::shared_ptr<webpp::Connection> connection)
		: m_wsserver(server), m_connection(connection) { }

	/** Sends a message to the client that is connected on the other end of this Websocket connection. */
	virtual void send_message(const std::string &payload, int opcode) {
		if (auto connection = m_connection.lock()) {
			std::shared_ptr<webpp::ws_server::SendStream> message_stream = std::make_shared<webpp::ws_server::SendStream>();
			(*message_stream) << payload;
			m_wsserver->send(connection, message_stream, nullptr, opcode | 0x80);
		}
	}

	/** Closes this open Websocket connection. */
	virtual void close() {
		if (auto connection = m_connection.lock()) {
			m_wsserver->send_close(connection, 1000 /* normal close */);
		}
	}
};

http_manager::http_manager(bool active, short port, const char *root)
  : m_active(active), m_io_context(std::make_shared<asio::io_context>()), m_root(root)
{
	if (!active) return;

	m_server = std::make_unique<webpp::http_server>();
	m_server->m_config.port = port;
	m_server->set_io_context(m_io_context);
	m_wsserver = std::make_unique<webpp::ws_server>();

	auto& endpoint = m_wsserver->m_endpoint["/"];

	m_server->on_get([root](auto response, auto request) {
		std::string doc_root = root;

		auto request_impl = std::make_shared<http_request_impl>(request);

		std::string path = request_impl->get_path();
		// If path ends in slash (i.e. is a directory) then add "index.html".
		if (path[path.size() - 1] == '/')
		{
			path += "index.html";
		}

		// Determine the file extension.
		std::size_t last_slash_pos = path.find_last_of("/");
		std::size_t last_dot_pos = path.find_last_of(".");
		std::string extension;
		if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
		{
			extension = path.substr(last_dot_pos + 1);
		}

		// Open the file to send back.
		std::string full_path = doc_root + path;
		std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
		if (!is)
		{
			response->status(400).send("Error");
		}
		else
		{
			// Fill out the reply to be sent to the client.
			std::string content;
			char buf[512];
			while (is.read(buf, sizeof(buf)).gcount() > 0)
				content.append(buf, size_t(is.gcount()));

			response->type(extension_to_type(extension));
			response->status(200).send(content);
		}
	});

	endpoint.on_open = [&](auto connection) {
		auto send_stream = std::make_shared<webpp::ws_server::SendStream>();
		*send_stream << "update_machine";
		m_wsserver->send(connection, send_stream);
	};

	m_server->on_upgrade = [this](auto socket, auto request) {
		auto connection = std::make_shared<webpp::ws_server::Connection>(socket);
		connection->method = std::move(request->method);
		connection->path = std::move(request->path);
		connection->http_version = std::move(request->http_version);
		connection->header = std::move(request->header);
		connection->remote_endpoint_address = std::move(request->remote_endpoint_address);
		connection->remote_endpoint_port = request->remote_endpoint_port;
		m_wsserver->upgrade(connection);
	};

	m_server->start();

	m_server_thread = std::thread([this]() {
		m_io_context->run();
	});
}

http_manager::~http_manager()
{
	if (!m_server) return;

	m_server->stop();
	m_io_context->stop();
	if (m_server_thread.joinable())
		m_server_thread.join();
}

static void on_get(http_manager::http_handler handler, std::shared_ptr<webpp::Response> response, std::shared_ptr<webpp::Request> request) {
	auto request_impl = std::make_shared<http_request_impl>(request);
	auto response_impl = std::make_shared<http_response_impl>(response);

	handler(request_impl, response_impl);

	response_impl->send();
}

void http_manager::on_open(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection) {
	std::lock_guard<std::mutex> lock(m_connections_mutex);
	webpp::ws_server *ws_server = m_wsserver.get();

	http_manager::websocket_connection_ptr connection_impl = std::make_shared<websocket_connection_impl>(ws_server, connection);
	m_connections[connection.get()] = connection_impl;

	if (endpoint->on_open) {
		endpoint->on_open(connection_impl);
	}
}

void http_manager::on_message(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection, const std::string &payload, int opcode) {
	if (endpoint->on_message) {
		std::lock_guard<std::mutex> lock(m_connections_mutex);
		auto i = m_connections.find(connection.get());
		if (i != m_connections.end()) {
			http_manager::websocket_connection_ptr websocket_connection_impl = (*i).second;
			endpoint->on_message(websocket_connection_impl, payload, opcode);
		}
	}
}

void http_manager::on_close(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection,
			  int status, const std::string& reason) {
	std::lock_guard<std::mutex> lock(m_connections_mutex);
	auto i = m_connections.find(connection.get());
	if (i != m_connections.end()) {
		if (endpoint->on_close) {
			http_manager::websocket_connection_ptr websocket_connection_impl = (*i).second;
			endpoint->on_close(websocket_connection_impl, status, reason);
		}

		m_connections.erase(connection.get());
	}
}

void http_manager::on_error(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection,
			  const std::error_code& error_code) {
	std::lock_guard<std::mutex> lock(m_connections_mutex);
	auto i = m_connections.find(connection.get());
	if (i != m_connections.end()) {
		if (endpoint->on_error) {
			http_manager::websocket_connection_ptr websocket_connection_impl = (*i).second;
			endpoint->on_error(websocket_connection_impl, error_code);
		}

		m_connections.erase(connection.get());
	}
}

bool http_manager::read_file(std::ostream &os, const std::string &path) {
	std::ostringstream full_path;
	full_path << m_root << path;
	util::core_file::ptr f;
	osd_file::error e = util::core_file::open(full_path.str(), OPEN_FLAG_READ, f);
	if (e == osd_file::error::NONE)
	{
		int c;
		while ((c = f->getc()) >= 0)
		{
			os.put(c);
		}
	}
	return e == osd_file::error::NONE;
}

void http_manager::serve_document(http_request_ptr request, http_response_ptr response, const std::string &filename) {
	if (!m_active) return;

	std::ostringstream os;
	if (read_file(os, filename))
	{
		response->set_status(200);
		response->set_body(os.str());
	}
	else
	{
		response->set_status(500);
	}
}

void http_manager::serve_template(http_request_ptr request, http_response_ptr response,
								  const std::string &filename, substitution substitute, char init, char term)
{
	if (!m_active) return;

	// printf("webserver: serving template '%s' at path '%s'\n", filename.c_str(), request->get_path().c_str());
	std::stringstream ss;
	if (read_file(ss, filename))
	{
		std::ostringstream os;
		while (ss.good())
		{
			std::string s;
			getline(ss, s, init);
			os << s;
			if (ss.good())
			{
				// printf("webserver: found initiator '%c'\n", init);
				getline(ss, s, term);
				if (ss.good())
				{
					if (substitute(s))
					{
						os << s;
					}
					else
					{
						os << init << s << term;
					}
				}
				else
				{
					// printf("webserver: reached end before terminator\n");
					os << init;
					os << s;
				}
			}
			else
			{
				// printf("webserver: reached end before initiator\n");
			}
		}

		response->set_status(200);
		response->set_body(os.str());
	}
	else
	{
		response->set_status(500);
	}
}

void http_manager::add_http_handler(const std::string &path, http_manager::http_handler handler)
{
	if (!m_active) return;

	using namespace std::placeholders;
	m_server->on_get(path, std::bind(on_get, handler, _1, _2));

	std::lock_guard<std::mutex> lock(m_handlers_mutex);
	m_handlers.emplace(path, handler);
}

void http_manager::remove_http_handler(const std::string &path) {
	if (!m_active) return;

	m_server->remove_handler(path);

	std::lock_guard<std::mutex> lock(m_handlers_mutex);
	m_handlers.erase(path);
}

void http_manager::clear() {
	if (!m_active) return;

	m_server->clear();

	std::lock_guard<std::mutex> lock(m_handlers_mutex);
	m_handlers.clear();
}

http_manager::websocket_endpoint_ptr http_manager::add_endpoint(const std::string &path,
		http_manager::websocket_open_handler on_open,
		http_manager::websocket_message_handler on_message,
		http_manager::websocket_close_handler on_close,
		http_manager::websocket_error_handler on_error)  {
	if (!m_active) return std::shared_ptr<websocket_endpoint_impl>(nullptr);

	auto i = m_endpoints.find(path);
	if (i == m_endpoints.end()) {
		using namespace std::placeholders;

		auto &endpoint = m_wsserver->m_endpoint[path];
		webpp::ws_server::Endpoint *endpoint_ptr(&endpoint);
		auto endpoint_impl = std::make_shared<websocket_endpoint_impl>(endpoint_ptr, on_open, on_message, on_close, on_error);

		endpoint.on_open = [&, this, endpoint_impl](std::shared_ptr<webpp::Connection> connection) {
			this->on_open(endpoint_impl, connection);
		};

		endpoint.on_message = [&, this, endpoint_impl](std::shared_ptr<webpp::Connection> connection, std::shared_ptr<webpp::ws_server::Message> message) {
			std::string payload = message->string();
			int opcode = message->fin_rsv_opcode & 0x0f;
			this->on_message(endpoint_impl, connection, payload, opcode);
		};

		endpoint.on_close = [&, this, endpoint_impl](std::shared_ptr<webpp::Connection> connection, int status, const std::string& reason) {
			this->on_close(endpoint_impl, connection, status, reason);
		};

		endpoint.on_error = [&, this, endpoint_impl](std::shared_ptr<webpp::Connection> connection, const std::error_code& error_code) {
			this->on_error(endpoint_impl, connection, error_code);
		};

		m_endpoints[path] = endpoint_impl;
		return endpoint_impl;
	} else {
		return (*i).second;
	}
}

void http_manager::remove_endpoint(const std::string &path) {
	if (!m_active) return;

	m_endpoints.erase(path);
}
