/**********************************************************************

    Sinclair ZX8302 emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - set time
    - read from microdrive
    - write to microdrive
    - DTR/CTS handling
    - network

*/

#include "emu.h"
#include <time.h>
#include "devices/microdrv.h"
#include "machine/devhelpr.h"
#include "zx8302.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


// IPC serial state
enum
{
	IPC_START,
	IPC_DATA,
	IPC_STOP
};


// baud rate
enum
{
	 BAUD_19200				= 0x00,
	 BAUD_9600				= 0x01,
	 BAUD_4800				= 0x02,
	 BAUD_2400				= 0x03,
	 BAUD_1200				= 0x04,
	 BAUD_600				= 0x05,
	 BAUD_300				= 0x06,
	 BAUD_75				= 0x07,
	 BAUD_MASK				= 0x07,
};

// transmit mode
enum
{
	 MODE_SER1				= 0x00,
	 MODE_SER2				= 0x08,
	 MODE_MDV				= 0x10,
	 MODE_NET				= 0x18,
	 MODE_MASK				= 0x18,
};

// interrupts
enum
{
	 INT_GAP				= 0x01,
	 INT_INTERFACE			= 0x02,
	 INT_TRANSMIT			= 0x04,
	 INT_FRAME				= 0x08,
	 INT_EXTERNAL			= 0x10,
};

// status register
enum
{
	 STATUS_NETWORK_PORT	= 0x01,
	 STATUS_TX_BUFFER_FULL	= 0x02,
	 STATUS_RX_BUFFER_FULL	= 0x04,
	 STATUS_MICRODRIVE_GAP	= 0x08,
};

// transmit bits
enum
{
	 TXD_START				= 0,
	 TXD_STOP				= 9,
	 TXD_STOP2				= 10,
};


// Monday 1st January 1979 00:00:00 UTC
enum
{
	 RTC_BASE_ADJUST		= 283996800,
};


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ZX8302 = &device_creator<zx8302_device>;

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void zx8302_device::device_config_complete()
{
	// inherit a copy of the static data
	const zx8302_interface *intf = reinterpret_cast<const zx8302_interface *>(static_config());
	if (intf != NULL)
		*static_cast<zx8302_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&out_ipl1l_cb, 0, sizeof(out_ipl1l_cb));
		memset(&out_baudx4_cb, 0, sizeof(out_baudx4_cb));
		memset(&out_comdata_cb, 0, sizeof(out_comdata_cb));
		memset(&out_txd1_cb, 0, sizeof(out_txd1_cb));
		memset(&out_txd2_cb, 0, sizeof(out_txd2_cb));
		memset(&in_dtr1_cb, 0, sizeof(in_dtr1_cb));
		memset(&in_cts2_cb, 0, sizeof(in_cts2_cb));
		memset(&out_netout_cb, 0, sizeof(out_netout_cb));
		memset(&in_netin_cb, 0, sizeof(in_netin_cb));
		memset(&out_mdselck_cb, 0, sizeof(out_mdselck_cb));
		memset(&out_mdseld_cb, 0, sizeof(out_mdseld_cb));
		memset(&out_mdrdw_cb, 0, sizeof(out_mdrdw_cb));
		memset(&out_erase_cb, 0, sizeof(out_erase_cb));
		memset(&out_raw1_cb, 0, sizeof(out_raw1_cb));
		memset(&in_raw1_cb, 0, sizeof(in_raw1_cb));
		memset(&out_raw2_cb, 0, sizeof(out_raw2_cb));
		memset(&in_raw2_cb, 0, sizeof(in_raw2_cb));
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------

inline void zx8302_device::trigger_interrupt(UINT8 line)
{
	m_irq |= line;

	m_out_ipl1l_func(ASSERT_LINE);
}


//-------------------------------------------------
//  transmit_ipc_data - transmit IPC data
//-------------------------------------------------

