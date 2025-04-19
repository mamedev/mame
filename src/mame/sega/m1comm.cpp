// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Comm PCB
--------

MODEL-1 COMMUNICATION BD 837-8842 171-6293B (C) SEGA 1992
( http://images.arianchen.de/sega-comm/model1-front.jpg / http://images.arianchen.de/sega-comm/model1-back.jpg )
|--------------------------------------------------------------------------------|
|                                                                                |
|    MB89237A            MB89374                                                 |
|       JP4                                                                 LED1 |
|    15112.17            Z80                                                     |
|    JP2  JP3                                                       75179        |
|    MB8464              315-5624                                     JP6        |
|                                                       315-5547                 |
|        315-5611                                            SW1    PC910     CN4|
|                                                                                |
|                                                                   PC910     CN5|
|     MB8421             MB8431                                JP7               |
|                                                                   JP5          |
|        JP8                                                                  CN7|
|                CN1                                    CN2                      |
| |---------------------------------|   |---------------------------------|   CN6|
| |---------------------------------|   |---------------------------------|      |
|--------------------------------------------------------------------------------|
Notes:
      15112.17 - AMD AM27C100 128k x8 EPROM (DIP32, labelled 'EPR-15112')
      Z80      - Zilog Z0840008PSC Z80 CPU, running at 8.000MHz (DIP40)
      MB8464   - Fujitsu MB8464 8k x8 SRAM (DIP28)
      MB8421   - Fujitsu MB8421-12LP 2k x8 SRAM (SDIP52)
      MB8431   - Fujitsu MB8431-90LP 2k x8 SRAM (SDIP52)
      MB89237A - Fujitsu MB89237A DMA-Controller (DIP20) [most likely i8237A clone]
      MB89374  - Fujitsu MB89374 Data Link Controller (SDIP42)
      75179    - Texas Instruments SN75179 Differential Driver and Receiver Pair (DIP8)
      315-5547 - AMI 18CV8PC-25 PAL (DIP20)
      315-5624 - MMI PAL16L8BCN PAL (DIP20)
      315-5611 - Lattice GAL16V8A PAL (DIP20)
      PC910    - Sharp PC910 opto-isolator (x2, DIP8)
      SW1      - Push Button Switch (enables board)
      CN1, CN2 - Connectors to join Comm board to Video board
      CN4      - 8 pin connector (DIFFERENTIAL port)
      CN5      - 6 pin connector (SERIAL port)
      CN6, CN7 - TOSLINK-Connectors for network optical cable link
      JP2      - Jumper, set to 2-3 (connected to EPROM A15)
      JP3      - Jumper, set to 1-2 (connected to EPROM A16)
      JP4      - Jumper, set to 1-2
      JP5      - Jumper, shorted (enables TOSLINK RX channel)
      JP6      - Jumper, not shorted (enables DIFFERERENTIAL RX channel)
      JP7      - Jumper, not shorted (enables SERIAL RX channel)
      JP8      - Jumper, set to 1-2 (selects CLOCK SOURCE)
*/

#include "emu.h"
#include "m1comm.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

#define Z80_TAG "commcpu"

#ifdef M1COMM_SIMULATION
class sega_m1comm_device::context
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
			LOG("M1COMM: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("M1COMM: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("M1COMM: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("M1COMM: remotehost resolve error: %s\n", err.message());
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
						osd_printf_verbose("M1COMM: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("M1COMM: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_rx_state = 0;
									}
									else
									{
										LOG("M1COMM: RX connection from %s\n", sock.remote_endpoint());
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
				LOG("M1COMM: RX failed - %d %s\n", err.value(), err.message());
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
				osd_printf_verbose("M1COMM: TX connecting to %s\n", *m_remoteaddr);
				m_tx_timeout.expires_after(std::chrono::seconds(10));
				m_tx_timeout.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_tx_state == 1)
							{
								osd_printf_verbose("M1COMM: TX connect timed out\n");
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
								osd_printf_verbose("M1COMM: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
							else
							{
								LOG("M1COMM: TX connection established\n");
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
			osd_printf_verbose("M1COMM: RX error receiving - %d %s\n", err.value(), err.message());
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
			osd_printf_verbose("M1COMM: TX error sending - %d %s\n", err.value(), err.message());
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

/*************************************
 *  M1COMM Memory Map
 *************************************/
void sega_m1comm_device::m1comm_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram();
	map(0xc000, 0xffff).mask(0x0fff).rw(FUNC(sega_m1comm_device::share_r), FUNC(sega_m1comm_device::share_w));
}

/*************************************
 *  M1COMM I/O Map
 *************************************/
void sega_m1comm_device::m1comm_io(address_map &map)
{
	map.global_mask(0x7f);
	map(0x00, 0x1f).rw(m_dlc, FUNC(mb89374_device::read), FUNC(mb89374_device::write));
	map(0x20, 0x2f).rw(m_dma, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x40, 0x5f).mask(0x01).rw(FUNC(sega_m1comm_device::syn_r), FUNC(sega_m1comm_device::syn_w));
	map(0x60, 0x7f).mask(0x01).rw(FUNC(sega_m1comm_device::zfg_r), FUNC(sega_m1comm_device::zfg_w));
}

ROM_START( m1comm )
	ROM_REGION( 0x20000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("epr15112")

	// found on Virtua Racing and WingWar
	ROM_SYSTEM_BIOS( 0, "epr15112", "EPR-15112" )
	ROMX_LOAD( "epr-15112.17", 0x0000, 0x20000, CRC(4950e771) SHA1(99014124e0324dd114cb22f55159d18b597a155a), ROM_BIOS(0) )

	// found on Virtua Formula
	ROM_SYSTEM_BIOS( 1, "epr15624", "EPR-15624" )
	ROMX_LOAD( "epr-15624.17", 0x0000, 0x20000, CRC(9b3ba315) SHA1(0cd0983cc8b2f2d6b41617d0d0a24cc6c188e62a), ROM_BIOS(1) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_MODEL1_COMM, sega_m1comm_device, "m1comm", "Sega Model-1 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_m1comm_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 32_MHz_XTAL / 4);
	m_cpu->set_memory_map(&sega_m1comm_device::m1comm_mem);
	m_cpu->set_io_map(&sega_m1comm_device::m1comm_io);

	AM9517A(config, m_dma, 32_MHz_XTAL / 4);
	m_dma->out_hreq_callback().set(FUNC(sega_m1comm_device::dma_hreq_w));
	m_dma->in_memr_callback().set(FUNC(sega_m1comm_device::dma_mem_r));
	m_dma->out_memw_callback().set(FUNC(sega_m1comm_device::dma_mem_w));
	m_dma->out_dack_callback<2>().set(m_dlc, FUNC(mb89374_device::pi3_w));
	m_dma->out_dack_callback<3>().set(m_dlc, FUNC(mb89374_device::pi2_w));
	m_dma->out_eop_callback().set(m_dlc, FUNC(mb89374_device::ci_w));
	m_dma->in_ior_callback<2>().set(m_dlc, FUNC(mb89374_device::dma_r));
	m_dma->out_iow_callback<3>().set(m_dlc, FUNC(mb89374_device::dma_w));

	MB89374(config, m_dlc, 32_MHz_XTAL / 4);
	m_dlc->out_po_callback<2>().set(m_dma, FUNC(am9517a_device::dreq3_w));
	m_dlc->out_po_callback<3>().set(m_dma, FUNC(am9517a_device::dreq2_w));
	m_dlc->out_irq_callback().set(FUNC(sega_m1comm_device::dlc_int7_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------
const tiny_rom_entry *sega_m1comm_device::device_rom_region() const
{
	return ROM_NAME( m1comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_m1comm_device - constructor
//-------------------------------------------------

sega_m1comm_device::sega_m1comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_MODEL1_COMM, tag, owner, clock),
	m_cpu(*this, Z80_TAG),
	m_dma(*this, "commdma"),
	m_dlc(*this, "commdlc")
{
#ifdef M1COMM_SIMULATION
	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_m1comm_device::device_start()
{
#ifdef M1COMM_SIMULATION
	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_m1comm_device::device_reset()
{
	std::fill(std::begin(m_shared), std::end(m_shared), 0);
	m_syn = 0;
	m_zfg = 0;
	m_cn = 0;
	m_fg = 0;
#ifdef M1COMM_SIMULATION
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

void sega_m1comm_device::device_stop()
{
#ifdef M1COMM_SIMULATION
	m_context->stop();
	m_context.reset();
#endif
}

void sega_m1comm_device::device_reset_after_children()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dma->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_dlc->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void sega_m1comm_device::dma_hreq_w(int state)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma->hack_w(state);
}

uint8_t sega_m1comm_device::dma_mem_r(offs_t offset)
{
	return m_cpu->space(AS_PROGRAM).read_byte(offset);
}

void sega_m1comm_device::dma_mem_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_PROGRAM).write_byte(offset, data);
}

void sega_m1comm_device::dlc_int7_w(int state)
{
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80
}

uint8_t sega_m1comm_device::syn_r()
{
	return m_syn | 0xfc;
}

void sega_m1comm_device::syn_w(uint8_t data)
{
	m_syn = data & 0x03;
}

uint8_t sega_m1comm_device::zfg_r()
{
	return m_zfg | (~m_fg << 7) | 0x7e;
}

void sega_m1comm_device::zfg_w(uint8_t data)
{
	m_zfg = data & 0x01;
}

uint8_t sega_m1comm_device::share_r(offs_t offset)
{
	return m_shared[offset];
}

void sega_m1comm_device::share_w(offs_t offset, uint8_t data)
{
	m_shared[offset] = data;
}

uint8_t sega_m1comm_device::cn_r()
{
	return m_cn | 0xfe;
}

void sega_m1comm_device::cn_w(uint8_t data)
{
	m_cn = data & 0x01;

#ifndef M1COMM_SIMULATION
	if (!m_cn)
	{
		device_reset();
		device_reset_after_children();
	}
	else
	{
		m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_dma->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_dlc->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
#else
	if (!m_cn)
	{
		// reset command
		osd_printf_verbose("M1COMM: board disabled\n");
		m_linkenable = 0x00;
		m_zfg = 0x00;
	}
	else
	{
		// init command
		osd_printf_verbose("M1COMM: board enabled\n");
		m_linkenable = 0x01;
		m_linkid = 0x00;
		m_linkalive = 0x00;
		m_linkcount = 0x00;
		m_linktimer = 0x00e8; // 58 fps * 4s
	}
#endif
}

uint8_t sega_m1comm_device::fg_r()
{
	return m_fg | (~m_zfg << 7) | 0x7e;
}

void sega_m1comm_device::fg_w(uint8_t data)
{
	if (!m_cn)
		return;

	m_fg = data & 0x01;
}

void sega_m1comm_device::check_vint_irq()
{
#ifndef M1COMM_SIMULATION
	if (m_syn & 0x02)
	{
		m_cpu->set_input_line_and_vector(0, HOLD_LINE, 0xef); // Z80
	}
#else
	comm_tick();
#endif
}

#ifdef M1COMM_SIMULATION
void sega_m1comm_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linkenable == 0x01)
	{
		unsigned frame_size = 0x01c4;
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
					// check message id
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
								m_linkid = m_buffer[1];
							}

							// forward message to other nodes
							send_frame(data_size);
						}
					}

					// 0xFE - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer[1];

							// forward message to other nodes
							send_frame(data_size);
						}

						// consider it done
						LOG("M1COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;
						m_zfg = 0x01;

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
						LOG("M1COMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;
						m_zfg = 0x01;

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

		if (m_linkalive == 0x01)
		{
			// link established
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
							// 0xFC - VSYNC
							m_linktimer = 0x00;
							if (!is_master)
								// forward message to other nodes
								send_frame(data_size);
						}
						if (idx == 0xfd)
						{
							// 0xFD - master addional bytes
							if (!is_master)
							{
								// save message to "ring buffer"
								unsigned frame_offset = 0x06;
								for (unsigned j = 0x00; j < 0x0a; j++)
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
					// master sends additional status bytes
					send_data(0xfd, 0x06, 0x0a, data_size);

					// send vsync
					m_buffer[0] = 0xfc;
					m_buffer[1] = 0x01;
					send_frame(data_size);
				}
			}

			// clear 05
			m_shared[5] = 0x00;
		}
	}
}

unsigned sega_m1comm_device::read_frame(unsigned data_size)
{
	unsigned bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("M1COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
		return 0;
	}
	return bytes_read;
}

void sega_m1comm_device::send_data(uint8_t frame_type, unsigned frame_start, unsigned frame_size, unsigned data_size)
{
	m_buffer[0] = frame_type;
	for (unsigned i = 0x00; i < frame_size; i++)
	{
		m_buffer[1 + i] = m_shared[frame_start + i];
	}
	send_frame(data_size);
}

void sega_m1comm_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("M1COMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
		}
	}
}
#endif
