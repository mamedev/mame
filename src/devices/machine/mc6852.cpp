// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MC6852 Synchronous Serial Data Adapter emulation

**********************************************************************/

/*

    TODO:

    - FIFO flags
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
	m_tx_pull_mode(false),
	m_tx_active(false),
	m_rx_clock(0),
	m_tx_clock(0),
	m_cts(1),
	m_dcd(1),
	m_sm_dtr(0),
	m_tuf(0),
	m_in_sync(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mc6852_device::device_start()
{
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
	save_item(NAME(m_in_sync));
	save_item(NAME(m_tx_active));
	save_item(NAME(m_tx_pull_mode));
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
	m_status |= S_TDRA;

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
	int trigger = (m_cr[1] & C2_1_2_BYTE) ? 1 : 2;
	int available = 3 - m_tx_fifo.size();
	uint8_t byte_to_send;

	if (available < 3)
	{
		// FIFO not empty - send the next byte.
		byte_to_send = m_tx_fifo.front();
		m_tx_fifo.pop();
		available++;
	}
	else
	{
		// TX underflow
		if (m_cr[1] & C2_TX_SYNC)
		{
			m_status |= S_TUF;
			byte_to_send = m_scr;   // Send Sync Code

			// TODO assert TUF pin for "approximately one Tx CLK high period"
		}
		else
		{
			byte_to_send = 0xff;    // Send a "Mark"
		}
	}

	transmit_register_setup(byte_to_send);

	if (available >= trigger)
	{
		m_status |= S_TDRA;
	}
}

//-------------------------------------------------
//  receive_byte -
//-------------------------------------------------
void mc6852_device::receive_byte(uint8_t data)
{
	// Ignore if the receiver is in reset or sync is not enabled
	if (m_cr[0] & (C1_RX_RS | C1_CLEAR_SYNC))
		return;

	// Handle sync detection
	if (!m_in_sync)
	{
		// TODO also handle two sync codes.
		if (data == m_scr)
		{
			m_in_sync = 1;
			// TODO handle the various SM responses
		}
		return;
	}

	if ((m_cr[0] & C1_STRIP_SYNC) && (data == m_scr))
		return;

	int size = m_rx_fifo.size();

	if (size < 3)
	{
		m_rx_fifo.push(data);
		size++;
	}
	else
	{
		// Overrun.
		// TODO this should override the last data pushed
		m_status |= S_RX_OVRN;
	}

	int trigger = (m_cr[1] & C2_1_2_BYTE) ? 1 : 2;

	if (size >= trigger)
	{
		m_status |= S_RDA;
	}
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

// TODO each RX fifo element needs an associated PE status flag, and reading
// the status register should return the PE for the last element of the fifo.

// TODO RX overrun should be cleared by reading the status register followed
// by reading the RX fifo.

uint8_t mc6852_device::read(offs_t offset)
{
	uint8_t data = 0;

	if (BIT(offset, 0))
	{
		int size = m_rx_fifo.size();
		if (size > 0)
		{
			data = m_rx_fifo.front();
			if (!machine().side_effects_disabled())
			{
				m_rx_fifo.pop();
				int trigger = (m_cr[1] & C2_1_2_BYTE) ? 1 : 2;
				if (size <= trigger)
				{
					m_status &= ~S_RDA;
				}
			}
		}
	}
	else
	{
		data = m_status;

		// TS reset inhibits the TDRA status bit (in the
		// one-sync-character and two-sync-character modes) The
		// m_status S_TDRA bit is allowed to reflect the actual fifo
		// availability, masking it here on a read of the status, so
		// that the TDRA status bit is simply unmasked here when the
		// TX is taken out of reset.
		if (m_cr[0] & C1_TX_RS)
		{
			data &= ~S_TDRA;
		}

		if (!machine().side_effects_disabled())
		{
			// TODO this might not be quite right, the datasheet
			// states that the RX overrun flag is cleared by
			// reading the status, and the RX data fifo?
			m_status &= ~S_RX_OVRN;
		}
	}

	return data;
}

//-------------------------------------------------
//  tx_start -
//-------------------------------------------------

// The corresponds in time to just before the first bit of the next word is
// transmitted by this device. At this time the TX shift register is loaded
// the TUF line may be asserted if there is a TX FIFO underflow.
uint8_t mc6852_device::get_tx_byte(int *tuf)
{
	if (m_cr[0] & C1_TX_RS)
	{
		// FIFO is not popped when the TX is reset, but may be loaded
		// so that it is pre-loaded when the reset is cleared.  But
		// will is send a sync code if that is enabled, of just ones?
		*tuf = 0;
		return 0xff;
	}

	int size = m_tx_fifo.size();

	if (size == 0)
	{
		// TX underflow
		if (m_cr[1] & C2_TX_SYNC)
		{
			m_status |= S_TUF;
			// TODO should the TUF callback be called, TUF is to
			// be pulsed.
			*tuf = 1;
			return m_scr;
		}

		*tuf = 0;
		return 0xff;
	}

	uint8_t data = m_tx_fifo.front();
	m_tx_fifo.pop();
	size--;

	int trigger = (m_cr[1] & C2_1_2_BYTE) ? 1 : 2;
	int available = 3 - size;

	if (available >= trigger)
	{
		m_status |= S_TDRA;
	}

	*tuf = 0;
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
			stop_bits_t stop_bits = STOP_BITS_0;

			switch ((data & C2_WS_MASK) >> C2_WS_SHIFT)
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

			set_data_frame(0, data_bit_count, parity, stop_bits);

			// The fifo trigger levels may have changed, so update
			// the status bits.

			int trigger = (m_cr[1] & C2_1_2_BYTE) ? 1 : 2;

			if (m_rx_fifo.size() >= trigger)
				m_status |= S_RDA;
			else
				m_status &= ~S_RDA;

			int tx_fifo_available = 3 - m_tx_fifo.size();
			if (tx_fifo_available >= trigger)
				m_status |= S_TDRA;
			else
				m_status &= ~S_TDRA;

			break;
			}

		case C1_AC_C3:
			/* control 3 */
			LOG("MC6852 Control 3 %02x\n", data);
			m_cr[2] = data;
			if (m_cr[2] & C3_CTUF)
			{
				m_cr[2] &= ~C3_CTUF;
				m_status &= ~S_TUF;
			}
			if (m_cr[2] & C3_CTS)
			{
				m_cr[2] &= ~C3_CTS;
				m_status &= ~S_CTS;
			}
			break;

		case C1_AC_SYNC:
			/* sync code */
			LOG("MC6852 Sync Code %02x\n", data);
			m_scr = data;
			break;

		case C1_AC_TX_FIFO: {
			/* transmit data FIFO */
			int available = 3 - m_tx_fifo.size();
			if (available > 0)
			{
				LOG("MC6852 Transmit FIFO %02x\n", data);
				if (!m_tx_pull_mode && !m_tx_active)
				{
					// transfer is idle: this emulates moving the first byte
					// into the shift register, and kicking off transmission.
					// tra_complete() will be called when the byte has been
					// sent.
					m_tx_active = true;
					transmit_register_setup(data);
				}
				else
				{
					m_tx_fifo.push(data);
					available--;
				}
			}
			else
			{
				LOG("MC6852 Transmit FIFO OVERFLOW %02x\n", data);
			}
			int trigger = (m_cr[1] & C2_1_2_BYTE) ? 1 : 2;
			if (available < trigger)
			{
				m_status &= ~S_TDRA;
			}
			break;
			}
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
			m_rx_fifo = std::queue<uint8_t>();

			receive_register_reset();
		}

		/* transmitter reset */
		if (data & C1_TX_RS)
		{
			// When Tx Rs is set, it clears the transmitter
			// control section, Transmitter Shift Register, Tx
			// Data FIFO Control (the Tx Data FIFO can be reloaded
			// after one E clock pulse), the Transmitter Underflow
			// status bit, and the CTS interrupt.

			LOG("MC6852 Transmitter Reset\n");

			m_status &= ~(S_TUF | S_CTS);
			m_status |= S_TDRA;
			m_tx_fifo = std::queue<uint8_t>();
			m_tx_active = false;

			transmit_register_reset();
		}

		if (data & C1_STRIP_SYNC)
			LOG("MC6852 Strip Synchronization Characters\n");

		if (data & C1_CLEAR_SYNC)
		{
			LOG("MC6852 Clear Synchronization\n");
			m_in_sync = 0;
		}

		m_cr[0] = data;
	}
}
