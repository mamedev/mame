// license:BSD-3-Clause
// copyright-holders:Curt Coder, Sylvain Glaize
/**********************************************************************

    Sinclair ZX8302 emulation

**********************************************************************/

/*

    TODO:

    - set time
    - DTR/CTS handling
    - network

*/

#include "emu.h"
#include "zx8302.h"

#include <ctime>



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG_MDV (1U << 1)   // microdrive path
#define LOG_IPC (1U << 2)   // IPC link per-bit (very high frequency)
#define LOG_IRQ (1U << 3)   // Interrupts

// #define VERBOSE (LOG_GENERAL | LOG_MDV)
#include "logmacro.h"


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
		LOGMASKED(LOG_IPC, "COMDATA Start Bit\n");

		m_out_comdata_cb(BIT(m_idr, 0));
		m_ipc_busy = 1;
		m_ipc_state = IPC_DATA;
		break;

	case IPC_DATA:
		LOGMASKED(LOG_IPC, "COMDATA Data Bit: %x\n", BIT(m_idr, 1));

		m_comdata_to_ipc = BIT(m_idr, 1);
		m_out_comdata_cb(m_comdata_to_ipc);
		m_ipc_state = IPC_STOP;
		break;

	case IPC_STOP:
		LOGMASKED(LOG_IPC, "COMDATA Stop Bit\n");

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
		m_dtr1(0),
		m_cts2(0),
		m_idr(1),
		m_irq(0),
		m_irq_mask(0),
		m_ctr(time(nullptr) + RTC_BASE_ADJUST),
		m_status(STATUS_MICRODRIVE_GAP),
		m_comdata_from_ipc(1),
		m_comdata_to_cpu(1),
		m_comdata_to_ipc(1),
		m_comctl(1),
		m_ipc_state(0),
		m_ipc_busy(0),
		m_baudx4(0),
		m_mdv_shift{0, 0},
		m_mdv_bit_count(0),
		m_mdv_sync_state(MDV_SYNC_IDLE),
		m_mdv_tx_buffer{0, 0},
		m_mdv_tx_count(0)
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

	m_rtc_timer->adjust(attotime::zero, 0, attotime::from_hz(m_rtc_clock / 32768));

	// register for state saving
	save_item(NAME(m_dtr1));
	save_item(NAME(m_cts2));
	save_item(NAME(m_idr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_irq));
	save_item(NAME(m_irq_mask));
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
	save_item(NAME(m_mdv_shift));
	save_item(NAME(m_mdv_bit_count));
	save_item(NAME(m_mdv_sync_state));
	save_item(NAME(m_mdv_tx_buffer));
	save_item(NAME(m_mdv_tx_count));
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
	LOG("Set Real Time Clock: %02x\n", data);
}


//-------------------------------------------------
//  control_w - serial transmit clock
//-------------------------------------------------

