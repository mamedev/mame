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
	m_timeout_tx(m_ioctx),
	m_state_rx(0U),
	m_state_tx(0U)
	{
	}

	void start()
	{

		m_thread = std::thread(
				[this] ()
				{
					LOG("S32COMM: network thread started\n");
					try {
						m_ioctx.run();
					} catch (const std::exception& e) {
						LOG("S32COMM: Exception in network thread: %s\n", e.what());
					} catch (...) { // Catch any other unknown exceptions
						LOG("S32COMM: Unknown exception in network thread\n");
					}
					LOG("S32COMM: network thread completed\n");
				});
	}

	void reset(std::string localhost, std::string localport, std::string remotehost, std::string remoteport)
	{
		m_ioctx.post(
				[this] ()
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
				});

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
		m_ioctx.post(
				[this] ()
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
				});
		m_work_guard.reset();
		if (m_thread.joinable()) {
			m_thread.join();
		}
	}

	void check_sockets()
	{
		// start acceptor if needed
		if (m_localaddr && m_state_rx.load() == 0)
		{
			m_ioctx.post(
					[this] ()
					{
						start_accept();
					});
		}

		// connect socket if needed
		if (m_remoteaddr && m_state_tx.load() == 0)
		{
			m_ioctx.post(
					[this] ()
					{
						start_connect();
					});
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
			LOG("S32COMM: TX buffer overflow\n");
			return UINT_MAX;
		}

		bool const sending = m_fifo_tx.used();
		m_fifo_tx.write(&buffer[0], data_size);
		if (!sending)
			m_ioctx.post(
					[this] ()
					{
						start_send_tx();
					});
		return data_size;
	}

private:
	class fifo
	{
	public:
		unsigned write(uint8_t *buffer, unsigned data_size)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
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
			std::lock_guard<std::mutex> lock(m_mutex);
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
			std::lock_guard<std::mutex> lock(m_mutex);
			m_rp = (m_rp + data_size) % m_buffer.size();
			m_used -= data_size;
		}

		unsigned used()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_used;
		}

		unsigned free()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_buffer.size() - m_used;
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_wp = m_rp = m_used = 0;
		}


	private:
		unsigned m_wp = 0;
		unsigned m_rp = 0;
		unsigned m_used = 0;
		std::array<uint8_t, 0x80000> m_buffer;
		std::mutex m_mutex;
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
					osd_printf_verbose("S32COMM: RX listen on %s\n", *m_localaddr);
					m_acceptor.async_accept(
							[this] (std::error_code const &err, asio::ip::tcp::socket sock)
							{
								if (err)
								{
									LOG("S32COMM: RX error accepting - %d %s\n", err.value(), err.message());
									std::error_code e;
									m_acceptor.close(e);
									m_state_rx.store(0);
								}
								else
								{
									LOG("S32COMM: RX connection from %s\n", sock.remote_endpoint());
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
			LOG("S32COMM: RX failed - %d %s\n", err.value(), err.message());
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
			osd_printf_verbose("S32COMM: TX connecting to %s\n", *m_remoteaddr);
			m_timeout_tx.expires_after(std::chrono::seconds(10));
			m_timeout_tx.async_wait(
					[this] (std::error_code const &err)
					{
						if (!err && m_state_tx.load() == 1)
						{
							osd_printf_verbose("S32COMM: TX connect timed out\n");
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
							osd_printf_verbose("S32COMM: TX connect error - %d %s\n", err.value(), err.message());
							std::error_code e;
							m_sock_tx.close(e);
							m_state_tx.store(0);
						}
						else
						{
							LOG("S32COMM: TX connection established\n");
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
						LOG("S32COMM: TX connection error: %s\n", err.message().c_str());
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
							LOG("S32COMM: RX connection error: %s\n", err.message());
						else
							LOG("S32COMM: RX connection lost\n");
						m_sock_rx.close();
						m_state_rx.store(0);
						m_fifo_rx.clear();
					}
					else
					{
						if (UINT_MAX == m_fifo_rx.write(&m_buffer_rx[0], length))
						{
							LOG("S32COMM: RX buffer overflow\n");
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

	std::thread m_thread;
	asio::io_context m_ioctx;
	asio::executor_work_guard<asio::io_context::executor_type> m_work_guard{m_ioctx.get_executor()};
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
	m_context->start();

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

	auto const &opts = mconfig().options();
	m_context->reset(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());

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
