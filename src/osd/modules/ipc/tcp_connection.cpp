// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#include "emu.h"
#include "tcp_connection.h"
#include <cstdint>  // uint8_t, etc
#include <cstdlib>  // std::malloc(), std::free()

/* Static methods for UV callbacks. */

static inline void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	static_cast<tcp_connection*>(handle->data)->on_uv_read_alloc(suggested_size, buf);
}

static inline void on_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
	static_cast<tcp_connection*>(handle->data)->on_uv_read(nread, buf);
}

static inline void on_write(uv_write_t* req, int status)
{
	tcp_connection::tcp_uv_write_data* write_data = static_cast<tcp_connection::tcp_uv_write_data*>(req->data);
	tcp_connection* connection = write_data->connection;

	// Delete the UvWriteData struct (which includes the uv_req_t and the store char[]).
	std::free(write_data);

	// Just notify the TCPConnection when error.
	if (status)
		connection->on_uv_write_error(status);
}

static inline void on_shutdown(uv_shutdown_t* req, int status)
{
	static_cast<tcp_connection*>(req->data)->on_uv_shutdown(req, status);
}

static inline void on_close(uv_handle_t* handle)
{
	static_cast<tcp_connection*>(handle->data)->on_uv_closed();
}

/* Instance methods. */

tcp_connection::tcp_connection(size_t bufferSize) :
	m_listener(nullptr),
	m_local_addr(nullptr),
	m_is_closing(false),
	m_is_closed_by_peer(false),
	m_has_error(false),
	m_buffer_size(bufferSize),
	m_buffer(nullptr),
	m_buffer_data_len(0),
	m_local_port(0),
	m_peer_port(0)
{
	m_uv_handle = new uv_tcp_t;
	m_uv_handle->data = (void*)this;

	// NOTE: Don't allocate the buffer here. Instead wait for the first uv_alloc_cb().
}

tcp_connection::~tcp_connection()
{
	if (m_uv_handle)
		delete m_uv_handle;
	if (m_buffer)
		delete[] m_buffer;
}

void tcp_connection::setup(uv_loop_t* loop, listener* listener, struct sockaddr_storage* localAddr, const std::string &localIP, uint16_t localPort)
{
	int err;

	// Set the UV handle.
	err = uv_tcp_init(loop, m_uv_handle);
	if (err)
	{
		delete m_uv_handle;
		m_uv_handle = nullptr;
		throw emu_fatalerror("uv_tcp_init() failed: %s", uv_strerror(err));
	}

	// Set the listener.
	m_listener = listener;

	// Set the local address.
	m_local_addr = localAddr;
	m_local_ip = localIP;
	m_local_port = localPort;
}

void tcp_connection::close()
{
	if (m_is_closing)
		return;

	int err;

	m_is_closing = true;

	// Don't read more.
	err = uv_read_stop((uv_stream_t*)m_uv_handle);
	if (err)
		throw emu_fatalerror("uv_read_stop() failed: %s", uv_strerror(err));

	// If there is no error and the peer didn't close its connection side then close gracefully.
	if (!m_has_error && !m_is_closed_by_peer)
	{
		// Use uv_shutdown() so pending data to be written will be sent to the peer
		// before closing.
		uv_shutdown_t* req = new uv_shutdown_t;
		req->data = (void*)this;
		err = uv_shutdown(req, (uv_stream_t*)m_uv_handle, (uv_shutdown_cb)on_shutdown);
		if (err)
			throw emu_fatalerror("uv_shutdown() failed: %s", uv_strerror(err));
	}
	// Otherwise directly close the socket.
	else
	{
		uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_close);
	}
}

void tcp_connection::terminate()
{
	if (m_is_closing)
		return;
	m_is_closing = true;
	uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_close);
}

void tcp_connection::dump()
{
	osd_printf_verbose("[TCP, local:%s :%d, remote:%s :%d, status:%s]\r\n",
		m_local_ip.c_str(), (uint16_t)m_local_port,
		m_peer_ip.c_str(), (uint16_t)m_peer_port,
		(!m_is_closing) ? "open" : "closed");
}

