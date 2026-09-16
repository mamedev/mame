// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    network.cpp

    Network output module.

***************************************************************************/

#include "output_module.h"

#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"

#include "emu.h"

#include "asio.h"

#include <cassert>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <locale>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <utility>


namespace osd {

namespace {

class output_network : public osd_module, public output_module
{
public:
	output_network() :
		osd_module(OSD_OUTPUT_PROVIDER, "network"),
		output_module(),
		m_acceptor(m_work_context),
		m_shutdown_timeout(m_work_context),
		m_machine(nullptr),
		m_started(false),
		m_stopping(false)
	{
	}

	virtual ~output_network()
	{
		if (!m_started)
			return;

		// doing work in a destructor is bad, but there's no other notification that it's really over
		asio::post(
				m_work_context,
				[this] ()
				{
					m_stopping = true;

					// don't accept any more connections
					std::error_code err;
					if (m_acceptor.is_open())
						m_acceptor.close(err);

					for (auto it = m_clients.begin(); it != m_clients.end(); )
					{
						if (it->send_pending)
						{
							// wait for it to complete
							++it;
						}
						else if (it->receive_pending)
						{
							// will get cleaned up when receive completes
							it->clean_up_socket();
							++it;
						}
						else
						{
							// reap clients with no pending I/O
							it->clean_up_socket();
							it = m_clients.erase(it);
						}
					}

					// set a timer to force shutdown
					if (!m_clients.empty())
					{
						m_shutdown_timeout.expires_after(std::chrono::seconds(10));
						m_shutdown_timeout.async_wait(
								[this] (std::error_code const &err)
								{
									for (auto &client : m_clients)
									{
										// don't bother with orderly shutdown
										assert(client.receive_pending || client.send_pending);
										if (client.socket.is_open())
										{
											std::error_code err;
											client.socket.close(err);
										}
									}
								});
					}
				});
		m_work_thread.join();
	}

	virtual int init(osd_interface &osd, const osd_options &options) override
	{
		{
			std::lock_guard<std::mutex> lock(m_machine_mutex);
			m_machine = &downcast<osd_common_t &>(osd).machine();
			if (m_machine->system().name == std::string_view("___empty"))
				m_machine = nullptr;
		}

		if (m_started)
		{
			if (m_machine)
				broadcast_message(util::string_format("mame_start = %s", m_machine->system().name));
		}
		else
		{
			// allocate some message structures
			try
			{
				m_queued_messages.emplace_back(std::string_view(), 0).first.reserve(MESSAGE_RESERVED_SIZE);
				for (unsigned i = 0; 32 > i; ++i)
					m_free_messages.emplace_back(std::string_view(), 0).first.reserve(MESSAGE_RESERVED_SIZE);
			}
			catch (std::bad_alloc const &)
			{
				osd_printf_error("Output: error allocating message buffers.\n");
				return -1;
			}

			// open a listening socket
			std::error_code err;
			asio::ip::tcp::endpoint local(asio::ip::address_v6::any(), 8000);
			m_acceptor.open(local.protocol(), err);
			if (!err)
				m_acceptor.bind(local, err);
			if (!err)
				m_acceptor.listen(asio::ip::tcp::socket::max_listen_connections, err);
			if (err)
			{
				osd_printf_error("Output: error opening listening socket: %s (%s:%d)\n",
						err.message(), err.category().name(), err.value());
				return -1;
			}

			// start worker thread
			m_work_thread = std::thread(
					[this] ()
					{
						start_accept();
						m_work_context.run();
						osd_printf_verbose("Output: worker thread completed\n");
					});
			m_started = true;
		}

		return 0;
	}

	virtual void exit() override
	{
		if (m_machine)
		{
			{
				std::lock_guard<std::mutex> lock(m_machine_mutex);
				m_machine = nullptr;
			}
			if (m_started)
				broadcast_message("mame_stop = 1");
		}
	}

	// output_module

	virtual void notify(const output_item &item, std::int32_t seconds, std::int64_t attoseconds) override
	{
		if (m_started && m_machine)
			broadcast_message(util::string_format("%s = %d", item.qualified_name(), item.value()));
	}

