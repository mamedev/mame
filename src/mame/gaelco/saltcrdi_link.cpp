// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Tomás García-Merás Capote
/***********************************************************************

  High-level simulation of the Salter Cardioline link.

  Two machines can be connected to race against each other (the
  "2 CONTRINCANTES" option when selecting the opponent).  Each main
  board drives a clock line and a data line through outputs of an
  addressable latch (CLOCKOUT2 and DATAOUT2 in the schematics of the
  REF. 970410 board), and reads the other machine's lines on two input
  bits (CLOCKIN2 and DATAIN2).  An input bit reads the inverse of the
  other machine's latch output, and unconnected inputs read high,
  which looks the same as the other machine driving both lines low.

  Both lines are high when idle.  Bytes are transferred one bit at a
  time, least significant bit first, using a four-phase handshake:

  Sender            Receiver
  clock=0 data=1                      request
                    clock=1 data=0    ready
  clock=1 data=~D                     data bit
                    clock=1 data=1    acknowledge
  clock=0 data=1                      request next bit
  ...
                    clock=0 data=0    end (instead of eighth
                                      acknowledge)
  clock=0 data=0                      end
                    clock=1 data=1    idle
  clock=1 data=1                      idle

  The game polls the lines and only acts on a value after reading it
  twice in a row.  If both machines start sending at the same time, a
  machine backs off and receives the other machine's byte, unless it's
  the one that selected the "2 CONTRINCANTES" option.

  Relaying every change of the lines over the network would make each
  bit cost several round trips.  Instead, this device simulates the
  remote machine's side of the handshake locally, and exchanges whole
  bytes with the other instance over TCP.

  To use this:
  * Enable Link Simulation in MAME's Machine Configuration menu.  The
    link is started when the system is reset.
  * Start one instance in listening mode by setting comm_localhost to
    the listening address and comm_remotehost to an empty string.
  * Start the other instance in connecting mode by setting
    comm_remotehost to the address to connect to (you should probably
    set comm_localhost to an empty string, but if you set it to
    something else, the connecting socket will be bound to this
    address).
  * On both machines, select a stage time and move the cursor to
    "2 CONTRINCANTES".  Selecting it on one machine makes the other
    machine follow its selections.

  mame sltpcycld -comm_localhost 0.0.0.0 -comm_localport 1234 -comm_remotehost ""
  mame sltpcycld -comm_localhost "" -comm_remotehost 127.0.0.1 -comm_remoteport 1234

 ***********************************************************************/

#include "emu.h"
#include "saltcrdi_link.h"

#include "emuopts.h"

#include "asio.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iterator>
#include <limits>
#include <locale>
#include <optional>
#include <sstream>
#include <system_error>
#include <thread>
#include <utility>

//#define VERBOSE 1

#include "logmacro.h"


namespace {

// lines driven by each side: bit 0 is clock, bit 1 is data
constexpr u8 LINES_END = 0x00;
constexpr u8 LINES_READY = 0x01;
constexpr u8 LINES_REQUEST = 0x02;
constexpr u8 LINES_IDLE = 0x03; // also used to acknowledge a bit

constexpr u8 data_lines(int bit)
{
	return bit ? 0x01 : 0x03; // clock high, data inverted
}

constexpr unsigned SETTLE_USEC = 10;
constexpr unsigned POLL_MSEC = 1;

constexpr u16 RESET_POLLS = 20; // the games read the lines at least once every 10 ms
constexpr u16 YIELD_POLLS = 50;
constexpr u16 TIMEOUT_POLLS = 1000;


INPUT_PORTS_START(saltcrdi_link)
	PORT_START("CFG")
	PORT_CONFNAME(0x01, 0x00, "Link Simulation")
	PORT_CONFSETTING(   0x00, "Disable")
	PORT_CONFSETTING(   0x01, "Enable")
INPUT_PORTS_END

} // anonymous namespace



