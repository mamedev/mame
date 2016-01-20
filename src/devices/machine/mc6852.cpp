// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MC6852 Synchronous Serial Data Adapter emulation

**********************************************************************/

/*

    TODO:

    - FIFO
    - receive
    - transmit
    - parity
    - 1-sync-character mode
    - 2-sync-character mode
    - external sync mode
    - interrupts

*/

#include "mc6852.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MC6852 = &device_creator<mc6852_device>;



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define S_RDA           0x01
#define S_TDRA          0x02
#define S_DCD           0x04
#define S_CTS           0x08
#define S_TUF           0x10
#define S_RX_OVRN       0x20
#define S_PE            0x40
#define S_IRQ           0x80


#define C1_RX_RS        0x01
#define C1_TX_RS        0x02
#define C1_STRIP_SYNC   0x04
#define C1_CLEAR_SYNC   0x08
#define C1_TIE          0x10
#define C1_RIE          0x20
#define C1_AC_MASK      0xc0
#define C1_AC_C2        0x00
#define C1_AC_C3        0x40
#define C1_AC_SYNC      0x80
#define C1_AC_TX_FIFO   0xc0


#define C2_PC1          0x01
#define C2_PC2          0x02
#define C2_1_2_BYTE     0x04
#define C2_WS_MASK      0x38
#define C2_WS_6_E       0x00
#define C2_WS_6_O       0x08
#define C2_WS_7         0x10
#define C2_WS_8         0x18
#define C2_WS_7_E       0x20
#define C2_WS_7_O       0x28
#define C2_WS_8_E       0x30
#define C2_WS_8_O       0x38
#define C2_TX_SYNC      0x40
#define C2_EIE          0x80


#define C3_E_I_SYNC     0x01
#define C3_1_2_SYNC     0x02
#define C3_CLEAR_CTS    0x04
#define C3_CTUF         0x08



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc6852_device - constructor
//-------------------------------------------------

mc6852_device::mc6852_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MC6852, "MC6852", tag, owner, clock, "mc6852", __FILE__),
	device_serial_interface(mconfig, *this),
	m_write_tx_data(*this),
	m_write_irq(*this),
	m_write_sm_dtr(*this),
	m_write_tuf(*this),
	m_rx_clock(0),
	m_tx_clock(0),
	m_cts(1),
	m_dcd(1),
	m_sm_dtr(0),
	m_tuf(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6852_device::device_start()
{
	// resolve callbacks
	m_write_tx_data.resolve_safe();
	m_write_irq.resolve_safe();
	m_write_sm_dtr.resolve_safe();
	m_write_tuf.resolve_safe();

	// register for state saving
	save_item(NAME(m_status));
	save_item(NAME(m_cr));
	save_item(NAME(m_scr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_rdr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_sm_dtr));
	save_item(NAME(m_tuf));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mc6852_device::device_reset()
{
	m_rx_fifo = std::queue<UINT8>();
	m_tx_fifo = std::queue<UINT8>();

	transmit_register_reset();
	receive_register_reset();

	set_rcv_rate(m_rx_clock);
	set_tra_rate(m_tx_clock);

	/* set receiver shift register to all 1's */
	m_rsr = 0xff;

	/* reset and inhibit receiver/transmitter sections */
	m_cr[0] |= (C1_TX_RS | C1_RX_RS);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void mc6852_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void mc6852_device::tra_callback()
{
	m_write_tx_data(transmit_register_get_data_bit());
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void mc6852_device::tra_complete()
{
	// TODO
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void mc6852_device::rcv_complete()
{
	// TODO
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mc6852_device::read )
{
	UINT8 data = 0;

	if (BIT(offset, 0))
	{
		if (m_rx_fifo.size() > 0)
		{
			data = m_rx_fifo.front();
			m_rx_fifo.pop();
		}
	}
	else
	{
		data = m_status;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mc6852_device::write )
{
	if (BIT(offset, 0))
	{
		switch (m_cr[0] & C1_AC_MASK)
		{
		case C1_AC_C2:
			/* control 2 */
			m_cr[1] = data;
			break;

		case C1_AC_C3:
			/* control 3 */
			m_cr[2] = data;
			break;

		case C1_AC_SYNC:
			/* sync code */
			m_scr = data;
			break;

		case C1_AC_TX_FIFO:
			/* transmit data FIFO */
			if (m_tx_fifo.size() < 3)
			{
				m_tx_fifo.push(data);
			}
			break;
		}
	}
	else
	{
		/* receiver reset */
		if (data & C1_RX_RS)
		{
			/* When Rx Rs is set, it clears the receiver
			control logic, sync logic, error logic, Rx Data FIFO Control,
			Parity Error status bit, and DCD interrupt. The Receiver Shift
			Register is set to ones.
			*/

			if (LOG) logerror("MC6852 '%s' Receiver Reset\n", tag().c_str());

			m_status &= ~(S_RX_OVRN | S_PE | S_DCD | S_RDA);
			m_rsr = 0xff;
		}

		/* transmitter reset */
		if (data & C1_TX_RS)
		{
			/* When Tx Rs is set, it clears the transmitter
			control section, Transmitter Shift Register, Tx Data FIFO
			Control (the Tx Data FIFO can be reloaded after one E clock
			pulse), the Transmitter Underflow status bit, and the CTS interrupt,
			and inhibits the TDRA status bit (in the one-sync-character
			and two-sync-character modes).*/

			if (LOG) logerror("MC6852 '%s' Transmitter Reset\n", tag().c_str());

			m_status &= ~(S_TUF | S_CTS | S_TDRA);
		}

		if (LOG)
		{
			if (data & C1_STRIP_SYNC) logerror("MC6852 '%s' Strip Synchronization Characters\n", tag().c_str());
			if (data & C1_CLEAR_SYNC) logerror("MC6852 '%s' Clear Synchronization\n", tag().c_str());
		}

		m_cr[0] = data;
	}
}
