// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

http.cpp

HTTP server handling

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_HTTP_H
#define MAME_EMU_HTTP_H

#include <thread>
#include <time.h>
#include "server_http.hpp"
#include "server_ws.hpp"

//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

// ======================> http_manager
namespace asio
{
	class io_context;
}

class http_manager
{
	DISABLE_COPYING(http_manager);
public:

	/** An HTTP Request. */
	struct http_request
	{
		/** Retrieves the requested resource. */
		virtual const std::string get_resource() = 0; // The entire resource: path, query and fragment.

		/** Returns the path part of the requested resource. */
		virtual const std::string get_path() = 0;

		/** Returns the query part of the requested resource. */
		virtual const std::string get_query() = 0;

		/** Returns the fragment part of the requested resource. */
		virtual const std::string get_fragment() = 0;

		/** Retrieves a header from the HTTP request. */
		virtual const std::string get_header(const std::string &header_name) = 0;

		/** Retrieves a header from the HTTP request. */
		virtual const std::list<std::string> get_headers(const std::string &header_name) = 0;

		/** Returns the body that was submitted with the HTTP request. */
		virtual const std::string get_body() = 0;
	};
	typedef std::shared_ptr<http_request> http_request_ptr;

	/** An HTTP response. */
	struct http_response
	{
		/** Sets the HTTP status to be returned to the client. */
		virtual void set_status(int status) = 0;

		/** Sets the HTTP content type to be returned to the client. */
		virtual void set_content_type(const std::string &type) = 0;

		/** Sets the body to be sent to the client. */
		virtual void set_body(const std::string &body) = 0;

		/** Appends something to the body to be sent to the client. */
		virtual void append_body(const std::string &body) = 0;
	};
	typedef std::shared_ptr<http_response> http_response_ptr;

	/** Identifies a Websocket connection. */
	struct websocket_connection
	{
		/** Sends a message to the client that is connected on the other end of this Websocket connection. */
		virtual void send_message(const std::string &payload, int opcode) = 0;

		/** Closes this open Websocket connection. */
		virtual void close() = 0;

		virtual ~websocket_connection() { }
	};
	typedef std::shared_ptr<websocket_connection> websocket_connection_ptr;

	/** Handles opening an incoming Websocket connection. */
	typedef std::function<void(websocket_connection_ptr connection)> websocket_open_handler;

	/** Handles an incoming message on an open Websocket connection. */
	typedef std::function<void(websocket_connection_ptr connection, const std::string &payload, int opcode)> websocket_message_handler;

	/** Handles when the client has closed a websocket connection. */
	typedef std::function<void(websocket_connection_ptr connection, int status, const std::string& reason)> websocket_close_handler;

	/** Handles when there has been an error on a websocket connection. */
	typedef std::function<void(websocket_connection_ptr connection, const std::error_code& error_code)> websocket_error_handler;

	/** Handles an incoming HTTP request, generating an HTTP response. */
	typedef std::function<void(http_request_ptr, http_response_ptr)> http_handler;

	struct websocket_endpoint : public std::enable_shared_from_this<websocket_endpoint> {
		websocket_open_handler on_open;
		websocket_message_handler on_message;
		websocket_close_handler on_close;
		websocket_error_handler on_error;
	};
	typedef std::shared_ptr<websocket_endpoint> websocket_endpoint_ptr;

	/** Substitutes one string with another, and returns whether the substitution should be performed.
	 * Used when evaluating a template. */
	typedef std::function<bool(std::string &)> substitution;

	http_manager(bool active, short port, const char *root);
	virtual ~http_manager();
	void clear();

	/** Adds a template to the web server. When the path is requested, the template will be read and parsed:
	 * strings between each pair of <init, term> characters will be passed to the substitute function and the result
	 * will be used instead.
	 */
	void add_template(const std::string &path, substitution substitute, char init, char term);

	/** Removes a template from the web server. */
	void remove_template(const std::string &path);

	/** Serves a template at an explicit path, which may be diffrent from the request path, under the document root.
	 * The template will be read and parsed:
	 * strings between each pair of <init, term> characters will be passed to the substitute function and the result
	 * will be used instead.
	 */
	void serve_template(http_request_ptr request, http_response_ptr response, const std::string &path, substitution substitute, char init, char term);

	void serve_document(http_request_ptr request, http_response_ptr response, const std::string &path);

	/** Adds an HTTP handler. When the specified path is requested, the specified HTTP handler will be called. */
	void add_http_handler(const std::string &path, http_handler handler);

	/** Removes the HTTP handler at the specified path. */
	void remove_http_handler(const std::string &path);

	/** Retrieves a websocket endpoint, possibly adding it if it does not already exist. */
	websocket_endpoint_ptr add_endpoint(const std::string &path, websocket_open_handler on_open, websocket_message_handler on_message, websocket_close_handler on_close, websocket_error_handler on_error);

	/** Removes the websocket endpoint at the specified path. */
	void remove_endpoint(const std::string &path);

	bool is_active() {
		return m_active;
	}

private:
	void on_open(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection);

	void on_message(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection, const std::string& payload, int opcode);

	void on_close(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection, int status, const std::string& reason);

	void on_error(http_manager::websocket_endpoint_ptr endpoint, std::shared_ptr<webpp::Connection> connection, const std::error_code& error_code);

	bool read_file(std::ostream &os, const std::string &path);

	bool m_active;

	std::shared_ptr<asio::io_context>   m_io_context;
	std::unique_ptr<webpp::http_server> m_server;
	std::unique_ptr<webpp::ws_server>   m_wsserver;
	std::thread                         m_server_thread;
	std::string                         m_root;

	std::unordered_map<std::string, http_handler> m_handlers;
	std::mutex                                    m_handlers_mutex;

	std::unordered_map<std::string, websocket_endpoint_ptr> m_endpoints;
	std::mutex                                              m_endpoints_mutex;

	std::unordered_map<void *, websocket_connection_ptr> m_connections;  // the keys are really webpp::ws_server::Connection pointers
	std::mutex                                           m_connections_mutex;

};


#endif  /* MAME_EMU_HTTP_H */