inline void zx8302_device::transmit_ipc_data()
{
	/*

        IPC <-> ZX8302 serial link protocol
        ***********************************

        Send bit to IPC
        ---------------

        1. ZX start bit (COMDATA = 0)
        2. IPC clock (COMCTL = 0, COMTL = 1)
        3. ZX data bit (COMDATA = 0/1)
        4. IPC clock (COMCTL = 0, COMTL = 1)
        5. ZX stop bit (COMDATA = 1)

        Receive bit from IPC
        --------------------

        1. ZX start bit (COMDATA = 0)
        2. IPC clock (COMCTL = 0, COMTL = 1)
        3. IPC data bit (COMDATA = 0/1)
        4. IPC clock (COMCTL = 0, COMTL = 1)
        5. IPC stop bit (COMDATA = 1)

    */

	switch (m_ipc_state)
	{
	case IPC_START:
		if (LOG) logerror("ZX8302 '%s' COMDATA Start Bit\n", tag());

		m_out_comdata_func(0);
		m_ipc_busy = 1;
		m_ipc_state = IPC_DATA;
		break;

	case IPC_DATA:
		if (LOG) logerror("ZX8302 '%s' COMDATA Data Bit: %x\n", tag(), BIT(m_idr, 1));

		m_comdata = BIT(m_idr, 1);
		m_out_comdata_func(m_comdata);
		m_ipc_state = IPC_STOP;
		break;

	case IPC_STOP:
		if (!m_ipc_rx)
		{
			if (LOG) logerror("ZX8302 '%s' COMDATA Stop Bit\n", tag());

			m_out_comdata_func(1);
			m_ipc_busy = 0;
			m_ipc_state = IPC_START;
		}
		break;
	}
}


//-------------------------------------------------
//  transmit_bit - transmit serial bit
//-------------------------------------------------

inline void zx8302_device::transmit_bit(int state)
{
	switch (m_tcr & MODE_MASK)
	{
	case MODE_SER1:
		m_out_txd1_func(state);
		break;

	case MODE_SER2:
		m_out_txd2_func(state);
		break;

	case MODE_MDV:
		// TODO
		break;

	case MODE_NET:
		m_out_netout_func(state);
		break;
	}
}


//-------------------------------------------------
//  transmit_data - transmit serial data
//-------------------------------------------------

inline void zx8302_device::transmit_serial_data()
{
	switch (m_tx_bits)
	{
	case TXD_START:
		if (!(m_irq & INT_TRANSMIT))
		{
			transmit_bit(0);
			m_tx_bits++;
		}
		break;

	default:
		transmit_bit(BIT(m_tdr, 0));
		m_tdr >>= 1;
		m_tx_bits++;
		break;

	case TXD_STOP:
		transmit_bit(1);
		m_tx_bits++;
		break;

	case TXD_STOP2:
		transmit_bit(1);
		m_tx_bits = TXD_START;
		m_status &= ~STATUS_TX_BUFFER_FULL;
		trigger_interrupt(INT_TRANSMIT);
		break;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zx8302_device - constructor
//-------------------------------------------------
zx8302_device::zx8302_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, ZX8302, "Sinclair ZX8302", tag, owner, clock),
	  m_idr(1),
	  m_irq(0),
	  m_ctr(time(NULL) + RTC_BASE_ADJUST),
	  m_status(0),
	  m_comdata(1),
	  m_comctl(1),
	  m_ipc_state(0),
	  m_ipc_rx(0),
	  m_ipc_busy(0),
	  m_track(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zx8302_device::device_start()
{
	// resolve callbacks
	m_out_ipl1l_func.resolve(out_ipl1l_cb, *this);
	m_out_baudx4_func.resolve(out_baudx4_cb, *this);
	m_out_comdata_func.resolve(out_comdata_cb, *this);
	m_out_txd1_func.resolve(out_txd1_cb, *this);
	m_out_txd2_func.resolve(out_txd2_cb, *this);
	m_in_dtr1_func.resolve(in_dtr1_cb, *this);
	m_in_cts2_func.resolve(in_cts2_cb, *this);
	m_out_netout_func.resolve(out_netout_cb, *this);
	m_in_netin_func.resolve(in_netin_cb, *this);
	m_out_mdselck_func.resolve(out_mdselck_cb, *this);
	m_out_mdseld_func.resolve(out_mdseld_cb, *this);
	m_out_mdrdw_func.resolve(out_mdrdw_cb, *this);
	m_out_erase_func.resolve(out_erase_cb, *this);
	m_out_raw1_func.resolve(out_raw1_cb, *this);
	m_in_raw1_func.resolve(in_raw1_cb, *this);
	m_out_raw2_func.resolve(out_raw2_cb, *this);
	m_in_raw2_func.resolve(in_raw2_cb, *this);

	// allocate timers
	m_txd_timer = timer_alloc(TIMER_TXD);
	m_baudx4_timer = timer_alloc(TIMER_BAUDX4);
	m_rtc_timer = timer_alloc(TIMER_RTC);
	m_gap_timer = timer_alloc(TIMER_GAP);
	m_ipc_timer = timer_alloc(TIMER_IPC);

	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_hz(rtc_clock / 32768));
	m_gap_timer->adjust(attotime::zero, 0, attotime::from_msec(31));

	// register for state saving
	save_item(NAME(m_idr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_irq));
	save_item(NAME(m_ctr));
	save_item(NAME(m_status));
	save_item(NAME(m_comdata));
	save_item(NAME(m_comctl));
	save_item(NAME(m_ipc_state));
	save_item(NAME(m_ipc_rx));
	save_item(NAME(m_ipc_busy));
	save_item(NAME(m_baudx4));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_mdv_data));
	save_item(NAME(m_track));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void zx8302_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_TXD:
		transmit_serial_data();
		break;

	case TIMER_BAUDX4:
		m_baudx4 = !m_baudx4;
		m_out_baudx4_func(m_baudx4);
		break;

	case TIMER_RTC:
		m_ctr++;
		break;

	case TIMER_GAP:
		trigger_interrupt(INT_GAP);
		break;

	case TIMER_IPC:
		m_idr = param;
		m_ipc_state = IPC_START;
		m_ipc_rx = 0;

		transmit_ipc_data();
		break;
	}
}


