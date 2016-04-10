// license:BSD-3-Clause
// copyright-holders:Inaki Baz Castillo,Miodrag Milanovic

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "tcp_connection.h"
#include <string>
#include <unordered_set>
#include <uv.h>

class tcp_server : public tcp_connection::listener
{
public:
	tcp_server(uv_loop_t* m_loop, const std::string &ip, uint16_t port, int backlog);
	virtual ~tcp_server();

	void close();
	void terminate();
	virtual void dump();
	void send_to_all(const uint8_t* data, size_t len);
	bool is_closing();
	const struct sockaddr* get_local_address();
	const std::string& get_local_ip();
	uint16_t get_local_port();
	size_t get_num_connections();

private:
	bool set_local_address();

	/* Pure virtual methods that must be implemented by the subclass. */
protected:
	virtual void user_on_tcp_connection_alloc(tcp_connection** connection) = 0;
	virtual void user_on_new_tcp_connection(tcp_connection* connection) = 0;
	virtual void user_on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer) = 0;
	virtual void user_on_tcp_server_closed() = 0;

	/* Callbacks fired by UV events. */
public:
	void on_uv_connection(int status);
	void on_uv_closed();

	/* Methods inherited from tcp_connection::listener. */
	virtual void on_tcp_connection_closed(tcp_connection* connection, bool is_closed_by_peer) override;

private:
	// Allocated by this (may be passed by argument).
	uv_tcp_t* m_uv_handle;
	uv_loop_t* m_loop;
	// Others.
	std::unordered_set<tcp_connection*> m_connections;
	bool m_is_closing;

protected:
	struct sockaddr_storage m_local_addr;
	std::string m_local_ip;
	uint16_t m_local_port;
};

/* Inline methods. */

inline bool tcp_server::is_closing()
{
	return m_is_closing;
}

inline size_t tcp_server::get_num_connections()
{
	return m_connections.size();
}

#endif
