// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Motorola MC2661/MC68661 Enhanced Programmable Communications Interface

***************************************************************************/

#include "mc2661.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MC2661 = &device_creator<mc2661_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


UINT32 baud_rates[16] =
{
	50, 75, 110, 135 /*134.5*/, 150, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200, 9600, 19200
};

enum
{
	REGISTER_HOLDING = 0,
	REGISTER_STATUS,
	REGISTER_SYNC = REGISTER_STATUS,
	REGISTER_MODE,
	REGISTER_COMMAND
};


#define MODE_BAUD_RATE      (m_mr[0] & 0x03)
#define MODE_CHARACTER      ((m_mr[0] >> 2) & 0x03)
#define MODE_PARITY         BIT(m_mr[0], 4)
#define MODE_PARITY_EVEN    BIT(m_mr[0], 5)
#define MODE_TRANSPARENT    BIT(m_mr[0], 6)
#define MODE_SINGLE_SYN     BIT(m_mr[0], 7)
#define MODE_STOP_BITS      ((m_mr[0] >> 6) & 0x03)


#define SYN1            m_sync[0]
#define SYN2            m_sync[1]
#define DLE             m_sync[2]


#define COMMAND_TXEN    BIT(m_cr, 0)
#define COMMAND_DTR     BIT(m_cr, 1)
#define COMMAND_RXEN    BIT(m_cr, 2)
#define COMMAND_BREAK   BIT(m_cr, 3)
#define COMMAND_DLE     BIT(m_cr, 3)
#define COMMAND_RESET   BIT(m_cr, 4)
#define COMMAND_RTS     BIT(m_cr, 5)
#define COMMAND_MODE    (m_cr >> 6)


enum
{
	MODE_NORMAL = 0,
	MODE_ASYNC,
	MODE_LOCAL_LOOP_BACK,
	MODE_REMOTE_LOOP_BACK
};


#define STATUS_TXRDY    0x01
#define STATUS_RXRDY    0x02
#define STATUS_TXEMT    0x04
#define STATUS_PE       0x08
#define STATUS_DLE      0x08
#define STATUS_OVERRUN  0x10
#define STATUS_FE       0x20
#define STATUS_SYN      0x20
#define STATUS_DCD      0x40
#define STATUS_DSR      0x80



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc2661_device - constructor
//-------------------------------------------------