//-------------------------------------------------
//  rtc_r - real time clock read
//-------------------------------------------------

READ8_MEMBER( zx8302_device::rtc_r )
{
	UINT8 data = 0;

	switch (offset)
	{
	case 0:
		data = (m_ctr >> 24) & 0xff;
		break;
	case 1:
		data = (m_ctr >> 16) & 0xff;
		break;
	case 2:
		data = (m_ctr >> 8) & 0xff;
		break;
	case 3:
		data = m_ctr & 0xff;
		break;
	}

	return data;
}


//-------------------------------------------------
//  rtc_w - real time clock write
//-------------------------------------------------

WRITE8_MEMBER( zx8302_device::rtc_w )
{
	if (LOG) logerror("ZX8302 '%s' Set Real Time Clock: %02x\n", tag(), data);
}


//-------------------------------------------------
//  control_w - serial transmit clock
//-------------------------------------------------

WRITE8_MEMBER( zx8302_device::control_w )
{
	if (LOG) logerror("ZX8302 '%s' Transmit Control: %02x\n", tag(), data);

	int baud = (19200 >> (data & BAUD_MASK));
	int baudx4 = baud * 4;

	m_tcr = data;

	m_txd_timer->adjust(attotime::zero, 0, attotime::from_hz(baud));
	m_baudx4_timer->adjust(attotime::zero, 0, attotime::from_hz(baudx4));
}


//-------------------------------------------------
//  mdv_track_r - microdrive track data
//-------------------------------------------------

READ8_MEMBER( zx8302_device::mdv_track_r )
{
	if (LOG) logerror("ZX8302 '%s' Microdrive Track %u: %02x\n", tag(), m_track, m_mdv_data[m_track]);

	UINT8 data = m_mdv_data[m_track];

	m_track = !m_track;

	return data;
}


//-------------------------------------------------
//  status_r - status register
//-------------------------------------------------

READ8_MEMBER( zx8302_device::status_r )
{
	/*

        bit     description

        0       Network port
        1       Transmit buffer full
        2       Receive buffer full
        3       Microdrive GAP
        4       SER1 DTR
        5       SER2 CTS
        6       IPC busy
        7       COMDATA

    */

	UINT8 data = 0;

	// TODO network port

	// serial status
	data |= m_status;

	// TODO microdrive GAP

	// data terminal ready
	data |= m_in_dtr1_func() << 4;

	// clear to send
	data |= m_in_cts2_func() << 5;

	// IPC busy
	data |= m_ipc_busy << 6;

	// COMDATA
	data |= m_comdata << 7;

	if (LOG) logerror("ZX8302 '%s' Status: %02x\n", tag(), data);

	return data;
}