	virtual void pause() override
	{
		if (m_started && m_machine)
			broadcast_message("pause = 1");
	}

	virtual void resume() override
	{
		if (m_started && m_machine)
			broadcast_message("pause = 0");
	}

	virtual void update() override
	{
		m_emulation_context.restart();
		m_emulation_context.run();
	}

private:
	static constexpr unsigned MESSAGE_RESERVED_SIZE = 128;

	using queued_message = std::pair<std::string, unsigned>;
	using queued_message_list = std::list<queued_message>;
	using queued_message_iterator = queued_message_list::iterator;

	struct client
	{
	public:
		client(asio::ip::tcp::socket &&sock, queued_message_iterator next) :
			socket(std::move(sock)),
			next_message(next),
			received(0),
			queued(0),
			receive_pending(false),
			send_pending(false),
			overlong(false)
		{
		}

		void clean_up_socket()
		{
			if (socket.is_open())
			{
				std::error_code err;
				socket.shutdown(asio::ip::tcp::socket::shutdown_both, err);
				socket.close(err);
			}
		}

		auto make_receive_buffer()
		{
			auto const avail = std::size(receive_buffer) - received;
			assert(0 < avail);
			return asio::buffer(&receive_buffer[received], avail);
		}

		auto make_send_buffer()
		{
			assert(queued);
			return asio::const_buffer(send_buffer, queued);
		}

		void consume_sent(std::size_t length)
		{
			assert(length <= queued);
			std::copy_n(&send_buffer[length], queued - length, &send_buffer[0]);
			queued -= length;
		}

		bool queue_message(std::string_view message)
		{
			if (std::size(send_buffer) < (queued + message.length() + 1))
				return false;

			std::copy_n(message.data(), message.length(), &send_buffer[queued]);
			send_buffer[queued + message.length()] = '\r';
			queued += message.length() + 1;
			return true;
		}

		asio::ip::tcp::socket socket;
		queued_message_iterator next_message;
		std::size_t received;
		std::size_t queued;
		bool receive_pending;
		bool send_pending;
		bool overlong;
		char receive_buffer[256];
		char send_buffer[2048];
	};

	using client_list = std::list<client>;

	running_machine &machine() { return *m_machine; }

	void start_accept()
	{
		m_acceptor.async_accept(
				[this] (std::error_code err, asio::ip::tcp::socket sock)
				{
					if (m_stopping)
						return;

					if (!err)
					{
						sock.set_option(asio::ip::tcp::no_delay(true), err);
						if (err)
						{
							osd_printf_warning("Output: error setting no delay option on connection %s <- %s: %s (%s:%d)\n",
									sock.local_endpoint(), sock.remote_endpoint(),
									err.message(), err.category().name(), err.value());
						}

						try
						{
							assert(!m_queued_messages.empty());
							assert(m_queued_messages.back().first.empty());
							assert(!m_queued_messages.back().second);
							auto const ins = m_clients.emplace(m_clients.end(), std::move(sock), std::prev(m_queued_messages.end()));
							osd_printf_verbose("Output: accepted connection %s <- %s\n",
									ins->socket.local_endpoint(), ins->socket.remote_endpoint());
							start_receive(ins);
							{
								std::lock_guard<std::mutex> lock(m_machine_mutex);
								if (m_machine)
								{
									ins->queue_message(util::string_format("mame_start = %s", m_machine->system().name));
									assert(ins->queued);
								}
							}
							if (ins->queued)
								start_send(ins);
						}
						catch (std::bad_alloc const &)
						{
							osd_printf_error("Output: error allocating error for client connection\n");
						}
					}
					else
					{
						osd_printf_warning("Output: error accepting connection: %s (%s:%d)\n",
								err.message(), err.category().name(), err.value());
					}

					start_accept();
				});
	}