class saltcrdi_link_device::context
{
public:
	context(
			saltcrdi_link_device &device,
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
		, m_pending_bytes(0U)
		, m_queue_wp(0U)
		, m_queue_rp(0U)
	{
	}

	device_t& device() { return m_device; }

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

	bool connected() const
	{
		return m_connected.load(std::memory_order_acquire);
	}

	void send(u8 data)
	{
		asio::post(
				m_ioctx,
				[this, data] ()
				{
					if (!m_connected.load(std::memory_order_relaxed))
						return;
					if (std::size(m_pending_buf) > m_pending_bytes)
						m_pending_buf[m_pending_bytes++] = data;
					else
						LOG("Transmit buffer full, dropping byte 0x%02X\n", data);
					send_pending();
				});
	}

	bool receive(u8 &data)
	{
		unsigned const rp = m_queue_rp.load(std::memory_order_relaxed);
		unsigned const wp = m_queue_wp.load(std::memory_order_acquire);
		if (rp == wp)
			return false;
		data = m_queue[rp];
		m_queue_rp.store((rp + 1) % std::size(m_queue), std::memory_order_release);
		return true;
	}

	void flush()
	{
		m_queue_rp.store(m_queue_wp.load(std::memory_order_acquire), std::memory_order_release);
	}

private:
	void start_connect()
	{
		LOG("Initiating connection to %s\n", *m_remote);
		m_connect_timeout.expires_after(std::chrono::seconds(10));
		m_connect_timeout.async_wait(
				[this] (std::error_code const &err)
				{
					if (!err && !m_stopping && !m_connected.load(std::memory_order_relaxed) && m_sock.is_open())
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
						m_connected.store(true, std::memory_order_release);
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
						m_connected.store(true, std::memory_order_release);
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
				asio::buffer(m_receive_buf),
				[this] (std::error_code const &err, std::size_t length)
				{
					if (err || !length)
					{
						if (err)
							LOG("Error receiving: %s\n", err.message());
						else
							LOG("Connection lost\n");
						receive_failed();
					}
					else
					{
						queue_received(length);
						start_receive();
					}
				});
	}

	void queue_received(std::size_t length)
	{
		unsigned wp = m_queue_wp.load(std::memory_order_relaxed);
		unsigned const rp = m_queue_rp.load(std::memory_order_acquire);
		for (std::size_t i = 0; length > i; ++i)
		{
			unsigned const next = (wp + 1) % std::size(m_queue);
			if (next == rp)
			{
				LOG("Receive queue full, dropping %u bytes\n", length - i);
				break;
			}
			m_queue[wp] = m_receive_buf[i];
			wp = next;
		}
		m_queue_wp.store(wp, std::memory_order_release);
	}

	void receive_failed()
	{
		m_connected.store(false, std::memory_order_release);
		m_pending_bytes = 0U;
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
		if (!m_connected.load(std::memory_order_relaxed) || m_sending || !m_pending_bytes)
			return;

		unsigned const bytes = m_pending_bytes;
		std::copy_n(&m_pending_buf[0], bytes, &m_transmit_buf[0]);
		m_pending_bytes = 0U;

		m_sending = true;
		asio::async_write(
				m_sock,
				asio::buffer(&m_transmit_buf[0], bytes),
				[this] (std::error_code err, std::size_t length)
				{
					m_sending = false;
					if (err)
					{
						LOG("Error sending: %s\n", err.message());
						if (m_connected.load(std::memory_order_relaxed))
							m_sock.close(); // will cause a receive error, triggering reconnection
					}
					else
					{
						send_pending();
					}
				});
	}

	saltcrdi_link_device &m_device;
	std::thread m_thread;
	asio::io_context m_ioctx;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_sock;
	asio::steady_timer m_connect_timeout;
	std::optional<asio::ip::tcp::endpoint> const m_local;
	std::optional<asio::ip::tcp::endpoint> const m_remote;
	bool m_stopping;
	std::atomic_bool m_connected;
	bool m_sending;

	unsigned m_pending_bytes;
	std::atomic_uint m_queue_wp;
	std::atomic_uint m_queue_rp;
	std::array<u8, 64> m_pending_buf;
	std::array<u8, 64> m_transmit_buf;
	std::array<u8, 64> m_receive_buf;
	std::array<u8, 256> m_queue;
};



DEFINE_DEVICE_TYPE(SALTCRDI_LINK, saltcrdi_link_device, "saltcrdi_link", "Salter Cardioline link simulation")



saltcrdi_link_device::saltcrdi_link_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SALTCRDI_LINK, tag, owner, clock)
	, m_config(*this, "CFG")
	, m_settle_timer(nullptr)
	, m_poll_timer(nullptr)
{
}


saltcrdi_link_device::~saltcrdi_link_device()
{
}


void saltcrdi_link_device::clock_w(int state)
{
	m_board_lines = (m_board_lines & 0x02) | (state ? 0x01 : 0x00);
	m_settle_timer->adjust(attotime::from_usec(SETTLE_USEC));
}


void saltcrdi_link_device::data_w(int state)
{
	m_board_lines = (m_board_lines & 0x01) | (state ? 0x02 : 0x00);
	m_settle_timer->adjust(attotime::from_usec(SETTLE_USEC));
}


int saltcrdi_link_device::clock_r()
{
	return BIT(~m_peer_lines, 0);
}


int saltcrdi_link_device::data_r()
{
	return BIT(~m_peer_lines, 1);
}


ioport_constructor saltcrdi_link_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(saltcrdi_link);
}


