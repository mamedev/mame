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

//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

// ======================> http_manager
namespace asio
{
	class io_context;
}
namespace webpp
{
	class http_server;
	class ws_server;
}

class http_manager
{
	DISABLE_COPYING(http_manager);
public:
	http_manager(bool active, short port, const char *root);
	virtual ~http_manager();

	void clear() { m_handlers.clear(); update(); }
	void add(const char *url, std::function<std::tuple<std::string,int,std::string>(std::string)> func) { m_handlers.emplace(url, func); }
	void update();
private:
	std::shared_ptr<asio::io_context>   m_io_context;
	std::unique_ptr<webpp::http_server> m_server;
	std::unique_ptr<webpp::ws_server>   m_wsserver;
	std::thread                         m_server_thread;
	std::unordered_map<const char *, std::function<std::tuple<std::string, int, std::string>(std::string)>> m_handlers;
};


#endif  /* MAME_EMU_HTTP_H */
