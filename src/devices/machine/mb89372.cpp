// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
/**
    MB89372
    Fujitsu
    Multi-Protocol Controller
 **/

/*
    registers are guesswork!
    port 00-0f - SIU A
    port 10-1f - SIU B
    port 20-2f - DMA A-D
        20,21,22 chan a address (siu-a rx)
        23       chan a command/status
        24,25,26 chan b address (siu-a tx)
        27       chan b command/status
        28,29,2a chan c address (siu-b rx)
        2b       chan c command/status
        2c,2d,2e chan d address (siu-b tx)
        2f       chan d command/status
    port 30-3f - probably INTC?

    TODO: basically everything, consider this a skeleton
*/

#include "emu.h"
#include "mb89372.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

class mb89372_device::context
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
			LOG("MB89372: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("MB89372: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("MB89372: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("MB89372: remotehost resolve error: %s\n", err.message());
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
						osd_printf_verbose("MB89372: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("MB89372: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_state_rx = 0;
									}
									else
									{
										LOG("MB89372: RX connection from %s\n", sock.remote_endpoint());
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
				LOG("MB89372: RX failed - %d %s\n", err.value(), err.message());
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
				osd_printf_verbose("MB89372: TX connecting to %s\n", *m_remoteaddr);
				m_timeout_tx.expires_after(std::chrono::seconds(10));
				m_timeout_tx.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_state_tx == 1)
							{
								osd_printf_verbose("MB89372: TX connect timed out\n");
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
								osd_printf_verbose("MB89372: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_state_tx = 0;
							}
							else
							{
								LOG("MB89372: TX connection established\n");
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

	unsigned receive(uint8_t *buffer)
	{
		if (m_state_rx < 2)
			return UINT_MAX;

		m_ioctx.poll();

		// check for packet length (2 bytes)
		unsigned data_size = 2;
		if (data_size > m_fifo_rx.used())
			return 0;

		m_fifo_rx.read(&buffer[0], data_size, true);
		data_size += (buffer[0x01] << 8 | buffer[0x00]);

		// check if whole packet is ready to be read
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
			LOG("MB89372: TX buffer overflow\n");
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
						LOG("MB89372: TX connection error: %s\n", err.message().c_str());
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
							LOG("MB89372: RX connection error: %s\n", err.message());
						else
							LOG("MB89372: RX connection lost\n");
						m_sock_rx.close();
						m_state_rx = 0;
						m_fifo_rx.clear();
					}
					else
					{
						if (UINT_MAX == m_fifo_rx.write(&m_buffer_rx[0], length))
						{
							LOG("MB89372: RX buffer overflow\n");
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
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB89372, mb89372_device, "mb89372", "Fujitsu MB89372 Multi-Protocol Controller")

//-------------------------------------------------
//  mb89372_device - constructor
//-------------------------------------------------

mb89372_device::mb89372_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock ) :
	device_t(mconfig, MB89372, tag, owner, clock),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_hack(0),
	m_out_hreq_cb(*this),
	m_out_irq_cb(*this),
	m_in_memr_cb(*this, 0),
	m_out_memw_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//------------------------------------------------

void mb89372_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	for (auto &elem : m_channel)
	{
		elem.m_address = 0;
		elem.m_count = 0;
		elem.m_base_address = 0;
		elem.m_base_flags = 0;
		elem.m_state = 0;
	}

	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);

	// state saving
	save_item(NAME(m_hreq));
	save_item(NAME(m_hack));
	save_item(NAME(m_irq));
	save_item(NAME(m_reg));

	save_item(STRUCT_MEMBER(m_channel, m_address));
	save_item(STRUCT_MEMBER(m_channel, m_count));
	save_item(STRUCT_MEMBER(m_channel, m_base_address));
	save_item(STRUCT_MEMBER(m_channel, m_base_flags));
	save_item(STRUCT_MEMBER(m_channel, m_state));

	save_item(NAME(m_current_channel));
	save_item(NAME(m_last_channel));

	save_item(NAME(m_intr_delay));
	save_item(NAME(m_sock_delay));

	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rx_offset));
	save_item(NAME(m_rx_length));

	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_offset));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb89372_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);

	set_hreq(0);
	m_hack = 0;
	set_irq(0);

	m_current_channel = -1;
	m_last_channel = 3;

	m_dma_delay = 0;
	m_intr_delay = 0;
	m_sock_delay = 0x20;

	m_rx_length = 0x0000;
	m_rx_offset = 0x0000;

	m_tx_offset = 0x0000;

	m_context->stop();

	auto const &opts = mconfig().options();
	m_context->start(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());
}

void mb89372_device::device_stop()
{
	m_context->stop();
	m_context.reset();
}

