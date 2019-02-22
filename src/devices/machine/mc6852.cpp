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

#include "emu.h"
#include "mc6852.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MC6852, mc6852_device, "mc6852", "Motorola MC6852 SSDA")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mc6852_device - constructor
//-------------------------------------------------

mc6852_device::mc6852_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MC6852, tag, owner, clock),
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

	set_rcv_rate(m_rx_clock);
	set_tra_rate(m_tx_clock);

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
	m_rx_fifo = std::queue<uint8_t>();
	m_tx_fifo = std::queue<uint8_t>();

	receive_register_reset();
	transmit_register_reset();

	/* reset and inhibit receiver/transmitter sections */
	m_cr[0] |= (C1_TX_RS | C1_RX_RS);
	m_cr[1] &= ~(C2_EIE | C2_PC2 | C2_PC1);
	m_status &= ~S_TDRA;

	/* set receiver shift register to all 1's */
	m_rsr = 0xff;
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

uint8_t mc6852_device::read(offs_t offset)
{
	uint8_t data = 0;

	if (BIT(offset, 0))
	{
		if (m_rx_fifo.size() > 0)
		{
			data = m_rx_fifo.front();
			if (!machine().side_effects_disabled())
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

void mc6852_device::write(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
	{
		switch (m_cr[0] & C1_AC_MASK)
		{
		case C1_AC_C2: {
			/* control 2 */
			LOG("MC6852 Control 2 %02x\n", data);
			m_cr[1] = data;

			int data_bit_count = 0;
			parity_t parity = PARITY_NONE;
			stop_bits_t stop_bits = STOP_BITS_1;

			switch (data & C2_WS_MASK)
			{
			case 0: data_bit_count = 6; parity = PARITY_EVEN; break;
			case 1: data_bit_count = 6; parity = PARITY_ODD; break;
			case 2: data_bit_count = 7; parity = PARITY_NONE; break;
			case 3: data_bit_count = 8; parity = PARITY_NONE; break;
			case 4: data_bit_count = 7; parity = PARITY_EVEN; break;
			case 5: data_bit_count = 7; parity = PARITY_ODD; break;
			case 6: data_bit_count = 8; parity = PARITY_EVEN; break;
			case 7: data_bit_count = 8; parity = PARITY_ODD; break;
			}

			set_data_frame(1, data_bit_count, parity, stop_bits);
			}
			break;

		case C1_AC_C3:
			/* control 3 */
			LOG("MC6852 Control 3 %02x\n", data);
			m_cr[2] = data;
			break;

		case C1_AC_SYNC:
			/* sync code */
			LOG("MC6852 Sync Code %02x\n", data);
			m_scr = data;
			break;

		case C1_AC_TX_FIFO:
			/* transmit data FIFO */
			if (m_tx_fifo.size() < 3)
			{
				LOG("MC6852 Transmit FIFO %02x\n", data);
				m_tx_fifo.push(data);
			}
			break;
		}
	}
	else
	{
		LOG("MC6852 Control 1 %02x\n", data);

		/* receiver reset */
		if (data & C1_RX_RS)
		{
			/* When Rx Rs is set, it clears the receiver
			control logic, sync logic, error logic, Rx Data FIFO Control,
			Parity Error status bit, and DCD interrupt. The Receiver Shift
			Register is set to ones.
			*/

			LOG("MC6852 Receiver Reset\n");

			m_status &= ~(S_RX_OVRN | S_PE | S_DCD | S_RDA);
			m_rsr = 0xff;

			receive_register_reset();
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

			LOG("MC6852 Transmitter Reset\n");

			m_status &= ~(S_TUF | S_CTS | S_TDRA);

			transmit_register_reset();
		}

		if (data & C1_STRIP_SYNC) LOG("MC6852 Strip Synchronization Characters\n");
		if (data & C1_CLEAR_SYNC) LOG("MC6852 Clear Synchronization\n");

		m_cr[0] = data;
	}
}