void zx8302_device::control_w(uint8_t data)
{
	LOG("Transmit Control: %02x\n", data);

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

uint8_t zx8302_device::mdv_track_r(offs_t offset)
{
	const int track = offset & 1;
	uint8_t data = m_mdv_data[track];

	LOGMASKED(LOG_MDV, "MDV Track %u: %02x (rxfull=%d)\n",
		track + 1, data, (m_status & STATUS_RX_BUFFER_FULL) ? 1 : 0);

	if (!machine().side_effects_disabled())
	{
		if (track == 1)
		{
			// reading track 2 (pc_trak2) releases the buffer for the next byte pair
			m_status &= ~STATUS_RX_BUFFER_FULL;
		}
	}

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

	// serial + MDV status
	data |= m_status;

	// data terminal ready
	data |= m_dtr1 << 4;

	// clear to send
	data |= m_cts2 << 5;

	// IPC busy
	data |= m_ipc_busy << 6;

	// COMDATA
	data |= m_comdata_to_cpu << 7;

	LOGMASKED(LOG_IPC, "Read Status: %02x\n", data);

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
	LOGMASKED(LOG_IPC, "IPC Command: %02x\n", data);

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

	LOGMASKED(LOG_MDV, "Microdrive Control: %02x\n", data);

	m_out_mdseld_cb(BIT(data, 0));
	m_out_mdselck_cb(BIT(data, 1));
	m_out_mdrdw_cb(BIT(data, 2));
	m_out_erase_cb(BIT(data, 3));

	if (BIT(data, 1))
	{
		// MDSELCK: clears RX buffer and arms preamble sync search.
		// The ZX8302 hardware then waits for 0xFF/0xFF in the bit stream
		// before delivering bytes to the CPU.
		m_status &= ~STATUS_RX_BUFFER_FULL;
		m_mdv_sync_state = MDV_SYNC_SEARCH;
		m_mdv_bit_count = 0;
		m_mdv_shift[0] = 0;
		m_mdv_shift[1] = 0;
		LOGMASKED(LOG_MDV, "MDV sync armed (MDSELCK)\n");
	}

	if (!BIT(data, 2))
	{
		// write line dropped: drop current data
		m_mdv_tx_count = 0;
		m_status &= ~STATUS_TX_BUFFER_FULL;
	}
}


//-------------------------------------------------
//  mdv_tx_pop
//
//  gets the current written pair (or 0 if not entirely pushed yet)
//  also resets the buffer
//-------------------------------------------------

uint16_t zx8302_device::mdv_tx_pop()
{
	uint16_t pair = 0;

	if (m_mdv_tx_count == 2)
	{
		pair = (m_mdv_tx_buffer[0] << 8) | m_mdv_tx_buffer[1];
		m_mdv_tx_count = 0;
		m_status &= ~STATUS_TX_BUFFER_FULL;
	}

	return pair;
}


//-------------------------------------------------
//  irq_status_r - interrupt status
//-------------------------------------------------

uint8_t zx8302_device::irq_status_r()
{
	LOGMASKED(LOG_IRQ, "Interrupt Status: %02x\n", m_irq);

	return m_irq;
}


//-------------------------------------------------
//  irq_acknowledge_w - interrupt acknowledge
//-------------------------------------------------

void zx8302_device::irq_acknowledge_w(uint8_t data)
{
	LOGMASKED(LOG_IRQ, "Interrupt Acknowledge: %02x\n", data);

	// bits 7-5 are the interrupt mask, bits 4-0 clear pending interrupts
	m_irq_mask = data & 0xe0;
	m_irq &= ~(data & 0x1f);

	if (!m_irq)
	{
		m_out_ipl1l_cb(CLEAR_LINE);
	}

	// The gap interrupt regenerates as long as the gap level is present and
	// the mask is enabled. Minerva relies on this.
	if ((m_irq_mask & MASK_GAP) && (m_status & STATUS_MICRODRIVE_GAP))
	{
		trigger_interrupt(INT_GAP);
	}
}


//-------------------------------------------------
//  data_w - transmit buffer
//-------------------------------------------------

void zx8302_device::data_w(uint8_t data)
{
	LOGMASKED(LOG_MDV, "Data Register: %02x\n", data);

	if ((m_tcr & MODE_MASK) == MODE_MDV)
	{
		// fills the write buffer for the microdrive
		if (m_mdv_tx_count < 2)
		{
			m_mdv_tx_buffer[m_mdv_tx_count++] = data;
		}
		else
		{
			logerror("MDV TX overflow, byte %02x lost\n", data);
		}

		if (m_mdv_tx_count == 2)
		{
			m_status |= STATUS_TX_BUFFER_FULL;
		}

		return;
	}

	// previous code for other modes than microdrive
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
		LOGMASKED(LOG_IRQ, "Frame Interrupt\n");

		trigger_interrupt(INT_FRAME);
	}
}


//-------------------------------------------------
//  comctl_w - IPC COMCTL
//-------------------------------------------------

void zx8302_device::comctl_w(int state)
{
	LOGMASKED(LOG_IPC, "COMCTL: %x\n", state);

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
	LOGMASKED(LOG_IPC, "COMDATA->CPU(pending): %x\n", state);

	m_comdata_from_ipc = state;
}


//-------------------------------------------------
//  extint_w - external interrupt
//-------------------------------------------------

void zx8302_device::extint_w(int state)
{
	LOGMASKED(LOG_IRQ, "EXTINT: %x\n", state);

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


//-------------------------------------------------
//  raw1_w - receive bit from microdrive track 1
//-------------------------------------------------

void zx8302_device::raw1_w(int state)
{
	m_mdv_shift[0] = (m_mdv_shift[0] << 1) | (state & 1);
}


//-------------------------------------------------
//  raw2_w - receive bit from microdrive track 2.
//-------------------------------------------------

void zx8302_device::raw2_w(int state)
{
	m_mdv_shift[1] = (m_mdv_shift[1] << 1) | (state & 1);

	switch (m_mdv_sync_state)
	{
	case MDV_SYNC_IDLE:
		return;

	case MDV_SYNC_SEARCH:
		// The preamble is consumed, never delivered to the CPU.
		if (m_mdv_shift[0] == 0xFF && m_mdv_shift[1] == 0xFF)
		{
			m_mdv_sync_state = MDV_SYNC_DELIVER;
			m_mdv_bit_count = 0;
			LOGMASKED(LOG_MDV, "MDV preamble sync found\n");
		}
		return;

	case MDV_SYNC_DELIVER:
		m_mdv_bit_count++;
		if (m_mdv_bit_count < 8)
			return;

		m_mdv_bit_count = 0;
		m_mdv_data[0] = m_mdv_shift[0];
		m_mdv_data[1] = m_mdv_shift[1];
		m_status |= STATUS_RX_BUFFER_FULL;

		LOGMASKED(LOG_MDV, "MDV byte: t1=%02x t2=%02x\n", m_mdv_data[0], m_mdv_data[1]);
	}
}


//-------------------------------------------------
//  gap_w - gap signal from microdrive
//-------------------------------------------------

void zx8302_device::gap_w(int state)
{
	if (state)
	{
		LOGMASKED(LOG_MDV, "MDV gap HIGH\n");
		m_status |= STATUS_MICRODRIVE_GAP;
		if (m_irq_mask & MASK_GAP)
			trigger_interrupt(INT_GAP);
	}
	else
	{
		LOGMASKED(LOG_MDV, "MDV gap LOW\n");
		m_status &= ~STATUS_MICRODRIVE_GAP;
	}
}
