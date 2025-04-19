// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Sega System 32 Comm PCB 837-9409
( http://images.arianchen.de/sega-comm/f1superlap_comm.jpg )
|--------------------------------------------------------------------------------|
|                 |-----------------|   |---------------------------------|      |
|                 |-----------------|   |---------------------------------|      |
|                         CNJ        PAL                CNH                      |
|                                                       MB89374                  |
|                                    Z80                                         |
|                                                       MB89237A                 |
|    PAL                           15612.17                                      |
|                                                                                |
|    MB8464                          MB8464                                      |
|                CN?                                                             |
| |---------------------------------|                                            |
| |---------------------------------|  LED     CN? CN?                 CNK       |
|--------------------------------------------------------------------------------|
    Setup:
        Z80      - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz (DIP40)
        MB89237A - Fujitsu MB89237A DMA-Controller (DIP20) [most likely i8237A clone]
        MB89374  - Fujitsu MB89374 Data Link Controller (SDIP42)
        MB8464   - Fujitsu MB8464 8k x8 SRAM (DIP28)
        MB8421   - Fujitsu MB8421-12LP 2k x8 SRAM (SDIP52)

    Board:
        837-9409    F1 Super Lap

    EEPROM:
        14084.17    Rad Rally
        15612.17    F1 Super Lap

Sega System Multi32 Comm PCB 837-8792-91
( http://images.arianchen.de/sega-comm/orunners-front.jpg / http://images.arianchen.de/sega-comm/orunners-back.jpg )
|--------------------------------------------------------------------------------|
| |---------------------------------|   |---------------------------------|      |
| |---------------------------------|   |---------------------------------|      |
|                CN3                                    CN4                      |
|                                                                                |
|    Z80                   MB89374                MB89237A            MB8421     |
|                                                                                |
|    15033.17             315-5610                                               |
|                                                                                |
|    MB8464A                                                        315-5506     |
|                CN1                                         CN2                 |
| |---------------------------------|        |---------------------------------| |
| |---------------------------------|        |---------------------------------| |
|                                  CN8 CN9                                       |
|--------------------------------------------------------------------------------|
    Setup:
        15033.17 - INTEL D27C100 128k x8 EPROM (DIP32, labelled 'EPR-15033')
        Z80      - Zilog Z0840004PSC Z80 CPU, running at 4.000MHz (DIP40)
        MB89237A - Fujitsu MB89237A DMA-Controller (DIP20) [most likely i8237A clone]
        MB89374  - Fujitsu MB89374 Data Link Controller (SDIP42)
        MB8421   - Fujitsu MB8421-12LP 2k x8 SRAM (SDIP52)
        MB8464A  - Fujitsu MB8464-10LL 8k x8 SRAM (DIP28)
        315-5611 - Lattice GAL16V8A PAL (DIP20)
        315-5506 - Lattice GAL16V8A PAL (DIP20)

    Board:
        837-8792  OutRunners, Stadium Cross

    EEPROM:
        15033.17  OutRunners, Stadium Cross
*/

#include "emu.h"
#include "s32comm.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

#ifdef S32COMM_SIMULATION
class sega_s32comm_device::context
{
public:
	context() :
	m_acceptor(m_ioctx),
	m_sock_rx(m_ioctx),
	m_sock_tx(m_ioctx),
	m_tx_timeout(m_ioctx)
	{
	}

	void start(std::string localhost, std::string localport, std::string remotehost, std::string remoteport)
	{
		std::error_code err;
		asio::ip::tcp::resolver resolver(m_ioctx);

		for (auto &&resolveIte : resolver.resolve(localhost, localport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_localaddr = resolveIte.endpoint();
			LOG("S32COMM: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("S32COMM: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("S32COMM: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("S32COMM: remotehost resolve error: %s\n", err.message());
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
		m_tx_timeout.cancel();
		m_tx_state = 0;
		m_rx_state = 0;
	}

	void check_sockets()
	{
		// if async operation in progress, poll context
		if ((m_rx_state == 1) || (m_tx_state == 1))
			m_ioctx.poll();

		// start acceptor if needed
		if (m_localaddr && m_rx_state == 0)
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
						osd_printf_verbose("S32COMM: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("S32COMM: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_rx_state = 0;
									}
									else
									{
										LOG("S32COMM: RX connection from %s\n", sock.remote_endpoint());
										std::error_code e;
										m_acceptor.close(e);
										m_sock_rx = std::move(sock);
										m_sock_rx.non_blocking(true);
										m_sock_rx.set_option(asio::socket_base::receive_buffer_size(524288));
										m_sock_rx.set_option(asio::socket_base::keep_alive(true));
										m_rx_state = 2;
									}
								});
						m_rx_state = 1;
					}
				}
			}
			if (err)
			{
				LOG("S32COMM: RX failed - %d %s\n", err.value(), err.message());
			}
		}

		// connect socket if needed
		if (m_remoteaddr && m_tx_state == 0)
		{
			std::error_code err;
			if (m_sock_tx.is_open())
				m_sock_tx.close(err);
			m_sock_tx.open(m_remoteaddr->protocol(), err);
			if (!err)
			{
				m_sock_tx.non_blocking(true);
				m_sock_tx.set_option(asio::ip::tcp::no_delay(true));
				m_sock_tx.set_option(asio::socket_base::send_buffer_size(65536));
				m_sock_tx.set_option(asio::socket_base::keep_alive(true));
				osd_printf_verbose("S32COMM: TX connecting to %s\n", *m_remoteaddr);
				m_tx_timeout.expires_after(std::chrono::seconds(10));
				m_tx_timeout.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_tx_state == 1)
							{
								osd_printf_verbose("S32COMM: TX connect timed out\n");
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
						});
				m_sock_tx.async_connect(
						*m_remoteaddr,
						[this] (std::error_code const &err)
						{
							m_tx_timeout.cancel();
							if (err)
							{
								osd_printf_verbose("S32COMM: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
							else
							{
								LOG("S32COMM: TX connection established\n");
								m_tx_state = 2;
							}
						});
				m_tx_state = 1;
			}
		}
	}

	bool connected()
	{
		return m_rx_state == 2 && m_tx_state == 2;
	}

	unsigned receive(uint8_t *buffer, unsigned data_size)
	{
		if (m_rx_state != 2)
			return 0;

		std::error_code err;
		std::size_t bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), asio::socket_base::message_peek, err);
		if (err == asio::error::would_block)
			return 0;

		if (bytes_read == data_size)
			bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), 0, err);

		if (err)
		{
			osd_printf_verbose("S32COMM: RX error receiving - %d %s\n", err.value(), err.message());
			m_sock_rx.close(err);
			m_rx_state = 0;
			return UINT_MAX;
		}

		if (bytes_read == data_size)
			return bytes_read;
		return 0;
	}

	unsigned send(uint8_t *buffer, unsigned data_size)
	{
		if (m_tx_state != 2)
			return 0;

		std::error_code err;
		std::size_t bytes_sent = m_sock_tx.send(asio::buffer(&buffer[0], data_size), 0, err);
		if (err || bytes_sent != data_size)
		{
			osd_printf_verbose("S32COMM: TX error sending - %d %s\n", err.value(), err.message());
			m_sock_tx.close(err);
			m_tx_state = 0;
			return UINT_MAX;
		}
		return data_size;
	}