	void start_receive(client_list::iterator it)
	{
		it->receive_pending = true;
		it->socket.async_read_some(
				it->make_receive_buffer(),
				[this, it] (std::error_code const &err, std::size_t length)
				{
					it->receive_pending = false;
					if (err)
					{
						if (it->socket.is_open())
						{
							osd_printf_verbose("Output: error receiving from connection %s <- %s: %s (%s:%s)\n",
									it->socket.local_endpoint(), it->socket.remote_endpoint(),
									err.message(), err.category().name(), err.value());
						}
						forget_client(it);
					}
					else if (!length)
					{
						osd_printf_verbose("Output: received end of stream from connection %s <- %s\n",
								it->socket.local_endpoint(), it->socket.remote_endpoint());
						forget_client(it);
					}
					else
					{
						check_command(it, length);
						if (!m_stopping)
						{
							start_receive(it);
						}
						else if (!it->send_pending)
						{
							forget_client(it);
						}
					}
				});
	}

	void start_send(client_list::iterator it)
	{
		assert(it->queued);
		assert(!it->send_pending);

		it->send_pending = true;
		it->socket.async_write_some(
				it->make_send_buffer(),
				[this, it] (std::error_code const &err, std::size_t length)
				{
					it->send_pending = false;
					if (err)
					{
						if (it->socket.is_open())
						{
							osd_printf_verbose("Output: sending to connection %s <- %s: %s (%s:%s)\n",
									it->socket.local_endpoint(), it->socket.remote_endpoint(),
									err.message(), err.category().name(), err.value());
						}

						while (!it->next_message->first.empty())
						{
							assert(it->next_message->second);
							auto const done = it->next_message++;
							if (!--done->second)
							{
								done->first.clear();
								m_free_messages.splice(m_free_messages.begin(), m_queued_messages, done);
							}
						}
						assert(!it->next_message->second);
						++it->next_message;
						assert(m_queued_messages.end() == it->next_message);

						forget_client(it);
					}
					else
					{
						it->consume_sent(length);
						while (!it->next_message->first.empty())
						{
							assert(it->next_message->second);
							if (it->queue_message(it->next_message->first) || !it->queued)
							{
								auto const done = it->next_message++;
								if (!--done->second)
								{
									done->first.clear();
									m_free_messages.splice(m_free_messages.begin(), m_queued_messages, done);
								}
							}
							else
							{
								break;
							}
						}

						if (m_stopping)
						{
							while (!it->next_message->first.empty())
							{
								assert(it->next_message->second);
								auto const done = it->next_message++;
								if (!--done->second)
								{
									done->first.clear();
									m_free_messages.splice(m_free_messages.begin(), m_queued_messages, done);
								}
							}
							assert(!it->next_message->second);
							++it->next_message;
							assert(m_queued_messages.end() == it->next_message);

							forget_client(it);
						}
						else if (it->queued)
						{
							start_send(it);
						}
					}
				});

	}

	void forget_client(client_list::iterator it)
	{
		it->clean_up_socket();
		if (!it->receive_pending && !it->send_pending)
		{
			m_clients.erase(it);
			if (m_clients.empty())
				m_shutdown_timeout.cancel();
		}
	}

	void check_command(client_list::iterator it, std::size_t length)
	{
		assert(std::size(it->receive_buffer) >= (it->received + length));
		while (length)
		{
			// see if the newly received portion contains a CR or LF
			std::size_t i = 0;
			while ((i < length) && ('\r' != it->receive_buffer[it->received + i]) && ('\n' != it->receive_buffer[it->received + i]))
				++i;

			if (std::size(it->receive_buffer) == (it->received + i))
			{
				// ignore overly long lines
				length = 0;
				it->received = 0;
				it->overlong = true;
			}
			else if (i == length)
			{
				// incomplete command
				if (!it->overlong)
					it->received += length;
				length = 0;
			}
			else
			{
				if (it->overlong)
				{
					// end of overly long line
					it->overlong = false;
				}
				else
				{
					// process command
					handle_command(std::string_view(it->receive_buffer, it->received + i));
				}

				// move leftovers up to the front of the buffer
				std::copy_n(&it->receive_buffer[it->received + i + 1], length - i - 1, &it->receive_buffer[0]);
				it->received = 0;
				length -= i + 1;
			}
		}
	}

