// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair ZX8302 emulation

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
#include "zx8302.h"

#include "imagedev/microdrv.h"

#include <ctime>



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


// Monday 1st January 1979 00:00:00 UTC
static const int RTC_BASE_ADJUST = 283996800;



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ZX8302, zx8302_device, "zx8302", "Sinclair ZX8302")

//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------

inline void zx8302_device::trigger_interrupt(uint8_t line)
{
	m_irq |= line;

	m_out_ipl1l_cb(ASSERT_LINE);
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

		m_out_comdata_cb(BIT(m_idr, 0));
		m_ipc_busy = 1;
		m_ipc_state = IPC_DATA;
		break;

	case IPC_DATA:
		if (LOG) logerror("ZX8302 '%s' COMDATA Data Bit: %x\n", tag(), BIT(m_idr, 1));

		m_comdata_to_ipc = BIT(m_idr, 1);
		m_out_comdata_cb(m_comdata_to_ipc);
		m_ipc_state = IPC_STOP;
		break;

	case IPC_STOP:
		if (LOG) logerror("ZX8302 '%s' COMDATA Stop Bit\n", tag());

		m_out_comdata_cb(BIT(m_idr, 2));
		m_ipc_busy = 0;
		break;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  zx8302_device - constructor
//-------------------------------------------------

zx8302_device::zx8302_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZX8302, tag, owner, clock),
		device_serial_interface(mconfig, *this),
		m_rtc_clock(0),
		m_out_ipl1l_cb(*this),
		m_out_baudx4_cb(*this),
		m_out_comdata_cb(*this),
		m_out_txd1_cb(*this),
		m_out_txd2_cb(*this),
		m_out_netout_cb(*this),
		m_out_mdselck_cb(*this),
		m_out_mdseld_cb(*this),
		m_out_mdrdw_cb(*this),
		m_out_erase_cb(*this),
		m_out_raw1_cb(*this),
		m_in_raw1_cb(*this, 0),
		m_out_raw2_cb(*this),
		m_in_raw2_cb(*this, 0),
		m_dtr1(0),
		m_cts2(0),
		m_idr(1),
		m_irq(0),
		m_ctr(time(nullptr) + RTC_BASE_ADJUST),
		m_status(0),
		m_comdata_from_ipc(1),
		m_comdata_to_cpu(1),
		m_comdata_to_ipc(1),
		m_comctl(1),
		m_ipc_state(0),
		m_ipc_busy(0),
		m_baudx4(0),
		m_track(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void zx8302_device::device_start()
{
	// allocate timers
	m_baudx4_timer = timer_alloc(FUNC(zx8302_device::baudx4_tick), this);
	m_rtc_timer = timer_alloc(FUNC(zx8302_device::rtc_tick), this);
	m_gap_timer = timer_alloc(FUNC(zx8302_device::trigger_gap_int), this);

	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_hz(m_rtc_clock / 32768));
	m_gap_timer->adjust(attotime::zero, 0, attotime::from_msec(31));

	// register for state saving
	save_item(NAME(m_dtr1));
	save_item(NAME(m_cts2));
	save_item(NAME(m_idr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_irq));
	save_item(NAME(m_ctr));
	save_item(NAME(m_status));
	save_item(NAME(m_comdata_from_ipc));
	save_item(NAME(m_comdata_to_cpu));
	save_item(NAME(m_comdata_to_ipc));
	save_item(NAME(m_comctl));
	save_item(NAME(m_ipc_state));
	save_item(NAME(m_ipc_busy));
	save_item(NAME(m_baudx4));
	save_item(NAME(m_mdv_data));
	save_item(NAME(m_track));
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(zx8302_device::baudx4_tick)
{
	m_baudx4 = !m_baudx4;
	m_out_baudx4_cb(m_baudx4);
}

TIMER_CALLBACK_MEMBER(zx8302_device::rtc_tick)
{
	m_ctr++;
}

TIMER_CALLBACK_MEMBER(zx8302_device::trigger_gap_int)
{
	trigger_interrupt(INT_GAP);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void zx8302_device::tra_callback()
{
	switch (m_tcr & MODE_MASK)
	{
	case MODE_SER1:
		m_out_txd1_cb(transmit_register_get_data_bit());
		break;

	case MODE_SER2:
		m_out_txd2_cb(transmit_register_get_data_bit());
		break;

	case MODE_MDV:
		// TODO
		break;

	case MODE_NET:
		m_out_netout_cb(transmit_register_get_data_bit());
		break;
	}
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void zx8302_device::tra_complete()
{
	m_status &= ~STATUS_TX_BUFFER_FULL;

	trigger_interrupt(INT_TRANSMIT);
}


//-------------------------------------------------
//  rcv_callback -
//-------------------------------------------------

void zx8302_device::rcv_callback()
{
	switch (m_tcr & MODE_MASK)
	{
	case MODE_NET:
		receive_register_update_bit(m_rs232_rx);
		break;
	}
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void zx8302_device::rcv_complete()
{
	// TODO
}


//-------------------------------------------------
//  rtc_r - real time clock read
//-------------------------------------------------

uint8_t zx8302_device::rtc_r(offs_t offset)
{
	uint8_t data = 0;

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

void zx8302_device::rtc_w(uint8_t data)
{
	if (LOG) logerror("ZX8302 '%s' Set Real Time Clock: %02x\n", tag(), data);
}


//-------------------------------------------------
//  control_w - serial transmit clock
//-------------------------------------------------

void zx8302_device::control_w(uint8_t data)
{
	if (LOG) logerror("ZX8302 '%s' Transmit Control: %02x\n", tag(), data);

	int baud = (19200 >> (data & BAUD_MASK));
	int baudx4 = baud * 4;

	m_tcr = data;

	m_baudx4_timer->adjust(attotime::zero, 0, attotime::from_hz(baudx4));

	set_tra_rate(baud);
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
}


//-------------------------------------------------
//  mdv_track_r - microdrive track data
//-------------------------------------------------

uint8_t zx8302_device::mdv_track_r()
{
	if (LOG) logerror("ZX8302 '%s' Microdrive Track %u: %02x\n", tag(), m_track, m_mdv_data[m_track]);

	uint8_t data = m_mdv_data[m_track];

	m_track = !m_track;

	return data;
}


//-------------------------------------------------
//  status_r - status register
//-------------------------------------------------

uint8_t zx8302_device::status_r()
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

	uint8_t data = 0;

	// TODO network port

	// serial status
	data |= m_status;

	// TODO microdrive GAP

	// data terminal ready
	data |= m_dtr1 << 4;

	// clear to send
	data |= m_cts2 << 5;

	// IPC busy
	data |= m_ipc_busy << 6;

	// COMDATA
	data |= m_comdata_to_cpu << 7;

	if (LOG) logerror("ZX8302 '%s' Status: %02x\n", tag(), data);

	return data;
}


//-------------------------------------------------
//  ipc_command_w - IPC command
//-------------------------------------------------
// When the main CPU writes to the IPC it writes the lsn of 18003
// this is encoded as follows :
//
// b0 start bit
// b1 data bit
// b2 stop bit
// b3 extra stop bit.
//
// At startup the IPC sits in a loop waiting for the comdata bit to go
// high, the main CPU does this by writing 0x01 to output register.

void zx8302_device::ipc_command_w(uint8_t data)
{
	if (LOG) logerror("ZX8302 '%s' IPC Command: %02x\n", tag(), data);

	m_idr = data;
	m_ipc_state = IPC_START;

	transmit_ipc_data();
}


//-------------------------------------------------
//  mdv_control_w - microdrive control
//-------------------------------------------------

void zx8302_device::mdv_control_w(uint8_t data)
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

	m_out_mdseld_cb(BIT(data, 0));
	m_out_mdselck_cb(BIT(data, 1));
	m_out_mdrdw_cb(BIT(data, 2));
	m_out_erase_cb(BIT(data, 3));

	if (BIT(data, 1))
	{
		m_status &= ~STATUS_RX_BUFFER_FULL;
	}
}


//-------------------------------------------------
//  irq_status_r - interrupt status
//-------------------------------------------------

uint8_t zx8302_device::irq_status_r()
{
	if (LOG) logerror("ZX8302 '%s' Interrupt Status: %02x\n", tag(), m_irq);

	return m_irq;
}


//-------------------------------------------------
//  irq_acknowledge_w - interrupt acknowledge
//-------------------------------------------------

void zx8302_device::irq_acknowledge_w(uint8_t data)
{
	if (LOG) logerror("ZX8302 '%s' Interrupt Acknowledge: %02x\n", tag(), data);

	m_irq &= ~data;

	if (!m_irq)
	{
		m_out_ipl1l_cb(CLEAR_LINE);
	}
}


//-------------------------------------------------
//  data_w - transmit buffer
//-------------------------------------------------

void zx8302_device::data_w(uint8_t data)
{
	if (LOG) logerror("ZX8302 '%s' Data Register: %02x\n", tag(), data);

	m_tdr = data;
	m_status |= STATUS_TX_BUFFER_FULL;
}


//-------------------------------------------------
//  vsync_w - vertical sync
//-------------------------------------------------

void zx8302_device::vsync_w(int state)
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

void zx8302_device::comctl_w(int state)
{
	if (LOG) logerror("ZX8302 '%s' COMCTL: %x\n", tag(), state);

	if (state)
	{
		transmit_ipc_data();
		m_comdata_to_cpu = m_comdata_from_ipc;
	}
}


//-------------------------------------------------
//  comdata_w - IPC COMDATA
//-------------------------------------------------
// IPC writing comdata to CPU
//

void zx8302_device::comdata_w(int state)
{
	if (LOG) logerror("ZX8302 '%s' COMDATA->CPU(pending): %x\n", tag(), state);

	m_comdata_from_ipc = state;
}


//-------------------------------------------------
//  extint_w - external interrupt
//-------------------------------------------------

void zx8302_device::extint_w(int state)
{
	if (LOG) logerror("ZX8302 '%s' EXTINT: %x\n", tag(), state);

	if (state == ASSERT_LINE)
	{
		trigger_interrupt(INT_EXTERNAL);
	}
}

void zx8302_device::write_netin(int state)
{
	m_rs232_rx = state;
	device_serial_interface::rx_w(state);
}

void zx8302_device::write_dtr1(int state)
{
	m_dtr1 = state;
}

void zx8302_device::write_cts2(int state)
{
	m_cts2 = state;
}
