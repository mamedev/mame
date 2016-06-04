// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <string>
#include <uv.h>

extern void GetAddressInfo(const sockaddr * addr, int * family, std::string & ip, uint16_t * port);

class tcp_connection
{
public:
	class listener
	{
	public:
		virtual ~listener() { }
		virtual void on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer) = 0;
	};

	/* Struct for the data field of uv_req_t when writing into the connection. */
	struct tcp_uv_write_data
	{
		tcp_connection* connection;
		uv_write_t     req;
		uint8_t        store[1];
	};

	tcp_connection(size_t bufferSize);
	virtual ~tcp_connection();

	void close();
	void terminate();
	virtual void dump();
	void setup(uv_loop_t* loop, listener* listener, struct sockaddr_storage* localAddr, const std::string &localIP, uint16_t localPort);
	bool is_closing();
	uv_tcp_t* get_uv_handle();
	void start();
	void write(const uint8_t* data, size_t len);
	void write(const uint8_t* data1, size_t len1, const uint8_t* data2, size_t len2);
	void write(const std::string &data);
	const struct sockaddr* get_local_address();
	const std::string& get_local_ip();
	uint16_t get_local_port();
	const struct sockaddr* get_peer_address();
	const std::string& get_peer_ip();
	uint16_t get_peer_port();

private:
	bool set_peer_address();

	/* Callbacks fired by UV events. */
public:
	void on_uv_read_alloc(size_t suggested_size, uv_buf_t* buf);
	void on_uv_read(ssize_t nread, const uv_buf_t* buf);
	void on_uv_write_error(int error);
	void on_uv_shutdown(uv_shutdown_t* req, int status);
	void on_uv_closed();

	/* Pure virtual methods that must be implemented by the subclass. */
protected:
	virtual void user_on_tcp_connection_read() = 0;

private:
	// Passed by argument.
	listener* m_listener;
	// Allocated by this.
	uv_tcp_t* m_uv_handle;
	// Others.
	struct sockaddr_storage* m_local_addr;
	bool m_is_closing;
	bool m_is_closed_by_peer;
	bool m_has_error;

protected:
	// Passed by argument.
	size_t m_buffer_size;
	// Allocated by this.
	uint8_t* m_buffer;
	// Others.
	size_t m_buffer_data_len;
	std::string m_local_ip;
	uint16_t m_local_port;
	struct sockaddr_storage m_peer_addr;
	std::string m_peer_ip;
	uint16_t m_peer_port;
};

/* Inline methods. */

inline bool tcp_connection::is_closing()
{
	return m_is_closing;
}

inline uv_tcp_t* tcp_connection::get_uv_handle()
{
	return m_uv_handle;
}

inline void tcp_connection::write(const std::string &data)
{
	write((const uint8_t*)data.c_str(), data.size());
}

inline const sockaddr* tcp_connection::get_local_address()
{
	return (const struct sockaddr*)m_local_addr;
}

inline const std::string& tcp_connection::get_local_ip()
{
	return m_local_ip;
}

inline uint16_t tcp_connection::get_local_port()
{
	return m_local_port;
}

inline const sockaddr* tcp_connection::get_peer_address()
{
	return (const struct sockaddr*)&m_peer_addr;
}

inline const std::string& tcp_connection::get_peer_ip()
{
	return m_peer_ip;
}

inline uint16_t tcp_connection::get_peer_port()
{
	return m_peer_port;
}

#endif
