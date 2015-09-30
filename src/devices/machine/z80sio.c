// license:BSD-3-Clause
// copyright-holders:Curt Coder, Joakim Larsson Edstrom
/***************************************************************************

    Z80-SIO Serial Input/Output emulation

    The variants in the SIO family are only different in the packaging
    but has the same register features. However, since some signals are
    not connected to the pins on the package or share a pin with another
    signal the functionality is limited. However, this driver does not
    check that an operation is invalid because of package type but relies
    on the software to be adapated for the particular version.

    Package:                DIP40  SIO/0, SIO/1, SIO/2,
                            QFP44  SIO/3
                            PLCC44 SIO/4
    -------------------------------------------------------------------
    Channels / Full Duplex  2 / Y
    Synch data rates  2Mhz  500Kbps
                      4MHz  800Kbps
                      6MHz 1200Kbps
                     10MHz 2500Kbps
   -- Asynchrounous features -------------------------------------------
    5-8 bit per char         Y
    1,1.5,2 stop bits        Y
    odd/even parity          Y
    x1,x16,x32,x64           Y
    break det/gen            Y
    parity, framing &        Y
    overrun error det        Y
    -- Byte oriented synchrounous features -------------------------------
    Int/ext char sync        Y
    1/2 synch chars          Y
    Aut synch char insertion Y
    Aut CRC gen/det          Y
    -- SDLC/HDLC capabilities --------------------------------------------
    Abort seq gen/chk        Y
    Aut zero ins/det         Y
    Aut flag insert          Y
    Addr field rec           Y
    1-fld resid hand         Y
    Valid rec msg protection Y
    --
    Receiver FIFO            3
    Transmitter FIFO         1
    -------------------------------------------------------------------------
    * = Features that has been implemented  n/a = features that will not
***************************************************************************/

#include "z80sio.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE == 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define LLFORMAT "%I64%"
#define FUNCNAME __func__
#else
#define LLFORMAT "%lld"
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type Z80SIO = &device_creator<z80sio_device>;
const device_type Z80SIO_CHANNEL = &device_creator<z80sio_channel>;

//-------------------------------------------------
//  device_mconfig_additions -
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( z80sio )
	MCFG_DEVICE_ADD(CHANA_TAG, Z80SIO_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, Z80SIO_CHANNEL, 0)
MACHINE_CONFIG_END

machine_config_constructor z80sio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( z80sio );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80sio_device - constructor
//-------------------------------------------------

z80sio_device::z80sio_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_z80daisy_interface(mconfig, *this),
		m_chanA(*this, CHANA_TAG),
		m_chanB(*this, CHANB_TAG),
		m_rxca(0),
		m_txca(0),
		m_rxcb(0),
		m_txcb(0),
		m_out_txda_cb(*this),
		m_out_dtra_cb(*this),
		m_out_rtsa_cb(*this),
		m_out_wrdya_cb(*this),
		m_out_synca_cb(*this),
		m_out_txdb_cb(*this),
		m_out_dtrb_cb(*this),
		m_out_rtsb_cb(*this),
		m_out_wrdyb_cb(*this),
		m_out_syncb_cb(*this),
		m_out_int_cb(*this),
		m_out_rxdrqa_cb(*this),
		m_out_txdrqa_cb(*this),
		m_out_rxdrqb_cb(*this),
		m_out_txdrqb_cb(*this),
		m_variant(variant)
{
	for (int i = 0; i < 8; i++)
		m_int_state[i] = 0;
}

