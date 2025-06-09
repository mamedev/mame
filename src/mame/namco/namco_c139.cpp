// license:BSD-3-Clause
// copyright-holders:Angelo Salese, John Bennett, Ariane Fugmann
/***************************************************************************

    Namco C139 - Serial I/F Controller

    (from assault schematics, page 5-18 and 5-19)
    connected to M5M5179P RAM with a 13-bit address bus, and 9 bit data bus
    connected to host cpu with a 14*-bit address bus, and 13 bit data bus
    2 clock inputs - 16M and 12M
    currently there are 4 known modes of operation:

    mode 0x08:
    - ridgera2
    - raverace

    mode 0x09:
    - fourtrax
    - suzuka8h
    - suzuk8h2
    - winrungp
    - winrun91
    - driveyes (center)
    - cybsled
    - cybrcomm
    - acedrive
    - victlap
    - cybrcycc
    - adillor

    mode 0x0c:
    - ridgeracf

    mode 0x0d:
    - finallap
    - finallap2
    - finallap3
    - driveyes (sides)
    - tokyowar
    - aircomb
    - dirtdash
    - alpiner2b (uses 0xfd)

    mode 0x0f (configuration mode)
    - 0x02 used to setup byte/word adressing

    NOTES:
      apparently mode 0x09 and 0x0d modify the received data.
      mode 0x09 does not update *anything* after data got changed. might be automatic.
      mode 0x0d updates the tx offset pointing to the rx buffer (which is not supported right now)

    TODO:
    - hook a real chip and test in detail
    - mode 0x0d shows 1 machine in service mode and attract mode, however most games seem to work "okay" in multiplayer.
    - C422 seems to be a pin compatible upgrade to C139, probably supporting higher clock speeds? hook up c139 in system23
    - mode 0x0b is used by s23 games to test interrupts.

***************************************************************************/

#include "emu.h"
#include "namco_c139.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"


class namco_c139_device::context
{
public:
	context() :
	m_acceptor(m_ioctx),
	m_sock_rx(m_ioctx),
	m_sock_tx(m_ioctx),
	m_timeout_tx(m_ioctx)
	{
	}

	void start(std::string localhost, std::string localport, std::string remotehost, std::string remoteport)
	{
		std::error_code err;
		asio::ip::tcp::resolver resolver(m_ioctx);

		for (auto &&resolveIte : resolver.resolve(localhost, localport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_localaddr = resolveIte.endpoint();
			LOG("C139: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("C139: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("C139: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("C139: remotehost resolve error: %s\n", err.message());
		}
	}

	void stop()
	{
		std::error_code err;
		if (m_acceptor.is_open())
			m_acceptor.close(err);
		if (m_sock_rx.is_open())
			m_sock_rx.close(err);
		if (m_sock_tx.is_open())
			m_sock_tx.close(err);
		m_timeout_tx.cancel();
		m_state_tx = 0;
		m_state_rx = 0;
	}

	void check_sockets()
	{
		// if async operation in progress, poll context
		if ((m_state_rx > 0) || (m_state_tx > 0))
			m_ioctx.poll();

		// start acceptor if needed
		if (m_localaddr && m_state_rx == 0)
		{
			std::error_code err;
			m_acceptor.open(m_localaddr->protocol(), err);
			m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
			if (!err)
			{
				m_acceptor.bind(*m_localaddr, err);
				if (!err)
				{
					m_acceptor.listen(1, err);
					if (!err)
					{
						osd_printf_verbose("C139: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("C139: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_state_rx = 0;
									}
									else
									{
										LOG("C139: RX connection from %s\n", sock.remote_endpoint());
										std::error_code e;
										m_acceptor.close(e);
										m_sock_rx = std::move(sock);
										m_sock_rx.set_option(asio::socket_base::keep_alive(true));
										m_state_rx = 2;
										start_receive_rx();
									}
								});
						m_state_rx = 1;
					}
				}
			}
			if (err)
			{
				LOG("C139: RX failed - %d %s\n", err.value(), err.message());
			}
		}

		// connect socket if needed
		if (m_remoteaddr && m_state_tx == 0)
		{
			std::error_code err;
			if (m_sock_tx.is_open())
				m_sock_tx.close(err);
			m_sock_tx.open(m_remoteaddr->protocol(), err);
			if (!err)
			{
				m_sock_tx.non_blocking(true);
				m_sock_tx.set_option(asio::ip::tcp::no_delay(true));
				m_sock_tx.set_option(asio::socket_base::keep_alive(true));
				osd_printf_verbose("C139: TX connecting to %s\n", *m_remoteaddr);
				m_timeout_tx.expires_after(std::chrono::seconds(10));
				m_timeout_tx.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_state_tx == 1)
							{
								osd_printf_verbose("C139: TX connect timed out\n");
								std::error_code e;
								m_sock_tx.close(e);
								m_state_tx = 0;
							}
						});
				m_sock_tx.async_connect(
						*m_remoteaddr,
						[this] (std::error_code const &err)
						{
							m_timeout_tx.cancel();
							if (err)
							{
								osd_printf_verbose("C139: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_state_tx = 0;
							}
							else
							{
								LOG("C139: TX connection established\n");
								m_state_tx = 2;
							}
						});
				m_state_tx = 1;
			}
		}
	}

	bool connected()
	{
		return m_state_rx == 2 && m_state_tx == 2;
	}

	unsigned receive(uint8_t *buffer, unsigned data_size)
	{
		if (m_state_rx < 2)
			return UINT_MAX;

		m_ioctx.poll();

		if (data_size > m_fifo_rx.used())
			return 0;

		return m_fifo_rx.read(&buffer[0], data_size, false);
	}

	unsigned send(uint8_t *buffer, unsigned data_size)
	{
		if (m_state_tx < 2)
			return UINT_MAX;

		if (data_size > m_fifo_tx.free())
		{
			LOG("C139: TX buffer overflow\n");
			return UINT_MAX;
		}

		bool const sending = m_fifo_tx.used();
		m_fifo_tx.write(&buffer[0], data_size);
		if (!sending)
			start_send_tx();
		return data_size;
	}

