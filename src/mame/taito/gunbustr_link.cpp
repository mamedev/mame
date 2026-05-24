// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

  High-level simulation of Taito Gunbuster communication.

  (Thanks to Alex "trap15" Marshall for investigating the protocol and
  writing a proof of concept implementation.)

  To use this:
  * Enable Link Simulation in MAME's Machine Configuration menu.
  * Access the game's Tet Mode Menu, and select Configuration.
  * Set Link Play to On, and set the ID Number to 0 for one instance
    and 1 for the other instance (it helps to have separate NVRAM
    directories for the two instances).
  * Start one instance in listening mode by setting the comm_localhost
    to the listening address and comm_remotehost to an empty string.
  * Start the other instance in connecting mode by setting
    comm_remotehost to the address to connect to (you should probably
    set comm_localhost to an empty string, but if you set it, the
    connecting socket will be bound to this address).
  * Reset the two instances at approximately the same time, it may
    help to reset the instance with ID 0 slightly before resetting the
    instance with ID 1.

  mame gunbustr -comm_localhost 0.0.0.0 -comm_localport 1234 -comm_remotehost ""
  mame gunbustr -comm_localhost "" -comm_remotehost 127.0.0.1 -comm_remoteport 1234

  Note that this implementation only works for the protocol used by
  the Gunbuster game.

  Communication uses a pair of MB8421 dual-port 2K*8 static RAMs, one
  on each game board.  The RAMs are connected to the most significant
  byte of the data bus (D24-D31), corresponding to the least
  significant byte (the CPU uses the big Endian convention).  The
  system accesses the "right" port of its own RAM and the "left" port
  of the remote RAM.

  The link between boards is a simple shared address/data bus,
  carrying 11 address lines and 8 data lines, as well as CS, RW and
  BUSY signals.  A GAL16V8 is used to generate the control signals and
  arbitrate the shared address/data bus.

  The protocol used by the game treats the first 512 bytes of RAM as a
  pair of 256-byte buffers.  The first buffer is used by the system
  with ID 0 to send data to the system with ID 1; the second buffer is
  used by the system with ID 1 to send data to the system with ID 0.
  The remainder of the RAM is cleared but never used.

  The last four bytes of each buffer are used to coordinate
  communication.  The system will write 0xff0x to the last two bytes
  of its buffer, where x is the node ID (0 or 1).  This is used to
  detect the presence of the remote system.  "COMMUNICATION ERROR" is
  displayed if the remote system is not detected.  The byte at offset
  0x00fd is used for handshaking.  0x01 indicates that the system is
  reading data received from the remote system; 0x02 indicates that
  the system is writing data.

  The two systems synchronise each frame.  A frame counter is stored
  in the long word at offset 0x00f8 in each buffer.  If the value
  doesn't increase for sixteen consecutive frames, "COMMUNICATION CUT"
  is displayed.

 ***********************************************************************/

#include "emu.h"
#include "gunbustr_link.h"

#include "emuopts.h"

#include "asio.h"

#include "util/multibyte.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <locale>
#include <mutex>
#include <optional>
#include <sstream>
#include <system_error>
#include <thread>
#include <utility>

//#define VERBOSE 1

#include "logmacro.h"


namespace {

constexpr unsigned BUFFER_BYTES = 256;

constexpr unsigned OFFSET_BUFFER = 0;
constexpr unsigned OFFSET_BEGIN = 2;
constexpr unsigned OFFSET_LENGTH = 4;
constexpr unsigned OFFSET_DATA = 6;

constexpr unsigned HEADER_LENGTH = 6;


INPUT_PORTS_START(gunbustr_link)
	PORT_START("CFG")
	PORT_CONFNAME(0x01, 0x00, "Link Simulation")
	PORT_CONFSETTING(   0x00, "Disable")
	PORT_CONFSETTING(   0x01, "Enable")
INPUT_PORTS_END

} // anonymous namespace



class gunbustr_link_device::context
{
public:
	context(
			gunbustr_link_device &device,
			std::optional<asio::ip::tcp::endpoint> const &local,
			std::optional<asio::ip::tcp::endpoint> const &remote)
		: m_device(device)
		, m_acceptor(m_ioctx)
		, m_sock(m_ioctx)
		, m_connect_timeout(m_ioctx)
		, m_local(local)
		, m_remote(remote)
		, m_stopping(false)
		, m_connected(false)
		, m_sending(false)
		, m_pending_begin{ BUFFER_BYTES, BUFFER_BYTES }
		, m_pending_end{ 0, 0 }
		, m_last_sent(0)
		, m_received_bytes(0)
	{
	}

