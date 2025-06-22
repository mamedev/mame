// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Sega MODEL2 COMMUNICATION BOARD 837-10537
( http://images.arianchen.de/sega-comm/model2-front.jpg / http://images.arianchen.de/sega-comm/model2-back.jpg )
|-----------------------------------------------------------------------------|
| |-------------------|                             |-------------------|     |
| |-------------------|                             |-------------------|     |
|          CN5                                               CN4           -- |
| LED    Z80        LH5268A          16726.7        JP6 JP5 JP4 JP1        || |
|                                                                         C|| |
|        315-5751   315-5752                        uPD72103              N|| |
|                                                                     JP7 8|| |
|        LH5268A    LH5268A                                           JP8  || |
|                                                                     JP9  -- |
|        LH5268A    LH5268A                         315-5753A    315-5547     |
|                                                                             |
|                                                                         CN6 |
| CN9                                                                         |
|                                                                         CN7 |
|          CN3                      CN2                      CN1              |
| |-------------------|    |-------------------|    |-------------------|     |
| |-------------------|    |-------------------|    |-------------------|     |
|-----------------------------------------------------------------------------|

    Setup:
        Z80        Zilog Z0840008PSC Z80 CPU (DIP40)
        LH5268A    SHARP LH5268AD-10LL 8k x8 SRAM (DIP28)
        uPD72103   NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        16726.7    M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-16726')
        315-5751   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A  AMI 18CV8PC-15 PAL (DIP20)
        315-5547   AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-10537  Daytona USA

    EEPROM:
        16726.7    Daytona USA


Sega PC BD MODEL2 A-CRX COMMUNICATION 837-11525
( http://images.arianchen.de/sega-comm/model2a-front.jpg / http://images.arianchen.de/sega-comm/model2a-back.jpg )
|-------------------------------------------------------------------------------------------|
| |-------------------|                             |-------------------|    |---------|    |
| |-------------------|                             |-------------------|    |---------|    |
|          CN5                                               CN4                CN10        |
| LED    Z80        LH5268A          16726.7        JP6 JP5 JP4 JP1                         |
|                                                                                           |
|        315-5751   315-5752                        uPD72103                                |
|                                                                                           |
|        LH5268A    LH5268A                                                                 |
|                                                                                        -- |
|        LH5268A    LH5268A                         315-5753A    315-5547                || |
|                                                                                       C|| |
|                                                                                       N|| |
| CN9                                                                                   8|| |
|                                                                                   JP7  || |
|          CN3                      CN2                      CN1                    JP8  -- |
| |-------------------|    |-------------------|    |-------------------|           JP9     |
| |-------------------|    |-------------------|    |-------------------|                   |
|-------------------------------------------------------------------------------------------|

    Setup:
        Z80        Zilog Z0840008PSC Z80 CPU (DIP40)
        LH5268A    SHARP LH5268AD-10LL 8k x8 SRAM (DIP28)
        uPD72103   NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        16726.7    M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-16726')
        315-5751   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A  AMI 18CV8PC-15 PAL (DIP20)
        315-5547   AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-11525
        837-11572  Sega Rally Championship

    EEPROM:
        16726.7    Sega Rally Championship
        18643.7    ManxTT
        18643A.7   ManxTT


Sega PC BD MODEL2 B-CRX COMMUNICATION 837-11615
( http://images.arianchen.de/sega-comm/model2b-com_top.jpg / http://images.arianchen.de/sega-comm/model2b-com_bot.jpg )
|-------------------------------------------------------------------------------------------|
|                                                                                           |
|  --                                                                                       |
|  ||          18643A.7    MB84256A        Z80                                              |
| C||                                                      MB84256A                         |
| N||  JP9                       uPD72103A 315-5751                                         |
| 8||  JP8                                                 MB84256A                         |
|  ||  JP7                                 315-5752                                         |
|  --                                                      MB84256A                         |
|                                                                                           |
|                                                          MB84256A                         |
|                                          315-5753A                                        |
| LED                                                                                       |
|                                                    315-5547A                              |
|                                                                                           |
|                                                                              CN1          |
|                                                                    |-------------------|  |
|                                                  CN3               |-------------------|  |
|-------------------------------------------------------------------------------------------|

    Setup:
        Z80          Zilog Z0840008PSC Z80 CPU (DIP40)
        MB84256A     Fujitsu MB84256A-70LL 32k x8 SRAM (SDIP28)
        uPD72103A    NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        18643A.7     M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-18643A')
        315-5751     Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752     Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A    ICT PEEL18CV8P-15 PAL (DIP20)
        315-5547A    AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-11615    Virtua On
        837-11615-02 Virtua On

    EEPROM:
        18643.7      Virtua On
        18643A.7     Virtua On




Sega PC BD MODEL2 C-CRX COMMUNICATION 837-12839
( http://images.arianchen.de/sega-comm/model2c-com_top.jpg )
|-------------------------------------------------------------------------------------------|
|                                                                                           |
|  --                                                                                       |
|  ||          18643A.7    MB84256A        Z80                                              |
| C||                                                      MB84256A                         |
| N||  JP9                       uPD72103A 315-5751                                         |
| 8||  JP8                                                 MB84256A                         |
|  ||  JP7                                 315-5752                                         |
|  --                                                      MB84256A                         |
|                                                                                           |
|                                                          MB84256A                         |
|                                          315-5753A                                        |
| LED                                                                                       |
|                                                    315-5547A                              |
|                                                                                           |
|                                                                              CN1          |
|                                                                    |-------------------|  |
|                                                  CN3               |-------------------|  |
|-------------------------------------------------------------------------------------------|

    Setup:
        Z80        Zilog Z0840008PSC Z80 CPU (DIP40)
        MB84256A   Fujitsu MB84256A-70LL 32k x8 SRAM (SDIP28)
        uPD72103A  NEC uPD72103 HDLC/Frame Relay Controller (QFP80)
        18643A.7   M27C1001 128k x8 EPROM (DIP32, labelled 'EPR-18643A')
        315-5751   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5752   Lattice GAL16V8B-25LP PAL (DIP20)
        315-5753A  ICT PEEL18CV8P-15 PAL (DIP20)
        315-5547A  AMI 18CV8PC-25 PAL (DIP20)

    Board:
        837-12839  Sega Touring Car Championship

    EEPROM:
        18643A.7   Sega Touring Car Championship
*/

#include "emu.h"
#include "m2comm.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

#ifdef M2COMM_SIMULATION
class sega_m2comm_device::context
{
public:
	context() :
	m_acceptor(m_ioctx),
	m_sock_rx(m_ioctx),
	m_sock_tx(m_ioctx),
	m_timeout_tx(m_ioctx),
	m_state_rx(0U),
	m_state_tx(0U)
	{
	}

	void start()
	{
	}

	void reset(std::string localhost, std::string localport, std::string remotehost, std::string remoteport)
	{
		std::error_code err;
		if (m_acceptor.is_open())
			m_acceptor.close(err);
		if (m_sock_rx.is_open())
			m_sock_rx.close(err);
		if (m_sock_tx.is_open())
			m_sock_tx.close(err);
		m_timeout_tx.cancel();
		m_state_rx.store(0);
		m_state_tx.store(0);

		asio::ip::tcp::resolver resolver(m_ioctx);

		for (auto &&resolveIte : resolver.resolve(localhost, localport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_localaddr = resolveIte.endpoint();
			LOG("M2COMM: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("M2COMM: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("M2COMM: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("M2COMM: remotehost resolve error: %s\n", err.message());
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
		m_state_rx.store(0);
		m_state_tx.store(0);
		m_ioctx.stop();
	}

	void check_sockets()
	{
		// if async operation in progress, poll context
		if ((m_state_rx > 0) || (m_state_tx > 0))
			m_ioctx.poll();

		// start acceptor if needed
		if (m_localaddr && m_state_rx.load() == 0)
		{
			start_accept();
		}

		// connect socket if needed
		if (m_remoteaddr && m_state_tx.load() == 0)
		{
			start_connect();
		}
	}

	bool connected()
	{
		return m_state_rx.load() == 2 && m_state_tx.load() == 2;
	}

	unsigned receive(uint8_t *buffer, unsigned data_size)
	{
		if (m_state_rx.load() < 2)
			return UINT_MAX;

		m_ioctx.poll();

		if (data_size > m_fifo_rx.used())
			return 0;

		return m_fifo_rx.read(&buffer[0], data_size, false);
	}

	unsigned send(uint8_t *buffer, unsigned data_size)
	{
		if (m_state_tx.load() < 2)
			return UINT_MAX;

		if (data_size > m_fifo_tx.free())
		{
			LOG("M2COMM: TX buffer overflow\n");
			return UINT_MAX;
		}

		bool const sending = m_fifo_tx.used();
		m_fifo_tx.write(&buffer[0], data_size);
		if (!sending)
			start_send_tx();

		m_ioctx.poll();

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
				used = std::min<unsigned>(m_buffer.size() - m_wp, data_size);
				std::copy_n(&buffer[0], used, &m_buffer[m_wp]);
				m_wp = (m_wp + used) % m_buffer.size();
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
				used = std::min<std::size_t>(m_buffer.size() - rp, data_size);
				std::copy_n(&m_buffer[rp], used, &buffer[0]);
				rp = (rp + used) % m_buffer.size();
			}
			unsigned const block = std::min<unsigned>(data_size - used, m_wp - rp);
			if (block)
			{
				std::copy_n(&m_buffer[rp], block, &buffer[used]);
				used += block;
				rp += block;
			}
			if (!peek)
			{
				m_rp = (m_rp + used) % m_buffer.size();
				m_used -= used;
			}
			return used;
		}

		void consume(unsigned data_size)
		{
			m_rp = (m_rp + data_size) % m_buffer.size();
			m_used -= data_size;
		}

		unsigned used()
		{
			return m_used;
		}

		unsigned free()
		{
			return m_buffer.size() - m_used;
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

	void start_accept()
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
					osd_printf_verbose("M2COMM: RX listen on %s\n", *m_localaddr);
					m_acceptor.async_accept(
							[this] (std::error_code const &err, asio::ip::tcp::socket sock)
							{
								if (err)
								{
									LOG("M2COMM: RX error accepting - %d %s\n", err.value(), err.message());
									std::error_code e;
									m_acceptor.close(e);
									m_state_rx.store(0);
								}
								else
								{
									LOG("M2COMM: RX connection from %s\n", sock.remote_endpoint());
									std::error_code e;
									m_acceptor.close(e);
									m_sock_rx = std::move(sock);
									m_sock_rx.set_option(asio::socket_base::keep_alive(true));
									m_state_rx.store(2);
									start_receive_rx();
								}
							});
					m_state_rx.store(1);
				}
			}
		}
		if (err)
		{
			LOG("M2COMM: RX failed - %d %s\n", err.value(), err.message());
		}
	}

	void start_connect()
	{
		std::error_code err;
		if (m_sock_tx.is_open())
			m_sock_tx.close(err);
		m_sock_tx.open(m_remoteaddr->protocol(), err);
		if (!err)
		{
			m_sock_tx.set_option(asio::ip::tcp::no_delay(true));
			m_sock_tx.set_option(asio::socket_base::keep_alive(true));
			osd_printf_verbose("M2COMM: TX connecting to %s\n", *m_remoteaddr);
			m_timeout_tx.expires_after(std::chrono::seconds(10));
			m_timeout_tx.async_wait(
					[this] (std::error_code const &err)
					{
						if (!err && m_state_tx.load() == 1)
						{
							osd_printf_verbose("M2COMM: TX connect timed out\n");
							std::error_code e;
							m_sock_tx.close(e);
							m_state_tx.store(0);
						}
					});
			m_sock_tx.async_connect(
					*m_remoteaddr,
					[this] (std::error_code const &err)
					{
						m_timeout_tx.cancel();
						if (err)
						{
							osd_printf_verbose("M2COMM: TX connect error - %d %s\n", err.value(), err.message());
							std::error_code e;
							m_sock_tx.close(e);
							m_state_tx.store(0);
						}
						else
						{
							LOG("M2COMM: TX connection established\n");
							m_state_tx.store(2);
						}
					});
			m_state_tx.store(1);
		}
	}

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
						LOG("M2COMM: TX connection error: %s\n", err.message().c_str());
						m_sock_tx.close();
						m_state_tx.store(0);
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
							LOG("M2COMM: RX connection error: %s\n", err.message());
						else
							LOG("M2COMM: RX connection lost\n");
						m_sock_rx.close();
						m_state_rx.store(0);
						m_fifo_rx.clear();
					}
					else
					{
						if (UINT_MAX == m_fifo_rx.write(&m_buffer_rx[0], length))
						{
							LOG("M2COMM: RX buffer overflow\n");
							m_sock_rx.close();
							m_state_rx.store(0);
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
	std::atomic_uint m_state_rx;
	std::atomic_uint m_state_tx;
	fifo m_fifo_rx;
	fifo m_fifo_tx;
	std::array<uint8_t, 0x400> m_buffer_rx;
	std::array<uint8_t, 0x400> m_buffer_tx;
};
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_MODEL2_COMM, sega_m2comm_device, "m2comm", "Sega Model2 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_m2comm_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_m2comm_device - constructor
//-------------------------------------------------

sega_m2comm_device::sega_m2comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_MODEL2_COMM, tag, owner, clock)
{
	m_frameoffset = 0x1c0; // default
#ifdef M2COMM_SIMULATION
	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_m2comm_device::device_start()
{
#ifdef M2COMM_SIMULATION
	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);
	m_context->start();
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_m2comm_device::device_reset()
{
	std::fill(std::begin(m_shared), std::end(m_shared), 0);
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
#ifdef M2COMM_SIMULATION
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);

	auto const &opts = mconfig().options();
	m_context->reset(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

	m_linkenable = 0;
	m_linktimer = 0;
	m_linkalive = 0;
	m_linkid = 0;
	m_linkcount = 0;
	m_zfg_delay = 0;
#endif
}

void sega_m2comm_device::device_stop()
{
#ifdef M2COMM_SIMULATION
	m_context->stop();
	m_context.reset();
#endif
}

uint8_t sega_m2comm_device::zfg_r(offs_t offset)
{
	uint8_t result = m_zfg | (~m_fg << 7) | 0x7e;
	LOG("m2comm-zfg_r: read register %02x for value %02x\n", offset, result);
	return result;
}

void sega_m2comm_device::zfg_w(uint8_t data)
{
	LOG("m2comm-zfg_w: %02x\n", data);
	m_zfg = data & 0x01;
}

uint8_t sega_m2comm_device::share_r(offs_t offset)
{
	uint8_t result = m_shared[offset];
	LOG("m2comm-share_r: read shared memory %02x for value %02x\n", offset, result);
	return result;
}

void sega_m2comm_device::share_w(offs_t offset, uint8_t data)
{
	LOG("m2comm-share_w: %02x %02x\n", offset, data);
	m_shared[offset] = data;
}

uint8_t sega_m2comm_device::cn_r()
{
	return m_cn | 0xfe;
}

void sega_m2comm_device::cn_w(uint8_t data)
{
	m_cn = data & 0x01;

#ifndef M2COMM_SIMULATION
	if (!m_cn)
		device_reset();
#else
	if (!m_cn)
	{
		// reset command
		LOG("M2COMM: board disabled\n");
		m_linkenable = 0x00;
		m_zfg = 0;
		m_cn = 0;
		m_fg = 0;
	}
	else
	{
		// init command
		LOG("M2COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00e8; // 58 fps * 4s

		// zero memory
		for (unsigned i = 0; i < 0x4000; i++)
		{
			m_shared[i] = 0x00;
		}

		m_shared[0x01] = 0x02;
		// TODO - check EPR-16726 on Daytona USA and Sega Rally Championship
		// EPR-18643(A) - these are accessed by VirtuaON and Sega Touring Car Championship

		// frame_size - 0x0e00. is it = frameoffset*8 ? if true - it should be 0xc00 for Power Sled
		m_shared[0x12] = 0x00;
		m_shared[0x13] = 0x0e;

		// frame_offset - 0x01c0 in most games or 0x180 in Power Sled
		m_shared[0x14] = m_frameoffset & 0xff;
		m_shared[0x15] = m_frameoffset >> 8;

		comm_tick();
	}
#endif
}

uint8_t sega_m2comm_device::fg_r()
{
#ifdef M2COMM_SIMULATION
	read_fg();
#endif
	return m_fg | (~m_zfg << 7) | 0x7e;
}

void sega_m2comm_device::fg_w(uint8_t data)
{
	m_fg = data & 0x01;
}

void sega_m2comm_device::check_vint_irq()
{
#ifndef M2COMM_SIMULATION
#else
	comm_tick();
#endif
}

#ifdef M2COMM_SIMULATION
void sega_m2comm_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linkenable == 0x01)
	{
		unsigned frame_size = m_shared[0x13] << 8 | m_shared[0x12];
		unsigned data_size = frame_size + 1;

		// EPR-16726 uses m_fg for Master/Slave
		// EPR-18643(A) seems to check m_shared[1], with a fallback to m_fg
		bool is_master = (m_fg == 0x01 || m_shared[1] == 0x01);
		bool is_slave = (m_fg == 0x00 && m_shared[1] == 0x02);
		bool is_relay = (m_fg == 0x00 && m_shared[1] == 0x00);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[0] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			m_shared[0] = 0x00;
			m_shared[2] = 0xff;
			m_shared[3] = 0xff;

			// if both sockets are there check ring
			if (m_context->connected())
			{
				m_zfg ^= 0x01;

				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer[0];

					// 0xFF - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer[1];
							m_linktimer = 0x00;
						}
						else
						{
							// slave get own id, relay does nothing
							if (is_slave)
							{
								m_buffer[1]++;
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (is_slave)
						{
							// fetch linkid and linkcount, then decrease linkid
							m_linkid = m_buffer[1];
							m_linkcount = m_buffer[2];
							m_buffer[1]--;

							// forward message to other nodes
							send_frame(data_size);
						}
						else if (is_relay)
						{
							// fetch linkid and linkcount, then decrease linkid
							m_linkid = 0x00;
							m_linkcount = m_buffer[2];

							// forward message to other nodes
							send_frame(data_size);
						}

						// consider it done
						LOG("M2COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[0] = 0x01;
						m_shared[2] = m_linkid;
						m_shared[3] = m_linkcount;
					}

					if (m_linkalive == 0x00)
						recv = read_frame(data_size);
					else
						recv = 0;
				}

				// if we are master and link is not yet established
				if (is_master && (m_linkalive == 0x00))
				{
					// send first packet
					if (m_linktimer == 0x01)
					{
						m_buffer[0] = 0xff;
						m_buffer[1] = 0x01;
						m_buffer[2] = 0x00;
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xfe;
						m_buffer[1] = m_linkcount;
						m_buffer[2] = m_linkcount;
						send_frame(data_size);

						// consider it done
						LOG("M2COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[0] = 0x01;
						m_shared[2] = m_linkid;
						m_shared[3] = m_linkcount;
					}

					else if (m_linktimer > 0x01)
					{
						// decrease delay timer
						m_linktimer--;
					}
				}
			}
		}

		// if link established
		if (m_linkalive == 0x01)
		{
			unsigned frame_start = 0x2000;
			unsigned frame_offset = frame_start | m_shared[0x15] << 8 | m_shared[0x14];

			do
			{
				// try to read a message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer[0];
					if (idx >= 0 && idx <= m_linkcount)
					{
						// save message to "ring buffer"
						std::copy_n(&m_buffer[1], frame_size, &m_shared[frame_offset]);

						m_zfg ^= 0x01;
						if (is_slave)
							send_data(m_linkid, frame_start, frame_size, data_size);
						else if (is_relay)
							send_frame(data_size);
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xFC - VSYNC
							m_linktimer = 0x00;

							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			if (is_master)
			{
				// update "ring buffer" if link established
				send_data(m_linkid, frame_start, frame_size, data_size);

				// send vsync
				m_buffer[0] = 0xfc;
				m_buffer[1] = 0x01;
				send_frame(data_size);
			}

			m_zfg_delay = 0x02;
		}
	}
}

void sega_m2comm_device::read_fg()
{
	if (m_zfg_delay > 0x00)
	{
		m_zfg_delay--;
		return;
	}
	if (m_linkalive == 0x01)
	{
		unsigned frame_start = 0x2000;
		unsigned frame_offset = frame_start | m_shared[0x15] << 8 | m_shared[0x14];

		unsigned frame_size = m_shared[0x13] << 8 | m_shared[0x12];
		unsigned data_size = frame_size + 1;

		// EPR-16726 uses m_fg for Master/Slave
		// EPR-18643(A) seems to check m_shared[1], with a fallback to m_fg
		bool is_master = (m_fg == 0x01 || m_shared[1] == 0x01);
		bool is_slave = (m_fg == 0x00 && m_shared[1] == 0x02);
		bool is_relay = (m_fg == 0x00 && m_shared[1] == 0x00);

		do
		{
			// try to read a message
			unsigned recv = read_frame(data_size);
			while (recv > 0)
			{
				// check if valid id
				uint8_t idx = m_buffer[0];
				if (idx >= 0 && idx <= m_linkcount)
				{
					// save message to "ring buffer"
					for (unsigned j = 0x00; j < frame_size; j++)
					{
						m_shared[frame_offset + j] = m_buffer[1 + j];
					}
					m_zfg ^= 0x01;
					if (is_slave)
						send_data(m_linkid, frame_start, frame_size, data_size);
					else if (is_relay)
						send_frame(data_size);
				}
				else
				{
					if (idx == 0xfc)
					{
						// 0xFC - VSYNC
						m_linktimer = 0x00;

						if (!is_master)
							// forward message to other nodes
							send_frame(data_size);
					}
				}

				// try to read another message
				recv = read_frame(data_size);
			}
		}
		while (m_linktimer == 0x01);
	}
}

unsigned sega_m2comm_device::read_frame(unsigned data_size)
{
	unsigned bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("M2COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
		return 0;
	}
	return bytes_read;
}

void sega_m2comm_device::send_data(uint8_t frame_type, unsigned frame_start, unsigned frame_size, unsigned data_size)
{
	m_buffer[0] = frame_type;
	std::copy_n(&m_shared[frame_start], frame_size, &m_buffer[1]);
	send_frame(data_size);
}

void sega_m2comm_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("M2COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
	}
}
#endif
