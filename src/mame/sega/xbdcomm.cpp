// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann

/*
Top    : 834-6780
Sticker: 834-7112
|---------| |--| |----------------------|
|         RX   TX            315-5336   |
|             315-5337                  |
|                                       |
|            16MHz      6264            |
|                    EPR-12587.14       |
| MB89372P-SH     Z80E        MB8421    |
|---------------------------------------|
Notes:
      315-5337 - PAL16L8
      315-5336 - PAL16L8
      Z80 clock: 8.000MHz [16/2]
      6264     : 8k x8 SRAM
      MB8421   : Fujitsu 2k x8 Dual-Port SRAM (SDIP52)
      MB89372  : Fujitsu Multi-Protocol Controller (SDIP64)
      EPR-12587: 27C256 EPROM
*/

#include "emu.h"
#include "xbdcomm.h"

#include "asio.h"

#include <iostream>

#include "emuopts.h"

#define VERBOSE 0
#include "logmacro.h"

#define Z80_TAG "commcpu"

#ifdef XBDCOMM_SIMULATION
class sega_xbdcomm_device::context
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
			LOG("XBDCOMM: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("XBDCOMM: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("XBDCOMM: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("XBDCOMM: remotehost resolve error: %s\n", err.message());
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
			LOG("XBDCOMM: TX buffer overflow\n");
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
					osd_printf_verbose("XBDCOMM: RX listen on %s\n", *m_localaddr);
					m_acceptor.async_accept(
							[this] (std::error_code const &err, asio::ip::tcp::socket sock)
							{
								if (err)
								{
									LOG("XBDCOMM: RX error accepting - %d %s\n", err.value(), err.message());
									std::error_code e;
									m_acceptor.close(e);
									m_state_rx.store(0);
								}
								else
								{
									LOG("XBDCOMM: RX connection from %s\n", sock.remote_endpoint());
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
			LOG("XBDCOMM: RX failed - %d %s\n", err.value(), err.message());
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
			osd_printf_verbose("XBDCOMM: TX connecting to %s\n", *m_remoteaddr);
			m_timeout_tx.expires_after(std::chrono::seconds(10));
			m_timeout_tx.async_wait(
					[this] (std::error_code const &err)
					{
						if (!err && m_state_tx.load() == 1)
						{
							osd_printf_verbose("XBDCOMM: TX connect timed out\n");
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
							osd_printf_verbose("XBDCOMM: TX connect error - %d %s\n", err.value(), err.message());
							std::error_code e;
							m_sock_tx.close(e);
							m_state_tx.store(0);
						}
						else
						{
							LOG("XBDCOMM: TX connection established\n");
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
						LOG("XBDCOMM: TX connection error: %s\n", err.message().c_str());
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
							LOG("XBDCOMM: RX connection error: %s\n", err.message());
						else
							LOG("XBDCOMM: RX connection lost\n");
						m_sock_rx.close();
						m_state_rx.store(0);
						m_fifo_rx.clear();
					}
					else
					{
						if (UINT_MAX == m_fifo_rx.write(&m_buffer_rx[0], length))
						{
							LOG("XBDCOMM: RX buffer overflow\n");
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

/*************************************
 *  XBDCOMM Memory Map
 *************************************/
void sega_xbdcomm_device::xbdcomm_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x47ff).rw(m_dpram, FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
}

/*************************************
 *  XBDCOMM I/O Map
 *************************************/
void sega_xbdcomm_device::xbdcomm_io(address_map &map)
{
	map.global_mask(0xff);
	//map(0x00, 0x3f).rw(m_mpc, FUNC(mb89372_device::read), FUNC(mb89372_device::write));
	map(0x40, 0x40).rw(FUNC(sega_xbdcomm_device::z80_stat_r), FUNC(sega_xbdcomm_device::z80_debug_w));
	map(0x80, 0x80).w(FUNC(sega_xbdcomm_device::z80_stat_w));
}

ROM_START( xbdcomm )
	ROM_REGION( 0x10000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("epr12587")

	// found on Super Monaco GP
	ROM_SYSTEM_BIOS( 0, "epr12587", "EPR-12587" )
	ROMX_LOAD( "epr-12587.14", 0x00000, 0x08000, CRC(2afe648b) SHA1(b5bf86f3acbcc23c136185110acecf2c971294fa), ROM_BIOS(0) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_XBOARD_COMM, sega_xbdcomm_device, "xbdcomm", "Sega X-Board Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_xbdcomm_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_cpu, 16_MHz_XTAL / 2);
	m_cpu->set_memory_map(&sega_xbdcomm_device::xbdcomm_mem);
	m_cpu->set_io_map(&sega_xbdcomm_device::xbdcomm_io);

	MB8421(config, m_dpram).intl_callback().set(FUNC(sega_xbdcomm_device::dpram_int5_w));

	/*
	MB89372(config, m_mpc, 16_MHz_XTAL / 2);
	m_mpc->out_hreq_callback().set(FUNC(sega_xbdcomm_device::mpc_hreq_w));
	m_mpc->out_irq_callback().set(FUNC(sega_xbdcomm_device::mpc_int7_w));
	m_mpc->in_memr_callback().set(FUNC(sega_xbdcomm_device::mpc_mem_r));
	m_mpc->out_memw_callback().set(FUNC(sega_xbdcomm_device::mpc_mem_w));
	*/
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------
const tiny_rom_entry *sega_xbdcomm_device::device_rom_region() const
{
	return ROM_NAME( xbdcomm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_xbdcomm_device - constructor
//-------------------------------------------------

sega_xbdcomm_device::sega_xbdcomm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_XBOARD_COMM, tag, owner, clock),
	m_cpu(*this, Z80_TAG),
	m_dpram(*this, "dpram")
	//m_mpc(*this, "commmpc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_xbdcomm_device::device_start()
{
	// state saving
	save_item(NAME(m_ex_page));
	save_item(NAME(m_xbd_stat));
	save_item(NAME(m_z80_stat));

#ifdef XBDCOMM_SIMULATION
	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);
	m_context->start();

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

void sega_xbdcomm_device::device_reset()
{
	m_ex_page = 0;
	m_xbd_stat = 0;
	m_z80_stat = 0;

#ifdef XBDCOMM_SIMULATION
	std::fill(std::begin(m_buffer), std::end(m_buffer), 0);

	auto const &opts = mconfig().options();
	m_context->reset(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

	m_linkenable = 0;
	m_linktimer = 0;
	m_linkalive = 0;
	m_linkid = 0;
	m_linkcount = 0;
#endif
}

void sega_xbdcomm_device::device_stop()
{
#ifdef XBDCOMM_SIMULATION
	m_context->stop();
	m_context.reset();
#endif
}

void sega_xbdcomm_device::device_reset_after_children()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	//m_mpc->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

uint8_t sega_xbdcomm_device::ex_r(offs_t offset)
{
	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			return m_dpram->right_r(m_ex_page << 3 | offset);

		case 0x08:
			// page latch
			return m_ex_page;

		case 0x10:
			// status register?
			if (!machine().side_effects_disabled())
				LOG("xbdcomm-ex_r: %02x %02x\n", offset, m_z80_stat);
#ifdef XBDCOMM_SIMULATION
			comm_tick();
#endif
			return m_z80_stat;

		default:
			if (!machine().side_effects_disabled())
				logerror("xbdcomm-ex_r: %02x\n", offset);
			return 0xff;
	}
}


void sega_xbdcomm_device::ex_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			m_dpram->right_w(m_ex_page << 3 | offset, data);
			break;

		case 0x08:
			// page latch
			m_ex_page = data;
			break;

		case 0x10:
			// status register?
			// bit 7 = on/off toggle
			// bit 1 = test flag?
			// bit 0 = ready to send?
			m_xbd_stat = data;
#ifndef XBDCOMM_SIMULATION
			if (m_xbd_stat & 0x80)
			{
				m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				//m_mpc->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
			{
				device_reset();
				device_reset_after_children();
			}
#else
			if (m_xbd_stat & 0x80)
			{
				// link active
				if (!m_linkenable)
				{
					// init command
					osd_printf_verbose("XBDCOMM: board enabled\n");
					m_linkenable = 0x01;
					m_linkid = 0x00;
					m_linkalive = 0x00;
					m_linkcount = 0x00;
					m_linktimer = 0x003a;
				}
			}
			else
			{
				if (m_linkenable)
				{
					// reset command
					osd_printf_verbose("XBDCOMM: board disabled\n");
					m_linkenable = 0x00;
				}
			}
			comm_tick();
#endif
			break;

		case 0x19:
		case 0x1d:
			// unknown registers
			// 19 - 11 byte writes (smgp)
			// 1d - completes write cycle (smgp)
			break;

		default:
			logerror("xbdcomm-ex_w: %02x %02x\n", offset, data);
			break;
	}
}

void sega_xbdcomm_device::mpc_hreq_w(int state)
{
	m_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	//m_mpc->hack_w(state);
}

void sega_xbdcomm_device::dpram_int5_w(int state)
{
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xef); // Z80 INT5
}

void sega_xbdcomm_device::mpc_int7_w(int state)
{
	logerror("mpc_int7_w: %02x\n", state);
	m_cpu->set_input_line_and_vector(0, state ? ASSERT_LINE : CLEAR_LINE, 0xff); // Z80 INT7
}

uint8_t sega_xbdcomm_device::mpc_mem_r(offs_t offset)
{
	return m_cpu->space(AS_PROGRAM).read_byte(offset);
}

void sega_xbdcomm_device::mpc_mem_w(offs_t offset, uint8_t data)
{
	m_cpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t sega_xbdcomm_device::z80_stat_r()
{
	return m_xbd_stat;
}

void sega_xbdcomm_device::z80_stat_w(uint8_t data)
{
	m_z80_stat = data;
}

void sega_xbdcomm_device::z80_debug_w(uint8_t data)
{
	m_ex_page = data;
	m_z80_stat = 0;
}


#ifdef XBDCOMM_SIMULATION
void sega_xbdcomm_device::comm_tick()
{
	m_context->check_sockets();

	if (m_linkenable == 0x01)
	{
		uint8_t cab_index = mpc_mem_r(0x4000);
		uint8_t cab_count = mpc_mem_r(0x4001);

		unsigned frame_start_tx = 0x4010;
		unsigned frame_start_rx = 0x4310;
		unsigned frame_size = 0;
		switch (cab_count)
		{
			case 1:
				frame_size = 0x300;
				break;
			case 2:
				frame_size = 0x180;
				break;
			case 3:
				frame_size = 0x110;
				break;
			case 4:
				frame_size = 0x0c0;
				break;
			case 5:
			case 6:
				frame_size = 0x080;
				break;
			case 7:
			case 8:
				frame_size = 0x060;
				break;
			default:
				frame_size = 0x000;
				break;
		}

		unsigned data_size = frame_size + 1;

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
			m_z80_stat = 0x01;

			// if both sockets are there check ring
			if (m_context->connected())
			{
				// try to read one message
				int recv = read_frame(data_size);
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
						osd_printf_verbose("XBDCOMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_z80_stat = 0x00;
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
						osd_printf_verbose("XBDCOMM: link established - id %02x of %02x\n", m_linkid, m_linkcount);
						m_linkalive = 0x01;

						// write to shared mem
						m_z80_stat = 0x00;
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
			// try to read a message
			unsigned recv = read_frame(data_size);
			while (recv > 0)
			{
				// check if valid id
				uint8_t idx = m_buffer[0];
				if (idx > 0 && idx <= m_linkcount)
				{
					// save message to "ring buffer"
					unsigned frame_offset = frame_start_tx + ((idx - 1) * frame_size);
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
						m_z80_stat = 0x00;
					}
				}

				// try to read another message
				recv = read_frame(data_size);
			}

			// update buffers... guesswork
			for (unsigned j = 0x00; j < 0x300; j++)
			{
				mpc_mem_w(frame_start_rx + j, mpc_mem_r(frame_start_tx + j));
			}

			// update "ring buffer" if link established
			// live relay does not send data
			if (cab_index != 0x00)
			{
				// check ready-to-send flag
				if (m_xbd_stat & 0x01)
				{
					unsigned frame_offset = frame_start_tx + ((cab_index - 1) * frame_size);
					send_data(cab_index, frame_offset, frame_size, data_size);
					m_z80_stat = 0x01;
				}
			}

			// clear ready-to-send flag
			m_xbd_stat &= 0xfe;
		}
	}
}

unsigned sega_xbdcomm_device::read_frame(unsigned data_size)
{
	unsigned bytes_read = m_context->receive(&m_buffer[0], data_size);
	if (bytes_read == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("XBDCOMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_z80_stat = 0xff;
		}
		return 0;
	}
	return bytes_read;
}

void sega_xbdcomm_device::send_data(uint8_t frame_type, unsigned frame_offset, unsigned frame_size, unsigned data_size)
{
	m_buffer[0] = frame_type;
	for (unsigned i = 0x00; i < frame_size; i++)
	{
		m_buffer[1 + i] = mpc_mem_r(frame_offset + i);
	}
	send_frame(data_size);
}

void sega_xbdcomm_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		if (m_linkalive == 0x01)
		{
			LOG("XBDCOMM: link lost\n");
			m_linkalive = 0x02;
			m_linktimer = 0x00;
			m_z80_stat = 0xff;
		}
	}
}
#endif