void saltcrdi_link_device::device_start()
{
	m_settle_timer = timer_alloc(FUNC(saltcrdi_link_device::settle), this);
	m_poll_timer = timer_alloc(FUNC(saltcrdi_link_device::poll), this);

	m_state = STATE_OFFLINE;
	m_board_lines = 0U;
	m_peer_lines = LINES_END;
	m_bit = 0U;
	m_shift = 0U;
	m_tx_data = 0U;
	m_tx_pending = false;
	m_stall = 0U;

	save_item(NAME(m_state));
	save_item(NAME(m_board_lines));
	save_item(NAME(m_peer_lines));
	save_item(NAME(m_bit));
	save_item(NAME(m_shift));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_pending));
	save_item(NAME(m_stall));
}


void saltcrdi_link_device::device_stop()
{
	if (m_context)
	{
		m_context->stop();
		m_context.reset();
	}
}


void saltcrdi_link_device::device_reset()
{
	if (!m_context)
		start_link();

	enter_state(STATE_OFFLINE, LINES_END);
	m_tx_pending = false;

	if (m_context)
	{
		m_context->flush();
		m_poll_timer->adjust(attotime::from_msec(POLL_MSEC), 0, attotime::from_msec(POLL_MSEC));
	}
	else
	{
		m_poll_timer->reset();
	}
}


void saltcrdi_link_device::device_post_load()
{
	// the network connection isn't part of the saved state
	if (m_context)
	{
		m_poll_timer->adjust(attotime::from_msec(POLL_MSEC), 0, attotime::from_msec(POLL_MSEC));
	}
	else
	{
		m_poll_timer->reset();
		m_tx_pending = false;
		enter_state(STATE_OFFLINE, LINES_END);
	}
}


TIMER_CALLBACK_MEMBER(saltcrdi_link_device::settle)
{
	if (m_context)
		update();
}


TIMER_CALLBACK_MEMBER(saltcrdi_link_device::poll)
{
	if (std::numeric_limits<u16>::max() > m_stall)
		++m_stall;

	// don't act on outputs that are still changing
	if (!m_settle_timer->enabled())
		update();
}