//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mb89372_device::execute_run()
{
	while (m_icount > 0)
	{
		// TODO waste some cycles before triggering ints
		if (m_intr_delay > 0)
		{
			m_intr_delay--;
		}
		else
		{
			//m_intr_delay = 0x04;
			check_ints();
		}

		if (m_sock_delay > 0)
		{
			m_sock_delay--;
		}
		else
		{
			m_sock_delay = 0x20;
			comm_tick();
		}

		if (m_dma_delay > 0)
		{
			m_dma_delay--;
		}
		else
		{
			check_dma();
		}

		m_icount--;
	}
}


//-------------------------------------------------
//  read - handler for register reading
//-------------------------------------------------

uint8_t mb89372_device::read(offs_t offset)
{
	uint8_t data = 0xff;
	switch (offset & 0x3f)
	{
		case 0x08:
			data = (m_rx_length > 0) ? 1 : 0;
			break;

		case 0x0f:
			data = rx_read();
			break;

		default:
			data = m_reg[offset & 0x3f];
			if (!machine().side_effects_disabled())
				logerror("MB89372 unimplemented register read @%02X\n", offset);
	}
	return data;
}


//-------------------------------------------------
//  write - handler for register writing
//-------------------------------------------------

void mb89372_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x3f)
	{
		case 0x20:
		case 0x21:
		case 0x22:
			// dma A desc pointer
		case 0x23:
			// dma A command register?
			m_reg[offset & 0x3f] = data;
			break;

		case 0x24:
		case 0x25:
		case 0x26:
			// dma B desc pointer
		case 0x27:
			// dma B command register?
			m_reg[offset & 0x3f] = data;
			break;

		default:
			m_reg[offset & 0x3f] = data;
			logerror("MB89372 unimplemented register write @%02X = %02X\n", offset, data);
	}
	trigger(1);
}


//-------------------------------------------------
//  hack_w - hold acknowledge
//-------------------------------------------------

void mb89372_device::hack_w(int state)
{
	m_hack = state;
	trigger(1);
}


//**************************************************************************
//  int logic
//**************************************************************************
void mb89372_device::check_ints()
{
	int active = (m_reg[0x23] & 0x01) | (m_reg[0x27] & 0x01);
	set_irq(active);
}


//**************************************************************************
//  dma logic
//**************************************************************************
void mb89372_device::check_dma()
{
	if (m_current_channel != -1)
	{
		int reg = 0x20 + (m_current_channel * 4);
		int addr = (m_reg[reg + 2] << 16) | (m_reg[reg + 1] << 8) | m_reg[reg];
		switch (m_channel[m_current_channel].m_state)
		{
			case 0:
				// inactive
				m_current_channel = -1;
				break;

			case 1:
				// waiting for hack
				set_hreq(1);
				if (m_hack)
				{
					m_channel[m_current_channel].m_state = 2;
				}
				break;

			case 2:
				// read command
				m_channel[m_current_channel].m_base_address = addr;
				m_channel[m_current_channel].m_count = (m_in_memr_cb(addr + 1) << 8) | m_in_memr_cb(addr);
				m_channel[m_current_channel].m_address = (m_in_memr_cb(addr + 4) << 16) | (m_in_memr_cb(addr + 3) << 8) | m_in_memr_cb(addr + 2);
				m_channel[m_current_channel].m_base_flags = m_in_memr_cb(addr + 5);

				m_dma_delay = 6*4;
				m_channel[m_current_channel].m_state = 3;
				logerror("read %01x\n", m_current_channel);

				break;

			case 3:
				// check command
				if (m_current_channel == 0)
				{
					// rx channel
					if (m_channel[m_current_channel].m_count > m_rx_length)
					{
						logerror("not enough data for %01x\n", m_current_channel);
						m_current_channel = -1;
						set_hreq(0);
					}
					else
					{
						m_channel[m_current_channel].m_state = 4;
					}
				}
				else
				{
					// tx channel
					m_channel[m_current_channel].m_state = 5;
				}
				break;

			case 4:
				// execute rx
				set_hreq(1);
				if (m_hack)
				{
					if (m_channel[m_current_channel].m_count > 0)
					{
						m_out_memw_cb(m_channel[m_current_channel].m_address, rx_read());
						m_channel[m_current_channel].m_address++;
						m_channel[m_current_channel].m_count--;
						m_dma_delay = 4;
						logerror("rx %01x\n", m_current_channel);
					}
					else
					{
						m_channel[m_current_channel].m_state = 6;
						logerror("rc %01x\n", m_current_channel);
					}
				}
				break;

			case 5:
				// execute tx
				set_hreq(1);
				if (m_hack)
				{
					if (m_channel[m_current_channel].m_count > 0)
					{
						tx_write(m_in_memr_cb(m_channel[m_current_channel].m_address));
						m_channel[m_current_channel].m_address++;
						m_channel[m_current_channel].m_count--;
						m_dma_delay = 4;
						logerror("tx %01x\n", m_current_channel);
					}
					else
					{
						tx_complete();
						m_channel[m_current_channel].m_state = 6;
						logerror("tc %01x\n", m_current_channel);
					}
				}
				break;

			case 6:
				m_out_memw_cb(m_channel[m_current_channel].m_base_address + 5, m_channel[m_current_channel].m_base_flags | 0x60);
				if (m_channel[m_current_channel].m_base_flags & 0x10)
				{
					// continue chain
					m_reg[reg] = m_in_memr_cb(m_channel[m_current_channel].m_base_address + 9);
					m_reg[reg + 1] = m_in_memr_cb(m_channel[m_current_channel].m_base_address + 10);
					m_reg[reg + 2] = m_in_memr_cb(m_channel[m_current_channel].m_base_address + 11);
					m_channel[m_current_channel].m_state = 2;
					logerror("continue %01x\n", m_current_channel);
				}
				else
				{
					// chain complete
					m_reg[reg + 3] |= 0x01; // enable int flag?
					m_reg[reg + 3] &= 0x7f; // disable active flag?
					m_channel[m_current_channel].m_state = 0;
					logerror("completed %01x\n", m_current_channel);
					m_current_channel = -1;
					set_hreq(0);
				}
				break;

			default:
				logerror("unknown state?! %01x = %02x\n", m_current_channel, m_channel[m_current_channel].m_state);
				break;
		}
	}
	else
	{
		for (int channel = 3; channel >= 0; channel--)
		{
			if (m_channel[channel].m_state > 0)
			{
				m_current_channel = channel;
				return;
			}
			int reg = 0x20 + (channel * 4);
			if ((m_channel[channel].m_state == 0) && (m_reg[reg + 3] & 0x80))
			{
				m_channel[channel].m_state = 1;
				m_current_channel = channel;
				return;
			}
		}
	}
}