	std::error_code start()
	{
		std::error_code err;

		if (m_remote)
		{
			m_sock.open((m_local ? m_local : m_remote)->protocol(), err);
			if (!err)
				m_sock.set_option(asio::ip::tcp::no_delay(true), err);
			if (!err && m_local)
				m_sock.bind(*m_local, err);
		}
		else
		{
			assert(m_local);
			m_acceptor.open(m_local->protocol(), err);
			if (!err)
				m_acceptor.bind(*m_local, err);
			if (!err)
				m_acceptor.listen(1, err);
		}
		if (err)
			return err;

		m_thread = std::thread(
				[this] ()
				{
					if (m_remote)
						start_connect();
					else
						start_accept();
					m_ioctx.run();
					LOG("Network thread completed\n");
				});

		return err;
	}

	void stop()
	{
		asio::post(
				m_ioctx,
				[this] ()
				{
					m_stopping = true;
					std::error_code err;
					if (m_acceptor.is_open())
						m_acceptor.close(err);
					if (m_sock.is_open())
						m_sock.close(err);
					m_connect_timeout.cancel();
				});
		m_thread.join();
	}

	void update_buffer(unsigned buffer)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		std::copy_n(
				&m_buffer_copy[buffer][0],
				BUFFER_BYTES,
				&m_device.m_shared_ram[buffer * BUFFER_BYTES]);
		m_device.m_pending[buffer].store(false, std::memory_order_release);
	}

	void send_buffer(unsigned buffer, unsigned begin, unsigned end)
	{
		assert(begin < end);
		assert(begin < BUFFER_BYTES);
		assert(end <= BUFFER_BYTES);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_pending_begin[buffer] = std::min(m_pending_begin[buffer], begin);
			m_pending_end[buffer] = std::max(m_pending_end[buffer], end);
			std::copy_n(
					&m_device.m_shared_ram[buffer * BUFFER_BYTES],
					BUFFER_BYTES,
					&m_buffer_copy[buffer][0]);
		}
		asio::post(m_ioctx, [this] () { send_pending(); });
	}

