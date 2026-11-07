// license:BSD-3-Clause
// copyright-holders:Damian Rogers
/***********************************************************************

  High-level simulation of the puyopuy2 communication board.

  puyopuy2 supports a four-player mode with two cabinets connected
  by an optional communication board on CN4.  The board's firmware
  has not been dumped, so this device implements the interface the
  68000 program expects, and carries the data between two instances
  of MAME over TCP.

  To use this:
  * Select the link board for the CN4 slot on both instances
    (Slot Devices menu, or -cn4 link).
  * Start one instance in listening mode by setting comm_localhost to
    the listening address and comm_remotehost to an empty string.
    This instance is cabinet 1.
  * Start the other instance in connecting mode by setting
    comm_remotehost to the address to connect to.  This instance is
    cabinet 2.

  mame puyopuy2 -cn4 link -comm_localhost 0.0.0.0 -comm_localport 15112 -comm_remotehost ""
  mame puyopuy2 -cn4 link -comm_localhost "" -comm_remotehost 127.0.0.1 -comm_remoteport 15112

  The two instances must be assigned different cabinet numbers; with
  both set to 1 the four-player game stalls after rule selection.

  The COMMUNICATION TEST screen in the test menu shows the state of
  the board ("BORD") and of the connection between the cabinets
  ("LOOP").

  Registers (CN4 offsets; 68000 addresses are odd bytes):

  0x880101-0x88010f  eight-byte window onto a page of board memory
  0x880111           page select
  0x880131           bit 0: board reset (held high for six frames)

  Pages:
  0x00-0x1f  transmit ring buffer, 256 bytes
  0x20-0x3f  receive ring buffer, 256 bytes
  0xfc       board ID, "PUYO2Z80"
  0xfd       byte 0 reads 0 after reset (non-zero shows CRC ERROR);
             the 68000 writes "PUYO268K" and waits for the board to
             overwrite all eight bytes with 0xff
  0xfe       byte 0: cabinet number (1 or 2) once the other cabinet
             has answered, 0 until then
  0xff       control: 0 transmit write position (68000 owned),
             1 receive read position (68000 owned), 2 bytes waiting
             to be sent, 3 bytes waiting to be read, 7 written with
             1 after each block transfer

  The 68000 copies at most 32 bytes per frame in each direction and
  adjusts the counts at control bytes 2 and 3 with read-modify-write
  instructions, so the board can change them between frames.  The
  game's own packets (header, sequence number and payload) are
  carried unchanged, and are checked by the game.

  Both programs restart the connection if a packet arrives out of
  sequence or none arrives for 1200 frames, so resetting one instance
  causes the other to reconnect after a delay; reset both together.

  TODO:
  * Determine how the real boards choose cabinet numbers.
  * Timing of the real board is unknown; data is sent as soon as the
    68000 commits it.

 ***********************************************************************/

#include "emu.h"
#include "puyopuy2_link.h"

#include "emuopts.h"

#include "asio.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iterator>
#include <locale>
#include <mutex>
#include <optional>
#include <sstream>
#include <system_error>
#include <thread>
#include <utility>

#define LOG_HANDSHAKE (1U << 1)
#define LOG_DATA      (1U << 2)
#define LOG_UNKNOWN   (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

constexpr u8 MSG_STATE = 0x53; // 'S': one byte, bit 0 = board has accepted the 68000's ID
constexpr u8 MSG_DATA = 0x44;  // 'D': up to 255 bytes of transmit data

constexpr u8 PAGE_ID = 0xfc;
constexpr u8 PAGE_CHECK = 0xfd;
constexpr u8 PAGE_LOOP = 0xfe;
constexpr u8 PAGE_CONTROL = 0xff;

constexpr u8 CTRL_TX_COUNT = 2;
constexpr u8 CTRL_RX_COUNT = 3;

constexpr char BOARD_ID[] = "PUYO2Z80";
constexpr char HOST_ID[] = "PUYO268K";


} // anonymous namespace



class puyopuy2_link_device::context
{
public:
	context(
			puyopuy2_link_device &device,
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
		, m_generation(0)
	{
	}

	device_t &device() { return m_device; }

	bool listening() const { return !m_remote; }

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
				m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true), err);
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

	// emulation thread: queue a message for the other cabinet
	void send(u8 type, u8 const *data, u32 length)
	{
		assert(length <= 255);
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_connected)
				return;
			m_outgoing.push_back(type);
			m_outgoing.push_back(u8(length));
			m_outgoing.insert(m_outgoing.end(), data, data + length);
		}
		asio::post(m_ioctx, [this] () { send_pending(); });
	}

	// emulation thread: collect received bytes; the generation changes
	// every time a connection is made or lost
	u32 receive(std::vector<u8> &dest)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		dest.insert(dest.end(), m_incoming.begin(), m_incoming.end());
		m_incoming.clear();
		return m_generation;
	}

	bool connected()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_connected;
	}

