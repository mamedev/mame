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

aakart_device::aakart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, AAKART, "aakart", tag, owner, clock)
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
	m_out_tx_func.resolve(m_out_tx_cb, *this);
	m_out_rx_func.resolve(m_out_rx_cb, *this);
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
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void aakart_device::device_config_complete()
{
	// inherit a copy of the static data
	const aakart_interface *intf = reinterpret_cast<const aakart_interface *>(static_config());
	if (intf != NULL)
		*static_cast<aakart_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_out_tx_cb, 0, sizeof(m_out_tx_cb));
		memset(&m_out_rx_cb, 0, sizeof(m_out_rx_cb));
	}
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void aakart_device::device_reset()
{
	m_status = STATUS_NORMAL;
	m_new_command = 0;
	m_rx = -1;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void aakart_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	#if 0
	if(id == KEYB_TIMER && m_keyb_enable && m_status == STATUS_NORMAL)
	{
		m_new_command |= 2;
		m_rx_latch = 0xd0 | 0; // keyb scancode (0xd0=up 0xc0=down, bits 3-0 row)
		m_status = STATUS_KEYUP;
		//m_ff ^= 1;
		return;
	}
	#endif

	if(id == MOUSE_TIMER && m_mouse_enable && m_status == STATUS_NORMAL)
	{
		m_new_command |= 2;
		m_rx_latch = 0; // mouse X position
		m_status = STATUS_MOUSE;
		//m_ff ^= 1;
		return;
	}

	if(m_new_command == 0)
		return;

	if(id == RX_TIMER && m_new_command & 2)
	{
		m_out_rx_func(ASSERT_LINE);
		m_new_command &= ~2;
		m_rx = m_rx_latch;
		return;
	}

	if(id == TX_TIMER && m_new_command & 1)
	{
		switch(m_status)
		{
			case STATUS_NORMAL:
			{
				switch(m_tx_latch)
				{
					case 0x00: // set leds
						break;
					case RQID:
						m_rx_latch = 0x81; //keyboard ID
						break;
					case SMAK:
					case MACK:
					case SACK:
					case NACK:
						if(m_tx_latch & 2) { m_mouse_enable = 1; }
						if(m_tx_latch & 1) { m_keyb_enable = 1; }
						break;
					case HRST:
						m_rx_latch = HRST;
						m_status = STATUS_HRST;
						break;
					default:
						//printf("%02x\n",m_tx_latch);
						break;
				}
				break;
			}
			case STATUS_KEYDOWN:
			{
				switch(m_tx_latch)
				{
					case BACK:
						m_rx_latch = 0xc0 | 0; // keyb scancode (0xd0=up 0xc0=down, bits 3-0 col)
						m_status = STATUS_NORMAL;
						break;
					case HRST:
						m_rx_latch = HRST;
						m_status = STATUS_HRST;
						break;
				}
				break;
			}
			case STATUS_KEYUP:
			{
				switch(m_tx_latch)
				{
					case BACK:
						m_rx_latch = 0xd0 | 0; // keyb scancode (0xd0=up 0xc0=down, bits 3-0 col)
						m_status = STATUS_NORMAL;
						break;
					case HRST:
						m_rx_latch = HRST;
						m_status = STATUS_HRST;
						break;
				}
				break;
			}
			case STATUS_MOUSE:
			{
				switch(m_tx_latch)
				{
					case BACK:
						m_rx_latch = 0; // mouse Y
						m_status = STATUS_NORMAL;
						break;
					case HRST:
						m_rx_latch = HRST;
						m_status = STATUS_HRST;
						break;
				}
				break;
			}
			case STATUS_HRST:
			{
				switch(m_tx_latch)
				{
					case HRST:  { m_rx_latch = HRST; m_keyb_enable = m_mouse_enable = 0; break; }
					case RAK1:  { m_rx_latch = RAK1; m_keyb_enable = m_mouse_enable = 0; break; }
					case RAK2:  { m_rx_latch = RAK2; m_status = STATUS_NORMAL; break; }
				}
				break;
			}
		}
		m_out_tx_func(ASSERT_LINE);
		m_new_command &= ~1;
		m_new_command |= 2;
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( aakart_device::read )
{
	m_out_tx_func(CLEAR_LINE);
	return m_rx;
}

WRITE8_MEMBER( aakart_device::write )
{
	if(m_new_command)
		printf("skip cmd %02x\n",data);

	m_tx_latch = data;
	m_new_command |= 1;
	m_out_rx_func(CLEAR_LINE);
}