private:
	class fifo
	{
	public:
		unsigned write(uint8_t *buffer, unsigned data_size)
		{
			unsigned used = 0;
			if (m_wp >= m_rp)
			{
				used = std::min<unsigned>(size() - m_wp, data_size);
				std::copy_n(&buffer[0], used, &m_buffer[m_wp]);
				m_wp = (m_wp + used) % size();
			}
			unsigned const block = std::min<unsigned>(data_size - used, m_rp - m_wp);
			if (block)
			{
				std::copy_n(&buffer[used], block, &m_buffer[m_wp]);
				used += block;
				m_wp += block;
			}
			m_used += used;
			return used;
		}

		unsigned read(uint8_t *buffer, unsigned data_size, bool peek)
		{
			unsigned rp = m_rp;
			unsigned used = 0;
			if (rp >= m_wp)
			{
				used = std::min<std::size_t>(size() - rp, data_size);
				std::copy_n(&m_buffer[rp], used, &buffer[0]);
				rp = (rp + used) % size();
			}
			unsigned const block = std::min<unsigned>(data_size - used, m_wp - rp);
			if (block)
			{
				std::copy_n(&m_buffer[rp], block, &buffer[used]);
				used += block;
				rp += block;
			}
			if (!peek)
				consume(used);
			return used;
		}

		void consume(unsigned data_size)
		{
			m_rp = (m_rp + data_size) % size();
			m_used -= data_size;
		}

		unsigned size()
		{
			return m_buffer.size();
		}

		unsigned used()
		{
			return m_used;
		}

		unsigned free()
		{
			return size() - used();
		}

		void clear()
		{
			m_wp = m_rp = m_used = 0;
		}


	private:
		unsigned m_wp = 0;
		unsigned m_rp = 0;
		unsigned m_used = 0;
		std::array<uint8_t, 0x80000> m_buffer;
	};

	void start_send_tx()
	{
		unsigned used = m_fifo_tx.read(&m_buffer_tx[0], std::min<unsigned>(m_fifo_tx.used(), m_buffer_tx.size()), true);
		m_sock_tx.async_write_some(
				asio::buffer(&m_buffer_tx[0], used),
				[this] (std::error_code const &err, std::size_t length)
				{
					m_fifo_tx.consume(length);
					if (err)
					{
						LOG("C139: TX connection error: %s\n", err.message().c_str());
						m_sock_tx.close();
						m_state_tx = 0;
						m_fifo_tx.clear();
					}
					else if (m_fifo_tx.used())
					{
						start_send_tx();
					}
				});
	}

	void start_receive_rx()
	{
		m_sock_rx.async_read_some(
				asio::buffer(m_buffer_rx),
				[this] (std::error_code const &err, std::size_t length)
				{
					if (err || !length)
					{
						if (err)
							LOG("C139: RX connection error: %s\n", err.message());
						else
							LOG("C139: RX connection lost\n");
						m_sock_rx.close();
						m_state_rx = 0;
						m_fifo_rx.clear();
					}
					else
					{
						if (UINT_MAX == m_fifo_rx.write(&m_buffer_rx[0], length))
						{
							LOG("C139: RX buffer overflow\n");
							m_sock_rx.close();
							m_state_rx = 0;
							m_fifo_rx.clear();
						}
						start_receive_rx();
					}
				});
	}

	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		util::stream_format(
				std::cerr,
				"%s",
				util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

	asio::io_context m_ioctx;
	std::optional<asio::ip::tcp::endpoint> m_localaddr;
	std::optional<asio::ip::tcp::endpoint> m_remoteaddr;
	asio::ip::tcp::acceptor m_acceptor;
	asio::ip::tcp::socket m_sock_rx;
	asio::ip::tcp::socket m_sock_tx;
	asio::steady_timer m_timeout_tx;
	uint8_t m_state_rx;
	uint8_t m_state_tx;
	fifo m_fifo_rx;
	fifo m_fifo_tx;
	std::array<uint8_t, 0x200> m_buffer_rx;
	std::array<uint8_t, 0x200> m_buffer_tx;
	
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
#define REG_0_STATUS 0
#define REG_1_MODE 1
#define REG_2_CONTROL 2
#define REG_3_START 3
#define REG_4_RXSIZE 4
#define REG_5_TXSIZE 5
#define REG_6_RXOFFSET 6
#define REG_7_TXOFFSET 7

// device type definition
DEFINE_DEVICE_TYPE(NAMCO_C139, namco_c139_device, "namco_c139", "Namco C139 Serial")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

void namco_c139_device::data_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(namco_c139_device::ram_r),FUNC(namco_c139_device::ram_w));
}