void tcp_connection::start()
{
	if (m_is_closing)
		return;

	int err;

	err = uv_read_start((uv_stream_t*)m_uv_handle, (uv_alloc_cb)on_alloc, (uv_read_cb)on_read);
	if (err)
		throw emu_fatalerror("uv_read_start() failed: %s", uv_strerror(err));

	// Get the peer address.
	if (!set_peer_address())
		throw emu_fatalerror("error setting peer IP and port");
}

void tcp_connection::write(const uint8_t* data, size_t len)
{
	if (m_is_closing)
		return;

	if (len == 0)
		return;

	uv_buf_t buffer;
	int written;
	int err;

	// First try uv_try_write(). In case it can not directly write all the given
	// data then build a uv_req_t and use uv_write().

	buffer = uv_buf_init((char*)data, len);
	written = uv_try_write((uv_stream_t*)m_uv_handle, &buffer, 1);

	// All the data was written. Done.
	if (written == (int)len)
	{
		return;
	}
	// Cannot write any data at first time. Use uv_write().
	else if (written == UV_EAGAIN || written == UV_ENOSYS)
	{
		// Set written to 0 so pending_len can be properly calculated.
		written = 0;
	}
	// Error. Should not happen.
	else if (written < 0)
	{
		osd_printf_warning("uv_try_write() failed, closing the connection: %s\n", uv_strerror(written));

		close();
		return;
	}

	// osd_printf_info("could just write %zu bytes (%zu given) at first time, using uv_write() now", (size_t)written, len);

	size_t pending_len = len - written;

	// Allocate a special UvWriteData struct pointer.
	tcp_uv_write_data* write_data = (tcp_uv_write_data*)std::malloc(sizeof(tcp_uv_write_data) + pending_len);

	write_data->connection = this;
	std::memcpy(write_data->store, data + written, pending_len);
	write_data->req.data = (void*)write_data;

	buffer = uv_buf_init((char*)write_data->store, pending_len);

	err = uv_write(&write_data->req, (uv_stream_t*)m_uv_handle, &buffer, 1, (uv_write_cb)on_write);
	if (err)
		throw emu_fatalerror("uv_write() failed: %s", uv_strerror(err));
}

void tcp_connection::write(const uint8_t* data1, size_t len1, const uint8_t* data2, size_t len2)
{
	if (m_is_closing)
		return;

	if (len1 == 0 && len2 == 0)
		return;

	size_t total_len = len1 + len2;
	uv_buf_t buffers[2];
	int written;
	int err;

	// First try uv_try_write(). In case it can not directly write all the given
	// data then build a uv_req_t and use uv_write().

	buffers[0] = uv_buf_init((char*)data1, len1);
	buffers[1] = uv_buf_init((char*)data2, len2);
	written = uv_try_write((uv_stream_t*)m_uv_handle, buffers, 2);

	// All the data was written. Done.
	if (written == (int)total_len)
	{
		return;
	}
	// Cannot write any data at first time. Use uv_write().
	else if (written == UV_EAGAIN || written == UV_ENOSYS)
	{
		// Set written to 0 so pending_len can be properly calculated.
		written = 0;
	}
	// Error. Should not happen.
	else if (written < 0)
	{
		osd_printf_warning("uv_try_write() failed, closing the connection: %s\n", uv_strerror(written));

		close();
		return;
	}

	// osd_printf_info("could just write %zu bytes (%zu given) at first time, using uv_write() now", (size_t)written, total_len);

	size_t pending_len = total_len - written;

	// Allocate a special UvWriteData struct pointer.
	tcp_uv_write_data* write_data = (tcp_uv_write_data*)std::malloc(sizeof(tcp_uv_write_data) + pending_len);

	write_data->connection = this;
	// If the first buffer was not entirely written then splice it.
	if ((size_t)written < len1)
	{
		std::memcpy(write_data->store, data1 + (size_t)written, len1 - (size_t)written);
		std::memcpy(write_data->store + (len1 - (size_t)written), data2, len2);
	}
	// Otherwise just take the pending data in the second buffer.
	else
	{
		std::memcpy(write_data->store, data2 + ((size_t)written - len1), len2 - ((size_t)written - len1));
	}
	write_data->req.data = (void*)write_data;

	uv_buf_t buffer = uv_buf_init((char*)write_data->store, pending_len);

	err = uv_write(&write_data->req, (uv_stream_t*)m_uv_handle, &buffer, 1, (uv_write_cb)on_write);
	if (err)
		throw emu_fatalerror("uv_write() failed: %s", uv_strerror(err));
}

