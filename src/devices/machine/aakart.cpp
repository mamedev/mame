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
DEFINE_DEVICE_TYPE(AAKART, aakart_device, "aakart", "Acorn Archimedes KART")

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

aakart_device::aakart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AAKART, tag, owner, clock), m_rxtimer(nullptr),
		m_txtimer(nullptr), m_mousetimer(nullptr), m_keybtimer(nullptr),
		m_out_tx_cb(*this),
		m_out_rx_cb(*this),
		m_tx_latch(0), m_rx(0), m_new_command(0), m_status(0), m_mouse_enable(0), m_keyb_enable(0)
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
	m_txtimer->adjust(attotime::from_hz(1), 0, attotime::from_hz(clock()));
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
	m_keyb_enable = 0;
	m_mouse_enable = 0;
	m_queue_size = 0;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void aakart_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if(id == TX_TIMER && m_new_command)
	{
		switch(m_tx_latch)
		{
			case 0x00:
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
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
				if(m_queue_size)
				{
					m_rx = m_queue[0] & 0xff;
					m_out_tx_cb(ASSERT_LINE);
				}
				break;
			case 0x3f:
				if(m_queue_size)
				{
					m_rx = (m_queue[0] >> 8) & 0xff;
					m_out_tx_cb(ASSERT_LINE);

					m_queue_size--;
					for(int i=0; i<m_queue_size; i++)
						m_queue[i] = m_queue[i + 1];
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

		m_new_command = 0;
		m_out_rx_cb(ASSERT_LINE);
	}
	else if(id == TX_TIMER && m_queue_size && (m_keyb_enable || m_mouse_enable))
	{
		m_rx = m_queue[0] & 0xff;
		m_out_tx_cb(ASSERT_LINE);
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


uint8_t aakart_device::read()
{
	m_out_tx_cb(CLEAR_LINE);
	//machine().debug_break();
	return m_rx;
}

void aakart_device::write(uint8_t data)
{
	// if(m_new_command) printf("skip cmd %02x\n",data);

	m_tx_latch = data;
	m_out_rx_cb(CLEAR_LINE);
	m_new_command |= 1;
}

void aakart_device::send_keycode_down(uint8_t row, uint8_t col)
{
	//printf("keycode down\n");
	if (m_keyb_enable && m_queue_size < 0x10)
		m_queue[m_queue_size++] = ((col | 0xc0) << 8) | (row | 0xc0);
}

void aakart_device::send_keycode_up(uint8_t row, uint8_t col)
{
	//printf("keycode up\n");
	if (m_keyb_enable && m_queue_size < 0x10)
		m_queue[m_queue_size++] = ((col | 0xd0) << 8) | (row | 0xd0);
}

void aakart_device::send_mouse(uint8_t x, uint8_t y)
{
	if (m_mouse_enable && m_queue_size < 0x10)
		m_queue[m_queue_size++] = ((y & 0x7f) << 8) | (x & 0x7f);
}