private:
	void set_connected(bool connected)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_connected = connected;
		m_incoming.clear();
		m_outgoing.clear();
		++m_generation;
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
						// binding the local port and connecting to the same address
						// produces a TCP self-connection; never treat that as a peer
						std::error_code e1, e2;
						auto const local = m_sock.local_endpoint(e1);
						auto const remote = m_sock.remote_endpoint(e2);
						if (!e1 && !e2 && (local == remote))
						{
							LOG("Connected to self, retrying\n");
							schedule_reconnect();
							return;
						}
						LOG("Connection established\n");
						set_connected(true);
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
						set_connected(true);
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
		m_connect_timeout.expires_after(std::chrono::seconds(2));
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
				asio::buffer(m_receive_buf, std::size(m_receive_buf)),
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

					{
						std::lock_guard<std::mutex> lock(m_mutex);
						m_incoming.insert(m_incoming.end(), m_receive_buf, m_receive_buf + length);
					}
					start_receive();
				});
	}

	void receive_failed()
	{
		set_connected(false);
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
		if (m_sending)
			return;

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (!m_connected || m_outgoing.empty())
				return;
			m_transmit_buf.swap(m_outgoing);
			m_outgoing.clear();
		}

		m_sending = true;
		asio::async_write(
				m_sock,
				asio::buffer(m_transmit_buf),
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

	puyopuy2_link_device &m_device;
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
	u32 m_generation;

	std::vector<u8> m_incoming;
	std::vector<u8> m_outgoing;
	std::vector<u8> m_transmit_buf;
	u8 m_receive_buf[1024];
};



DEFINE_DEVICE_TYPE(PUYOPUY2_LINK, puyopuy2_link_device, "puyopuy2_link", "Puyo Puyo 2 communication board (HLE)")



puyopuy2_link_device::puyopuy2_link_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, PUYOPUY2_LINK, tag, owner, clock)
	, device_segac2_cn4_interface(mconfig, *this)
	, m_node(0)
	, m_seen_generation(0)
{
}


puyopuy2_link_device::~puyopuy2_link_device()
{
}


u8 puyopuy2_link_device::read(offs_t offset)
{
	if (offset < 8)
		return window_r(offset);
	else if (offset == 8)
		return m_page;

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_UNKNOWN, "%s: read from unknown register %02X\n", machine().describe_context(), offset);
	return 0xff;
}


void puyopuy2_link_device::write(offs_t offset, u8 data)
{
	if (offset < 8)
		window_w(offset, data);
	else if (offset == 8)
		m_page = data;
	else if (offset == 0x18)
		reset_w(data);
	else
		LOGMASKED(LOG_UNKNOWN, "%s: write %02X to unknown register %02X\n", machine().describe_context(), data, offset);
}


void puyopuy2_link_device::device_start()
{
	m_remote_ready = false;
	std::fill(std::begin(m_ctrl), std::end(m_ctrl), 0);
	std::fill(std::begin(m_id_page), std::end(m_id_page), 0);
	std::fill(std::begin(m_tx_buf), std::end(m_tx_buf), 0);
	std::fill(std::begin(m_rx_buf), std::end(m_rx_buf), 0);

	save_item(NAME(m_page));
	save_item(NAME(m_in_reset));
	save_item(NAME(m_local_ready));
	save_item(NAME(m_remote_ready));
	save_item(NAME(m_tx_read));
	save_item(NAME(m_rx_write));
	save_item(NAME(m_ctrl));
	save_item(NAME(m_id_page));
	save_item(NAME(m_tx_buf));
	save_item(NAME(m_rx_buf));
}


void puyopuy2_link_device::device_stop()
{
	if (m_context)
	{
		m_context->stop();
		m_context.reset();
	}
}


void puyopuy2_link_device::device_reset()
{
	if (!m_context)
		start_link();

	m_in_reset = false;
	board_reset();
}


void puyopuy2_link_device::board_reset()
{
	m_page = PAGE_CONTROL;
	m_local_ready = false;
	m_tx_read = 0;
	m_rx_write = 0;
	std::fill(std::begin(m_ctrl), std::end(m_ctrl), 0);
	std::fill(std::begin(m_id_page), std::end(m_id_page), 0); // byte 0 = 0: firmware checksum good
	std::fill(std::begin(m_tx_buf), std::end(m_tx_buf), 0);
	std::fill(std::begin(m_rx_buf), std::end(m_rx_buf), 0);
	m_rx_backlog.clear();
	send_state();
}


u8 puyopuy2_link_device::window_r(offs_t offset)
{
	if (m_page < 0x20)
		return m_tx_buf[(m_page << 3) | offset];
	else if (m_page < 0x40)
		return m_rx_buf[((m_page & 0x1f) << 3) | offset];

	switch (m_page)
	{
	case PAGE_ID:
		return BOARD_ID[offset];

	case PAGE_CHECK:
		return m_id_page[offset];

	case PAGE_LOOP:
		if (offset)
			return 0;
		poll();
		return link_ok() ? m_node : 0;

	case PAGE_CONTROL:
		if (CTRL_RX_COUNT == offset)
			poll();
		return m_ctrl[offset];

	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_UNKNOWN, "%s: read from unknown page %02X offset %u\n", machine().describe_context(), m_page, offset);
		return 0xff;
	}
}