private:
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
	asio::steady_timer m_tx_timeout;
	uint8_t m_rx_state;
	uint8_t m_tx_state;
};
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_SYSTEM32_COMM, sega_s32comm_device, "s32comm", "Sega System 32 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_s32comm_device::device_add_mconfig(machine_config &config)
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_s32comm_device - constructor
//-------------------------------------------------

sega_s32comm_device::sega_s32comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_SYSTEM32_COMM, tag, owner, clock)
{
#ifdef S32COMM_SIMULATION
	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_s32comm_device::device_start()
{
#ifdef S32COMM_SIMULATION
	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_s32comm_device::device_reset()
{
	std::fill(std::begin(m_shared), std::end(m_shared), 0);
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
#ifdef S32COMM_SIMULATION
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
	m_context->stop();

	auto const &opts = mconfig().options();
	m_context->start(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

	m_linkenable = 0;
	m_linktimer = 0;
	m_linkalive = 0;
	m_linkid = 0;
	m_linkcount = 0;
#endif
}

void sega_s32comm_device::device_stop()
{
#ifdef S32COMM_SIMULATION
	m_context->stop();
	m_context.reset();
#endif
}

uint8_t sega_s32comm_device::zfg_r(offs_t offset)
{
	uint8_t result = m_zfg | 0xFE;
	if (!machine().side_effects_disabled())
		LOG("s32comm-zfg_r: read register %02x for value %02x\n", offset, result);
	return result;
}

void sega_s32comm_device::zfg_w(uint8_t data)
{
	LOG("s32comm-zfg_w: %02x\n", data);
	m_zfg = data & 0x01;
}

uint8_t sega_s32comm_device::share_r(offs_t offset)
{
	uint8_t result = m_shared[offset];
	if (!machine().side_effects_disabled())
		LOG("s32comm-share_r: read shared memory %02x for value %02x\n", offset, result);
	return result;
}

void sega_s32comm_device::share_w(offs_t offset, uint8_t data)
{
	LOG("s32comm-share_w: %02x %02x\n", offset, data);
	m_shared[offset] = data;
}

uint8_t sega_s32comm_device::cn_r()
{
	return m_cn | 0xfe;
}

void sega_s32comm_device::cn_w(uint8_t data)
{
	m_cn = data & 0x01;

#ifndef S32COMM_SIMULATION
	if (!m_cn)
		device_reset();
#else
	if (!m_cn)
	{
		// reset command
		osd_printf_verbose("S32COMM: board disabled\n");
		m_linkenable = 0x00;
	}
	else
	{
		// init command
		osd_printf_verbose("S32COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00e8; // 58 fps * 4s

		comm_tick();
	}
#endif
}

uint8_t sega_s32comm_device::fg_r()
{
	return m_fg | (~m_zfg << 7) | 0x7E;
}

void sega_s32comm_device::fg_w(uint8_t data)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void sega_s32comm_device::check_vint_irq()
{
#ifndef S32COMM_SIMULATION
#else
	comm_tick();
#endif
}

#ifdef S32COMM_SIMULATION
void sega_s32comm_device::set_linktype(uint16_t linktype)
{
	m_linktype = linktype;

	switch (m_linktype)
	{
		case 14084:
			// Rad Rally
			LOG("S32COMM: set mode 'EPR-14084 - Rad Rally'\n");
			break;
		case 15033:
			// Stadium Cross / OutRunners
			LOG("S32COMM: set mode 'EPR-15033 - Stadium Cross / OutRunners'\n");
			break;
		case 15612:
			// F1 Super Lap
			LOG("S32COMM: set mode 'EPR-15612 - F1 Super Lap'\n");
			break;
		default:
			logerror("S32COMM-set_linktype: unknown linktype %d\n", linktype);
			break;
	}
}

void sega_s32comm_device::comm_tick()
{
	m_context->check_sockets();

	switch (m_linktype)
	{
		case 14084:
			// Rad Rally
			comm_tick_14084();
			break;
		case 15033:
			// Stadium Cross / OutRunners
			comm_tick_15033();
			break;
		case 15612:
			// F1 Super Lap
			comm_tick_15612();
			break;
		default:
			// do nothing
			break;
	}
}

unsigned sega_s32comm_device::read_frame(unsigned data_size)
{
	int bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("S32COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
		return 0;
	}
	return bytes_read;
}

void sega_s32comm_device::send_data(uint8_t frame_type, unsigned frame_start, unsigned frame_size, unsigned data_size)
{
	m_buffer[0] = frame_type;
	for (unsigned i = 0x00; i < frame_size; i++)
	{
		m_buffer[1 + i] = m_shared[frame_start + i];
	}
	// forward message to next node
	send_frame(data_size);
}

void sega_s32comm_device::send_frame(unsigned data_size)
{
	int bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("S32COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
	}
}

void sega_s32comm_device::comm_tick_14084()
{
	// m_shared[0] = node count
	// m_shared[1] = node id
	// m_shared[2] = node mode (0 = slave, 1 = master, ff = relay)
	// m_shared[3] = node ready-to-send
	// m_shared[4] = node link status (0 = offline, 1 = online)
	if (m_linkenable == 0x01)
	{
		unsigned frame_size = 0x0080;
		unsigned data_size = frame_size + 1;

		bool is_master = (m_shared[2] == 0x01);
		bool is_slave = (m_shared[2] == 0x00);
		bool is_relay = (m_shared[2] == 0xFF);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[4] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			m_shared[4] = 0x00;

			// if both sockets are there check ring
			if (m_context->connected())
			{
				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer[0];

					// 0xff - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer[1];
							m_linktimer = 0x00;
						}
						else if (is_slave || is_relay)
						{
							// slave gets own id
							if (is_slave)
							{
								m_buffer[1]++;
								m_linkid = m_buffer[1];
							}

							// slave and relay forward message
							send_frame(data_size);
						}
					}

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer[1];

							// slave and relay forward message
							send_frame(data_size);
						}

						// consider it done
						LOG("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
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
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xfe;
						m_buffer[1] = m_linkcount;
						send_frame(data_size);

						// consider it done
						LOG("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
					}

					else if (m_linktimer > 0x01)
					{
						// decrease delay timer
						m_linktimer--;
					}
				}
			}
		}

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			do
			{
				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							unsigned frame_offset = idx * frame_size;
							for (unsigned j = 0x00; j < frame_size; j++)
							{
								m_shared[frame_offset + j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xfc - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xfd - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								unsigned frame_offset = 0x05;
								for (unsigned j = 0x00; j < 0x0b; j++)
								{
									m_shared[frame_offset + j] = m_buffer[1 + j];
								}

								// forward message to other nodes
								send_frame(data_size);
							}
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00)
			{
				// check ready-to-send flag
				if (m_shared[3] != 0x00)
				{
					unsigned frame_start = 0x0480;
					send_data(m_linkid, frame_start, frame_size, data_size);

					// save message to "ring buffer"
					unsigned frame_offset = m_linkid * frame_size;
					for (unsigned j = 0x00; j < frame_size; j++)
					{
						m_shared[frame_offset + j] = m_buffer[1 + j];
					}
				}

				if (is_master)
				{
					// master sends some additional status bytes
					send_data(0xfd, 0x05, 0x0b, data_size);

					// send vsync
					m_buffer[0] = 0xfc;
					m_buffer[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void sega_s32comm_device::comm_tick_15033()
{
	// m_shared[0] = node count
	// m_shared[1] = node id
	// m_shared[2] = node mode (0 = slave, 1 = master, ff = relay)
	// m_shared[3] = node ready-to-send
	// m_shared[4] = node link status (0 = offline, 1 = online)
	if (m_linkenable == 0x01)
	{
		unsigned frame_size = 0x00e0;
		unsigned data_size = frame_size + 1;

		bool is_master = (m_shared[2] == 0x01);
		bool is_slave = (m_shared[2] == 0x00);
		bool is_relay = (m_shared[2] == 0x02);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[0] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			if (m_shared[0] == 0x56 && m_shared[1] == 0x37 && m_shared[2] == 0x30)
			{
				for (unsigned j = 0x003; j < 0x0800; j++)
				{
					m_shared[j] = 0;
				}
				m_shared[0x08] = 0x5A;
				m_shared[0x09] = 0x38;
				m_shared[0x0A] = 0x30;
			}

			// waiting...
			m_shared[4] = 0x00;

			// if both sockets are there check ring
			if (m_context->connected())
			{
				// try to read one messages
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer[0];

					// 0xff - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer[1];
							m_linktimer = 0x00;
						}
						else if (is_slave || is_relay)
						{
							// slave gets own id
							if (is_slave)
							{
								m_buffer[1]++;
								m_linkid = m_buffer[1];
							}

							// slave and relay forward message
							send_frame(data_size);
						}
					}

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer[1];

							// slave and relay forward message
							send_frame(data_size);
						}

						// consider it done
						LOG("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
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
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xfe;
						m_buffer[1] = m_linkcount;
						send_frame(data_size);

						// consider it done
						LOG("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_shared[4] = 0x01;
						m_shared[1] = m_linkid;
						m_shared[0] = m_linkcount;
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
			unsigned frame_start_rx = 0x0010;
			unsigned frame_start_tx = 0x0710;

			do
			{
				// try to read a message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							unsigned frame_offset = frame_start_rx + ((idx - 1) * frame_size);
							for (unsigned j = 0x00; j < frame_size; j++)
							{
								m_shared[frame_offset + j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xfc - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xfd - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								unsigned frame_offset = 0x05;
								for (unsigned j = 0x00; j < 0x0b; j++)
								{
									m_shared[frame_offset + j] = m_buffer[1 + j];
								}

								// forward message to other nodes
								send_frame(data_size);
							}
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00)
			{
				// check ready-to-send flag
				if (m_shared[3] != 0x00)
				{
					send_data(m_linkid, frame_start_tx, frame_size, data_size);

					// save message to "ring buffer"
					unsigned frame_offset = frame_start_rx + ((m_linkid - 1) * frame_size);
					for (unsigned j = 0x00; j < frame_size; j++)
					{
						m_shared[frame_offset + j] = m_buffer[1 + j];
					}
				}

				if (is_master)
				{
					// master sends some additional status bytes
					send_data(0xfd, 0x05, 0x0b, data_size);

					// send vsync
					m_buffer[0] = 0xfc;
					m_buffer[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 03
			m_shared[3] = 0x00;
		}
	}
}

void sega_s32comm_device::comm_tick_15612()
{
	// m_shared[0] = node link status (5 = linking, 1 = online)
	// m_shared[1] = node mode (0 = relay, 1 = master, 2 = slave)
	// m_shared[2] = node id
	// m_shared[3] = node count
	// m_shared[4] = ready-to-send
	if (m_linkenable == 0x01)
	{
		unsigned frame_size = 0x00e0;
		unsigned data_size = frame_size + 1;

		bool is_master = (m_shared[1] == 0x01);
		bool is_slave = (m_shared[1] == 0x02);
		bool is_relay = (m_shared[1] == 0x00);

		if (m_linkalive == 0x02)
		{
			// link failed...
			m_shared[0] = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established...
			m_shared[0] = 0x05;

			// if both sockets are there check ring
			if (m_context->connected())
			{
				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if message id
					uint8_t idx = m_buffer[0];

					// 0xff - link id
					if (idx == 0xff)
					{
						if (is_master)
						{
							// master gets first id and starts next state
							m_linkid = 0x01;
							m_linkcount = m_buffer[1];
							m_linktimer = 0x00;
						}
						else if (is_slave || is_relay)
						{
							// slave gets own id
							if (is_slave)
							{
								m_buffer[1]++;
								m_linkid = m_buffer[1];
							}

							// slave and relay forward message
							send_frame(data_size);
						}
					}

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer[1];

							// slave and relay forward message
							send_frame(data_size);
						}

						// consider it done
						LOG("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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
						send_frame(data_size);
					}

					// send second packet
					else if (m_linktimer == 0x00)
					{
						m_buffer[0] = 0xfe;
						m_buffer[1] = m_linkcount;
						send_frame(data_size);

						// consider it done
						LOG("S32COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
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

		// update "ring buffer" if link established
		if (m_linkalive == 0x01)
		{
			unsigned frame_start = 0x0010;

			do
			{
				// try to read a message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check if valid id
					uint8_t idx = m_buffer[0];
					if (idx > 0 && idx <= m_linkcount)
					{
						// if not own message
						if (idx != m_linkid)
						{
							// save message to "ring buffer"
							unsigned frame_offset = frame_start + (idx * frame_size);
							for (unsigned j = 0x00; j < frame_size; j++)
							{
								m_shared[frame_offset + j] = m_buffer[1 + j];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}
					else
					{
						if (idx == 0xfc)
						{
							// 0xfc - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xfd - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								unsigned frame_offset = 0x05;
								for (unsigned j = 0x00; j < 0x0b; j++)
								{
									m_shared[frame_offset + j] = m_buffer[1 + j];
								}

								// forward message to other nodes
								send_frame(data_size);
							}
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			while (m_linktimer == 0x01);

			// enable wait for vsync
			m_linktimer = m_framesync;

			// update "ring buffer" if link established
			// live relay does not send data
			if (m_linkid != 0x00)
			{
				// check ready-to-send flag
				if (m_shared[4] != 0x00)
				{
					send_data(m_linkid, frame_start, frame_size, data_size);

					// save message to "ring buffer"
					unsigned frame_offset = frame_start + (m_linkid * frame_size);
					for (unsigned j = 0x00; j < frame_size; j++)
					{
						m_shared[frame_offset + j] = m_buffer[1 + j];
					}
				}

				if (is_master)
				{
					// master sends some additional status bytes
					// master sends additional status bytes
					send_data(0xfd, 0x05, 0x0b, data_size);

					// send vsync
					m_buffer[0] = 0xfc;
					m_buffer[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 04
			m_shared[4] = 0x00;
		}
	}
}
#endif