void saltcrdi_link_device::update()
{
	if (!m_context->connected())
	{
		if (STATE_OFFLINE != m_state)
		{
			LOG("Link lost\n");
			m_context->flush();
			m_tx_pending = false;
			enter_state(STATE_OFFLINE, LINES_END);
		}
		return;
	}

	if ((STATE_OFFLINE != m_state) && (STATE_RESET != m_state) && (STATE_IDLE != m_state) && (TIMEOUT_POLLS <= m_stall))
	{
		LOG("Transfer timed out\n");
		abort_transfer();
		return;
	}

	switch (m_state)
	{
	case STATE_OFFLINE:
		LOG("Link established\n");
		enter_state(STATE_IDLE, LINES_IDLE);
		break;

	case STATE_RESET:
		// the main board's acknowledge looks the same as idle, so its lines don't show it has given up
		if (RESET_POLLS <= m_stall)
			enter_state(STATE_IDLE, LINES_IDLE);
		break;

	case STATE_IDLE:
		if (LINES_REQUEST == m_board_lines)
		{
			// main board is starting to send a byte
			m_bit = 0U;
			m_shift = 0U;
			enter_state(STATE_RX_READY, LINES_READY);
		}
		else if (LINES_IDLE == m_board_lines)
		{
			if (!m_tx_pending)
				m_tx_pending = m_context->receive(m_tx_data);
			if (m_tx_pending)
			{
				m_bit = 0U;
				enter_state(STATE_TX_REQUEST, LINES_REQUEST);
			}
		}
		break;

	case STATE_RX_READY:
		if (BIT(m_board_lines, 0))
		{
			// data is inverted
			if (!BIT(m_board_lines, 1))
				m_shift |= 1U << m_bit;
			if (7U > m_bit)
			{
				++m_bit;
				enter_state(STATE_RX_ACK, LINES_IDLE);
			}
			else
			{
				enter_state(STATE_RX_END, LINES_END);
			}
		}
		else if (LINES_END == m_board_lines)
		{
			abort_transfer();
		}
		break;

	case STATE_RX_ACK:
		if (LINES_REQUEST == m_board_lines)
			enter_state(STATE_RX_READY, LINES_READY);
		else if (LINES_END == m_board_lines)
			abort_transfer();
		break;

	case STATE_RX_END:
		if (LINES_END == m_board_lines)
		{
			LOG("Received 0x%02X from main board\n", m_shift);
			m_context->send(m_shift);
			enter_state(STATE_IDLE, LINES_IDLE);
		}
		break;

	case STATE_TX_REQUEST:
		if (LINES_READY == m_board_lines)
		{
			enter_state(STATE_TX_DATA, data_lines(BIT(m_tx_data, m_bit)));
		}
		else if (LINES_REQUEST == m_board_lines)
		{
			// both sides are trying to send - back off if the main board doesn't
			if (YIELD_POLLS <= m_stall)
			{
				LOG("Backing off to receive byte from main board\n");
				m_bit = 0U;
				m_shift = 0U;
				enter_state(STATE_RX_READY, LINES_READY);
			}
		}
		else if (LINES_END == m_board_lines)
		{
			abort_transfer();
		}
		break;

	case STATE_TX_DATA:
		if (LINES_IDLE == m_board_lines)
		{
			// acknowledged
			if (7U > m_bit)
			{
				++m_bit;
				enter_state(STATE_TX_NEXT, LINES_REQUEST);
			}
			else
			{
				abort_transfer();
			}
		}
		else if (LINES_END == m_board_lines)
		{
			if (7U == m_bit)
				enter_state(STATE_TX_END, LINES_END);
			else
				abort_transfer();
		}
		else if (LINES_REQUEST == m_board_lines)
		{
			abort_transfer();
		}
		break;

	case STATE_TX_NEXT:
		if (LINES_READY == m_board_lines)
			enter_state(STATE_TX_DATA, data_lines(BIT(m_tx_data, m_bit)));
		else if (LINES_IDLE != m_board_lines)
			abort_transfer();
		break;

	case STATE_TX_END:
		if (LINES_IDLE == m_board_lines)
		{
			LOG("Sent 0x%02X to main board\n", m_tx_data);
			m_tx_pending = false;
			enter_state(STATE_IDLE, LINES_IDLE);
		}
		break;
	}
}


void saltcrdi_link_device::enter_state(u8 state, u8 lines)
{
	m_state = state;
	m_peer_lines = lines;
	m_stall = 0U;
}


void saltcrdi_link_device::abort_transfer()
{
	// drive both lines low long enough for the main board to give up, like disconnecting the cable
	LOG("Aborting transfer in state %u\n", m_state);
	enter_state(STATE_RESET, LINES_END);
}


void saltcrdi_link_device::start_link()
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

	m_context = std::move(ctx);
}