private:
	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		util::stream_format(
				std::cerr,
				"[%s] %s",
				m_device.tag(),
				util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	void start_connect()
	{
		LOG("Initiating connection to %s\n", *m_remote);
		m_connect_timeout.expires_after(std::chrono::seconds(10));
		m_connect_timeout.async_wait(
				[this] (std::error_code const &err)
				{
					if (!err && !m_stopping && !m_connected && m_sock.is_open())
					{
						LOG("Connection timed out\n");
						schedule_reconnect();
					}
				});
		m_sock.async_connect(
				*m_remote,
				[this] (std::error_code const &err)
				{
					m_connect_timeout.cancel();
					if (err)
					{
						LOG("Connection error: %s\n", err.message());
						if (!m_stopping)
							schedule_reconnect();
					}
					else
					{
						LOG("Connection established\n");
						m_connected = true;
						start_receive();
					}
				});
	}

	void start_accept()
	{
		LOG("Accepting connection on %s\n", *m_local);
		m_acceptor.async_accept(
				[this] (std::error_code err, asio::ip::tcp::socket sock)
				{
					if (!err)
						sock.set_option(asio::ip::tcp::no_delay(true), err);
					if (err)
					{
						LOG("Error accepting connection: %s\n", err.message());
						if (!m_stopping)
							start_accept();
					}
					else
					{
						LOG("Accepted connection from %s\n", sock.remote_endpoint());
						m_sock = std::move(sock);
						m_connected = true;
						start_receive();
					}
				});
	}

	void schedule_reconnect()
	{
		LOG("Scheduling reconnection\n");
		std::error_code err;
		if (m_sock.is_open())
			m_sock.close(err);
		m_connect_timeout.cancel();
		m_connect_timeout.expires_after(std::chrono::seconds(10));
		m_connect_timeout.async_wait(
				[this] (std::error_code const &err)
				{
					if (!err && !m_stopping)
					{
						std::error_code e;
						m_sock.open((m_local ? m_local : m_remote)->protocol(), e);
						if (!e)
							m_sock.set_option(asio::ip::tcp::no_delay(true), e);
						if (!e && m_local)
							m_sock.bind(*m_local, e);
						if (e)
						{
							LOG("Error opening socket: %s\n", e.message());
							schedule_reconnect();
						}
						else
						{
							start_connect();
						}
					}
				});
	}

	void start_receive()
	{
		m_sock.async_read_some(
				asio::buffer(&m_receive_buf[m_received_bytes], std::size(m_receive_buf) - m_received_bytes),
				[this] (std::error_code const &err, std::size_t length)
				{
					if (err || !length)
					{
						if (err)
							LOG("Error receiving: %s\n", err.message());
						else
							LOG("Connection lost\n");
						receive_failed();
						return;

					}

					m_received_bytes += length;
					while (m_received_bytes >= HEADER_LENGTH)
					{
						unsigned const buffer = get_u16be(&m_receive_buf[OFFSET_BUFFER]);
						unsigned const begin = get_u16be(&m_receive_buf[OFFSET_BEGIN]);
						unsigned const bytes = get_u16be(&m_receive_buf[OFFSET_LENGTH]);
						if ((buffer > 1) || (begin >= BUFFER_BYTES) || ((begin + bytes) > BUFFER_BYTES))
						{
							LOG("Received invalid header: buffer %u offset %u length %u\n", buffer, begin, bytes);
							receive_failed();
							return;
						}

						if (m_received_bytes < (bytes + HEADER_LENGTH))
						{
							start_receive();
							return;
						}

						LOG("Received %u bytes of buffer %u from offset %u\n", bytes, buffer, begin);
						{
							std::lock_guard<std::mutex> lock(m_mutex);
							std::copy_n(
									&m_receive_buf[HEADER_LENGTH],
									bytes,
									&m_buffer_copy[buffer][begin]);
							m_device.m_pending[buffer].store(true, std::memory_order_release);
						}

						m_received_bytes -= bytes + HEADER_LENGTH;
						std::copy_n(
								&m_receive_buf[0],
								m_received_bytes,
								&m_receive_buf[bytes + HEADER_LENGTH]);
					}
					start_receive();
				});
	}

	void receive_failed()
	{
		m_connected = false;
		m_received_bytes = 0;
		if (!m_stopping)
		{
			if (m_remote)
			{
				schedule_reconnect();
			}
			else
			{
				std::error_code e;
				if (m_sock.is_open())
					m_sock.close(e);
				start_accept();
			}
		}
	}

	void send_pending()
	{
		if (!m_connected || m_sending)
			return;

		unsigned bytes;
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			bool const pending[2] = {
					m_pending_end[0] > m_pending_begin[0],
					m_pending_end[1] > m_pending_begin[1] };
			if (!pending[0] && !pending[1])
				return;

			unsigned const buffer = ((0 == m_last_sent) && pending[1]) ? 1 : pending[0] ? 0 : 1;
			bytes = m_pending_end[buffer] - m_pending_begin[buffer];
			put_u16be(&m_transmit_buf[OFFSET_BUFFER], buffer);
			put_u16be(&m_transmit_buf[OFFSET_BEGIN], m_pending_begin[buffer]);
			put_u16be(&m_transmit_buf[OFFSET_LENGTH], bytes);
			std::copy_n(
					&m_buffer_copy[buffer][m_pending_begin[buffer]],
					bytes,
					&m_transmit_buf[OFFSET_DATA]);
			LOG("Sending %u bytes of buffer %u from offset %u\n", bytes, buffer, m_pending_begin[buffer]);

			m_pending_begin[buffer] = BUFFER_BYTES;
			m_pending_end[buffer] = 0;
			m_last_sent = buffer;
		}

		m_sending = true;
		asio::async_write(
				m_sock,
				asio::buffer(m_transmit_buf, bytes + HEADER_LENGTH),
				[this] (std::error_code err, std::size_t length)
				{
					m_sending = false;
					if (err)
					{
						LOG("Error sending: %s\n", err.message());
						if (m_connected)
							m_sock.close(); // will cause a receive error, triggering reconnection
					}
					else
					{
						send_pending();
					}
				});
	}

	gunbustr_link_device &m_device;
	std::thread m_thread;
	std::mutex m_mutex;
	asio::io_context m_ioctx;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_sock;
	asio::steady_timer m_connect_timeout;
	std::optional<asio::ip::tcp::endpoint> const m_local;
	std::optional<asio::ip::tcp::endpoint> const m_remote;
	bool m_stopping;
	bool m_connected;
	bool m_sending;

	unsigned m_pending_begin[2];
	unsigned m_pending_end[2];
	unsigned m_last_sent;
	unsigned m_received_bytes;
	u8 m_buffer_copy[2][512];
	u8 m_transmit_buf[512];
	u8 m_receive_buf[512];
};



DEFINE_DEVICE_TYPE(GUNBUSTR_LINK, gunbustr_link_device, "gunbustr_link", "Taito Gunbuster link simulation")



gunbustr_link_device::gunbustr_link_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, GUNBUSTR_LINK, tag, owner, clock)
	, m_config(*this, "CFG")
	, m_shared_ram(*this, "shared_ram", 0x800, ENDIANNESS_BIG)
{
}


gunbustr_link_device::~gunbustr_link_device()
{
}