z80sio_device::z80sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80SIO, "Z80 SIO", tag, owner, clock, "z80sio", __FILE__),
		device_z80daisy_interface(mconfig, *this),
		m_chanA(*this, CHANA_TAG),
		m_chanB(*this, CHANB_TAG),
		m_rxca(0),
		m_txca(0),
		m_rxcb(0),
		m_txcb(0),
		m_out_txda_cb(*this),
		m_out_dtra_cb(*this),
		m_out_rtsa_cb(*this),
		m_out_wrdya_cb(*this),
		m_out_synca_cb(*this),
		m_out_txdb_cb(*this),
		m_out_dtrb_cb(*this),
		m_out_rtsb_cb(*this),
		m_out_wrdyb_cb(*this),
		m_out_syncb_cb(*this),
		m_out_int_cb(*this),
		m_out_rxdrqa_cb(*this),
		m_out_txdrqa_cb(*this),
		m_out_rxdrqb_cb(*this),
		m_out_txdrqb_cb(*this),
		m_variant(TYPE_Z80SIO)
{
	for (int i = 0; i < 8; i++)
		m_int_state[i] = 0;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80sio_device::device_start()
{
		LOG(("%s\n", FUNCNAME));
	// resolve callbacks
	m_out_txda_cb.resolve_safe();
	m_out_dtra_cb.resolve_safe();
	m_out_rtsa_cb.resolve_safe();
	m_out_wrdya_cb.resolve_safe();
	m_out_synca_cb.resolve_safe();
	m_out_txdb_cb.resolve_safe();
	m_out_dtrb_cb.resolve_safe();
	m_out_rtsb_cb.resolve_safe();
	m_out_wrdyb_cb.resolve_safe();
	m_out_syncb_cb.resolve_safe();
	m_out_int_cb.resolve_safe();
	m_out_rxdrqa_cb.resolve_safe();
	m_out_txdrqa_cb.resolve_safe();
	m_out_rxdrqb_cb.resolve_safe();
	m_out_txdrqb_cb.resolve_safe();

	// configure channel A
	m_chanA->m_rxc = m_rxca;
	m_chanA->m_txc = m_txca;

	// configure channel B
	m_chanB->m_rxc = m_rxcb;
	m_chanB->m_txc = m_txcb;

	// state saving
	save_item(NAME(m_int_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80sio_device::device_reset()
{
	LOG(("%s \"%s\" \n", FUNCNAME, tag()));

	m_chanA->reset();
	m_chanB->reset();
}

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80sio_device::z80daisy_irq_state()
{
	int state = 0;
	int i;

	LOG(("Z80SIO \"%s\" : Interrupt State A:%d%d%d%d B:%d%d%d%d\n", tag(),
				m_int_state[0], m_int_state[1], m_int_state[2], m_int_state[3],
				m_int_state[4], m_int_state[5], m_int_state[6], m_int_state[7]));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_int_state[i];
	}

	LOG(("Z80SIO \"%s\" : Interrupt State %u\n", tag(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80sio_device::z80daisy_irq_ack()
{
	int i;

	LOG(("Z80SIO \"%s\" Interrupt Acknowledge\n", tag()));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[i] & Z80_DAISY_INT)
		{
			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[i] = Z80_DAISY_IEO;
			m_chanA->m_rr0 &= ~z80sio_channel::RR0_INTERRUPT_PENDING;
			check_interrupts();

			LOG(("Z80SIO \"%s\" : Interrupt Acknowledge Vector %02x\n", tag(), m_chanB->m_rr2));

			return m_chanB->m_rr2;
		}
	}

	//logerror("z80sio_irq_ack: failed to find an interrupt to ack!\n");

	return m_chanB->m_rr2;
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80sio_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80SIO \"%s\" Return from Interrupt\n", tag()));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// find the first channel with an IEO pending
		if (m_int_state[i] & Z80_DAISY_IEO)
		{
			// clear the IEO state and update the IRQs
			m_int_state[i] &= ~Z80_DAISY_IEO;
			check_interrupts();
			return;
		}
	}

	//logerror("z80sio_irq_reti: failed to find an interrupt to clear IEO on!\n");
}


//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void z80sio_device::check_interrupts()
{
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	m_out_int_cb(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------

void z80sio_device::reset_interrupts()
{
	for (int i = 0; i < 8; i++)
	{
		m_int_state[i] = 0;
	}

	check_interrupts();
}


//-------------------------------------------------
//  trigger_interrupt - TODO: needs attention for SIO
//-------------------------------------------------

void z80sio_device::trigger_interrupt(int index, int state)
{
	UINT8 vector = m_chanB->m_wr2;
	int priority;

#if 0
	if((m_variant == TYPE_I8274) || (m_variant == TYPE_UPD7201))
	{
		int prio_level = 0;
		switch(state)
		{
			case z80sio_channel::INT_TRANSMIT:
				prio_level = 1;
				break;
			case z80sio_channel::INT_RECEIVE:
			case z80sio_channel::INT_SPECIAL:
				prio_level = 0;
				break;
			case z80sio_channel::INT_EXTERNAL:
				prio_level = 2;
				break;
		}

		if(m_chanA->m_wr2 & z80sio_channel::WR2_PRIORITY)
		{
			priority = (prio_level * 2) + index;
		}
		else
		{
			priority = (prio_level == 2) ? index + 4 : ((index * 2) + prio_level);
		}
		if (m_chanB->m_wr1 & z80sio_channel::WR1_STATUS_VECTOR)
		{
			vector = (!index << 2) | state;
			if((m_chanA->m_wr1 & 0x18) == z80sio_channel::WR2_MODE_8086_8088)
			{
				vector = (m_chanB->m_wr2 & 0xf8) | vector;
			}
			else
			{
				vector = (m_chanB->m_wr2 & 0xe3) | (vector << 2);
			}
		}
	}
	else
	{
#endif
		priority = (index << 2) | state;
		if (m_chanB->m_wr1 & z80sio_channel::WR1_STATUS_VECTOR)
		{
			// status affects vector
			vector = (m_chanB->m_wr2 & 0xf1) | (!index << 3) | (state << 1);
		}
//  }

	LOG(("Z80SIO \"%s\" Channel %c : Interrupt Request %u\n", tag(), 'A' + index, state));

	// update vector register
	m_chanB->m_rr2 = vector;

	// trigger interrupt
	m_int_state[priority] |= Z80_DAISY_INT;
	m_chanA->m_rr0 |= z80sio_channel::RR0_INTERRUPT_PENDING;

	// check for interrupt
	check_interrupts();
}


//-------------------------------------------------
//  m1_r - interrupt acknowledge
//-------------------------------------------------

int z80sio_device::m1_r()
{
	return z80daisy_irq_ack();
}


//-------------------------------------------------
//  cd_ba_r -
//-------------------------------------------------

READ8_MEMBER( z80sio_device::cd_ba_r )
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

		//        LOG(("z80sio_device::cd_ba_r ba:%02x cd:%02x\n", ba, cd));

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  cd_ba_w -
//-------------------------------------------------

WRITE8_MEMBER( z80sio_device::cd_ba_w )
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

		LOG(("z80sio_device::cd_ba_w ba:%02x cd:%02x\n", ba, cd));

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}


//-------------------------------------------------
//  ba_cd_r -
//-------------------------------------------------

READ8_MEMBER( z80sio_device::ba_cd_r )
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

		//        LOG(("z80sio_device::ba_cd_r ba:%02x cd:%02x\n", ba, cd));

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  ba_cd_w -
//-------------------------------------------------

WRITE8_MEMBER( z80sio_device::ba_cd_w )
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80sio_channel *channel = ba ? m_chanB : m_chanA;

		LOG(("z80sio_device::ba_cd_w ba:%02x cd:%02x\n", ba, cd));

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}

//**************************************************************************
//  SIO CHANNEL
//**************************************************************************

//-------------------------------------------------
//  z80sio_channel - constructor
//-------------------------------------------------

z80sio_channel::z80sio_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80SIO_CHANNEL, "Z80 SIO channel", tag, owner, clock, "z80sio_channel", __FILE__),
		device_serial_interface(mconfig, *this),
		m_rx_error(0),
		m_rx_fifo(-1),
		m_rx_clock(0),
		m_rx_first(0),
		m_rx_break(0),
		m_rx_rr0_latch(0),
		m_rxd(0),
		m_sh(0),
		m_cts(0),
		m_dcd(0),
		m_tx_data(0),
		m_tx_clock(0),
		m_dtr(0),
		m_rts(0),
		m_sync(0)
{
		// Reset all registers
		m_rr0 = m_rr1 = m_rr2 = 0;
		m_wr0 = m_wr1 = m_wr2 = m_wr3 = m_wr4 = m_wr5 = m_wr6 = m_wr7 = 0;

	for (int i = 0; i < 3; i++)
	{
		m_rx_data_fifo[i] = 0;
		m_rx_error_fifo[i] = 0;
	}
}


//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void z80sio_channel::device_start()
{
		LOG(("%s\n",FUNCNAME));
	m_uart = downcast<z80sio_device *>(owner());
	m_index = m_uart->get_channel_index(this);

	// state saving
	save_item(NAME(m_rr0));
	save_item(NAME(m_rr1));
	save_item(NAME(m_rr2));
	save_item(NAME(m_wr0));
	save_item(NAME(m_wr1));
	save_item(NAME(m_wr2));
	save_item(NAME(m_wr3));
	save_item(NAME(m_wr4));
	save_item(NAME(m_wr5));
	save_item(NAME(m_wr6));
	save_item(NAME(m_wr7));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_error));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_rx_break));
	save_item(NAME(m_rx_rr0_latch));
	save_item(NAME(m_sh));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_sync));
	device_serial_interface::register_save_state(machine().save(), this);
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------

