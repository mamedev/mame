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

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB89374, mb89374_device, "mb89374", "MB89374 Data Link Controller")


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

mb89374_device::mb89374_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, MB89374, tag, owner, clock),
	device_execute_interface(mconfig, *this),
	m_icount(0),
	m_out_irq_cb(*this),
	m_out_po_cb{ { *this }, { *this }, { *this }, { *this } }
{
	// prepare localhost "filename"
	m_localhost[0] = 0;
	strcat(m_localhost, "socket.");
	strcat(m_localhost, mconfig.options().comm_localhost());
	strcat(m_localhost, ":");
	strcat(m_localhost, mconfig.options().comm_localport());

	// prepare remotehost "filename"
	m_remotehost[0] = 0;
	strcat(m_remotehost, "socket.");
	strcat(m_remotehost, mconfig.options().comm_remotehost());
	strcat(m_remotehost, ":");
	strcat(m_remotehost, mconfig.options().comm_remoteport());
}


//-------------------------------------------------
//  device_start - device-specific startup
//------------------------------------------------

void mb89374_device::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// resolve callbacks
	m_out_irq_cb.resolve_safe();
	for(auto &cb : m_out_po_cb)
		cb.resolve_safe();

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
				checkSockets();
			}
		}
		m_icount--;
	}
}


//-------------------------------------------------
//  read - handler for register reading
//-------------------------------------------------

READ8_MEMBER(mb89374_device::read)
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
			m_sdr = rxRead();
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

WRITE8_MEMBER(mb89374_device::write)
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
				rxReset();
			break;

		case REGISTER_RXIER:
			m_rxier = data;

		case REGISTER_TXSR:
			m_txsr = data | TXSR_MASK;
			txReset();
			break;

		case REGISTER_TXCR:
			m_txcr = data;
			if (TXCR_TXRST)
				txReset();
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

WRITE_LINE_MEMBER(mb89374_device::pi0_w)
{
	m_pi[0] = state;
}


//-------------------------------------------------
//  pi1_w - handler for TxCI#/PI1
//-------------------------------------------------

WRITE_LINE_MEMBER(mb89374_device::pi1_w)
{
	m_pi[1] = state;
}


//-------------------------------------------------
//  pi2_w - handler for TxDACK#/PI2
//-------------------------------------------------

WRITE_LINE_MEMBER(mb89374_device::pi2_w)
{
	m_pi[2] = state;
}


//-------------------------------------------------
//  pi3_w - handler for RxDACK#/PI3
//-------------------------------------------------

WRITE_LINE_MEMBER(mb89374_device::pi3_w)
{
	m_pi[3] = state;
}


//-------------------------------------------------
//  ci_w - handler for TxLAST#/CI#
//-------------------------------------------------

WRITE_LINE_MEMBER(mb89374_device::ci_w)
{
	m_ci = state;
	if (m_ci == 1 && m_pi[2] == 0)
	{
		txComplete();
	}

}


//-------------------------------------------------
//  read - handler for dma reading (rx buffer)
//-------------------------------------------------

READ8_MEMBER(mb89374_device::dma_r)
{
	uint8_t data = rxRead();
	if (m_rx_offset == m_rx_length)
		set_po3(0); // transfer finished; release dma
	return data;
}


//-------------------------------------------------
//  write - handler for dma writing (tx buffer)
//-------------------------------------------------

WRITE8_MEMBER(mb89374_device::dma_w)
{
	txWrite(data);
}


//**************************************************************************
//  buffer logic
//**************************************************************************

void mb89374_device::rxReset()
{
	m_rx_length = 0;
	m_rx_offset = 0;
	m_rxsr0 = 0x06; // RXIDL | DIDL
}

uint8_t mb89374_device::rxRead()
{
	uint8_t data = m_rx_buffer[m_rx_offset];
	m_rx_offset++;
	if (m_rx_offset == m_rx_length)
		m_rxsr0 |= 0x40; // EOF
	if (m_rx_offset >= m_rx_length)
		rxReset();
	return data;
}

void mb89374_device::txReset()
{
	m_tx_offset = 0;
	m_txsr |= 0x05;
}

void mb89374_device::txWrite(uint8_t data)
{
	m_tx_buffer[m_tx_offset] = data;
	m_tx_offset++;
	m_txsr = 0x6b;

	// prevent overflow
	if (m_tx_offset >= 0x0f00)
		m_tx_offset = 0x0eff;
}

void mb89374_device::txComplete()
{
	if (m_tx_offset > 0)
	{
		if (m_line_rx && m_line_tx)
		{
			m_socket_buffer[0x00] = m_tx_offset & 0xff;
			m_socket_buffer[0x01] = (m_tx_offset >> 8) & 0xff;
			for (int i = 0x00 ; i < m_tx_offset ; i++)
			{
				m_socket_buffer[i + 2] = m_tx_buffer[i];
			}

			std::uint32_t dataSize = m_tx_offset + 2;
			std::uint32_t written;

			m_line_tx->write(&m_socket_buffer, 0, dataSize, written);
		}
	}

	m_txsr = 0x6f;

	txReset();
}

void mb89374_device::checkSockets()
{
	// check rx socket
	if (!m_line_rx)
	{
		osd_printf_verbose("MB89374 listen on %s\n", m_localhost);
		uint64_t filesize; // unused
		osd_file::open(m_localhost, OPEN_FLAG_CREATE, m_line_rx, filesize);
	}

	// check tx socket
	if (!m_line_tx)
	{
		osd_printf_verbose("MB89374 connect to %s\n", m_remotehost);
		uint64_t filesize; // unused
		osd_file::open(m_remotehost, 0, m_line_tx, filesize);
	}

	if (m_line_rx && m_line_tx)
	{
		if (RXCR_RXE)
		{
			if (m_rx_length == 0)
			{
				std::uint32_t recv = 0;
				m_line_rx->read(m_socket_buffer, 0, 2, recv);
				if (recv > 0)
				{
					if (recv == 2)
						m_rx_length = m_socket_buffer[0x01] << 8 | m_socket_buffer[0x00];
					else
					{
						m_rx_length = m_socket_buffer[0x00];
						m_line_rx->read(m_socket_buffer, 0, 1, recv);
						while (recv == 0) {}
						m_rx_length |= m_socket_buffer[0x00] << 8;
					}

					int offset = 0;
					int togo = m_rx_length;
					while (togo > 0)
					{
						m_line_rx->read(m_socket_buffer, 0, togo, recv);
						for (int i = 0x00 ; i < recv ; i++)
						{
							m_rx_buffer[offset] = m_socket_buffer[i];
							offset++;
						}
						togo -= recv;
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