void namco_c139_device::regs_map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(namco_c139_device::reg_r), FUNC(namco_c139_device::reg_w));
}


//-------------------------------------------------
//  namco_c139_device - constructor
//-------------------------------------------------

namco_c139_device::namco_c139_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C139, tag, owner, clock),
	m_irq_cb(*this)
{
	auto const &opts = mconfig.options();

	m_localhost = opts.comm_localhost();
	m_localport = opts.comm_localport();
	m_remotehost = opts.comm_remotehost();
	m_remoteport = opts.comm_remoteport();

	// come up with some magic number for identification
	std::string remotehost = util::string_format("%s:%s", m_remotehost, m_remoteport);
	m_linkid = 0;
	for (int x = 0; x < sizeof(remotehost) && remotehost[x] != 0; x++)
	{
		m_linkid ^= remotehost[x];
	}

	LOG("C139: ID byte = %02d\n", m_linkid);

	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c139_device::device_start()
{
	m_tick_timer = timer_alloc(FUNC(namco_c139_device::tick_timer_callback), this);
	m_tick_timer->adjust(attotime::never);

	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);

	// state saving
	save_item(NAME(m_ram));
	save_item(NAME(m_reg));

	save_item(NAME(m_linktimer));
	save_item(NAME(m_linkid));

	save_item(NAME(m_txsize));
	save_item(NAME(m_txblock));
	save_item(NAME(m_reg_f3));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_c139_device::device_reset()
{
	std::fill(std::begin(m_ram), std::end(m_ram), 0);
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	m_context->stop();

	m_context->start(m_localhost, m_localport, m_remotehost, m_remoteport);

	m_linktimer = 0x0200;

	m_txsize = 0x00;
	m_txblock = 0x00;
	m_reg_f3 = 0x00;

	m_tick_timer->adjust(attotime::from_hz(600),0,attotime::from_hz(600));
}

void namco_c139_device::device_stop()
{
	m_tick_timer->adjust(attotime::never);

	m_context->stop();
	m_context.reset();
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint16_t namco_c139_device::ram_r(offs_t offset)
{
	return m_ram[offset];
}

void namco_c139_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[offset]);
	m_txsize = offset;
}

uint16_t namco_c139_device::reg_r(offs_t offset)
{
	uint16_t result = m_reg[offset];
	if (!machine().side_effects_disabled())
		LOG("C139: reg_r[%02x] = %04x\n", offset, result);
	return result;
}

