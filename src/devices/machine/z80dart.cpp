// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Intel 8274 Multi-Protocol Serial Controller emulation
    NEC uPD7201 Multiprotocol Serial Communications Controller emulation
    Z80-DART Dual Asynchronous Receiver/Transmitter emulation
    Z80-SIO/0/1/2/3/4 Serial Input/Output Controller emulation

    The z80dart/z80sio itself is based on an older intel serial chip, the i8274 MPSC
    (see http://doc.chipfind.ru/pdf/intel/8274.pdf), which also has almost identical
    behavior, except lacks the interrupt daisy chaining and has its own interrupt/dma
    scheme which uses write register 2 on channel A, that register which is unused on
    the z80dart and z80sio.

***************************************************************************/

/*

    TODO:

    - i8274 DMA scheme
    - break detection
    - wr0 reset tx interrupt pending
    - wait/ready
    - 1.5 stop bits
    - synchronous mode (Z80-SIO/1,2)
    - SDLC mode (Z80-SIO/1,2)

*/

#include "z80dart.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type Z80DART = &device_creator<z80dart_device>;
const device_type Z80DART_CHANNEL = &device_creator<z80dart_channel>;
const device_type Z80SIO0 = &device_creator<z80sio0_device>;
const device_type Z80SIO1 = &device_creator<z80sio1_device>;
const device_type Z80SIO2 = &device_creator<z80sio2_device>;
const device_type Z80SIO3 = &device_creator<z80sio3_device>;
const device_type Z80SIO4 = &device_creator<z80sio4_device>;
const device_type I8274 = &device_creator<i8274_device>;
const device_type UPD7201 = &device_creator<upd7201_device>;


//-------------------------------------------------
//  device_mconfig_additions -
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( z80dart )
	MCFG_DEVICE_ADD(CHANA_TAG, Z80DART_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, Z80DART_CHANNEL, 0)
MACHINE_CONFIG_END

machine_config_constructor z80dart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( z80dart );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80dart_device - constructor
//-------------------------------------------------

z80dart_device::z80dart_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, UINT32 variant, std::string shortname, std::string source)
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
	for (auto & elem : m_int_state)
		elem = 0;
}

z80dart_device::z80dart_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80DART, "Z80 DART", tag, owner, clock, "z80dart", __FILE__),
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
		m_variant(TYPE_DART)
{
	for (auto & elem : m_int_state)
		elem = 0;
}

z80sio0_device::z80sio0_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO0, "Z80 SIO/0", tag, owner, clock, TYPE_SIO0, "z80sio0", __FILE__)
{
}

z80sio1_device::z80sio1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO1, "Z80 SIO/1", tag, owner, clock, TYPE_SIO1, "z80sio1", __FILE__)
{
}

z80sio2_device::z80sio2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO2, "Z80 SIO/2", tag, owner, clock, TYPE_SIO2, "z80sio2", __FILE__)
{
}

z80sio3_device::z80sio3_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO3, "Z80 SIO/3", tag, owner, clock, TYPE_SIO3, "z80sio3", __FILE__)
{
}

z80sio4_device::z80sio4_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO4, "Z80 SIO/4", tag, owner, clock, TYPE_SIO4, "z80sio4", __FILE__)
{
}

i8274_device::i8274_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, I8274, "I8274", tag, owner, clock, TYPE_I8274, "i8274", __FILE__)
{
}