void puyopuy2_link_device::window_w(offs_t offset, u8 data)
{
	if (m_page < 0x20)
	{
		m_tx_buf[(m_page << 3) | offset] = data;
		return;
	}
	else if (m_page < 0x40)
	{
		m_rx_buf[((m_page & 0x1f) << 3) | offset] = data;
		return;
	}

	switch (m_page)
	{
	case PAGE_CHECK:
		m_id_page[offset] = data;
		if (!m_local_ready && std::equal(std::begin(m_id_page), std::end(m_id_page), HOST_ID))
		{
			LOGMASKED(LOG_HANDSHAKE, "68000 ID received, board ready\n");
			std::fill(std::begin(m_id_page), std::end(m_id_page), 0xff);
			m_local_ready = true;
			send_state();
		}
		break;

	case PAGE_CONTROL:
		if (CTRL_TX_COUNT == offset)
		{
			// the 68000 adds the number of bytes it has just placed in the transmit buffer
			transmit(u8(data - m_ctrl[CTRL_TX_COUNT]));
		}
		else
		{
			m_ctrl[offset] = data;
			if (CTRL_RX_COUNT == offset)
				fill_rx(); // the 68000 subtracts the number of bytes it has taken
		}
		break;

	default:
		LOGMASKED(LOG_UNKNOWN, "%s: write %02X to page %02X offset %u\n", machine().describe_context(), data, m_page, offset);
		break;
	}
}


void puyopuy2_link_device::reset_w(u8 data)
{
	bool const state = BIT(data, 0);
	if (state && !m_in_reset)
	{
		LOGMASKED(LOG_HANDSHAKE, "%s: board reset\n", machine().describe_context());
		board_reset();
	}
	m_in_reset = state;
}


void puyopuy2_link_device::poll()
{
	if (!m_context || machine().side_effects_disabled())
		return;

	std::vector<u8> received;
	u32 const generation = m_context->receive(received);
	if (generation != m_seen_generation)
	{
		// connection made or lost: discard anything from the previous connection
		LOGMASKED(LOG_HANDSHAKE, "connection %s\n", m_context->connected() ? "established" : "lost");
		m_seen_generation = generation;
		m_remote_ready = false;
		m_rx_backlog.clear();
		m_stream.clear();
		send_state();
	}
	m_stream.insert(m_stream.end(), received.begin(), received.end());

	auto pos = m_stream.begin();
	while ((m_stream.end() - pos) >= 2)
	{
		u32 const length = pos[1];
		if (u32(m_stream.end() - pos) < (2 + length))
			break;

		switch (pos[0])
		{
		case MSG_STATE:
			if (length >= 1)
			{
				bool const ready = BIT(pos[2], 0);
				if (ready != m_remote_ready)
					LOGMASKED(LOG_HANDSHAKE, "other cabinet %s\n", ready ? "ready" : "not ready");
				m_remote_ready = ready;
				if (!ready)
					m_rx_backlog.clear();
			}
			break;

		case MSG_DATA:
			if (link_ok())
				m_rx_backlog.insert(m_rx_backlog.end(), pos + 2, pos + 2 + length);
			break;

		default:
			logerror("unknown message type %02X from other cabinet\n", pos[0]);
			break;
		}
		pos += 2 + length;
	}
	m_stream.erase(m_stream.begin(), pos);

	fill_rx();
}


void puyopuy2_link_device::fill_rx()
{
	while (!m_rx_backlog.empty() && (m_ctrl[CTRL_RX_COUNT] < 0xff))
	{
		m_rx_buf[m_rx_write++] = m_rx_backlog.front();
		m_rx_backlog.pop_front();
		++m_ctrl[CTRL_RX_COUNT];
	}
}


void puyopuy2_link_device::transmit(u32 count)
{
	if (!count)
		return;

	u8 data[256];
	for (u32 i = 0; count > i; ++i)
		data[i] = m_tx_buf[m_tx_read++];
	LOGMASKED(LOG_DATA, "%s: send %u bytes\n", machine().describe_context(), count);

	// sent immediately, so the count of bytes waiting stays at zero
	if (m_context && link_ok())
		m_context->send(MSG_DATA, data, count);
}


void puyopuy2_link_device::send_state()
{
	if (m_context)
	{
		u8 const state = m_local_ready ? 0x01 : 0x00;
		m_context->send(MSG_STATE, &state, 1);
	}
}


void puyopuy2_link_device::start_link()
{
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
		osd_printf_error("[%s] no TCP addresses configured, disabling link.\n", tag());
		return;
	}

	auto ctx = std::make_unique<context>(*this, local, remote);
	err = ctx->start();
	if (err)
	{
		osd_printf_error("[%s] error opening/binding socket, disabling link (%s).\n", tag(), err.message());
		return;
	}

	m_node = ctx->listening() ? 1 : 2;
	m_context = std::move(ctx);
}