void namco_c139_device::reg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!machine().side_effects_disabled())
		LOG("C139: reg_w[%02x] = %04x\n", offset, data);
	m_reg[offset] = data;

	// status reset / irq ack?
	if (offset == REG_0_STATUS && data == 0)
		m_reg[offset] = 4;

	// hack to get alpinr2b working
	if (offset == REG_1_MODE && data >= 0x000f)
		m_reg[REG_1_MODE] &= 0x000f;

	// possibly setup/config mode?
	if (m_reg[REG_1_MODE] == 0x0f && offset == REG_3_START)
	{
		m_reg_f3 = data;
		logerror("C139: m_reg_f3 = %02x\n", m_reg_f3);
	}

	// mode 09 tx trigger
	if (offset == REG_1_MODE && data == 0x09 && m_txsize > 0)
		m_txblock = 0x00;

	// mode 08 & 0c tx trigger
	if (offset == REG_2_CONTROL && data == 0x03)
		m_txblock = 0x00;

	// mode 0d tx trigger
	if (offset == REG_5_TXSIZE && data > 0)
		m_txblock = 0x00;

	// hack to get raverace working
	if (m_reg[REG_1_MODE] == 0x08 && offset == REG_2_CONTROL)
	{
		if (data == 1)
			m_txsize = 0;
		if (data == 3)
			m_reg[REG_5_TXSIZE] = m_txsize + 2;
	}
}

void namco_c139_device::sci_de_hack(uint8_t data)
{
	// prepare "filenames"
	switch (data)
	{
		case 0:
			m_localhost = "127.0.0.1";
			m_localport = "15112";
			m_remotehost = "127.0.0.1";
			m_remoteport = "15113";
			break;
		case 1:
			m_localhost = "127.0.0.1";
			m_localport = "15113";
			m_remotehost = "127.0.0.1";
			m_remoteport = "15114";
			break;
		case 2:
			m_localhost = "127.0.0.1";
			m_localport = "15114";
			m_remotehost = "127.0.0.1";
			m_remoteport = "15112";
			break;
		default:
			m_localhost = "127.0.0.1";
			m_localport = "15112";
			m_remotehost = "127.0.0.1";
			m_remoteport = "15112";
			break;
	}

	// come up with some magic number for identification
	std::string remotehost = util::string_format("%s:%s", m_remotehost, m_remoteport);
	m_linkid = 0;
	for (int x = 0; x < sizeof(remotehost) && remotehost[x] != 0; x++)
	{
		m_linkid ^= remotehost[x];
	}

	LOG("C139: ID byte = %02d\n", m_linkid);
}

TIMER_CALLBACK_MEMBER(namco_c139_device::tick_timer_callback)
{
	comm_tick();
}

void namco_c139_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linktimer > 0x0000)
		m_linktimer--;

	if (m_linktimer == 0x0000)
	{
		// if both sockets are connected
		if (m_context->connected())
		{
			// link established
			unsigned data_size = 0x200;
			switch (m_reg[REG_1_MODE])
			{
				case 0x08:
					// ridgera2, raverace
					// 0b1000
					// reg2 - 1 > write mem > 3 > 1 > write mem > 3 etc.
					// txcount NOT cleared on send
					read_data(data_size);
					if (m_reg[REG_2_CONTROL] == 0x03 && m_reg[REG_5_TXSIZE] > 0x00)
						send_data(data_size);
					break;

				case 0x09:
					// suzuka8h, acedrive, winrungp, cybrcycc, driveyes (center)
					// 0b1001 - auto-send via sync bit (and auto offset)
					read_data(data_size);
					send_data(data_size);
					break;

				case 0x0c:
					// ridgeracf
					// 0b1100 - send by register / txwords
					// txcount IS cleared on send
					read_data(data_size);
					if (m_reg[REG_2_CONTROL] == 0x03 && m_reg[REG_3_START] == 0x00)
						send_data(data_size);
					break;

				case 0x0d:
					// final lap, driveyes (left & right)
					// 0b1101 - auto-send via register?
					read_data(data_size);
					if (m_reg[REG_3_START] == 0x00 && m_reg[REG_5_TXSIZE] > 0x00)
						send_data(data_size);
					break;

				case 0x0f:
					// init / reset
					break;

				default:
					// unknown mode
					break;
			}
		}
	}
}