void z80sio_channel::device_reset()
{
		LOG(("%s\n", FUNCNAME));
	receive_register_reset();
	transmit_register_reset();

	// disable receiver
	m_wr3 &= ~WR3_RX_ENABLE;

	// disable transmitter
	m_wr5 &= ~WR5_TX_ENABLE;
	m_rr0 |= RR0_TX_BUFFER_EMPTY;
	m_rr1 |= RR1_ALL_SENT;

	// reset external lines
	set_rts(1);
	set_dtr(1);

	// reset interrupts
	if (m_index == z80sio_device::CHANNEL_A)
	{
		m_uart->reset_interrupts();
	}
}

void z80sio_channel::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void z80sio_channel::tra_callback()
{
	if (!(m_wr5 & WR5_TX_ENABLE))
	{
		// transmit mark
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->m_out_txda_cb(1);
		else
			m_uart->m_out_txdb_cb(1);
	}
	else if (m_wr5 & WR5_SEND_BREAK)
	{
		// transmit break
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->m_out_txda_cb(0);
		else
			m_uart->m_out_txdb_cb(0);
	}
	else if (!is_transmit_register_empty())
	{
		// transmit data
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->m_out_txda_cb(transmit_register_get_data_bit());
		else
			m_uart->m_out_txdb_cb(transmit_register_get_data_bit());
	}
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void z80sio_channel::tra_complete()
{
	if ((m_wr5 & WR5_TX_ENABLE) && !(m_wr5 & WR5_SEND_BREAK) && !(m_rr0 & RR0_TX_BUFFER_EMPTY))
	{
		LOG(("Z80SIO \"%s\" Channel %c : Transmit Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, m_tx_data));

		transmit_register_setup(m_tx_data);

		// empty transmit buffer
		m_rr0 |= RR0_TX_BUFFER_EMPTY;

		if (m_wr1 & WR1_TX_INT_ENABLE)
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else if (m_wr5 & WR5_SEND_BREAK)
	{
		// transmit break
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->m_out_txda_cb(0);
		else
			m_uart->m_out_txdb_cb(0);
	}
	else
	{
		// transmit mark
		if (m_index == z80sio_device::CHANNEL_A)
			m_uart->m_out_txda_cb(1);
		else
			m_uart->m_out_txdb_cb(1);
	}

	// if transmit buffer is empty
	if (m_rr0 & RR0_TX_BUFFER_EMPTY)
	{
		// then all characters have been sent
		m_rr1 |= RR1_ALL_SENT;

		// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
		if (!m_rts)
			set_rts(1);
	}
}


//-------------------------------------------------
//  rcv_callback -
//-------------------------------------------------

void z80sio_channel::rcv_callback()
{
	if (m_wr3 & WR3_RX_ENABLE)
	{
		receive_register_update_bit(m_rxd);
	}
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void z80sio_channel::rcv_complete()
{
	receive_register_extract();
	receive_data(get_received_char());
}


//-------------------------------------------------
//  get_clock_mode - get clock divisor
//-------------------------------------------------

int z80sio_channel::get_clock_mode()
{
	int clocks = 1;

	switch (m_wr4 & WR4_CLOCK_RATE_MASK)
	{
	case WR4_CLOCK_RATE_X1:     clocks = 1;     break;
	case WR4_CLOCK_RATE_X16:    clocks = 16;    break;
	case WR4_CLOCK_RATE_X32:    clocks = 32;    break;
	case WR4_CLOCK_RATE_X64:    clocks = 64;    break;
	}

	return clocks;
}

/* From "uPD7201/7201A MULTI PROTOCOL SERIAL COMMUNICATION CONTROLLER" by NEC:
"RTSA (Request to Send A): The state of the RTS bit (01 of the CR5 register) controls this pin. If
the RTS bit is reset in the asynchronous mode, a high level will not be output on the RTS pin until
all transmit characters are written and the all sent bit (D0 of the SR1 register) is set. In the
synchronous mode, the state of the RTS bit is used as is. That is, when the RTS bit is 0, the RTS
pin is 1. When the RTS bit is 1, the RTS pin is O."

CR5 = m_wr5 and SR1 = m_rr1

*/

void z80sio_channel::set_rts(int state)
{
	if (m_index == z80sio_device::CHANNEL_A)
		m_uart->m_out_rtsa_cb(state);
	else
		m_uart->m_out_rtsb_cb(state);
}

void z80sio_channel::update_rts()
{
		if (m_wr5 & WR5_RTS)
		{
				// when the RTS bit is set, the _RTS output goes low
				set_rts(0);
				m_rts = 1;
		}
		else
		{
				// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
				m_rts = 0;
		}

		// data terminal ready output follows the state programmed into the DTR bit*/
		set_dtr((m_wr5 & WR5_DTR) ? 0 : 1);
}

//-------------------------------------------------
//  get_stop_bits - get number of stop bits
//-------------------------------------------------

device_serial_interface::stop_bits_t z80sio_channel::get_stop_bits()
{
	switch (m_wr4 & WR4_STOP_BITS_MASK)
	{
	case WR4_STOP_BITS_1: return STOP_BITS_1;
	case WR4_STOP_BITS_1_5: return STOP_BITS_1_5;
	case WR4_STOP_BITS_2: return STOP_BITS_2;
	}

	return STOP_BITS_0;
}


//-------------------------------------------------
//  get_rx_word_length - get receive word length
//-------------------------------------------------

int z80sio_channel::get_rx_word_length()
{
	int bits = 5;

	switch (m_wr3 & WR3_RX_WORD_LENGTH_MASK)
	{
	case WR3_RX_WORD_LENGTH_5:  bits = 5;       break;
	case WR3_RX_WORD_LENGTH_6:  bits = 6;       break;
	case WR3_RX_WORD_LENGTH_7:  bits = 7;       break;
	case WR3_RX_WORD_LENGTH_8:  bits = 8;       break;
	}

	return bits;
}


//-------------------------------------------------
//  get_tx_word_length - get transmit word length
//-------------------------------------------------

int z80sio_channel::get_tx_word_length()
{
	int bits = 5;

	switch (m_wr5 & WR5_TX_WORD_LENGTH_MASK)
	{
	case WR5_TX_WORD_LENGTH_5:  bits = 5;   break;
	case WR5_TX_WORD_LENGTH_6:  bits = 6;   break;
	case WR5_TX_WORD_LENGTH_7:  bits = 7;   break;
	case WR5_TX_WORD_LENGTH_8:  bits = 8;   break;
	}

	return bits;
}

/*
 * This register contains the status of the receive and transmit buffers; the
 * DCD, CTS, and SYNC inputs; the Transmit Underrun/EOM latch; and the
 * Break/Abort latch. */
UINT8 z80sio_channel::do_sioreg_rr0()
{
		return m_rr0;
}
/*
 * This register contains the Special Receive condition status bits and Residue
 * codes for the I-Field in the SDLC Receive Mode. */
UINT8 z80sio_channel::do_sioreg_rr1()
{
		return m_rr1;
}
/*
 * This register contains the interrupt vector written into WR2 if the Status
Affects Vector control bit is not set. If the control bit is set, it contains the
modified vector listed in the Status Affects Vector paragraph of the Write
Register 1 section. When this register is read, the vector returned is modi-
fied by the highest priority interrupting condition at the time of the read. If
no interrupts are pending, the vector is modified with V3 = 0, V2 = 1, and
V1 = 1. This register is read only through Channel B. */
UINT8 z80sio_channel::do_sioreg_rr2()
{
		// channel B only
		return m_index == z80sio_device::CHANNEL_B ? m_rr2 : 0;
}


//-------------------------------------------------
//  control_read - read control register
//-------------------------------------------------

UINT8 z80sio_channel::control_read()
{
	UINT8 data = 0;
		UINT8 reg  = m_wr0 & WR0_REGISTER_MASK;

	if (reg != 0)
	{
		// mask out register index
		m_wr0 &= ~WR0_REGISTER_MASK;
	}

	switch (reg)
	{
	case REG_RR0_STATUS:         data = do_sioreg_rr0(); break;
	case REG_RR1_SPEC_RCV_COND:  data = do_sioreg_rr1(); break;
	case REG_RR2_INTERRUPT_VECT: data = do_sioreg_rr2(); break;
	default:
		logerror("Z80SIO \"%s\" Channel %c : Unsupported RRx register:%02x\n", m_owner->tag(), 'A' + m_index, reg);
	}
	//LOG(("Z80SIO \"%s\" Channel %c : Register R%d read '%02x'\n", m_owner->tag(), 'A' + m_index, reg, data));

	return data;
}

/* SIO CRC Initialization Code handling
 Handle the WR0 CRC Reset/Init bits separatelly, needed by derived devices separatelly from the commands */
void z80sio_channel::do_sioreg_wr0_resets(UINT8 data)
{
		switch (data & WR0_CRC_RESET_CODE_MASK)
		{
		case WR0_CRC_RESET_NULL:
				LOG(("Z80SIO \"%s\" Channel %c : CRC_RESET_NULL\n", m_owner->tag(), 'A' + m_index));
				break;
		case WR0_CRC_RESET_RX: /* In Synchronous mode: all Os (zeros) (CCITT-O CRC-16) */
				LOG(("Z80SIO \"%s\" Channel %c : CRC_RESET_RX - not implemented\n", m_owner->tag(), 'A' + m_index));
				break;
		case WR0_CRC_RESET_TX: /* In HDLC mode: all 1s (ones) (CCITT-1) */
				LOG(("Z80SIO \"%s\" Channel %c : CRC_RESET_TX - not implemented\n", m_owner->tag(), 'A' + m_index));
				break;
		case WR0_CRC_RESET_TX_UNDERRUN: /* Resets Tx underrun/EOM bit (D6 of the SRO register) */
				LOG(("Z80SIO \"%s\" Channel %c : CRC_RESET_TX_UNDERRUN - not implemented\n", m_owner->tag(), 'A' + m_index));
				break;
		default: /* Will not happen unless someone messes with the mask */
				logerror("Z80SIO \"%s\" Channel %c : %s Wrong CRC reset/init command:%02x\n", m_owner->tag(), 'A' + m_index, FUNCNAME, data & WR0_CRC_RESET_CODE_MASK);
		}
}
void z80sio_channel::do_sioreg_wr0(UINT8 data)
{
		m_wr0 = data;
		switch (data & WR0_COMMAND_MASK)
	{
		case WR0_NULL:
				LOG(("Z80SIO \"%s\" Channel %c : Null\n", m_owner->tag(), 'A' + m_index));
				break;
		case WR0_RESET_EXT_STATUS:
				// reset external/status interrupt
				m_rr0 &= ~(RR0_DCD | RR0_SYNC_HUNT | RR0_CTS | RR0_BREAK_ABORT);
				// release the latch
				m_rx_rr0_latch = 0;
				// update register to reflect wire values TODO: Check if this will fire new interrupts
				if (!m_dcd) m_rr0 |= RR0_DCD;
				if (m_sync) m_rr0 |= RR0_SYNC_HUNT;
				if (m_cts)  m_rr0 |= RR0_CTS;

				LOG(("Z80SIO \"%s\" Channel %c : Reset External/Status Interrupt\n", m_owner->tag(), 'A' + m_index));
				break;
		case WR0_CHANNEL_RESET:
				// channel reset
				LOG(("Z80SIO \"%s\" Channel %c : Channel Reset\n", m_owner->tag(), 'A' + m_index));
				device_reset();
				break;
		case WR0_ENABLE_INT_NEXT_RX:
				// enable interrupt on next receive character
				LOG(("Z80SIO \"%s\" Channel %c : Enable Interrupt on Next Received Character\n", m_owner->tag(), 'A' + m_index));
				m_rx_first = 1;
				break;
		case WR0_RESET_TX_INT:
				// reset transmitter interrupt pending
				LOG(("Z80SIO \"%s\" Channel %c : Reset Transmitter Interrupt Pending\n", m_owner->tag(), 'A' + m_index));
				logerror("Z80SIO \"%s\" Channel %c : unsupported command: Reset Transmitter Interrupt Pending\n", m_owner->tag(), 'A' + m_index);
				break;
		case WR0_ERROR_RESET:
				// error reset
				LOG(("Z80SIO \"%s\" Channel %c : Error Reset\n", m_owner->tag(), 'A' + m_index));
				m_rr1 &= ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
				break;
		case WR0_RETURN_FROM_INT:
				// return from interrupt
				LOG(("Z80SIO \"%s\" Channel %c : Return from Interrupt\n", m_owner->tag(), 'A' + m_index));
				m_uart->z80daisy_irq_reti();
				break;
		default:
				LOG(("Z80SIO \"%s\" Channel %c : Unsupported WR0 command %02x mask %02x\n", m_owner->tag(), 'A' + m_index, data, WR0_REGISTER_MASK));

		}
		do_sioreg_wr0_resets(data);
}

void z80sio_channel::do_sioreg_wr1(UINT8 data)
{
/* TODO: implement vector modifications when WR1 bit D2 is changed */
		m_wr1 = data;
		LOG(("Z80SIO \"%s\" Channel %c : External Interrupt Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_EXT_INT_ENABLE) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Transmit Interrupt Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_TX_INT_ENABLE) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Status Affects Vector %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_STATUS_VECTOR) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Wait/Ready Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_WRDY_ENABLE) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Wait/Ready Function %s\n", m_owner->tag(), 'A' + m_index, (data & WR1_WRDY_FUNCTION) ? "Ready" : "Wait"));
		LOG(("Z80SIO \"%s\" Channel %c : Wait/Ready on %s\n", m_owner->tag(), 'A' + m_index, (data & WR1_WRDY_ON_RX_TX) ? "Receive" : "Transmit"));

		switch (data & WR1_RX_INT_MODE_MASK)
		{
		case WR1_RX_INT_DISABLE:
				LOG(("Z80SIO \"%s\" Channel %c : Receiver Interrupt Disabled\n", m_owner->tag(), 'A' + m_index));
				break;

		case WR1_RX_INT_FIRST:
				LOG(("Z80SIO \"%s\" Channel %c : Receiver Interrupt on First Character\n", m_owner->tag(), 'A' + m_index));
				break;

		case WR1_RX_INT_ALL_PARITY:
				LOG(("Z80SIO \"%s\" Channel %c : Receiver Interrupt on All Characters, Parity Affects Vector\n", m_owner->tag(), 'A' + m_index));
				break;

		case WR1_RX_INT_ALL:
				LOG(("Z80SIO \"%s\" Channel %c : Receiver Interrupt on All Characters\n", m_owner->tag(), 'A' + m_index));
				break;
		}
}

void z80sio_channel::do_sioreg_wr2(UINT8 data)
{
		m_wr2 = data;
		if (m_index == z80sio_device::CHANNEL_B)
		{
				if (m_wr1 & z80sio_channel::WR1_STATUS_VECTOR)
						m_rr2 = ( m_rr2 & 0x0e ) | ( m_wr2 & 0xF1);
				else
						m_rr2 = m_wr2;
		}
		m_uart->check_interrupts();
		LOG(("Z80SIO \"%s\" Channel %c : Interrupt Vector %02x\n", m_owner->tag(), 'A' + m_index, data));
}

void z80sio_channel::do_sioreg_wr3(UINT8 data)
{
		m_wr3 = data;
		LOG(("Z80SIO \"%s\" Channel %c : Receiver Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR3_RX_ENABLE) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Auto Enables %u\n", m_owner->tag(), 'A' + m_index, (data & WR3_AUTO_ENABLES) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Receiver Bits/Character %u\n", m_owner->tag(), 'A' + m_index, get_rx_word_length()));
}

void z80sio_channel::do_sioreg_wr4(UINT8 data)
{
		m_wr4 = data;
		LOG(("Z80SIO \"%s\" Channel %c : Parity Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR4_PARITY_ENABLE) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Parity %s\n", m_owner->tag(), 'A' + m_index, (data & WR4_PARITY_EVEN) ? "Even" : "Odd"));
		LOG(("Z80SIO \"%s\" Channel %c : Stop Bits %s\n", m_owner->tag(), 'A' + m_index, stop_bits_tostring(get_stop_bits())));
		LOG(("Z80SIO \"%s\" Channel %c : Clock Mode %uX\n", m_owner->tag(), 'A' + m_index, get_clock_mode()));
}

void z80sio_channel::do_sioreg_wr5(UINT8 data)
{
		m_wr5 = data;
		LOG(("Z80SIO \"%s\" Channel %c : Transmitter Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_TX_ENABLE) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Transmitter Bits/Character %u\n", m_owner->tag(), 'A' + m_index, get_tx_word_length()));
		LOG(("Z80SIO \"%s\" Channel %c : Send Break %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_SEND_BREAK) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Request to Send %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_RTS) ? 1 : 0));
		LOG(("Z80SIO \"%s\" Channel %c : Data Terminal Ready %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_DTR) ? 1 : 0));
}

void z80sio_channel::do_sioreg_wr6(UINT8 data)
{
		LOG(("Z80SIO \"%s\" Channel %c : Transmit Sync %02x\n", m_owner->tag(), 'A' + m_index, data));
		m_sync = (m_sync & 0xff00) | data;
}

void z80sio_channel::do_sioreg_wr7(UINT8 data)
{
		LOG(("Z80SIO \"%s\" Channel %c : Receive Sync %02x\n", m_owner->tag(), 'A' + m_index, data));
		m_sync = (data << 8) | (m_sync & 0xff);
}

//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------

void z80sio_channel::control_write(UINT8 data)
{
		UINT8   reg = m_wr0 & WR0_REGISTER_MASK;

	if (reg != 0)
	{
		// mask out register index
		m_wr0 &= ~WR0_REGISTER_MASK;
	}

	LOG(("Z80SIO control_write reg %02x, regmask %02x, WR0 %02x, data %02x\n", reg, WR0_REGISTER_MASK, m_wr0, data));

	switch (reg)
	{
	case REG_WR0_COMMAND_REGPT:      do_sioreg_wr0(data); break;
	case REG_WR1_INT_DMA_ENABLE:     do_sioreg_wr1(data); m_uart->check_interrupts(); break;
	case REG_WR2_INT_VECTOR:         do_sioreg_wr2(data); break;
	case REG_WR3_RX_CONTROL:         do_sioreg_wr3(data); update_serial(); break;
	case REG_WR4_RX_TX_MODES:        do_sioreg_wr4(data); update_serial(); break;
	case REG_WR5_TX_CONTROL:         do_sioreg_wr5(data); update_serial(); update_rts(); break;
	case REG_WR6_SYNC_OR_SDLC_A:     do_sioreg_wr6(data); break;
	case REG_WR7_SYNC_OR_SDLC_F:     do_sioreg_wr7(data); break;
	default:
		logerror("Z80SIO \"%s\" Channel %c : Unsupported WRx register:%02x\n", m_owner->tag(), 'A' + m_index, reg);
	}
}


//-------------------------------------------------
//  data_read - read data register
//-------------------------------------------------

UINT8 z80sio_channel::data_read()
{
	UINT8 data = 0;

	if (m_rx_fifo >= 0)
	{
		// load data from the FIFO
		data = m_rx_data_fifo[m_rx_fifo];

		// load error status from the FIFO
		m_rr1 = (m_rr1 & ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR)) | m_rx_error_fifo[m_rx_fifo];

		// decrease FIFO pointer
		m_rx_fifo--;

		if (m_rx_fifo < 0)
		{
			// no more characters available in the FIFO
			m_rr0 &= ~ RR0_RX_CHAR_AVAILABLE;
		}
	}

	LOG(("Z80SIO \"%s\" Channel %c : Data Register Read '%02x'\n", m_owner->tag(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------

void z80sio_channel::data_write(UINT8 data)
{
	m_tx_data = data;

	if ((m_wr5 & WR5_TX_ENABLE) && is_transmit_register_empty())
	{
		LOG(("Z80SIO \"%s\" Channel %c : Transmit Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, m_tx_data));

		transmit_register_setup(m_tx_data);

		// empty transmit buffer
		m_rr0 |= RR0_TX_BUFFER_EMPTY;

		if (m_wr1 & WR1_TX_INT_ENABLE)
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else
	{
		m_rr0 &= ~RR0_TX_BUFFER_EMPTY;
	}

	m_rr1 &= ~RR1_ALL_SENT;

	LOG(("Z80SIO \"%s\" Channel %c : Data Register Write '%02x'\n", m_owner->tag(), 'A' + m_index, data));
}


//-------------------------------------------------
//  receive_data - receive data word
//-------------------------------------------------

void z80sio_channel::receive_data(UINT8 data)
{
	LOG(("Z80SIO \"%s\" Channel %c : Receive Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, data));

	if (m_rx_fifo == 2)
	{
		// receive overrun error detected
		m_rx_error |= RR1_RX_OVERRUN_ERROR;

		switch (m_wr1 & WR1_RX_INT_MODE_MASK)
		{
		case WR1_RX_INT_FIRST:
			if (!m_rx_first)
						{
								m_uart->trigger_interrupt(m_index, INT_SPECIAL);
						}
			break;

		case WR1_RX_INT_ALL_PARITY:
		case WR1_RX_INT_ALL:
			m_uart->trigger_interrupt(m_index, INT_SPECIAL);
			break;
		}
	}
	else
	{
		m_rx_fifo++;
	}

	// store received character and error status into FIFO
	m_rx_data_fifo[m_rx_fifo] = data;
	m_rx_error_fifo[m_rx_fifo] = m_rx_error;

	m_rr0 |= RR0_RX_CHAR_AVAILABLE;

	// receive interrupt
	switch (m_wr1 & WR1_RX_INT_MODE_MASK)
	{
	case WR1_RX_INT_FIRST:
		if (m_rx_first)
		{
			m_uart->trigger_interrupt(m_index, INT_RECEIVE);

			m_rx_first = 0;
		}
		break;

	case WR1_RX_INT_ALL_PARITY:
	case WR1_RX_INT_ALL:
		m_uart->trigger_interrupt(m_index, INT_RECEIVE);
		break;
	}
}


//-------------------------------------------------
//  cts_w - clear to send handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sio_channel::cts_w )
{
	LOG(("Z80SIO \"%s\" Channel %c : CTS %u\n", m_owner->tag(), 'A' + m_index, state));

	if (m_cts != state)
	{
		// enable transmitter if in auto enables mode
		if (!state)
			if (m_wr3 & WR3_AUTO_ENABLES)
				m_wr5 |= WR5_TX_ENABLE;

		// set clear to send
		m_cts = state;

		if (!m_rx_rr0_latch)
		{
			if (!m_cts)
				m_rr0 |= RR0_CTS;
			else
				m_rr0 &= ~RR0_CTS;

			// trigger interrupt
			if (m_wr1 & WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				m_uart->trigger_interrupt(m_index, INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  dcd_w - data carrier detected handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sio_channel::dcd_w )
{
	LOG(("Z80SIO \"%s\" Channel %c : DCD %u\n", m_owner->tag(), 'A' + m_index, state));

	if (m_dcd != state)
	{
		// enable receiver if in auto enables mode
		if (!state)
			if (m_wr3 & WR3_AUTO_ENABLES)
				m_wr3 |= WR3_RX_ENABLE;

		// set data carrier detect
		m_dcd = state;

		if (!m_rx_rr0_latch)
		{
			if (m_dcd)
				m_rr0 |= RR0_DCD;
			else
				m_rr0 &= ~RR0_DCD;

			if (m_wr1 & WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				m_uart->trigger_interrupt(m_index, INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  sh_w - Sync Hunt handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sio_channel::sync_w )
{
	LOG(("Z80SIO \"%s\" Channel %c : Sync %u\n", m_owner->tag(), 'A' + m_index, state));

	if (m_sh != state)
	{
		// set ring indicator state
		m_sh = state;

		if (!m_rx_rr0_latch)
		{
			if (m_sh)
				m_rr0 |= RR0_SYNC_HUNT;
			else
				m_rr0 &= ~RR0_SYNC_HUNT;

			if (m_wr1 & WR1_EXT_INT_ENABLE)
			{
				// trigger interrupt
				m_uart->trigger_interrupt(m_index, INT_EXTERNAL);

				// latch read register 0
				m_rx_rr0_latch = 1;
			}
		}
	}
}


//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sio_channel::rxc_w )
{
	//LOG(("Z80SIO \"%s\" Channel %c : Receiver Clock Pulse\n", m_owner->tag(), m_index + 'A'));
	int clocks = get_clock_mode();
	if (clocks == 1)
		rx_clock_w(state);
	else if(state)
	{
		rx_clock_w(m_rx_clock < clocks/2);

		m_rx_clock++;
		if (m_rx_clock == clocks)
			m_rx_clock = 0;

	}
}


//-------------------------------------------------
//  txc_w - transmit clock
//-------------------------------------------------

WRITE_LINE_MEMBER( z80sio_channel::txc_w )
{
	//LOG(("Z80SIO \"%s\" Channel %c : Transmitter Clock Pulse\n", m_owner->tag(), m_index + 'A'));
	int clocks = get_clock_mode();
	if (clocks == 1)
		tx_clock_w(state);
	else if(state)
	{
		tx_clock_w(m_tx_clock < clocks/2);

		m_tx_clock++;
		if (m_tx_clock == clocks)
			m_tx_clock = 0;

	}
}


//-------------------------------------------------
//  update_serial -
//-------------------------------------------------
void z80sio_channel::update_serial()
{
	int data_bit_count = get_rx_word_length();
	stop_bits_t stop_bits = get_stop_bits();
	parity_t parity;

		LOG(("Z80SIO update_serial\n"));

	if (m_wr4 & WR4_PARITY_ENABLE)
	{
		if (m_wr4 & WR4_PARITY_EVEN)
			parity = PARITY_EVEN;
		else
			parity = PARITY_ODD;
	}
	else
		parity = PARITY_NONE;

	set_data_frame(1, data_bit_count, parity, stop_bits);

	int clocks = get_clock_mode();

	if (m_rxc > 0)
	{
		set_rcv_rate(m_rxc / clocks);
	}

	if (m_txc > 0)
	{
		set_tra_rate(m_txc / clocks);
	}
	receive_register_reset(); // if stop bits is changed from 0, receive register has to be reset
}


//-------------------------------------------------
//  set_dtr -
//-------------------------------------------------

void z80sio_channel::set_dtr(int state)
{
	m_dtr = state;

	if (m_index == z80sio_device::CHANNEL_A)
		m_uart->m_out_dtra_cb(m_dtr);
	else
		m_uart->m_out_dtrb_cb(m_dtr);
}

//-------------------------------------------------
//  write_rx -
//-------------------------------------------------

WRITE_LINE_MEMBER(z80sio_channel::write_rx)
{
	m_rxd = state;
	//only use rx_w when self-clocked
	if(m_rxc)
		device_serial_interface::rx_w(state);
}