void GetAddressInfo(const struct sockaddr* addr, int* family, std::string &ip, uint16_t* port)
{
	char _ip[INET6_ADDRSTRLEN + 1];
	int err;


	err = uv_inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, _ip, INET_ADDRSTRLEN);
	if (err)
		throw emu_fatalerror("uv_inet_ntop() failed: %s", uv_strerror(err));
	*port = (uint16_t)ntohs(((struct sockaddr_in*)addr)->sin_port);

	*family = addr->sa_family;
	ip.assign(_ip);
}
bool tcp_connection::set_peer_address()
{
	int err;
	int len = sizeof(m_peer_addr);

	err = uv_tcp_getpeername(m_uv_handle, (struct sockaddr*)&m_peer_addr, &len);
	if (err)
	{
		osd_printf_error("uv_tcp_getpeername() failed: %s\n", uv_strerror(err));

		return false;
	}

	int family;
	GetAddressInfo((const struct sockaddr*)&m_peer_addr, &family, m_peer_ip, &m_peer_port);

	return true;
}

void tcp_connection::on_uv_read_alloc(size_t suggested_size, uv_buf_t* buf)
{
	// If this is the first call to onUvReadAlloc() then allocate the receiving buffer now.
	if (!m_buffer)
		m_buffer = new uint8_t[m_buffer_size];

	// Tell UV to write after the last data byte in the buffer.
	buf->base = (char *)(m_buffer + m_buffer_data_len);
	// Give UV all the remaining space in the buffer.
	if (m_buffer_size > m_buffer_data_len)
	{
		buf->len = m_buffer_size - m_buffer_data_len;
	}
	else
	{
		buf->len = 0;

		osd_printf_warning("no available space in the buffer\n");
	}
}

void tcp_connection::on_uv_read(::ssize_t nread, const uv_buf_t* buf)
{
	if (m_is_closing)
		return;

	if (nread == 0)
		return;

	// Data received.
	if (nread > 0)
	{
		// Update the buffer data length.
		m_buffer_data_len += (size_t)nread;

		// Notify the subclass.
		user_on_tcp_connection_read();
	}
	// Client disconneted.
	else if (nread == UV_EOF || nread == UV_ECONNRESET)
	{
		osd_printf_verbose("connection closed by peer, closing server side\n");

		m_is_closed_by_peer = true;

		// Close server side of the connection.
		close();
	}
	// Some error.
	else
	{
		osd_printf_verbose("read error, closing the connection: %s\n", uv_strerror(nread));

		m_has_error = true;

		// Close server side of the connection.
		close();
	}
}

void tcp_connection::on_uv_write_error(int error)
{
	if (m_is_closing)
		return;

	if (error == UV_EPIPE || error == UV_ENOTCONN)
	{
		osd_printf_verbose("write error, closing the connection: %s\n", uv_strerror(error));
	}
	else
	{
		osd_printf_verbose("write error, closing the connection: %s\n", uv_strerror(error));

		m_has_error = true;
	}

	close();
}

void tcp_connection::on_uv_shutdown(uv_shutdown_t* req, int status)
{
	delete req;

	if (status == UV_EPIPE || status == UV_ENOTCONN || status == UV_ECANCELED)
		osd_printf_verbose("shutdown error: %s\n", uv_strerror(status));
	else if (status)
		osd_printf_verbose("shutdown error: %s\n", uv_strerror(status));

	// Now do close the handle.
	uv_close((uv_handle_t*)m_uv_handle, (uv_close_cb)on_close);
}

void tcp_connection::on_uv_closed()
{
	// Notify the listener.
	m_listener->on_tcp_connection_closed(this, m_is_closed_by_peer);

	// And delete this.
	delete this;
}
