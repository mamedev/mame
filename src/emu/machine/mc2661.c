// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Motorola MC2661/MC68661 Enhanced Programmable Communications Interface

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

mc2661_device::mc2661_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MC2661, "MC2661", tag, owner, clock, "mc2661", __FILE__),
		device_serial_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mc2661_device::device_config_complete()
{
	// inherit a copy of the static data
	const mc2661_interface *intf = reinterpret_cast<const mc2661_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mc2661_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_txd_cb, 0, sizeof(m_out_txd_cb));
		memset(&m_out_rxrdy_cb, 0, sizeof(m_out_rxrdy_cb));
		memset(&m_out_txrdy_cb, 0, sizeof(m_out_txrdy_cb));
		memset(&m_out_rts_cb, 0, sizeof(m_out_rts_cb));
		memset(&m_out_dtr_cb, 0, sizeof(m_out_dtr_cb));
		memset(&m_out_txemt_dschg_cb, 0, sizeof(m_out_txemt_dschg_cb));
		memset(&m_out_bkdet_cb, 0, sizeof(m_out_bkdet_cb));
		memset(&m_out_xsync_cb, 0, sizeof(m_out_xsync_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc2661_device::device_start()
{
	// resolve callbacks
	m_out_txd_func.resolve(m_out_txd_cb, *this);
	m_out_rxrdy_func.resolve(m_out_rxrdy_cb, *this);
	m_out_txrdy_func.resolve(m_out_txrdy_cb, *this);
	m_out_rts_func.resolve(m_out_rts_cb, *this);
	m_out_dtr_func.resolve(m_out_dtr_cb, *this);
	m_out_txemt_dschg_func.resolve(m_out_txemt_dschg_cb, *this);
	m_out_bkdet_func.resolve(m_out_bkdet_cb, *this);
	m_out_xsync_func.resolve(m_out_xsync_cb, *this);

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

	m_out_txd_func(1);
	m_out_rxrdy_func(CLEAR_LINE);
	m_out_txrdy_func(CLEAR_LINE);
	m_out_rts_func(1);
	m_out_dtr_func(1);
	m_out_txemt_dschg_func(CLEAR_LINE);
	m_out_bkdet_func(0);
	m_out_xsync_func(0);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void mc2661_device::tra_callback()
{
	m_out_txd_func(transmit_register_get_data_bit());
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void mc2661_device::tra_complete()
{
	// TODO
	m_sr |= STATUS_TXRDY;
	m_out_txrdy_func(ASSERT_LINE);
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
	m_out_rxrdy_func(ASSERT_LINE);
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
		m_out_rxrdy_func(CLEAR_LINE);
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
		if (LOG) logerror("MC2661 '%s' Transmit Holding Register: %02x\n", tag(), data);

		m_thr = data;
		if(COMMAND_TXEN)
		{
			if(COMMAND_MODE != 0x02)
				transmit_register_setup(m_thr);
			m_sr &= ~STATUS_TXRDY;
			m_out_txrdy_func(CLEAR_LINE);
		}
		if(COMMAND_MODE == 0x02)  // loopback - the Wicat will set this after enabling the transmitter
			m_rhr = data;
		break;

	case REGISTER_SYNC:
		if (LOG) logerror("MC2661 '%s' Sync Register %u: %02x\n", tag(), m_sync_index + 1, data);

		m_sync[m_sync_index] = data;

		m_sync_index++;
		if (m_sync_index == 3) m_sync_index = 0;
		break;

	case REGISTER_MODE:
		if (LOG) logerror("MC2661 '%s' Mode Register %u: %02x\n", tag(), m_mode_index + 1, data);

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
		if (LOG) logerror("MC2661 '%s' Command Register: %02x\n", tag(), data);

		m_cr = data & 0xef;

		m_out_dtr_func(!COMMAND_DTR);
		m_out_rts_func(!COMMAND_RTS);

		if (COMMAND_MODE == 0x02)  // local loopback
		{
			if(COMMAND_DTR && COMMAND_RTS)  // CR1 and CR5 must be set to 1 to use local loopback
			{
				// probably much more to it that this, but this is enough for the Wicat to be happy
				m_rhr = m_thr;
				m_sr |= STATUS_RXRDY;
				m_out_rxrdy_func(ASSERT_LINE);
				return;
			}
		}

		if (COMMAND_TXEN)
		{
			m_sr |= STATUS_TXRDY;
			m_out_txrdy_func(ASSERT_LINE);
		}
		else
		{
			m_sr &= ~STATUS_TXRDY;
			m_out_txrdy_func(CLEAR_LINE);
		}
		if (!COMMAND_RXEN)
		{
			m_sr &= ~STATUS_RXRDY;
			m_out_rxrdy_func(CLEAR_LINE);
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
	if (LOG) logerror("MC2661 '%s' Data Set Ready: %u\n", tag(), state);

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
	if (LOG) logerror("MC2661 '%s' Data Carrier Detect: %u\n", tag(), state);

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
	if (LOG) logerror("MC2661 '%s' Clear to Send: %u\n", tag(), state);
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
