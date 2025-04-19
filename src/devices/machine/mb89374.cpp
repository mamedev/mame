// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
/**
    MB89374

    Fujitsu
    Data Link Controller

 **/

#include "emu.h"
#include "mb89374.h"

#include "emuopts.h"

#include "asio.h"

#include <iostream>

#define VERBOSE 0
#include "logmacro.h"

class mb89374_device::context
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
			LOG("MB89374: localhost = %s\n", *m_localaddr);
		}
		if (err)
		{
			LOG("MB89374: localhost resolve error: %s\n", err.message());
		}

		for (auto &&resolveIte : resolver.resolve(remotehost, remoteport, asio::ip::tcp::resolver::flags::address_configured, err))
		{
			m_remoteaddr = resolveIte.endpoint();
			LOG("MB89374: remotehost = %s\n", *m_remoteaddr);
		}
		if (err)
		{
			LOG("MB89374: remotehost resolve error: %s\n", err.message());
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
						osd_printf_verbose("MB89374: RX listen on %s\n", *m_localaddr);
						m_acceptor.async_accept(
								[this] (std::error_code const &err, asio::ip::tcp::socket sock)
								{
									if (err)
									{
										LOG("MB89374: RX error accepting - %d %s\n", err.value(), err.message());
										std::error_code e;
										m_acceptor.close(e);
										m_rx_state = 0;
									}
									else
									{
										LOG("MB89374: RX connection from %s\n", sock.remote_endpoint());
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
				LOG("MB89374: RX failed - %d %s\n", err.value(), err.message());
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
				osd_printf_verbose("MB89374: TX connecting to %s\n", *m_remoteaddr);
				m_tx_timeout.expires_after(std::chrono::seconds(10));
				m_tx_timeout.async_wait(
						[this] (std::error_code const &err)
						{
							if (!err && m_tx_state == 1)
							{
								osd_printf_verbose("MB89374: TX connect timed out\n");
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
								osd_printf_verbose("MB89374: TX connect error - %d %s\n", err.value(), err.message());
								std::error_code e;
								m_sock_tx.close(e);
								m_tx_state = 0;
							}
							else
							{
								LOG("MB89374: TX connection established\n");
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

	unsigned receive(uint8_t *buffer)
	{
		if (m_rx_state != 2)
			return 0;

		// check for packet length (2 bytes)
		std::error_code err;
		std::size_t data_size = 2;
		std::size_t bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), asio::socket_base::message_peek, err);
		if (err == asio::error::would_block)
			return 0;

		if (!err)
		{
			// check if whole packet is ready to be read
			data_size += (buffer[0x01] << 8 | buffer[0x00]);
			bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), asio::socket_base::message_peek, err);

			// do actual read
			if (!err && bytes_read == data_size)
				bytes_read = m_sock_rx.receive(asio::buffer(&buffer[0], data_size), 0, err);
		}

		if (err)
		{
			osd_printf_verbose("MB89374: RX error receiving - %d %s\n", err.value(), err.message());
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
			osd_printf_verbose("MB89374: TX error sending - %d %s\n", err.value(), err.message());
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

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB89374, mb89374_device, "mb89374", "Fujitsu MB89374 Data Link Controller")


enum
{
	REGISTER_SMR0 = 0x00,
	REGISTER_SMR1,
	REGISTER_SMR2,
	REGISTER_CHRR0,
	REGISTER_CHRR1,

	REGISTER_MSR = 0x06,
	REGISTER_MCR,
	REGISTER_RXSR0,
	REGISTER_RXSR1,
	REGISTER_RXCR,
	REGISTER_RXIER,
	REGISTER_TXSR,
	REGISTER_TXCR,
	REGISTER_TXIER,
	REGISTER_SDR,
	REGISTER_TXBCR0,
	REGISTER_TXBCR1,
	REGISTER_TXFR0,
	REGISTER_TXFR1,
	REGISTER_SMR3,
	REGISTER_PORTR,
	REGISTER_REQR,
	REGISTER_MASKR,
	REGISTER_B1PSR,
	REGISTER_B1PCR,
	REGISTER_BG1DR,

	REGISTER_B2SR = 0x1c,
	REGISTER_B2CR,
	REGISTER_BG2DR
};

#define SMR0_MASK     0xf0

#define SMR1_MASK     0xe3

#define MSR_MASK      0x80

#define MCR_MASK      0x14

#define RXSR1_MASK    0x08

#define RXCR_MASK     0x7c
#define RXCR_HUNT     BIT(m_rxcr, 1)
#define RXCR_RXE      BIT(m_rxcr, 7)

#define RXIER_RXDI    BIT(m_rxier, 7)

#define TXSR_MASK     0x60
#define TXSR_TXRDY    BIT(m_txsr, 0)
#define TXSR_TXEND    BIT(m_txsr, 4)

#define TXCR_TXRST    BIT(m_txcr, 3)
#define TXCR_TXE      BIT(m_txcr, 7)

#define TXIER_MASK    0x60
#define TXIER_TXDI    BIT(m_txier, 7)

#define TXFR1_MASK    0x70

#define SMR3_MASK     0x80

#define REQR_MASK     0x38

#define MASKR_MASK    0x38
#define MASKR_MRXDRQ  BIT(m_maskr, 6)
#define MASKR_MTXDRQ  BIT(m_maskr, 7)

#define B1PSR_MASK    0xfc

#define B2SR_MASK     0xfe

//-------------------------------------------------
//  mb89374_device - constructor
//-------------------------------------------------

mb89374_device::mb89374_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock ) :
	device_t(mconfig, MB89374, tag, owner, clock),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_out_irq_cb(*this),
	m_out_po_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//------------------------------------------------

void mb89374_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	auto ctx = std::make_unique<context>();
	m_context = std::move(ctx);

	// state saving
	save_item(NAME(m_irq));
	save_item(NAME(m_po));
	save_item(NAME(m_pi));
	save_item(NAME(m_ci));

	save_item(NAME(m_smr0));
	save_item(NAME(m_smr1));
	save_item(NAME(m_smr2));
	save_item(NAME(m_chrr0));
	save_item(NAME(m_chrr1));
	save_item(NAME(m_msr));
	save_item(NAME(m_mcr));
	save_item(NAME(m_rxsr0));
	save_item(NAME(m_rxsr1));
	save_item(NAME(m_rxcr));
	save_item(NAME(m_rxier));
	save_item(NAME(m_txsr));
	save_item(NAME(m_txcr));
	save_item(NAME(m_txier));
	save_item(NAME(m_sdr));
	save_item(NAME(m_txbcr0));
	save_item(NAME(m_txbcr1));
	save_item(NAME(m_txfr0));
	save_item(NAME(m_txfr1));
	save_item(NAME(m_smr3));
	save_item(NAME(m_portr));
	save_item(NAME(m_reqr));
	save_item(NAME(m_maskr));
	save_item(NAME(m_b1psr));
	save_item(NAME(m_b1pcr));
	save_item(NAME(m_bg1dr));
	save_item(NAME(m_b2sr));
	save_item(NAME(m_b2cr));
	save_item(NAME(m_bg2dr));

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

void mb89374_device::device_reset()
{
	m_smr0   = SMR0_MASK;
	m_smr1   = SMR1_MASK;
	m_smr2   = 0x00;
	m_chrr0  = 0x00;
	m_chrr1  = 0x00;
	m_msr    = MSR_MASK;
	m_mcr    = MCR_MASK;
	m_rxsr0  = 0x00;
	m_rxsr1  = RXSR1_MASK;
	m_rxcr   = RXCR_MASK;
	m_rxier  = 0x00;
	m_txsr   = TXSR_MASK;
	m_txcr   = 0x00;
	m_txier  = TXIER_MASK;
	m_sdr    = 0x00;
	m_txbcr0 = 0x00;
	m_txbcr1 = 0x00;
	m_txfr0  = 0x00;
	m_txfr1  = TXFR1_MASK;
	m_smr3   = SMR3_MASK;
	m_portr  = 0x00;
	m_reqr   = REQR_MASK;
	m_maskr  = MASKR_MASK;
	m_b1psr  = B1PSR_MASK;
	m_b1pcr  = 0x00;
	m_bg1dr  = 0x00;
	m_b2sr   = B2SR_MASK;
	m_b2cr   = 0x00;
	m_bg2dr  = 0x00;

	set_irq(0);
	set_po0(0);
	set_po1(0);
	set_po2(0);
	set_po3(0);

	m_intr_delay = 0;
	m_sock_delay = 0x20;

	m_rx_length = 0x0000;
	m_rx_offset = 0x0000;

	m_tx_offset = 0x0000;

	m_context->stop();

	auto const &opts = mconfig().options();
	m_context->start(opts.comm_localhost(), opts.comm_localport(), opts.comm_remotehost(), opts.comm_remoteport());
}

void mb89374_device::device_stop()
{
	m_context->stop();
	m_context.reset();
}

//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mb89374_device::execute_run()
{
	while (m_icount > 0)
	{
		// TODO waste some cycles before triggering ints
		if (m_intr_delay > 0)
		{
			m_intr_delay--;
			if (m_intr_delay == 0)
			{
			}
		}
		if (m_sock_delay > 0)
		{
			m_sock_delay--;
			if (m_sock_delay == 0)
			{
				m_sock_delay = 0x20;
				comm_tick();
			}
		}
		m_icount--;
	}
}


//-------------------------------------------------
//  read - handler for register reading
//-------------------------------------------------

uint8_t mb89374_device::read(offs_t offset)
{
	uint8_t data = 0xff;
	switch (offset & 0x1f)
	{
		case REGISTER_RXSR0:
			data = m_rxsr0;
			break;

		case REGISTER_RXSR1:
			data = m_rxsr1;
			break;

		case REGISTER_TXSR:
			data = m_txsr;
			break;

		case REGISTER_SDR:
			m_sdr = rx_read();
			data = m_sdr;
			break;

		case REGISTER_PORTR:
			data = m_portr;
			break;

		default:
			logerror("MB89374 unimplemented register read @%02X\n", offset);
	}
	return data;
}


//-------------------------------------------------
//  write - handler for register writing
//-------------------------------------------------

void mb89374_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 0x1f)
	{
		case REGISTER_SMR2:
			m_smr2 = data;
			break;

		case REGISTER_MCR:
			m_mcr = data | MCR_MASK;
			break;

		case REGISTER_RXCR:
			m_rxcr = data | RXCR_MASK;
			if (RXCR_HUNT)
				rx_reset();
			break;

		case REGISTER_RXIER:
			m_rxier = data;
			break;

		case REGISTER_TXSR:
			m_txsr = data | TXSR_MASK;
			tx_reset();
			break;

		case REGISTER_TXCR:
			m_txcr = data;
			if (TXCR_TXRST)
				tx_reset();
			break;

		case REGISTER_TXIER:
			m_txier = data | TXIER_MASK;
			break;

		case REGISTER_SMR3:
			m_smr3 = data | SMR3_MASK;
			break;

		case REGISTER_PORTR:
			m_portr = data;
			break;

		case REGISTER_MASKR:
			m_maskr = data | MASKR_MASK;
			set_po2((!MASKR_MTXDRQ && TXCR_TXE && TXIER_TXDI) ? 1 : 0);
			break;

		default:
			logerror("MB89374 unimplemented register write @%02X = %02X\n", offset, data);
	}
}


//-------------------------------------------------
//  pi0_w - handler for RxCI#/PI0
//-------------------------------------------------

void mb89374_device::pi0_w(int state)
{
	m_pi[0] = state;
}


//-------------------------------------------------
//  pi1_w - handler for TxCI#/PI1
//-------------------------------------------------

void mb89374_device::pi1_w(int state)
{
	m_pi[1] = state;
}


//-------------------------------------------------
//  pi2_w - handler for TxDACK#/PI2
//-------------------------------------------------

void mb89374_device::pi2_w(int state)
{
	m_pi[2] = state;
}


//-------------------------------------------------
//  pi3_w - handler for RxDACK#/PI3
//-------------------------------------------------

void mb89374_device::pi3_w(int state)
{
	m_pi[3] = state;
}


//-------------------------------------------------
//  ci_w - handler for TxLAST#/CI#
//-------------------------------------------------

void mb89374_device::ci_w(int state)
{
	m_ci = state;
	if (m_ci == 1 && m_pi[2] == 0)
	{
		tx_complete();
	}
}


//-------------------------------------------------
//  read - handler for dma reading (rx buffer)
//-------------------------------------------------

uint8_t mb89374_device::dma_r()
{
	uint8_t data = rx_read();
	if (m_rx_offset == m_rx_length)
		set_po3(0); // transfer finished; release dma
	return data;
}


//-------------------------------------------------
//  write - handler for dma writing (tx buffer)
//-------------------------------------------------

void mb89374_device::dma_w(uint8_t data)
{
	tx_write(data);
}


//**************************************************************************
//  buffer logic
//**************************************************************************

void mb89374_device::rx_reset()
{
	m_rx_length = 0;
	m_rx_offset = 0;
	m_rxsr0 = 0x06; // RXIDL | DIDL
}

uint8_t mb89374_device::rx_read()
{
	uint8_t data = m_rx_buffer[m_rx_offset];
	m_rx_offset++;
	if (m_rx_offset == m_rx_length)
		m_rxsr0 |= 0x40; // EOF
	if (m_rx_offset >= m_rx_length)
		rx_reset();
	return data;
}

void mb89374_device::tx_reset()
{
	m_tx_offset = 0;
	m_txsr |= 0x05;
}

void mb89374_device::tx_write(uint8_t data)
{
	m_tx_buffer[m_tx_offset] = data;
	m_tx_offset++;
	m_txsr = 0x6b;

	// prevent overflow
	if (m_tx_offset >= 0x0f00)
		m_tx_offset = 0x0eff;
}

void mb89374_device::tx_complete()
{
	if (m_tx_offset > 0)
	{
		if (m_context->connected())
		{
			m_socket_buffer[0x00] = m_tx_offset & 0xff;
			m_socket_buffer[0x01] = (m_tx_offset >> 8) & 0xff;
			for (int i = 0x00; i < m_tx_offset; i++)
			{
				m_socket_buffer[i + 2] = m_tx_buffer[i];
			}
			send_frame(2 + m_tx_offset);
		}
	}

	m_txsr = 0x6f;

	tx_reset();
}

void mb89374_device::comm_tick()
{
	if (m_context->connected())
	{
		if (RXCR_RXE)
		{
			if (m_rx_length == 0)
			{
				unsigned recv = read_frame();
				if (recv > 0)
				{
					m_rx_length = m_socket_buffer[0x01] << 8 | m_socket_buffer[0x00];
					for (unsigned i = 0x00; i < m_rx_length; i++)
					{
						m_rx_buffer[i] = m_socket_buffer[2 + i];
					}

					m_rx_offset = 0;
					m_rxsr0 = 0x01; // RXRDY
					if (m_rx_offset + 1 == m_rx_length)
						m_rxsr0 |= 0x40; // EOF
					m_rxsr1 = 0xc8;
					set_po3(!MASKR_MRXDRQ && RXIER_RXDI ? 1 : 0);
				}
			}
		}
	}
}

unsigned mb89374_device::read_frame()
{
	unsigned bytes_read = m_context->receive(&m_socket_buffer[0]);
	if (bytes_read == UINT_MAX)
	{
		// error case, do nothing
		return 0;
	}
	return bytes_read;
}

void mb89374_device::send_frame(unsigned data_size){
	unsigned bytes_sent = m_context->send(&m_socket_buffer[0], data_size);
	if (bytes_sent == UINT_MAX)
	{
		// error case, do nothing
	}
}

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void mb89374_device::set_irq(int state)
{
	if (m_irq != state)
	{
		m_out_irq_cb(state);
		m_irq = state;
	}
}

inline void mb89374_device::set_po0(int state)
{
	if (m_po[0] != state)
	{
		m_out_po_cb[0](state);
		m_po[0] = state;
	}
}

inline void mb89374_device::set_po1(int state)
{
	if (m_po[1] != state)
	{
		m_out_po_cb[1](state);
		m_po[1] = state;
	}
}

inline void mb89374_device::set_po2(int state)
{
	if (m_po[2] != state)
	{
		m_out_po_cb[2](state);
		m_po[2] = state;
	}
}

inline void mb89374_device::set_po3(int state)
{
	if (m_po[3] != state)
	{
		m_out_po_cb[3](state);
		m_po[3] = state;
	}
}