//**************************************************************************
//  buffer logic
//**************************************************************************

void mb89372_device::rx_reset()
{
	m_rx_length = 0;
	m_rx_offset = 0;
}

uint8_t mb89372_device::rx_read()
{
	uint8_t data = m_rx_buffer[m_rx_offset];
	m_rx_offset++;

	/*
	if (m_rx_offset == m_rx_length)
	    m_rxsr0 |= 0x40; // EOF
	*/

	if (m_rx_offset >= m_rx_length)
		rx_reset();
	return data;
}

void mb89372_device::tx_reset()
{
	m_tx_offset = 0;
	//m_txsr |= 0x05;
}

void mb89372_device::tx_write(uint8_t data)
{
	m_tx_buffer[m_tx_offset] = data;
	m_tx_offset++;
	//m_txsr = 0x6b;

	// prevent overflow
	if (m_tx_offset >= 0x0f00)
		m_tx_offset = 0x0eff;
}

void mb89372_device::tx_complete()
{
	if (m_tx_offset > 0)
	{
		if (m_context->connected())
		{
			m_socket_buffer[0] = m_tx_offset & 0xff;
			m_socket_buffer[1] = (m_tx_offset >> 8) & 0xff;
			for (int i = 0x00; i < m_tx_offset; i++)
			{
				m_socket_buffer[2 + i] = m_tx_buffer[i];
			}
			send_frame(2 + m_tx_offset);
		}
	}

	//m_txsr = 0x6f;

	tx_reset();
}

void mb89372_device::comm_tick()
{
	m_context->check_sockets();

	if (m_context->connected())
	{
		// RXCR_RXE
		if (true)
		{
			if (m_rx_length == 0)
			{
				unsigned recv = read_frame();
				if (recv > 0)
				{
					m_rx_length = m_socket_buffer[1] << 8 | m_socket_buffer[0];
					for (unsigned i = 0x00; i < m_rx_length; i++)
					{
						m_rx_buffer[i] = m_socket_buffer[2 + i];
					}

					m_rx_offset = 0;
					//m_rxsr0 = 0x01; // RXRDY

					/*
					if (m_rx_offset + 1 == m_rx_length)
					    m_rxsr0 |= 0x40; // EOF
					*/

					//m_rxsr1 = 0xc8;

					//set_po3(!MASKR_MRXDRQ && RXIER_RXDI ? 1 : 0);
				}
			}
		}
	}
}

unsigned mb89372_device::read_frame()
{
	unsigned bytes_read = m_context->receive(&m_socket_buffer[0]);
	if (bytes_read == UINT_MAX)
	{
		// error case, do nothing
		return 0;
	}
	return bytes_read;
}

void mb89372_device::send_frame(unsigned data_size)
{
	unsigned bytes_sent = m_context->send(&m_socket_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		// error case, do nothing
	}
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void mb89372_device::set_hreq(int state)
{
	if (m_hreq != state)
	{
		m_out_hreq_cb(state);
		m_hreq = state;
	}
}

inline void mb89372_device::set_irq(int state)
{
	if (m_irq != state)
	{
		m_out_irq_cb(state);
		m_irq = state;
	}
}
