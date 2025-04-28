// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
834-6740
( http://images.arianchen.de/sega-comm/pdriftl_link_big.jpg )
|-------------| |-| |--------|---|-|
| _ 315-5336  RX  TX          CNC  |
||C|                               |
||N| MB8421 EPR-12028  MB89372P-SH |
||A|          5563                 |
||_|                      315-5337 |
|    SW1     LED   16MHz     Z80E  |
|----------------------------------|
Notes:
     PALs - 315-5337, 315-5336, both PAL16L8
      Z80 - Clock 8.000MHz [16/2]
     5563 - Toshiba TC5563APL-12L 8kBx8-bit SRAM
   MB8421 - Fujitsu 2kBx8-bit Dual-Port SRAM (SDIP52)
  MB89372 - Fujitsu Multi-Protocol Controller (SDIP64)
            Uses 3 serial data transfer protocols: ASYNC, COP & BOP. Has a built
            in DMA controller and Interrupt controller to handle interrupts
            from the serial interface unit (SIU) & DMA controller (DMAC)
EPR-12028 - 27C256 EPROM
      CNA - 50 Pin connector joining to main board
      CNC - 10 pin connector
      SW1 - 8-position DIP switch
*/

#include "emu.h"
#include "ybdcomm.h"
#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

#define Z80_TAG "commcpu"

#ifdef YBDCOMM_SIMULATION
class sega_ybdcomm_device::context
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
			LOG("YBDCOMM: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("YBDCOMM: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("YBDCOMM: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("YBDCOMM: remotehost resolve error: %s\n", err.message());
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
						osd_printf_verbose("YBDCOMM: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("YBDCOMM: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_rx_state = 0;
									}
									else
									{
										LOG("YBDCOMM: RX connection from %s\n", sock.remote_endpoint());
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
				LOG("YBDCOMM: RX failed - %d %s\n", err.value(), err.message());
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
				osd_printf_verbose("YBDCOMM: TX connecting to %s\n", *m_remoteaddr);
				m_tx_timeout.expires_after(std::chrono::seconds(10));
				m_tx_timeout.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_tx_state == 1)
							{
								osd_printf_verbose("YBDCOMM: TX connect timed out\n");
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
								osd_printf_verbose("YBDCOMM: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
							else
							{
								LOG("YBDCOMM: TX connection established\n");
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
			osd_printf_verbose("YBDCOMM: RX error receiving - %d %s\n", err.value(), err.message());
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
			osd_printf_verbose("YBDCOMM: TX error sending - %d %s\n", err.value(), err.message());
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
 *  YBDCOMM Memory Map
 *************************************/
void sega_ybdcomm_device::ybdcomm_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x47ff).rw(m_dpram, FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
}

/*************************************
 *  YBDCOMM I/O Map
 *************************************/
void sega_ybdcomm_device::ybdcomm_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x3f).rw(m_mpc, FUNC(mb89372_device::read), FUNC(mb89372_device::write));
	map(0x40, 0x40).r(FUNC(sega_ybdcomm_device::z80_stat_r));
	map(0x80, 0x80).w(FUNC(sega_ybdcomm_device::z80_stat_w));
	map(0xc0, 0xc0).portr("Link_SW1");
}

static INPUT_PORTS_START( ybdcomm )
	PORT_START("Link_SW1")
	PORT_DIPNAME( 0x0f, 0x01, "Cabinet ID" ) PORT_DIPLOCATION("Link_SW1:1,2,3,4")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPNAME( 0xf0, 0x10, "Cabinet Count" ) PORT_DIPLOCATION("Link_SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x50, "5" )
	PORT_DIPSETTING(    0x60, "6" )
	PORT_DIPSETTING(    0x70, "7" )
	PORT_DIPSETTING(    0x80, "8" )
INPUT_PORTS_END

ROM_START( ybdcomm )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("epr12028")

	// found on Power Drift Link
	ROM_SYSTEM_BIOS( 0, "epr12028", "EPR-12028" )
	ROMX_LOAD( "epr-12028", 0x000000, 0x008000, CRC(bb682a92) SHA1(0445bdbca0db9edecd826da37cd2d3afc57c5cf6), ROM_BIOS(0) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_YBOARD_COMM, sega_ybdcomm_device, "ybdcomm", "Sega Y-Board Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_ybdcomm_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 16_MHz_XTAL / 2);
	m_cpu->set_memory_map(&sega_ybdcomm_device::ybdcomm_mem);
	m_cpu->set_io_map(&sega_ybdcomm_device::ybdcomm_io);

	MB8421(config, m_dpram).intl_callback().set(FUNC(sega_ybdcomm_device::dpram_int5_w));

	MB89372(config, m_mpc, 16_MHz_XTAL / 2);
	m_mpc->out_hreq_callback().set(FUNC(sega_ybdcomm_device::mpc_hreq_w));
	m_mpc->out_irq_callback().set(FUNC(sega_ybdcomm_device::mpc_int7_w));
	m_mpc->in_memr_callback().set(FUNC(sega_ybdcomm_device::mpc_mem_r));
	m_mpc->out_memw_callback().set(FUNC(sega_ybdcomm_device::mpc_mem_w));
}

ioport_constructor sega_ybdcomm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ybdcomm );
}

const tiny_rom_entry *sega_ybdcomm_device::device_rom_region() const
{
	return ROM_NAME( ybdcomm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_ybdcomm_device - constructor
//-------------------------------------------------

sega_ybdcomm_device::sega_ybdcomm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_YBOARD_COMM, tag, owner, clock),
	m_cpu(*this, Z80_TAG),
	m_dpram(*this, "dpram"),
	m_mpc(*this, "commmpc"),
	m_dip_sw1(*this, "Link_SW1")
{
#ifdef YBDCOMM_SIMULATION
	m_framesync = mconfig.options().comm_framesync() ? 0x01 : 0x00;
#endif
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_ybdcomm_device::device_start()
{
	// state saving
	save_item(NAME(m_ybd_stat));
	save_item(NAME(m_z80_stat));

#ifdef YBDCOMM_SIMULATION
	m_tick_timer = timer_alloc(FUNC(sega_ybdcomm_device::tick_timer_callback), this);
	m_tick_timer->adjust(attotime::never);

	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);

	save_item(NAME(m_linkenable));
	save_item(NAME(m_linktimer));
	save_item(NAME(m_linkalive));
	save_item(NAME(m_linkid));
	save_item(NAME(m_linkcount));
#endif
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega_ybdcomm_device::device_reset()
{
	m_ybd_stat = 0;
	m_z80_stat = 0;

#ifdef YBDCOMM_SIMULATION
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);
	m_context->stop();

	auto const &opts = mconfig().options();
	m_context->start(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

	m_linkenable = 0;
	m_linktimer = 0;
	m_linkalive = 0;
	m_linkid = 0;
	m_linkcount = 0;

	m_tick_timer->adjust(attotime::from_hz(600), 0, attotime::from_hz(600));
#endif
}

void sega_ybdcomm_device::device_stop()
{
#ifdef YBDCOMM_SIMULATION
	m_tick_timer->adjust(attotime::never);

	m_context->stop();
	m_context.reset();
#endif
}

void sega_ybdcomm_device::device_reset_after_children()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_mpc->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

uint8_t sega_ybdcomm_device::ex_r(offs_t offset)
{
	int bank = offset >> 11;

	switch (bank)
	{
		case 0:
			return m_dpram->right_r(offset);

		case 1:
			// z80 status
			if (!machine().side_effects_disabled())
				LOG("ybdcomm-ex_r: %02x %02x\n", bank, m_z80_stat);
			return m_z80_stat;

		case 2:
			// status register?
			if (!machine().side_effects_disabled())
				LOG("ybdcomm-ex_r: %02x %02x\n", bank, m_ybd_stat);
			return m_ybd_stat;

		default:
			logerror("ybdcomm-ex_r: %02x\n", offset);
			return 0xff;
	}
}


void sega_ybdcomm_device::ex_w(offs_t offset, uint8_t data)
{
	int bank = offset >> 11;

	switch (bank)
	{
		case 0:
			m_dpram->right_w(offset, data);
			break;

		case 1:
			// z80 status
			if (!machine().side_effects_disabled())
				logerror("ybdcomm-ex_w: %02x %02x\n", offset, data);
			break;

		case 2:
			// status register?
			// bit 7 = on/off toggle
			// bit 1 = test flag?
			// bit 0 = ready to send?
			m_ybd_stat = data;
			if (!machine().side_effects_disabled())
				LOG("ybdcomm-ex_w: %02x %02x\n", offset, data);
#ifndef YBDCOMM_SIMULATION
			if (m_ybd_stat & 0x80)
			{
				m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				m_mpc->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				device_reset();
				device_reset_after_children();
			}
#else
			if (m_ybd_stat & 0x80)
			{
				// link active
				if (!m_linkenable)
				{
					// init command
					osd_printf_verbose("YBDCOMM: board enabled\n");
					m_linkenable = 0x01;
					m_linkid = 0x00;
					m_linkalive = 0x00;
					m_linkcount = 0x00;
					m_linktimer = 0x003a;

					// read dips and write to shared memory
					uint8_t sw1 = m_dip_sw1->read();
					mpc_mem_w(0x4000, sw1 & 0x0f);
					mpc_mem_w(0x4001, (sw1 >> 4) & 0x0f);
				}
			}
			else
			{
				if (m_linkenable)
				{
					// reset command
					osd_printf_verbose("YBDCOMM: board disabled\n");
					m_linkenable = 0x00;
				}
			}
#endif
			break;
		default:
			logerror("ybdcomm-ex_w: %02x %02x\n", offset, data);
			break;
	}
}

void sega_ybdcomm_device::mpc_hreq_w(int state)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_mpc->hack_w(state);
}

void sega_ybdcomm_device::dpram_int5_w(int state)
{
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xef); // Z80 INT5
}

void sega_ybdcomm_device::mpc_int7_w(int state)
{
	logerror("mpc_int7_w: %02x\n", state);
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80 INT7
}

uint8_t sega_ybdcomm_device::mpc_mem_r(offs_t offset)
{
	return m_cpu->space(AS_PROGRAM).read_byte(offset);
}

void sega_ybdcomm_device::mpc_mem_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t sega_ybdcomm_device::z80_stat_r()
{
	return m_ybd_stat;
}

void sega_ybdcomm_device::z80_stat_w(uint8_t data)
{
	m_z80_stat = data;
}


#ifdef YBDCOMM_SIMULATION
TIMER_CALLBACK_MEMBER(sega_ybdcomm_device::tick_timer_callback)
{
	comm_tick();
}

unsigned sega_ybdcomm_device::comm_frame_offset(uint8_t cab_index)
{
	switch (cab_index)
	{
		case 1:
			return 0x4010;

		case 2:
			return 0x4190;
		case 3:
			return 0x41c0;
		case 4:
			return 0x41f0;
		case 5:
			return 0x4220;
		case 6:
			return 0x4250;
		case 7:
			return 0x4280;
		case 8:
			return 0x42b0;

		default:
			return 0x4400;
	}
}

unsigned sega_ybdcomm_device::comm_frame_size(uint8_t cab_index)
{
	switch (cab_index)
	{
		case 1:
			return 0x0180;

		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
			return 0x0030;

		default:
			return 0x0000;
	}
}

void sega_ybdcomm_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linkenable == 0x01)
	{
		uint8_t cab_index = mpc_mem_r(0x4000);

		unsigned data_size = 0x180 + 1;

		bool is_master = (cab_index == 0x01);
		bool is_slave = (cab_index > 0x01);
		bool is_relay = (cab_index == 0x00);

		if (m_linkalive == 0x02)
		{
			// link failed... (guesswork)
			m_z80_stat = 0xff;
			return;
		}
		else if (m_linkalive == 0x00)
		{
			// link not yet established... (guesswork)
			m_z80_stat = 0x00;

			// if both sockets are there check ring
			if (m_context->connected())
			{
				// try to read one message
				unsigned recv = read_frame(data_size);
				while (recv > 0)
				{
					// check message id
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

					// 0xfe - link size
					else if (idx == 0xfe)
					{
						if (is_slave || is_relay)
						{
							m_linkcount = m_buffer[1];

							// forward message to other nodes
							send_frame(data_size);
						}

						// consider it done
						LOG("YBDCOMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_z80_stat = 0x01;
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
						LOG("YBDCOMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_z80_stat = 0x01;
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
						// save message to "ring buffer"
						unsigned frame_offset = comm_frame_offset(idx);
						unsigned frame_size = comm_frame_size(idx);
						for (unsigned j = 0x00; j < frame_size; j++)
						{
							mpc_mem_w(frame_offset + j, m_buffer[1 + j]);
						}

						// if not own message
						if (idx != cab_index)
						{
							// forward message to other nodes
							send_frame(data_size);
						}
						else
						{
							m_z80_stat &= 0x7f;
							m_linktimer = 0x00;
						}
					}

					// try to read another message
					recv = read_frame(data_size);
				}
			}
			while (m_linktimer == 0x01);

			m_z80_stat ^= 0x01;

			// update "ring buffer" if link established
			// live relay does not send data
			if (cab_index != 0x00)
			{
				// check ready-to-send flag
				if (m_ybd_stat & 0x01)
				{
					unsigned frame_offset = comm_frame_offset(cab_index);
					unsigned frame_size = comm_frame_size(cab_index);
					send_data(cab_index, frame_offset, frame_size, data_size);
					m_z80_stat |= 0x80;

					// enable wait for sync
					m_linktimer = m_framesync;
				}
			}

			// clear ready-to-send flag
			m_ybd_stat &= 0xfe;
		}
	}
}

unsigned sega_ybdcomm_device::read_frame(unsigned data_size)
{
	unsigned bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("YBDCOMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_z80_stat = 0xff;
		}
		return 0;
	}
	return bytes_read;
}

void sega_ybdcomm_device::send_data(uint8_t frame_type, unsigned frame_offset, unsigned frame_size, unsigned data_size)
{
	m_buffer[0] = frame_type;
	for (unsigned i = 0x00; i < frame_size; i++)
	{
		m_buffer[1 + i] = mpc_mem_r(frame_offset + i);
	}
	send_frame(data_size);
}

void sega_ybdcomm_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("YBDCOMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_z80_stat = 0xff;
		}
	}
}
#endif