mc2661_device::mc2661_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MC2661, "MC2661", tag, owner, clock, "mc2661", __FILE__),
	device_serial_interface(mconfig, *this),
	m_write_txd(*this),
	m_write_rxrdy(*this),
	m_write_txrdy(*this),
	m_write_rts(*this),
	m_write_dtr(*this),
	m_write_txemt_dschg(*this),
	m_write_bkdet(*this),
	m_write_xsync(*this),
	m_rxc(0),
	m_txc(0),
	m_sr(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc2661_device::device_start()
{
	// resolve callbacks
	m_write_txd.resolve_safe();
	m_write_rxrdy.resolve_safe();
	m_write_txrdy.resolve_safe();
	m_write_rts.resolve_safe();
	m_write_dtr.resolve_safe();
	m_write_txemt_dschg.resolve_safe();
	m_write_bkdet.resolve_safe();
	m_write_xsync.resolve_safe();

	// create the timers
	if (m_rxc > 0)
	{
		set_rcv_rate(m_rxc);
	}

	if (m_txc > 0)
	{
		set_tra_rate(m_txc);
	}

	// save state
	save_item(NAME(m_rhr));
	save_item(NAME(m_thr));
	save_item(NAME(m_cr));
	save_item(NAME(m_sr));
	save_item(NAME(m_mr));
	save_item(NAME(m_sync));
	save_item(NAME(m_mode_index));
	save_item(NAME(m_sync_index));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc2661_device::device_reset()
{
	receive_register_reset();
	transmit_register_reset();

	m_mr[0] = m_mr[1] = 0;
	m_sync[0] = m_sync[1] = m_sync[2] = 0;
	m_cr = 0;
	m_sr = 0;

	m_mode_index = 0;
	m_sync_index = 0;

	m_write_txd(1);
	m_write_rxrdy(CLEAR_LINE);
	m_write_txrdy(CLEAR_LINE);
	m_write_rts(1);
	m_write_dtr(1);
	m_write_txemt_dschg(CLEAR_LINE);
	m_write_bkdet(0);
	m_write_xsync(0);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void mc2661_device::tra_callback()
{
	m_write_txd(transmit_register_get_data_bit());
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void mc2661_device::tra_complete()
{
	// TODO
	m_sr |= STATUS_TXRDY;
	m_write_txrdy(ASSERT_LINE);
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void mc2661_device::rcv_complete()
{
	// TODO
	receive_register_extract();
	m_rhr = get_received_char();
	m_sr |= STATUS_RXRDY;
	m_write_rxrdy(ASSERT_LINE);
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

READ8_MEMBER( mc2661_device::read )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case REGISTER_HOLDING:
		data = m_rhr;
		m_sr &= ~STATUS_RXRDY;
		m_write_rxrdy(CLEAR_LINE);
		break;

	case REGISTER_STATUS:
		data = m_sr;
		break;

	case REGISTER_MODE:
		data = m_mr[m_mode_index];

		m_mode_index++;
		m_mode_index &= 0x01;

		break;

	case REGISTER_COMMAND:
		m_mode_index = 0;
		m_sync_index = 0;

		data = m_cr;
		break;
	}

	return data;
}


//-------------------------------------------------
//  write - register write
//-------------------------------------------------

WRITE8_MEMBER( mc2661_device::write )
{
	switch (offset & 0x03)
	{
	case REGISTER_HOLDING:
		if (LOG) logerror("MC2661 '%s' Transmit Holding Register: %02x\n", tag().c_str(), data);

		m_thr = data;
		if(COMMAND_TXEN)
		{
			if(COMMAND_MODE != 0x02)
				transmit_register_setup(m_thr);
			m_sr &= ~STATUS_TXRDY;
			m_write_txrdy(CLEAR_LINE);
		}
		if(COMMAND_MODE == 0x02)  // loopback - the Wicat will set this after enabling the transmitter
		{
			m_rhr = data;
			m_sr |= STATUS_RXRDY; // pcd expects this
			m_write_rxrdy(ASSERT_LINE);
		}
		break;

	case REGISTER_SYNC:
		if (LOG) logerror("MC2661 '%s' Sync Register %u: %02x\n", tag().c_str(), m_sync_index + 1, data);

		m_sync[m_sync_index] = data;

		m_sync_index++;
		if (m_sync_index == 3) m_sync_index = 0;
		break;

	case REGISTER_MODE:
		if (LOG) logerror("MC2661 '%s' Mode Register %u: %02x\n", tag().c_str(), m_mode_index + 1, data);

		m_mr[m_mode_index] = data;

		if (m_mode_index == 0)
		{
			int data_bit_count = 5 + MODE_CHARACTER;
			parity_t parity;

			if (!MODE_PARITY) parity = PARITY_NONE;
			else if (MODE_PARITY_EVEN) parity = PARITY_EVEN;
			else parity = PARITY_ODD;

			stop_bits_t stop_bits;

			switch (MODE_STOP_BITS)
			{
			case 0:
			default:
				stop_bits = STOP_BITS_0;
				break;

			case 1:
				stop_bits = STOP_BITS_1;
				break;

			case 2:
				stop_bits = STOP_BITS_1_5;
				break;

			case 3:
				stop_bits = STOP_BITS_2;
				break;
			}

			set_data_frame(1, data_bit_count, parity, stop_bits);
		}
		if(m_mode_index == 1)
		{
			UINT32 rx_baud = baud_rates[data & 0x0f];
			UINT32 tx_baud = baud_rates[data & 0x0f];
			if(data & 0x10)  // internal receiver clock
			{
//              if((m_mr[0] & 0x03) != 0)
//                  rx_baud *= 16;
			}
			else  // external receiver clock
			{
				switch(m_mr[0] & 0x03)
				{
				case 0x02:
					rx_baud *= 16;
					break;
				case 0x03:
					rx_baud *= 64;
					break;
				default:
					// x1
					break;
				}
			}
			if(data & 0x20)  // internal transmitter clock
			{
//              if((m_mr[0] & 0x03) != 0)
//                  tx_baud *= 16;
			}
			else  // external transmitter clock
			{
				switch(m_mr[0] & 0x03)
				{
				case 0x02:
					tx_baud *= 16;
					break;
				case 0x03:
					tx_baud *= 64;
					break;
				default:
					// x1
					break;
				}
			}

			set_rcv_rate(rx_baud);
			set_tra_rate(tx_baud);
		}

		m_mode_index++;
		m_mode_index &= 0x01;
		break;

	case REGISTER_COMMAND:
		if (LOG) logerror("MC2661 '%s' Command Register: %02x\n", tag().c_str(), data);

		m_cr = data & 0xef;

		m_write_dtr(!COMMAND_DTR);
		m_write_rts(!COMMAND_RTS);

		if (COMMAND_MODE == 0x02)  // local loopback
		{
			if(COMMAND_DTR && COMMAND_RTS)  // CR1 and CR5 must be set to 1 to use local loopback
			{
				// probably much more to it that this, but this is enough for the Wicat to be happy
				m_rhr = m_thr;
				m_sr |= STATUS_RXRDY;
				m_write_rxrdy(ASSERT_LINE);
				return;
			}
		}

		if (COMMAND_TXEN)
		{
			m_sr |= STATUS_TXRDY;
			m_write_txrdy(ASSERT_LINE);
		}
		else
		{
			m_sr &= ~STATUS_TXRDY;
			m_write_txrdy(CLEAR_LINE);
		}
		if (!COMMAND_RXEN)
		{
			m_sr &= ~STATUS_RXRDY;
			m_write_rxrdy(CLEAR_LINE);
		}
		if (COMMAND_RESET)
		{
			m_sr &= ~(STATUS_FE | STATUS_OVERRUN | STATUS_PE);
		}
		break;
	}
}

//-------------------------------------------------
//  dsr_w - data set ready
//-------------------------------------------------

WRITE_LINE_MEMBER( mc2661_device::dsr_w )
{
	if (LOG) logerror("MC2661 '%s' Data Set Ready: %u\n", tag().c_str(), state);

	if (state)
	{
		m_sr &= ~STATUS_DSR;
	}
	else
	{
		m_sr |= STATUS_DSR;
	}
}


//-------------------------------------------------
//  dcd_w - data carrier detect
//-------------------------------------------------

WRITE_LINE_MEMBER( mc2661_device::dcd_w )
{
	if (LOG) logerror("MC2661 '%s' Data Carrier Detect: %u\n", tag().c_str(), state);

	if (state)
	{
		m_sr &= ~STATUS_DCD;
	}
	else
	{
		m_sr |= STATUS_DCD;
	}
}


//-------------------------------------------------
//  cts_w - clear to send
//-------------------------------------------------

WRITE_LINE_MEMBER( mc2661_device::cts_w )
{
	if (LOG) logerror("MC2661 '%s' Clear to Send: %u\n", tag().c_str(), state);
}


//-------------------------------------------------
//  rxrdy_r - receiver ready
//-------------------------------------------------

READ_LINE_MEMBER( mc2661_device::rxrdy_r )
{
	return (m_sr & STATUS_RXRDY) ? ASSERT_LINE : CLEAR_LINE;
}


//-------------------------------------------------
//  txemt_r - transmitter empty
//-------------------------------------------------

READ_LINE_MEMBER( mc2661_device::txemt_r )
{
	return (m_sr & STATUS_TXEMT) ? ASSERT_LINE : CLEAR_LINE;
}


void mc2661_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}
