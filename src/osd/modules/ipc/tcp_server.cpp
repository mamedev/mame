// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#include "emu.h"
#include "tcp_server.h"

/* Static methods for UV callbacks. */

static inline void on_connection(uv_stream_t* handle, int status)
{
	static_cast<tcp_server*>(handle->data)->on_uv_connection(status);
}

static inline void on_close(uv_handle_t* handle)
{
	static_cast<tcp_server*>(handle->data)->on_uv_closed();
}

static inline void on_error_close(uv_handle_t* handle)
{
	delete handle;
}

/* Instance methods. */

tcp_server::tcp_server(uv_loop_t* loop, const std::string &ip, uint16_t port, int backlog)
	: m_uv_handle(nullptr),
	m_loop(nullptr),
	m_is_closing(false),
	m_local_port(0)
{
	int err;
	int flags = 0;

	m_uv_handle = new uv_tcp_t;
	m_uv_handle->data = (void*)this;
	m_loop = loop;

	err = uv_tcp_init(loop, m_uv_handle);
	if (err)
	{
		delete m_uv_handle;
		m_uv_handle = nullptr;
		throw emu_fatalerror("uv_tcp_init() failed: %s", uv_strerror(err));
	}

	struct sockaddr_storage bind_addr;


	err = uv_ip4_addr(ip.c_str(), (int)port, (struct sockaddr_in*)&bind_addr);
	if (err)
		throw emu_fatalerror("uv_ipv4_addr() failed: %s", uv_strerror(err));

	err = uv_tcp_bind(m_uv_handle, (const struct sockaddr*)&bind_addr, flags);
	if (err)
	{
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_error_close);
		throw emu_fatalerror("uv_tcp_bind() failed: %s", uv_strerror(err));
	}

	err = uv_listen((uv_stream_t*)m_uv_handle, backlog, (uv_connection_cb)on_connection);
	if (err)
	{
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_error_close);
		throw emu_fatalerror("uv_listen() failed: %s", uv_strerror(err));
	}

	// Set local address.
	if (!set_local_address())
	{
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_error_close);
		throw emu_fatalerror("error setting local IP and port");
	}
}

tcp_server::~tcp_server()
{
	if (m_uv_handle)
		delete m_uv_handle;
}

void tcp_server::close()
{
	if (m_is_closing)
		return;

	m_is_closing = true;

	// If there are no connections then close now.
	if (m_connections.empty())
	{
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_close);
	}
	// Otherwise close all the connections (but not the TCP server).
	else
	{
		osd_printf_info("closing %d active connections", (int)m_connections.size());

		for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
		{
			tcp_connection* connection = *it;
			connection->close();
		}
	}
}

void tcp_server::terminate()
{
	if (m_is_closing)
		return;

	m_is_closing = true;

	// If there are no connections then close now.
	if (m_connections.empty())
	{
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_close);
	}
	// Otherwise close all the connections (but not the TCP server).
	else
	{
		osd_printf_info("closing %d active connections", (int)m_connections.size());

		for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
		{
			tcp_connection* connection = *it;
			connection->terminate();
		}
	}
}

void tcp_server::send_to_all(const uint8_t* data, size_t len)
{
	// If there are no connections then close now.
	if (!m_connections.empty())
	{
		for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
		{
			tcp_connection* connection = *it;
			connection->write(data, len);
		}
	}
}

void tcp_server::dump()
{
	osd_printf_info("[TCP, local:%s :%d, status:%s, connections:%d]",
		m_local_ip.c_str(), (uint16_t)m_local_port,
		(!m_is_closing) ? "open" : "closed",
		(int)m_connections.size());
}

const sockaddr* tcp_server::get_local_address()
{
	return (const struct sockaddr*)&m_local_addr;
}

const std::string& tcp_server::get_local_ip()
{
	return m_local_ip;
}

uint16_t tcp_server::get_local_port()
{
	return m_local_port;
}

bool tcp_server::set_local_address()
{
	int err;
	int len = sizeof(m_local_addr);

	err = uv_tcp_getsockname(m_uv_handle, (struct sockaddr*)&m_local_addr, &len);
	if (err)
	{
		osd_printf_error("uv_tcp_getsockname() failed: %s", uv_strerror(err));

		return false;
	}

	int family;
	GetAddressInfo((const struct sockaddr*)&m_local_addr, &family, m_local_ip, &m_local_port);

	return true;
}

inline void tcp_server::on_uv_connection(int status)
{
	if (m_is_closing)
		return;

	int err;

	if (status)
	{
		osd_printf_error("error while receiving a new TCP connection: %s", uv_strerror(status));

		return;
	}

	// Notify the subclass so it provides an allocated derived class of TCPConnection.
	tcp_connection* connection = nullptr;
	user_on_tcp_connection_alloc(&connection);
	if (connection != nullptr)
		osd_printf_error("tcp_server pointer was not allocated by the user");

	try
	{
		connection->setup(m_loop, this, &(m_local_addr), m_local_ip, m_local_port);
	}
	catch (...)
	{
		delete connection;
		return;
	}

	// Accept the connection.
	err = uv_accept((uv_stream_t*)m_uv_handle, (uv_stream_t*)connection->get_uv_handle());
	if (err)
		throw emu_fatalerror("uv_accept() failed: %s", uv_strerror(err));

	// Insert the TCPConnection in the set.
	m_connections.insert(connection);

	// Start receiving data.
	try
	{
		connection->start();
	}
	catch (emu_exception &error)
	{
		osd_printf_error("cannot run the TCP connection, closing the connection: %s", error.what());
		connection->close();
		// NOTE: Don't return here so the user won't be notified about a "onclose" for a TCP connection
		// for which there was not a previous "onnew" event.
	}

	osd_printf_info("new TCP connection:");
	connection->dump();

	// Notify the subclass.
	user_on_new_tcp_connection(connection);
}

void tcp_server::on_uv_closed()
{
	// Motify the subclass.
	user_on_tcp_server_closed();
}

void tcp_server::on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer)
{
	// NOTE:
	// Worst scenario is that in which this is the latest connection,
	// which is remotely closed (no tcp_server.Close() was called) and the user
	// call tcp_server.Close() on userOnTCPConnectionClosed() callback, so Close()
	// is called with zero connections and calls uv_close(), but then
	// onTCPConnectionClosed() continues and finds that isClosing is true and
	// there are zero connections, so calls uv_close() again and get a crash.
	//
	// SOLUTION:
	// Check isClosing value *before* onTCPConnectionClosed() callback.

	bool wasClosing = m_is_closing;

	osd_printf_info("TCP connection closed:");
	connection->dump();

	// Remove the TCPConnection from the set.
	m_connections.erase(connection);

	// Notify the subclass.
	user_on_tcp_connection_closed(connection, is_closed_by_peer);

	// Check if the server was closing connections, and if this is the last
	// connection then close the server now.
	if (wasClosing && m_connections.empty())
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_close);
}