void gunbustr_link_device::map(address_map &map)
{
	map(0x000000, 0x0007ff).rw(FUNC(gunbustr_link_device::ram_r), FUNC(gunbustr_link_device::ram_w));

	map(0x0000fd, 0x0000fd).w(FUNC(gunbustr_link_device::handshake_w<0>));
	map(0x0000fe, 0x0000ff).rw(FUNC(gunbustr_link_device::sense_r<0>), FUNC(gunbustr_link_device::sense_w<0>));
	map(0x0001fd, 0x0001fd).w(FUNC(gunbustr_link_device::handshake_w<1>));
	map(0x0001fe, 0x0001ff).rw(FUNC(gunbustr_link_device::sense_r<1>), FUNC(gunbustr_link_device::sense_w<1>));
}


ioport_constructor gunbustr_link_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(gunbustr_link);
}


void gunbustr_link_device::device_start()
{
	m_pending[0].store(false, std::memory_order_relaxed);
	m_pending[1].store(false, std::memory_order_relaxed);
}


void gunbustr_link_device::device_stop()
{
	if (m_context)
	{
		m_context->stop();
		m_context.reset();
	}
}


void gunbustr_link_device::device_reset()
{
	if (!m_context)
		start_link();
}


u8 gunbustr_link_device::ram_r(offs_t offset)
{
	return m_shared_ram[offset];
}


void gunbustr_link_device::ram_w(offs_t offset, u8 data)
{
	m_shared_ram[offset] = data;
}



template <unsigned Buffer>
void gunbustr_link_device::handshake_w(u8 data)
{
	u8 &val = m_shared_ram[(Buffer * BUFFER_BYTES) + (BUFFER_BYTES - 3)];
	if (BIT(data, 0) && !BIT(val, 0))
		check_pending(Buffer ^ 1);
	if (!BIT(data, 1) && BIT(val, 1) && m_context)
		m_context->send_buffer(Buffer, 0, BUFFER_BYTES - 4);
	val = data;
}


template <unsigned Buffer>
u16 gunbustr_link_device::sense_r()
{
	check_pending(Buffer);
	return get_u16be(&m_shared_ram[(Buffer * BUFFER_BYTES) + (BUFFER_BYTES - 2)]);
}


template <unsigned Buffer>
void gunbustr_link_device::sense_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_shared_ram[(Buffer * BUFFER_BYTES) + (BUFFER_BYTES - 1)] = data & 0x00ff;

	if (ACCESSING_BITS_8_15)
	{
		m_shared_ram[(Buffer * BUFFER_BYTES) + (BUFFER_BYTES - 2)] = data >> 8;
		if (m_context && ((data & 0xff00) == 0xff00))
			m_context->send_buffer(Buffer, 0, BUFFER_BYTES);
	}
}


void gunbustr_link_device::check_pending(unsigned buffer)
{
	if (m_pending[buffer].load(std::memory_order_acquire))
	{
		assert(m_context);
		m_context->update_buffer(buffer);
	}
}


void gunbustr_link_device::start_link()
{
	if (!BIT(m_config->read(), 0))
		return;

	auto const &opts = mconfig().options();
	std::error_code err;
	std::istringstream parsestr;
	parsestr.imbue(std::locale::classic());

	std::optional<asio::ip::tcp::endpoint> local;
	if (*opts.comm_localhost())
	{
		asio::ip::address const bindaddr = asio::ip::make_address(opts.comm_localhost(), err);
		if (err)
		{
			osd_printf_error("[%s] invalid local IP address %s, disabling link.\n", tag(), opts.comm_localhost());
			return;
		}

		parsestr.str(opts.comm_localport());
		parsestr.seekg(0, std::ios_base::beg);
		asio::ip::port_type bindport;
		parsestr >> bindport;
		if (!parsestr || !bindport)
		{
			osd_printf_error("[%s] invalid local TCP port %s, disabling link.\n", tag(), opts.comm_localport());
			return;
		}

		local.emplace(bindaddr, bindport);
	}

	std::optional<asio::ip::tcp::endpoint> remote;
	if (*opts.comm_remotehost())
	{
		asio::ip::address const connaddr = asio::ip::make_address(opts.comm_remotehost(), err);
		if (err)
		{
			osd_printf_error("[%s] invalid remote address %s, disabling link.\n", tag(), opts.comm_remotehost());
			return;
		}

		parsestr.str(opts.comm_remoteport());
		parsestr.seekg(0, std::ios_base::beg);
		asio::ip::port_type connport;
		parsestr >> connport;
		if (!parsestr || !connport)
		{
			osd_printf_error("[%s] invalid remote TCP port %s, disabling link.\n", tag(), opts.comm_remoteport());
			return;
		}

		remote.emplace(connaddr, connport);
	}

	if (!local && !remote)
	{
		osd_printf_error("[%s] no TCP addresses configured, disabling communication.\n", tag());
		return;
	}

	auto ctx = std::make_unique<context>(*this, local, remote);
	err = ctx->start();
	if (err)
	{
		osd_printf_error("[%s] error opening/binding socket, disabling link (%s).\n", tag(), err.message());
		return;
	}

	m_context = std::move(ctx);
}