	void handle_command(std::string_view message)
	{
		// see if the first word is what we expect
		{
			auto const delimiter = message.find("=");
			if (std::string_view::npos == delimiter)
				return;
			std::string_view name = message.substr(0, delimiter);
			while (!name.empty() && (name.back() == ' '))
				name.remove_suffix(1);
			if (name != "mame_message")
				return;

			// strip off the name, delimiter and leading whitespace
			message.remove_prefix(delimiter + 1);
			auto const nonwhite = message.find_first_not_of(" ");
			if (std::string_view::npos == nonwhite)
				return;
			message.remove_prefix(nonwhite);
		}

		// split it at commas
		auto const delim1 = message.find(",");
		if (std::string_view::npos == delim1)
			return;
		std::string_view const command = message.substr(0, delim1);
		message.remove_prefix(delim1 + 1);
		auto const delim2 = message.find(",");
		std::string_view const value = (std::string_view::npos != delim2) ? message.substr(0, delim2) : message;

		// parse numbers if possible
		int id, valueval;
		try
		{
			std::istringstream str;
			str.imbue(std::locale::classic());
			str.str(std::string(command));
			str >> id;
			if (!str)
				return;
			str.str(std::string(value));
			str.seekg(0);
			str >> valueval;
			if (!str)
				return;
		}
		catch (...)
		{
			return;
		}

		// handle commands
		switch (id)
		{
		case IM_MAME_PAUSE:
			asio::post(
					m_emulation_context,
					[this, valueval] ()
					{
						if (m_machine)
						{
							if ((1 == valueval) && !m_machine->paused())
								m_machine->pause();
							else if ((0 == valueval) && m_machine->paused())
								m_machine->resume();
						}
					});
			break;
		case IM_MAME_SAVESTATE:
			asio::post(
					m_emulation_context,
					[this, valueval] ()
					{
						if (m_machine)
						{
							if (0 == valueval)
								m_machine->schedule_load("auto");
							else if (1 == valueval)
								m_machine->schedule_save("auto");
						}
					});
			break;
		}
	}

	void broadcast_message(std::string &&message)
	{
		asio::post(
				m_work_context,
				[this, m = std::move(message)] ()
				{
					if (m_stopping || m_clients.empty())
						return;

					// the last buffer is always a placeholder for the next message
					assert(!m.empty());
					assert(!m_queued_messages.empty());
					auto const buffer = std::prev(m_queued_messages.end());
					assert(buffer->first.empty());
					assert(!buffer->second);

					// queue the message
					try
					{
						if (m_free_messages.empty())
							m_free_messages.emplace_back(std::string_view(), 0).first.reserve(MESSAGE_RESERVED_SIZE);
						assert(m_free_messages.front().first.empty());
						assert(!m_free_messages.front().second);
						buffer->first = m;
						m_queued_messages.splice(m_queued_messages.end(), m_free_messages, m_free_messages.begin());
					}
					catch (std::bad_alloc const &)
					{
						return;
					}

					// try to push it out to clients
					for (auto it = m_clients.begin(); it != m_clients.end(); ++it)
					{
						if (buffer == it->next_message)
						{
							if (it->queue_message(buffer->first))
							{
								++it->next_message;
								if (!it->send_pending)
									start_send(it);
							}
							else if (!it->queued)
							{
								// message is unreasonably long, drop it
								++it->next_message;
							}
							else
							{
								// increase outstanding client count
								++buffer->second;
							}
						}
						else if (m_queued_messages.end() != it->next_message)
						{
							// client already has other messages waiting
							assert(it->send_pending);
							++buffer->second;
						}
					}

					// drop the message if all clients have queued it locally
					if (!buffer->second)
					{
						buffer->first.clear();
						m_free_messages.splice(m_free_messages.begin(), m_queued_messages, buffer);
					}
				});
	}

	std::mutex m_machine_mutex;
	std::thread m_work_thread;
	asio::io_context m_work_context;
	asio::io_context m_emulation_context;
	asio::ip::tcp::acceptor m_acceptor;
	asio::steady_timer m_shutdown_timeout;
	client_list m_clients;
	queued_message_list m_queued_messages;
	queued_message_list m_free_messages;
	running_machine *m_machine;
	bool m_started;
	bool m_stopping;
};

} // anonymous namespace

} // namespace osd

MODULE_DEFINITION(OUTPUT_NETWORK, osd::output_network)