upd7201_device::upd7201_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, UPD7201, "uPD7201", tag, owner, clock, TYPE_UPD7201, "upd7201", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80dart_device::device_start()
{
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

void z80dart_device::device_reset()
{
	LOG(("Z80DART \"%s\" Reset\n", tag().c_str()));

	m_chanA->reset();
	m_chanB->reset();
}

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80dart_device::z80daisy_irq_state()
{
	int state = 0;
	int i;

	LOG(("Z80DART \"%s\" : Interrupt State A:%d%d%d%d B:%d%d%d%d\n", tag().c_str(),
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

	LOG(("Z80DART \"%s\" : Interrupt State %u\n", tag().c_str(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80dart_device::z80daisy_irq_ack()
{
	int i;

	LOG(("Z80DART \"%s\" Interrupt Acknowledge\n", tag().c_str()));

	// loop over all interrupt sources
	for (i = 0; i < 8; i++)
	{
		// find the first channel with an interrupt requested
		if (m_int_state[i] & Z80_DAISY_INT)
		{
			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[i] = Z80_DAISY_IEO;
			m_chanA->m_rr[0] &= ~z80dart_channel::RR0_INTERRUPT_PENDING;
			check_interrupts();

			LOG(("Z80DART \"%s\" : Interrupt Acknowledge Vector %02x\n", tag().c_str(), m_chanB->m_rr[2]));

			return m_chanB->m_rr[2];
		}
	}

	//logerror("z80dart_irq_ack: failed to find an interrupt to ack!\n");

	return m_chanB->m_rr[2];
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80dart_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80DART \"%s\" Return from Interrupt\n", tag().c_str()));

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

	//logerror("z80dart_irq_reti: failed to find an interrupt to clear IEO on!\n");
}


//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void z80dart_device::check_interrupts()
{
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	m_out_int_cb(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------

void z80dart_device::reset_interrupts()
{
	for (auto & elem : m_int_state)
	{
		elem = 0;
	}

	check_interrupts();
}


//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------

void z80dart_device::trigger_interrupt(int index, int state)
{
	UINT8 vector = m_chanB->m_wr[2];
	int priority;

	if((m_variant == TYPE_I8274) || (m_variant == TYPE_UPD7201))
	{
		int prio_level = 0;
		switch(state)
		{
			case z80dart_channel::INT_TRANSMIT:
				prio_level = 1;
				break;
			case z80dart_channel::INT_RECEIVE:
			case z80dart_channel::INT_SPECIAL:
				prio_level = 0;
				break;
			case z80dart_channel::INT_EXTERNAL:
				prio_level = 2;
				break;
		}

		if(m_chanA->m_wr[2] & z80dart_channel::WR2_PRIORITY)
		{
			priority = (prio_level * 2) + index;
		}
		else
		{
			priority = (prio_level == 2) ? index + 4 : ((index * 2) + prio_level);
		}
		if (m_chanB->m_wr[1] & z80dart_channel::WR1_STATUS_VECTOR)
		{
			vector = (!index << 2) | state;
			if((m_chanA->m_wr[1] & 0x18) == z80dart_channel::WR2_MODE_8086_8088)
			{
				vector = (m_chanB->m_wr[2] & 0xf8) | vector;
			}
			else
			{
				vector = (m_chanB->m_wr[2] & 0xe3) | (vector << 2);
			}
		}
	}
	else
	{
		priority = (index << 2) | state;
		if (m_chanB->m_wr[1] & z80dart_channel::WR1_STATUS_VECTOR)
		{
			// status affects vector
			vector = (m_chanB->m_wr[2] & 0xf1) | (!index << 3) | (state << 1);
		}
	}

	LOG(("Z80DART \"%s\" Channel %c : Interrupt Request %u\n", tag().c_str(), 'A' + index, state));

	// update vector register
	m_chanB->m_rr[2] = vector;

	// trigger interrupt
	m_int_state[priority] |= Z80_DAISY_INT;
	m_chanA->m_rr[0] |= z80dart_channel::RR0_INTERRUPT_PENDING;

	// check for interrupt
	check_interrupts();
}


//-------------------------------------------------
//  m1_r - interrupt acknowledge
//-------------------------------------------------

int z80dart_device::m1_r()
{
	return z80daisy_irq_ack();
}


//-------------------------------------------------
//  cd_ba_r -
//-------------------------------------------------

READ8_MEMBER( z80dart_device::cd_ba_r )
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80dart_channel *channel = ba ? m_chanB : m_chanA;

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  cd_ba_w -
//-------------------------------------------------

WRITE8_MEMBER( z80dart_device::cd_ba_w )
{
	int ba = BIT(offset, 0);
	int cd = BIT(offset, 1);
	z80dart_channel *channel = ba ? m_chanB : m_chanA;

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}


//-------------------------------------------------
//  ba_cd_r -
//-------------------------------------------------

READ8_MEMBER( z80dart_device::ba_cd_r )
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80dart_channel *channel = ba ? m_chanB : m_chanA;

	return cd ? channel->control_read() : channel->data_read();
}


//-------------------------------------------------
//  ba_cd_w -
//-------------------------------------------------

WRITE8_MEMBER( z80dart_device::ba_cd_w )
{
	int ba = BIT(offset, 1);
	int cd = BIT(offset, 0);
	z80dart_channel *channel = ba ? m_chanB : m_chanA;

	if (cd)
		channel->control_write(data);
	else
		channel->data_write(data);
}



//**************************************************************************
//  DART CHANNEL
//**************************************************************************

//-------------------------------------------------
//  dart_channel - constructor
//-------------------------------------------------

z80dart_channel::z80dart_channel(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80DART_CHANNEL, "Z80 DART channel", tag, owner, clock, "z80dart_channel", __FILE__),
		device_serial_interface(mconfig, *this),
		m_rx_error(0),
		m_rx_fifo(-1),
		m_rx_clock(0),
		m_rx_first(0),
		m_rx_break(0),
		m_rx_rr0_latch(0),
		m_rxd(0),
		m_ri(0),
		m_cts(0),
		m_dcd(0),
		m_tx_data(0),
		m_tx_clock(0),
		m_dtr(0),
		m_rts(0),
		m_sync(0)
{
	for (auto & elem : m_rr)
		elem = 0;

	for (auto & elem : m_wr)
		elem = 0;

	for (int i = 0; i < 3; i++)
	{
		m_rx_data_fifo[i] = 0;
		m_rx_error_fifo[i] = 0;
	}
}


//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void z80dart_channel::device_start()
{
	m_uart = downcast<z80dart_device *>(owner());
	m_index = m_uart->get_channel_index(this);

	// state saving
	save_item(NAME(m_rr));
	save_item(NAME(m_wr));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_error));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_rx_break));
	save_item(NAME(m_rx_rr0_latch));
	save_item(NAME(m_ri));
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

void z80dart_channel::device_reset()
{
	receive_register_reset();
	transmit_register_reset();

	// disable receiver
	m_wr[3] &= ~WR3_RX_ENABLE;

	// disable transmitter
	m_wr[5] &= ~WR5_TX_ENABLE;
	m_rr[0] |= RR0_TX_BUFFER_EMPTY;
	m_rr[1] |= RR1_ALL_SENT;

	// reset external lines
	set_rts(1);
	set_dtr(1);

	// reset interrupts
	if (m_index == z80dart_device::CHANNEL_A)
	{
		m_uart->reset_interrupts();
	}
}

void z80dart_channel::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}


//-------------------------------------------------
//  tra_callback -
//-------------------------------------------------

void z80dart_channel::tra_callback()
{
	if (!(m_wr[5] & WR5_TX_ENABLE))
	{
		// transmit mark
		if (m_index == z80dart_device::CHANNEL_A)
			m_uart->m_out_txda_cb(1);
		else
			m_uart->m_out_txdb_cb(1);
	}
	else if (m_wr[5] & WR5_SEND_BREAK)
	{
		// transmit break
		if (m_index == z80dart_device::CHANNEL_A)
			m_uart->m_out_txda_cb(0);
		else
			m_uart->m_out_txdb_cb(0);
	}
	else if (!is_transmit_register_empty())
	{
		// transmit data
		if (m_index == z80dart_device::CHANNEL_A)
			m_uart->m_out_txda_cb(transmit_register_get_data_bit());
		else
			m_uart->m_out_txdb_cb(transmit_register_get_data_bit());
	}
}


//-------------------------------------------------
//  tra_complete -
//-------------------------------------------------

void z80dart_channel::tra_complete()
{
	if ((m_wr[5] & WR5_TX_ENABLE) && !(m_wr[5] & WR5_SEND_BREAK) && !(m_rr[0] & RR0_TX_BUFFER_EMPTY))
	{
		LOG(("Z80DART \"%s\" Channel %c : Transmit Data Byte '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, m_tx_data));

		transmit_register_setup(m_tx_data);

		// empty transmit buffer
		m_rr[0] |= RR0_TX_BUFFER_EMPTY;

		if (m_wr[1] & WR1_TX_INT_ENABLE)
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else if (m_wr[5] & WR5_SEND_BREAK)
	{
		// transmit break
		if (m_index == z80dart_device::CHANNEL_A)
			m_uart->m_out_txda_cb(0);
		else
			m_uart->m_out_txdb_cb(0);
	}
	else
	{
		// transmit mark
		if (m_index == z80dart_device::CHANNEL_A)
			m_uart->m_out_txda_cb(1);
		else
			m_uart->m_out_txdb_cb(1);
	}

	// if transmit buffer is empty
	if (m_rr[0] & RR0_TX_BUFFER_EMPTY)
	{
		// then all characters have been sent
		m_rr[1] |= RR1_ALL_SENT;

		// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
		if (!m_rts)
			set_rts(1);
	}
}


//-------------------------------------------------
//  rcv_callback -
//-------------------------------------------------

void z80dart_channel::rcv_callback()
{
	if (m_wr[3] & WR3_RX_ENABLE)
	{
		receive_register_update_bit(m_rxd);
	}
}


//-------------------------------------------------
//  rcv_complete -
//-------------------------------------------------

void z80dart_channel::rcv_complete()
{
	receive_register_extract();
	receive_data(get_received_char());
}


//-------------------------------------------------
//  get_clock_mode - get clock divisor
//-------------------------------------------------

int z80dart_channel::get_clock_mode()
{
	int clocks = 1;

	switch (m_wr[4] & WR4_CLOCK_RATE_MASK)
	{
	case WR4_CLOCK_RATE_X1:     clocks = 1;     break;
	case WR4_CLOCK_RATE_X16:    clocks = 16;    break;
	case WR4_CLOCK_RATE_X32:    clocks = 32;    break;
	case WR4_CLOCK_RATE_X64:    clocks = 64;    break;
	}

	return clocks;
}


//-------------------------------------------------
//  get_stop_bits - get number of stop bits
//-------------------------------------------------

device_serial_interface::stop_bits_t z80dart_channel::get_stop_bits()
{
	switch (m_wr[4] & WR4_STOP_BITS_MASK)
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

int z80dart_channel::get_rx_word_length()
{
	int bits = 5;

	switch (m_wr[3] & WR3_RX_WORD_LENGTH_MASK)
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

int z80dart_channel::get_tx_word_length()
{
	int bits = 5;

	switch (m_wr[5] & WR5_TX_WORD_LENGTH_MASK)
	{
	case WR5_TX_WORD_LENGTH_5:  bits = 5;   break;
	case WR5_TX_WORD_LENGTH_6:  bits = 6;   break;
	case WR5_TX_WORD_LENGTH_7:  bits = 7;   break;
	case WR5_TX_WORD_LENGTH_8:  bits = 8;   break;
	}

	return bits;
}


//-------------------------------------------------
//  control_read - read control register
//-------------------------------------------------

UINT8 z80dart_channel::control_read()
{
	UINT8 data = 0;

	int reg = m_wr[0] & WR0_REGISTER_MASK;

	if (reg != 0)
	{
		// mask out register index
		m_wr[0] &= ~WR0_REGISTER_MASK;
	}

	switch (reg)
	{
	case 0:
	case 1:
		data = m_rr[reg];
		break;

	case 2:
		// channel B only
		if (m_index == z80dart_device::CHANNEL_B)
			data = m_rr[reg];
		break;
	}

	//LOG(("Z80DART \"%s\" Channel %c : Control Register Read '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------

void z80dart_channel::control_write(UINT8 data)
{
	int reg = m_wr[0] & WR0_REGISTER_MASK;

	LOG(("Z80DART \"%s\" Channel %c : Control Register Write '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, data));

	// write data to selected register
	if (reg < 6)
		m_wr[reg] = data;

	if (reg != 0)
	{
		// mask out register index
		m_wr[0] &= ~WR0_REGISTER_MASK;
	}

	switch (reg)
	{
	case 0:
		switch (data & WR0_COMMAND_MASK)
		{
		case WR0_NULL:
			LOG(("Z80DART \"%s\" Channel %c : Null\n", m_owner->tag().c_str(), 'A' + m_index));
			break;

		case WR0_SEND_ABORT:
			LOG(("Z80DART \"%s\" Channel %c : Send Abort\n", m_owner->tag().c_str(), 'A' + m_index));
			logerror("Z80DART \"%s\" Channel %c : unsupported command: Send Abort\n", m_owner->tag().c_str(), 'A' + m_index);
			break;

		case WR0_RESET_EXT_STATUS:
			// reset external/status interrupt
			m_rr[0] &= ~(RR0_DCD | RR0_RI | RR0_CTS | RR0_BREAK_ABORT);

			if (!m_dcd) m_rr[0] |= RR0_DCD;
			if (m_ri) m_rr[0] |= RR0_RI;
			if (m_cts) m_rr[0] |= RR0_CTS;

			m_rx_rr0_latch = 0;

			LOG(("Z80DART \"%s\" Channel %c : Reset External/Status Interrupt\n", m_owner->tag().c_str(), 'A' + m_index));
			break;

		case WR0_CHANNEL_RESET:
			// channel reset
			LOG(("Z80DART \"%s\" Channel %c : Channel Reset\n", m_owner->tag().c_str(), 'A' + m_index));
			device_reset();
			break;

		case WR0_ENABLE_INT_NEXT_RX:
			// enable interrupt on next receive character
			LOG(("Z80DART \"%s\" Channel %c : Enable Interrupt on Next Received Character\n", m_owner->tag().c_str(), 'A' + m_index));
			m_rx_first = 1;
			break;

		case WR0_RESET_TX_INT:
			// reset transmitter interrupt pending
			LOG(("Z80DART \"%s\" Channel %c : Reset Transmitter Interrupt Pending\n", m_owner->tag().c_str(), 'A' + m_index));
			logerror("Z80DART \"%s\" Channel %c : unsupported command: Reset Transmitter Interrupt Pending\n", m_owner->tag().c_str(), 'A' + m_index);
			break;

		case WR0_ERROR_RESET:
			// error reset
			LOG(("Z80DART \"%s\" Channel %c : Error Reset\n", m_owner->tag().c_str(), 'A' + m_index));
			m_rr[1] &= ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
			break;

		case WR0_RETURN_FROM_INT:
			// return from interrupt
			LOG(("Z80DART \"%s\" Channel %c : Return from Interrupt\n", m_owner->tag().c_str(), 'A' + m_index));
			m_uart->z80daisy_irq_reti();
			break;
		}
		break;

	case 1:
		LOG(("Z80DART \"%s\" Channel %c : External Interrupt Enable %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR1_EXT_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmit Interrupt Enable %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR1_TX_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Status Affects Vector %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR1_STATUS_VECTOR) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready Enable %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR1_WRDY_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready Function %s\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR1_WRDY_FUNCTION) ? "Ready" : "Wait"));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready on %s\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR1_WRDY_ON_RX_TX) ? "Receive" : "Transmit"));

		switch (data & WR1_RX_INT_MODE_MASK)
		{
		case WR1_RX_INT_DISABLE:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt Disabled\n", m_owner->tag().c_str(), 'A' + m_index));
			break;

		case WR1_RX_INT_FIRST:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on First Character\n", m_owner->tag().c_str(), 'A' + m_index));
			break;

		case WR1_RX_INT_ALL_PARITY:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters, Parity Affects Vector\n", m_owner->tag().c_str(), 'A' + m_index));
			break;

		case WR1_RX_INT_ALL:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters\n", m_owner->tag().c_str(), 'A' + m_index));
			break;
		}

		m_uart->check_interrupts();
		break;

	case 2:
		// interrupt vector
		if (m_index == z80dart_device::CHANNEL_B)
		{
			if(m_wr[1] & z80dart_channel::WR1_STATUS_VECTOR)
				m_rr[2] = ( m_rr[2] & 0x0e ) | ( m_wr[2] & 0xF1);
			else
				m_rr[2] = m_wr[2];
		}
		m_uart->check_interrupts();
		LOG(("Z80DART \"%s\" Channel %c : Interrupt Vector %02x\n", m_owner->tag().c_str(), 'A' + m_index, data));
		break;

	case 3:
		LOG(("Z80DART \"%s\" Channel %c : Receiver Enable %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR3_RX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Auto Enables %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR3_AUTO_ENABLES) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Receiver Bits/Character %u\n", m_owner->tag().c_str(), 'A' + m_index, get_rx_word_length()));

		update_serial();
		break;

	case 4:
		LOG(("Z80DART \"%s\" Channel %c : Parity Enable %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR4_PARITY_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Parity %s\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR4_PARITY_EVEN) ? "Even" : "Odd"));
		LOG(("Z80DART \"%s\" Channel %c : Stop Bits %s\n", m_owner->tag().c_str(), 'A' + m_index, stop_bits_tostring(get_stop_bits())));
		LOG(("Z80DART \"%s\" Channel %c : Clock Mode %uX\n", m_owner->tag().c_str(), 'A' + m_index, get_clock_mode()));

		update_serial();
		break;

	case 5:
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Enable %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR5_TX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Bits/Character %u\n", m_owner->tag().c_str(), 'A' + m_index, get_tx_word_length()));
		LOG(("Z80DART \"%s\" Channel %c : Send Break %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR5_SEND_BREAK) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Request to Send %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR5_RTS) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Data Terminal Ready %u\n", m_owner->tag().c_str(), 'A' + m_index, (data & WR5_DTR) ? 1 : 0));

		update_serial();

		if (data & WR5_RTS)
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
		set_dtr((data & WR5_DTR) ? 0 : 1);
		break;

	case 6:
		LOG(("Z80DART \"%s\" Channel %c : Transmit Sync %02x\n", m_owner->tag().c_str(), 'A' + m_index, data));
		m_sync = (m_sync & 0xff00) | data;
		break;

	case 7:
		LOG(("Z80DART \"%s\" Channel %c : Receive Sync %02x\n", m_owner->tag().c_str(), 'A' + m_index, data));
		m_sync = (data << 8) | (m_sync & 0xff);
		break;
	}
}


//-------------------------------------------------
//  data_read - read data register
//-------------------------------------------------

UINT8 z80dart_channel::data_read()
{
	UINT8 data = 0;

	if (m_rx_fifo >= 0)
	{
		// load data from the FIFO
		data = m_rx_data_fifo[m_rx_fifo];

		// load error status from the FIFO
		m_rr[1] = (m_rr[1] & ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR)) | m_rx_error_fifo[m_rx_fifo];

		// decrease FIFO pointer
		m_rx_fifo--;

		if (m_rx_fifo < 0)
		{
			// no more characters available in the FIFO
			m_rr[0] &= ~ RR0_RX_CHAR_AVAILABLE;
		}
	}

	LOG(("Z80DART \"%s\" Channel %c : Data Register Read '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------

void z80dart_channel::data_write(UINT8 data)
{
	m_tx_data = data;

	if ((m_wr[5] & WR5_TX_ENABLE) && is_transmit_register_empty())
	{
		LOG(("Z80DART \"%s\" Channel %c : Transmit Data Byte '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, m_tx_data));

		transmit_register_setup(m_tx_data);

		// empty transmit buffer
		m_rr[0] |= RR0_TX_BUFFER_EMPTY;

		if (m_wr[1] & WR1_TX_INT_ENABLE)
			m_uart->trigger_interrupt(m_index, INT_TRANSMIT);
	}
	else
	{
		m_rr[0] &= ~RR0_TX_BUFFER_EMPTY;
	}

	m_rr[1] &= ~RR1_ALL_SENT;

	LOG(("Z80DART \"%s\" Channel %c : Data Register Write '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, data));
}


//-------------------------------------------------
//  receive_data - receive data word
//-------------------------------------------------

void z80dart_channel::receive_data(UINT8 data)
{
	LOG(("Z80DART \"%s\" Channel %c : Receive Data Byte '%02x'\n", m_owner->tag().c_str(), 'A' + m_index, data));

	if (m_rx_fifo == 2)
	{
		// receive overrun error detected
		m_rx_error |= RR1_RX_OVERRUN_ERROR;

		switch (m_wr[1] & WR1_RX_INT_MODE_MASK)
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

	m_rr[0] |= RR0_RX_CHAR_AVAILABLE;

	// receive interrupt
	switch (m_wr[1] & WR1_RX_INT_MODE_MASK)
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

WRITE_LINE_MEMBER( z80dart_channel::cts_w )
{
	LOG(("Z80DART \"%s\" Channel %c : CTS %u\n", m_owner->tag().c_str(), 'A' + m_index, state));

	if (m_cts != state)
	{
		// enable transmitter if in auto enables mode
		if (!state)
			if (m_wr[3] & WR3_AUTO_ENABLES)
				m_wr[5] |= WR5_TX_ENABLE;

		// set clear to send
		m_cts = state;

		if (!m_rx_rr0_latch)
		{
			if (!m_cts)
				m_rr[0] |= RR0_CTS;
			else
				m_rr[0] &= ~RR0_CTS;

			// trigger interrupt
			if (m_wr[1] & WR1_EXT_INT_ENABLE)
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

WRITE_LINE_MEMBER( z80dart_channel::dcd_w )
{
	LOG(("Z80DART \"%s\" Channel %c : DCD %u\n", m_owner->tag().c_str(), 'A' + m_index, state));

	if (m_dcd != state)
	{
		// enable receiver if in auto enables mode
		if (!state)
			if (m_wr[3] & WR3_AUTO_ENABLES)
				m_wr[3] |= WR3_RX_ENABLE;

		// set data carrier detect
		m_dcd = state;

		if (!m_rx_rr0_latch)
		{
			if (m_dcd)
				m_rr[0] |= RR0_DCD;
			else
				m_rr[0] &= ~RR0_DCD;

			if (m_wr[1] & WR1_EXT_INT_ENABLE)
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
//  ri_w - ring indicator handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80dart_channel::ri_w )
{
	LOG(("Z80DART \"%s\" Channel %c : RI %u\n", m_owner->tag().c_str(), 'A' + m_index, state));

	if (m_ri != state)
	{
		// set ring indicator state
		m_ri = state;

		if (!m_rx_rr0_latch)
		{
			if (m_ri)
				m_rr[0] |= RR0_RI;
			else
				m_rr[0] &= ~RR0_RI;

			if (m_wr[1] & WR1_EXT_INT_ENABLE)
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
//  sync_w - sync handler
//-------------------------------------------------

WRITE_LINE_MEMBER( z80dart_channel::sync_w )
{
	LOG(("Z80DART \"%s\" Channel %c : SYNC %u\n", m_owner->tag().c_str(), 'A' + m_index, state));
}


//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------

WRITE_LINE_MEMBER( z80dart_channel::rxc_w )
{
	//LOG(("Z80DART \"%s\" Channel %c : Receiver Clock Pulse\n", m_owner->tag().c_str(), m_index + 'A'));
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

WRITE_LINE_MEMBER( z80dart_channel::txc_w )
{
	//LOG(("Z80DART \"%s\" Channel %c : Transmitter Clock Pulse\n", m_owner->tag().c_str(), m_index + 'A'));
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

void z80dart_channel::update_serial()
{
	int data_bit_count = get_rx_word_length();
	stop_bits_t stop_bits = get_stop_bits();

	parity_t parity;
	if (m_wr[4] & WR4_PARITY_ENABLE)
	{
		if (m_wr[4] & WR4_PARITY_EVEN)
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

void z80dart_channel::set_dtr(int state)
{
	m_dtr = state;

	if (m_index == z80dart_device::CHANNEL_A)
		m_uart->m_out_dtra_cb(m_dtr);
	else
		m_uart->m_out_dtrb_cb(m_dtr);
}


//-------------------------------------------------
//  set_rts -
//-------------------------------------------------

void z80dart_channel::set_rts(int state)
{
	if (m_index == z80dart_device::CHANNEL_A)
		m_uart->m_out_rtsa_cb(state);
	else
		m_uart->m_out_rtsb_cb(state);
}


//-------------------------------------------------
//  write_rx -
//-------------------------------------------------

WRITE_LINE_MEMBER(z80dart_channel::write_rx)
{
	m_rxd = state;
	//only use rx_w when self-clocked
	if(m_rxc)
		device_serial_interface::rx_w(state);
}