void namco_c139_device::read_data(unsigned data_size)
{
	if (m_reg[REG_0_STATUS] != 0x06)
	{
		// try to read a message
		unsigned recv = read_frame(data_size);
		if (recv > 0)
		{
			// (hack) update linkcount for mode 09
			if (m_buffer[0] == m_linkid && m_reg[REG_1_MODE] == 0x09 && (m_reg_f3 & 0x2) == 2)
				m_buffer[0x07] = m_buffer[0x1ff];

			// save message to "rx buffer"
			unsigned rx_size = m_buffer[2] << 8 | m_buffer[1];
			unsigned rx_offset = m_reg[REG_6_RXOFFSET]; // rx offset in words
			LOG("C139: rx_offset = %04x, rx_size == %02x\n", rx_offset, rx_size);
			unsigned buf_offset = 3;
			for (unsigned j = 0x00; j < rx_size; j++)
			{
				m_ram[0x1000 + (rx_offset & 0x0fff)] = m_buffer[buf_offset + 1] << 8 | m_buffer[buf_offset];
				rx_offset++;
				buf_offset += 2;
			}

			// relay messages
			if (m_buffer[0] != m_linkid)
			{
				if (m_reg[REG_1_MODE] == 0x09 && (m_reg_f3 & 0x2) == 2)
					m_buffer[0x1ff]++;

				send_frame(data_size);
			}
			else
			{
				if (m_reg[REG_1_MODE] == 0x09)
					m_reg[REG_5_TXSIZE] = 0x00;
			}

			// update regs
			m_reg[REG_0_STATUS] = 0x06;
			if (m_reg[REG_1_MODE] != 0x0d)
				m_reg[REG_4_RXSIZE] += rx_size;
			else
				m_reg[REG_4_RXSIZE] -= rx_size;
			m_reg[REG_6_RXOFFSET] += rx_size;

			// prevent overflow
			m_reg[REG_4_RXSIZE] &= 0x0fff;
			m_reg[REG_6_RXOFFSET] &= 0x0fff;

			// fire interrupt
			m_irq_cb(ASSERT_LINE);
		}
	}
	else
	{
		// fire interrupt (again)
		m_irq_cb(ASSERT_LINE);
	}
}

unsigned namco_c139_device::read_frame(unsigned data_size)
{
	unsigned bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		m_linktimer = 0x0100;
		m_txblock = 0x00;
		return 0;
	}
	return bytes_read;
}

void namco_c139_device::send_data(unsigned data_size)
{
	if (m_txblock == 0x01)
		return;

	if (m_reg[REG_0_STATUS] == 0x06)
		return;

	unsigned tx_offset = m_reg[REG_7_TXOFFSET]; // tx offset in words
	if ((m_reg_f3 & 0x02) == 0x02)
		tx_offset >>= 1; // tx offset in bytes

	unsigned tx_mask = 0x0fff;
	if (m_reg[REG_1_MODE] == 0x0d)
		tx_mask = 0x1fff; // allow txPointer to rxBuffer

	unsigned tx_size = m_reg[REG_5_TXSIZE];
	if (m_reg[REG_1_MODE] == 0x09)
	{
		tx_size = find_sync_bit(tx_offset, tx_mask);
		if (tx_size == 0x01)
		{
			m_reg[REG_7_TXOFFSET] += 0x100;
			tx_size = 0;
		}
	}

	LOG("C139: tx_offset = %04x, tx_size == %02x\n", tx_offset, tx_size);
	if (tx_size == 0)
		return;

	m_buffer[0] = m_linkid;
	m_buffer[1] = tx_size & 0xff;
	m_buffer[2] = (tx_size & 0xff00) >> 8;
	m_buffer[0x1ff] = 1;

	unsigned buf_offset = 3;
	for (unsigned j = 0x00; j < tx_size; j++)
	{
		m_buffer[buf_offset] = m_ram[tx_offset & tx_mask] & 0xff;
		m_buffer[buf_offset + 1] = 0;

		tx_offset++;
		buf_offset += 2;
	}

	// set bit-8 on last byte
	m_buffer[buf_offset -1] |= 0x01;

	// based on mode, reset tx counter
	switch (m_reg[REG_1_MODE])
	{
		case 0x08:
		case 0x09:
			// do nothing
			m_txblock = 0x01;
			break;
		case 0x0c:
		case 0x0d:
			m_reg[REG_5_TXSIZE] = 0;
			m_txblock = 0x01;
			break;
		default:
			// do nothing
			break;
	}

	m_txsize = 0;
	send_frame(data_size);
}

void namco_c139_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		m_linktimer = 0x0100;
		m_txblock = 0x00;
	}
}

unsigned namco_c139_device::find_sync_bit(unsigned tx_offset, unsigned tx_mask)
{
	// cybrcycc
	if ((m_ram[(tx_offset) & tx_mask] & 0x01ff) == 0x1ff)
		return 0;

	// hack to find sync bit in data area
	for (unsigned i = 0; i < 0x08; i++)
	{
		unsigned subOffset = i * 0x80;
		for (unsigned j = 0; j < 0x100; j++)
		{
			if (m_ram[(tx_offset + subOffset + j) & tx_mask] & 0x0100)
			{
				if (i > 0)
					m_reg[REG_7_TXOFFSET] += subOffset * 2;
				return j + 1;
			}
		}
	}
	return 0;
}