//-------------------------------------------------
//  ipc_command_w - IPC command
//-------------------------------------------------

WRITE8_MEMBER( zx8302_device::ipc_command_w )
{
	if (LOG) logerror("ZX8302 '%s' IPC Command: %02x\n", tag(), data);

	if (data != 0x01)
	{
		m_ipc_timer->adjust(attotime::from_nsec(480), data);
	}
}


//-------------------------------------------------
//  mdv_control_w - microdrive control
//-------------------------------------------------

WRITE8_MEMBER( zx8302_device::mdv_control_w )
{
	/*

        bit     description

        0       MDSELDH
        1       MDSELCKH
        2       MDRDWL
        3       ERASE
        4
        5
        6
        7

    */

	if (LOG) logerror("ZX8302 '%s' Microdrive Control: %02x\n", tag(), data);

	m_out_mdseld_func(BIT(data, 0));
	m_out_mdselck_func(BIT(data, 1));
	m_out_mdrdw_func(BIT(data, 2));
	m_out_erase_func(BIT(data, 3));

	if (BIT(data, 1))
	{
		m_status &= ~STATUS_RX_BUFFER_FULL;
	}
}


//-------------------------------------------------
//  irq_status_r - interrupt status
//-------------------------------------------------

READ8_MEMBER( zx8302_device::irq_status_r )
{
	if (LOG) logerror("ZX8302 '%s' Interrupt Status: %02x\n", tag(), m_irq);

	return m_irq;
}


//-------------------------------------------------
//  irq_acknowledge_w - interrupt acknowledge
//-------------------------------------------------

WRITE8_MEMBER( zx8302_device::irq_acknowledge_w )
{
	if (LOG) logerror("ZX8302 '%s' Interrupt Acknowledge: %02x\n", tag(), data);

	m_irq &= ~data;

	if (!m_irq)
	{
		m_out_ipl1l_func(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  data_w - transmit buffer
//-------------------------------------------------

WRITE8_MEMBER( zx8302_device::data_w )
{
	if (LOG) logerror("ZX8302 '%s' Data Register: %02x\n", tag(), data);

	m_tdr = data;
	m_status |= STATUS_TX_BUFFER_FULL;
}


//-------------------------------------------------
//  vsync_w - vertical sync
//-------------------------------------------------

WRITE_LINE_MEMBER( zx8302_device::vsync_w )
{
	if (state)
	{
		if (LOG) logerror("ZX8302 '%s' Frame Interrupt\n", tag());

		trigger_interrupt(INT_FRAME);
	}
}


//-------------------------------------------------
//  comctl_w - IPC COMCTL
//-------------------------------------------------

WRITE_LINE_MEMBER( zx8302_device::comctl_w )
{
	if (LOG) logerror("ZX8302 '%s' COMCTL: %x\n", tag(), state);

	if (state)
	{
		transmit_ipc_data();
	}
}


//-------------------------------------------------
//  comdata_w - IPC COMDATA
//-------------------------------------------------

WRITE_LINE_MEMBER( zx8302_device::comdata_w )
{
	if (LOG) logerror("ZX8302 '%s' COMDATA: %x\n", tag(), state);

	if (m_ipc_state == IPC_DATA || m_ipc_state == IPC_STOP)
	{
		if (m_ipc_rx)
		{
			m_ipc_rx = 0;
			m_ipc_busy = 0;
			m_ipc_state = IPC_START;
		}
		else
		{
			m_ipc_rx = 1;
			m_comdata = state;
		}
	}
}


//-------------------------------------------------
//  extint_w - external interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( zx8302_device::extint_w )
{
	if (LOG) logerror("ZX8302 '%s' EXTINT: %x\n", tag(), state);

	if (state == ASSERT_LINE)
	{
		trigger_interrupt(INT_EXTERNAL);
	}
}
