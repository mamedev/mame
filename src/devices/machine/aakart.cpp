// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Acorn Archimedes KART interface

TODO:
- FIFO

***************************************************************************/

#include "emu.h"
#include "machine/aakart.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type AAKART = &device_creator<aakart_device>;

#define HRST 0xff
#define RAK1 0xfe
#define RAK2 0xfd
#define BACK 0x3f
#define SMAK 0x33   /* keyboard + mouse ack */
#define MACK 0x32   /* mouse ack */
#define SACK 0x31   /* keyboard ack */
#define NACK 0x30   /* no data ack */
#define RQID 0x20

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aakart_device - constructor
//-------------------------------------------------

aakart_device::aakart_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AAKART, "AAKART", tag, owner, clock, "aakart", __FILE__), m_rxtimer(nullptr), m_txtimer(nullptr), m_mousetimer(nullptr), m_keybtimer(nullptr),
		m_out_tx_cb(*this),
		m_out_rx_cb(*this), m_tx_latch(0), m_rx(0), m_new_command(0), m_status(0), m_mouse_enable(0), m_keyb_enable(0), m_keyb_row(0), m_keyb_col(0), m_keyb_state(0)
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void aakart_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aakart_device::device_start()
{
	m_out_tx_cb.resolve_safe();
	m_out_rx_cb.resolve_safe();
	m_rxtimer = timer_alloc(RX_TIMER);
	m_rxtimer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	m_txtimer = timer_alloc(TX_TIMER);
	m_txtimer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	m_mousetimer = timer_alloc(MOUSE_TIMER);
	m_mousetimer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
	m_keybtimer = timer_alloc(KEYB_TIMER);
	m_keybtimer->adjust(attotime::from_hz(clock()), 0, attotime::from_hz(clock()));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aakart_device::device_reset()
{
	m_status = STATUS_HRST;
	m_new_command = 0;
	m_rx = -1;
	m_mouse_enable = 0;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void aakart_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id == TX_TIMER && m_new_command & 1)
	{
		switch(m_tx_latch)
		{
			case 0x00:
			case 0x02:
			case 0x03:
			case 0x07:
				// ---- -x-- scroll lock
				// ---- --x- num lock
				// ---- ---x caps lock
				break;
			case 0x20:
				m_rx = 0x81;
				m_out_tx_cb(ASSERT_LINE);
				break;
			case 0x30:
			case 0x31:
			case 0x32:
			case 0x33:
				m_keyb_enable = m_tx_latch & 1;
				m_mouse_enable = (m_tx_latch & 2) >> 1;
				if(m_keyb_enable & 1 && m_keyb_state & 1)
				{
					//printf("Got row\n");
					m_rx = m_keyb_row;
					m_out_tx_cb(ASSERT_LINE);
				}

				break;
			case 0x3f:
				if(m_keyb_enable & 1 && m_keyb_state & 1)
				{
					//printf("Got col\n");
					m_rx = m_keyb_col;
					m_out_tx_cb(ASSERT_LINE);
					m_keyb_state = 0;
				}

				break;
			case 0xfd:
				m_rx = 0xfd;
				m_out_tx_cb(ASSERT_LINE);
				break;
			case 0xfe:
				m_rx = 0xfe;
				m_out_tx_cb(ASSERT_LINE);
				break;
			case 0xff:
				m_rx = 0xff;
				m_out_tx_cb(ASSERT_LINE);
				break;
			default:
				//printf("%02x %02x %02x\n",m_tx_latch,m_rx_latch,m_keyb_enable);
				break;
		}

		//m_new_command &= ~1;
		m_out_rx_cb(ASSERT_LINE);
	}

}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

#include "debugger.h"

READ8_MEMBER( aakart_device::read )
{
	m_out_tx_cb(CLEAR_LINE);
	//debugger_break(machine());
	return m_rx;
}

WRITE8_MEMBER( aakart_device::write )
{
	// if(m_new_command) printf("skip cmd %02x\n",data);

	m_tx_latch = data;
	m_out_rx_cb(CLEAR_LINE);
	m_new_command |= 1;
}

void aakart_device::send_keycode_down(UINT8 row, UINT8 col)
{
	//printf("keycode down\n");
	m_keyb_row = row | 0xc0;
	m_keyb_col = col | 0xc0;
	m_keyb_state = 1;
}

void aakart_device::send_keycode_up(UINT8 row, UINT8 col)
{
	//printf("keycode up\n");
	m_keyb_row = row | 0xd0;
	m_keyb_col = col | 0xd0;
	m_keyb_state = 1;
}
