// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

main.cpp

Controls execution of the core MAME system.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "main.h"
#include "server_ws.hpp"
#include "server_http.hpp"
#include <fstream>

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

machine_manager::machine_manager(emu_options& options, osd_interface& osd)
  : m_osd(osd),
	m_options(options),
	m_machine(nullptr),
	m_io_context(std::make_shared<asio::io_context>())
{
}

machine_manager::~machine_manager()
{
	if (options().http())
		m_server->stop();
	if (m_server_thread.joinable())
		m_server_thread.join();
}

void machine_manager::start_http_server()
{
	if (options().http())
	{
		m_server = std::make_unique<webpp::http_server>();
		m_server->m_config.port = options().http_port();
		m_server->set_io_context(m_io_context);
		m_wsserver = std::make_unique<webpp::ws_server>();

		auto& endpoint = m_wsserver->endpoint["/"];

		m_server->on_get([this](auto response, auto request) {
			std::string doc_root = this->options().http_root();

			std::string path = request->path;
			// If path ends in slash (i.e. is a directory) then add "index.html".
			if (path[path.size() - 1] == '/')
			{
				path += "index.html";
			}

			std::size_t last_qmark_pos = path.find_last_of("?");
			if (last_qmark_pos != std::string::npos)
				path = path.substr(0, last_qmark_pos - 1);

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

			// Fill out the reply to be sent to the client.
			std::string content;
			char buf[512];
			while (is.read(buf, sizeof(buf)).gcount() > 0)
				content.append(buf, size_t(is.gcount()));

			response->type(extension_to_type(extension));
			response->status(200).send(content);

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
	}
}

void machine_manager::start_context()
{
	m_server_thread = std::thread([this]() {
		m_io_context->run();
	});
}

