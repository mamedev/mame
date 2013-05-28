/***************************************************************************

    Intel 8274 Multi-Protocol Serial Controller emulation
    NEC uPD7201 Multiprotocol Serial Communications Controller emulation
    Z80-DART Dual Asynchronous Receiver/Transmitter emulation
    Z80-SIO/0/1/2/3/4 Serial Input/Output Controller emulation

    Copyright (c) 2008, The MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

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

#include "emu.h"
#include "z80dart.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define CHANA_TAG   "cha"
#define CHANB_TAG   "chb"



//**************************************************************************
//  MACROS
//**************************************************************************

#define RXD \
	m_in_rxd_func()

#define TXD(_state) \
	m_out_txd_func(_state)

#define RTS(_state) \
	m_out_rts_func(_state)

#define DTR(_state) \
	m_out_dtr_func(_state)



//**************************************************************************
//  LIVE DEVICE
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


MACHINE_CONFIG_FRAGMENT( z80dart )
	MCFG_DEVICE_ADD(CHANA_TAG, Z80DART_CHANNEL, 0)
	MCFG_DEVICE_ADD(CHANB_TAG, Z80DART_CHANNEL, 0)
MACHINE_CONFIG_END

machine_config_constructor z80dart_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( z80dart );
}


//-------------------------------------------------
//  z80dart_device - constructor
//-------------------------------------------------

z80dart_device::z80dart_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_z80daisy_interface(mconfig, *this),
		m_chanA(*this, CHANA_TAG),
		m_chanB(*this, CHANB_TAG),
		m_variant(variant)
{
	for (int i = 0; i < 8; i++)
		m_int_state[i] = 0;
}

z80dart_device::z80dart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80DART, "Z80-DART", tag, owner, clock, "z80dart", __FILE__),
		device_z80daisy_interface(mconfig, *this),
		m_chanA(*this, CHANA_TAG),
		m_chanB(*this, CHANB_TAG),
		m_variant(TYPE_DART)
{
	for (int i = 0; i < 8; i++)
		m_int_state[i] = 0;
}

z80sio0_device::z80sio0_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO0, "Z80-SIO/0", tag, owner, clock, TYPE_SIO0, "z80sio0", __FILE__)
{
}

z80sio1_device::z80sio1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO1, "Z80-SIO/1", tag, owner, clock, TYPE_SIO1, "z80sio1", __FILE__)
{
}

z80sio2_device::z80sio2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO2, "Z80-SIO/2", tag, owner, clock, TYPE_SIO2, "z80sio2", __FILE__)
{
}

z80sio3_device::z80sio3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO3, "Z80-SIO/3", tag, owner, clock, TYPE_SIO3, "z80sio3", __FILE__)
{
}

z80sio4_device::z80sio4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, Z80SIO4, "Z80-SIO/4", tag, owner, clock, TYPE_SIO4, "z80sio4", __FILE__)
{
}

i8274_device::i8274_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, I8274, "I8274", tag, owner, clock, TYPE_I8274, "i8274", __FILE__)
{
}

upd7201_device::upd7201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80dart_device(mconfig, UPD7201, "uPD7201", tag, owner, clock, TYPE_UPD7201, "upd7201", __FILE__)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void z80dart_device::device_config_complete()
{
	// inherit a copy of the static data
	const z80dart_interface *intf = reinterpret_cast<const z80dart_interface *>(static_config());
	if (intf != NULL)
		*static_cast<z80dart_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		m_rxca = m_txca = m_rxcb = m_txcb = 0;

		memset(&m_in_rxda_cb, 0, sizeof(m_in_rxda_cb));
		memset(&m_out_txda_cb, 0, sizeof(m_out_txda_cb));
		memset(&m_out_dtra_cb, 0, sizeof(m_out_dtra_cb));
		memset(&m_out_rtsa_cb, 0, sizeof(m_out_rtsa_cb));
		memset(&m_out_wrdya_cb, 0, sizeof(m_out_wrdya_cb));
		memset(&m_out_synca_cb, 0, sizeof(m_out_synca_cb));
		memset(&m_in_rxdb_cb, 0, sizeof(m_in_rxdb_cb));
		memset(&m_out_txdb_cb, 0, sizeof(m_out_txdb_cb));
		memset(&m_out_dtrb_cb, 0, sizeof(m_out_dtrb_cb));
		memset(&m_out_rtsb_cb, 0, sizeof(m_out_rtsb_cb));
		memset(&m_out_wrdyb_cb, 0, sizeof(m_out_wrdyb_cb));
		memset(&m_out_syncb_cb, 0, sizeof(m_out_syncb_cb));
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
		memset(&m_out_rxdrqa_cb, 0, sizeof(m_out_rxdrqa_cb));
		memset(&m_out_txdrqa_cb, 0, sizeof(m_out_txdrqa_cb));
		memset(&m_out_rxdrqb_cb, 0, sizeof(m_out_rxdrqb_cb));
		memset(&m_out_txdrqb_cb, 0, sizeof(m_out_txdrqb_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80dart_device::device_start()
{
	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);

	// configure channel A
	m_chanA->m_rxc = m_rxca;
	m_chanA->m_txc = m_txca;

	m_chanA->m_in_rxd_cb = m_in_rxda_cb;
	m_chanA->m_out_txd_cb = m_out_txda_cb;
	m_chanA->m_out_dtr_cb = m_out_dtra_cb;
	m_chanA->m_out_rts_cb = m_out_rtsa_cb;
	m_chanA->m_out_wrdy_cb = m_out_wrdya_cb;
	m_chanA->m_out_sync_cb = m_out_synca_cb;
	m_chanA->m_out_rxdrq_cb = m_out_rxdrqa_cb;
	m_chanA->m_out_txdrq_cb = m_out_txdrqa_cb;

	// configure channel B
	m_chanB->m_rxc = m_rxcb;
	m_chanB->m_txc = m_txcb;

	m_chanB->m_in_rxd_cb = m_in_rxdb_cb;
	m_chanB->m_in_rxd_cb = m_in_rxdb_cb;
	m_chanB->m_out_txd_cb = m_out_txdb_cb;
	m_chanB->m_out_dtr_cb = m_out_dtrb_cb;
	m_chanB->m_out_rts_cb = m_out_rtsb_cb;
	m_chanB->m_out_wrdy_cb = m_out_wrdyb_cb;
	m_chanB->m_out_sync_cb = m_out_syncb_cb;
	m_chanB->m_out_rxdrq_cb = m_out_rxdrqb_cb;
	m_chanB->m_out_txdrq_cb = m_out_txdrqb_cb;

	// state saving
	save_item(NAME(m_int_state));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80dart_device::device_reset()
{
	LOG(("Z80DART \"%s\" Reset\n", tag()));

	m_chanA->reset();
	m_chanB->reset();
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - get interrupt status
//-------------------------------------------------

int z80dart_device::z80daisy_irq_state()
{
	int state = 0;
	int i;

	LOG(("Z80DART \"%s\" : Interrupt State A:%d%d%d%d B:%d%d%d%d\n", tag(),
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

	LOG(("Z80DART \"%s\" : Interrupt State %u\n", tag(), state));

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - interrupt acknowledge
//-------------------------------------------------

int z80dart_device::z80daisy_irq_ack()
{
	int i;

	LOG(("Z80DART \"%s\" Interrupt Acknowledge\n", tag()));

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

			LOG(("Z80DART \"%s\" : Interrupt Acknowledge Vector %02x\n", tag(), m_chanB->m_rr[2]));

			return m_chanB->m_rr[2];
		}
	}

	logerror("z80dart_irq_ack: failed to find an interrupt to ack!\n");

	return m_chanB->m_rr[2];
}


//-------------------------------------------------
//  z80daisy_irq_reti - return from interrupt
//-------------------------------------------------

void z80dart_device::z80daisy_irq_reti()
{
	int i;

	LOG(("Z80DART \"%s\" Return from Interrupt\n", tag()));

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

	logerror("z80dart_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  check_interrupts -
//-------------------------------------------------

void z80dart_device::check_interrupts()
{
	int state = (z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;
	m_out_int_func(state);
}


//-------------------------------------------------
//  reset_interrupts -
//-------------------------------------------------

void z80dart_device::reset_interrupts()
{
	for (int i = 0; i < 8; i++)
	{
		m_int_state[i] = 0;
	}

	check_interrupts();
}


//-------------------------------------------------
//  trigger_interrupt -
//-------------------------------------------------

void z80dart_device::trigger_interrupt(int index, int state)
{
	UINT8 vector = m_chanB->m_wr[2];
	int priority = (index << 2) | state;

	LOG(("Z80DART \"%s\" Channel %c : Interrupt Request %u\n", tag(), 'A' + index, state));

	if ((index == CHANNEL_B) && (m_chanB->m_wr[1] & z80dart_channel::WR1_STATUS_VECTOR))
	{
		// status affects vector
		vector = (m_chanB->m_wr[2] & 0xf1) | (!index << 3) | (state << 1);
	}

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



//**************************************************************************
//  DART CHANNEL
//**************************************************************************

//-------------------------------------------------
//  dart_channel - constructor
//-------------------------------------------------

z80dart_channel::z80dart_channel(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80DART_CHANNEL, "Z80-DART channel", tag, owner, clock),
		m_rx_shift(0),
		m_rx_error(0),
		m_rx_fifo(-1),
		m_rx_clock(0),
		m_rx_state(0),
		m_rx_bits(0),
		m_rx_first(0),
		m_rx_parity(0),
		m_rx_break(0),
		m_rx_rr0_latch(0),
		m_ri(0),
		m_cts(0),
		m_dcd(0),
		m_tx_data(0),
		m_tx_shift(0),
		m_tx_clock(0),
		m_tx_state(0),
		m_tx_bits(0),
		m_tx_parity(0),
		m_dtr(0),
		m_rts(0),
		m_sync(0)
{
	memset(&m_in_rxd_func, 0, sizeof(m_in_rxd_func));
	memset(&m_out_txd_func, 0, sizeof(m_out_txd_func));
	memset(&m_out_dtr_func, 0, sizeof(m_out_dtr_func));
	memset(&m_out_rts_func, 0, sizeof(m_out_rts_func));
	memset(&m_out_wrdy_func, 0, sizeof(m_out_wrdy_func));
	memset(&m_rr, 0, sizeof(m_rr));
	memset(&m_wr, 0, sizeof(m_wr));
	memset(&m_rx_data_fifo, 0, sizeof(m_rx_data_fifo));
	memset(&m_rx_error_fifo, 0, sizeof(m_rx_error_fifo));
}


//-------------------------------------------------
//  start - channel startup
//-------------------------------------------------

void z80dart_channel::device_start()
{
	m_uart = downcast<z80dart_device *>(owner());
	m_index = m_uart->get_channel_index(this);

	// resolve callbacks
	m_in_rxd_func.resolve(m_in_rxd_cb, *m_uart);
	m_out_txd_func.resolve(m_out_txd_cb, *m_uart);
	m_out_dtr_func.resolve(m_out_dtr_cb, *m_uart);
	m_out_rts_func.resolve(m_out_rts_cb, *m_uart);
	m_out_wrdy_func.resolve(m_out_wrdy_cb, *m_uart);
	m_out_sync_func.resolve(m_out_sync_cb, *m_uart);
	m_out_rxdrq_func.resolve(m_out_rxdrq_cb, *m_uart);
	m_out_txdrq_func.resolve(m_out_txdrq_cb, *m_uart);

	// allocate timers
	if (m_rxc > 0)
	{
		m_rx_timer = timer_alloc(TIMER_RX);
		m_rx_timer->adjust(attotime::from_hz(m_rxc), 0, attotime::from_hz(m_rxc));
	}

	if (m_txc > 0)
	{
		m_tx_timer = timer_alloc(TIMER_TX);
		m_tx_timer->adjust(attotime::from_hz(m_txc), 0, attotime::from_hz(m_txc));
	}

	// state saving
	save_item(NAME(m_rr));
	save_item(NAME(m_wr));
	save_item(NAME(m_rx_data_fifo));
	save_item(NAME(m_rx_error_fifo));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_error));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_clock));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_first));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_rx_break));
	save_item(NAME(m_rx_rr0_latch));
	save_item(NAME(m_ri));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_tx_data));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_clock));
	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_dtr));
	save_item(NAME(m_rts));
	save_item(NAME(m_sync));
}


//-------------------------------------------------
//  reset - reset channel status
//-------------------------------------------------

void z80dart_channel::device_reset()
{
	// disable receiver
	m_wr[3] &= ~WR3_RX_ENABLE;
	m_rx_state = STATE_START;

	// disable transmitter
	m_wr[5] &= ~WR5_TX_ENABLE;
	m_tx_state = STATE_START;
	m_rr[0] |= RR0_TX_BUFFER_EMPTY;

	// reset external lines
	RTS(1);
	DTR(1);

	// reset interrupts
	if (m_index == z80dart_device::CHANNEL_A)
	{
		m_uart->reset_interrupts();
	}
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void z80dart_channel::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RX:
		rxc_w(1);
		break;

	case TIMER_TX:
		txc_w(1);
		break;
	}
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

float z80dart_channel::get_stop_bits()
{
	float bits = 1;

	switch (m_wr[4] & WR4_STOP_BITS_MASK)
	{
	case WR4_STOP_BITS_1:       bits = 1;       break;
	case WR4_STOP_BITS_1_5:     bits = 1.5;     break;
	case WR4_STOP_BITS_2:       bits = 2;       break;
	}

	return bits;
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
//  detect_start_bit - detect start bit
//-------------------------------------------------

int z80dart_channel::detect_start_bit()
{
	if (!(m_wr[3] & WR3_RX_ENABLE)) return 0;

	return !RXD;
}


//-------------------------------------------------
//  shift_data_in - shift in serial data
//-------------------------------------------------

void z80dart_channel::shift_data_in()
{
	if (m_rx_bits < 8)
	{
		int rxd = RXD;

		m_rx_shift >>= 1;
		m_rx_shift = (rxd << 7) | (m_rx_shift & 0x7f);
		m_rx_parity ^= rxd;
		m_rx_bits++;
	}
}


//-------------------------------------------------
//  character_completed - check if complete
//  data word has been transferred
//-------------------------------------------------

bool z80dart_channel::character_completed()
{
	return m_rx_bits == get_rx_word_length();
}


//-------------------------------------------------
//  detect_parity_error - detect parity error
//-------------------------------------------------

void z80dart_channel::detect_parity_error()
{
	int parity = (m_wr[1] & WR4_PARITY_EVEN) ? 1 : 0;

	if (RXD != (m_rx_parity ^ parity))
	{
		// parity error detected
		m_rx_error |= RR1_PARITY_ERROR;

		switch (m_wr[1] & WR1_RX_INT_MODE_MASK)
		{
		case WR1_RX_INT_FIRST:
			if (!m_rx_first)
			{
				m_uart->trigger_interrupt(m_index, INT_SPECIAL);
			}
			break;

		case WR1_RX_INT_ALL_PARITY:
			m_uart->trigger_interrupt(m_index, INT_SPECIAL);
			break;

		case WR1_RX_INT_ALL:
			m_uart->trigger_interrupt(m_index, INT_RECEIVE);
			break;
		}
	}
}


//-------------------------------------------------
//  detect_framing_error - detect framing error
//-------------------------------------------------

void z80dart_channel::detect_framing_error()
{
	if (!RXD)
	{
		// framing error detected
		m_rx_error |= RR1_CRC_FRAMING_ERROR;

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
}


//-------------------------------------------------
//  receive - receive serial data
//-------------------------------------------------

void z80dart_channel::receive()
{
	float stop_bits = get_stop_bits();

	switch (m_rx_state)
	{
	case STATE_START:
		// check for start bit
		if (detect_start_bit())
		{
			// start bit detected
			m_rx_shift = 0;
			m_rx_error = 0;
			m_rx_bits = 0;
			m_rx_parity = 0;

			// next bit is a data bit
			m_rx_state = STATE_DATA;
		}
		break;

	case STATE_DATA:
		// shift bit into shift register
		shift_data_in();

		if (character_completed())
		{
			// all data bits received
			if (m_wr[4] & WR4_PARITY_ENABLE)
			{
				// next bit is the parity bit
				m_rx_state = STATE_PARITY;
			}
			else
			{
				// next bit is a STOP bit
				if (stop_bits == 1)
					m_rx_state = STATE_STOP2;
				else
					m_rx_state = STATE_STOP;
			}
		}
		break;

	case STATE_PARITY:
		// shift bit into shift register
		shift_data_in();

		// check for parity error
		detect_parity_error();

		// next bit is a STOP bit
		if (stop_bits == 1)
			m_rx_state = STATE_STOP2;
		else
			m_rx_state = STATE_STOP;
		break;

	case STATE_STOP:
		// shift bit into shift register
		shift_data_in();

		// check for framing error
		detect_framing_error();

		// next bit is the second STOP bit
		m_rx_state = STATE_STOP2;
		break;

	case STATE_STOP2:
		// shift bit into shift register
		shift_data_in();

		// check for framing error
		detect_framing_error();

		// store data into FIFO
		receive_data(m_rx_shift);

		// next bit is the START bit
		m_rx_state = STATE_START;
		break;
	}
}


//-------------------------------------------------
//  transmit - transmit serial data
//-------------------------------------------------

void z80dart_channel::transmit()
{
	int word_length = get_tx_word_length();
	float stop_bits = get_stop_bits();

	switch (m_tx_state)
	{
	case STATE_START:
		if ((m_wr[5] & WR5_TX_ENABLE) && !(m_rr[0] & RR0_TX_BUFFER_EMPTY))
		{
			// transmit start bit
			TXD(0);

			m_tx_bits = 0;
			m_tx_shift = m_tx_data;

			// empty transmit buffer
			m_rr[0] |= RR0_TX_BUFFER_EMPTY;

			if (m_wr[1] & WR1_TX_INT_ENABLE)
				m_uart->trigger_interrupt(m_index, INT_TRANSMIT);

			m_tx_state = STATE_DATA;
		}
		else if (m_wr[5] & WR5_SEND_BREAK)
		{
			// transmit break
			TXD(0);
		}
		else
		{
			// transmit marking line
			TXD(1);
		}

		break;

	case STATE_DATA:
		// transmit data bit
		TXD(BIT(m_tx_shift, 0));

		// shift data
		m_tx_shift >>= 1;
		m_tx_bits++;

		if (m_tx_bits == word_length)
		{
			if (m_wr[4] & WR4_PARITY_ENABLE)
				m_tx_state = STATE_PARITY;
			else
			{
				if (stop_bits == 1)
					m_tx_state = STATE_STOP2;
				else
					m_tx_state = STATE_STOP;
			}
		}
		break;

	case STATE_PARITY:
		// TODO: calculate parity
		if (stop_bits == 1)
			m_tx_state = STATE_STOP2;
		else
			m_tx_state = STATE_STOP;
		break;

	case STATE_STOP:
		// transmit stop bit
		TXD(1);

		m_tx_state = STATE_STOP2;
		break;

	case STATE_STOP2:
		// transmit stop bit
		TXD(1);

		// if transmit buffer is empty
		if (m_rr[0] & RR0_TX_BUFFER_EMPTY)
		{
			// then all characters have been sent
			m_rr[1] |= RR1_ALL_SENT;

			// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
			if (!m_rts)
				RTS(1);
		}

		m_tx_state = STATE_START;
		break;
	}
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

	LOG(("Z80DART \"%s\" Channel %c : Control Register Read '%02x'\n", m_owner->tag(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  control_write - write control register
//-------------------------------------------------

void z80dart_channel::control_write(UINT8 data)
{
	int reg = m_wr[0] & WR0_REGISTER_MASK;

	LOG(("Z80DART \"%s\" Channel %c : Control Register Write '%02x'\n", m_owner->tag(), 'A' + m_index, data));

	// write data to selected register
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
			LOG(("Z80DART \"%s\" Channel %c : Null\n", m_owner->tag(), 'A' + m_index));
			break;

		case WR0_SEND_ABORT:
			LOG(("Z80DART \"%s\" Channel %c : Send Abort\n", m_owner->tag(), 'A' + m_index));
			logerror("Z80DART \"%s\" Channel %c : unsupported command: Send Abort\n", m_owner->tag(), 'A' + m_index);
			break;

		case WR0_RESET_EXT_STATUS:
			// reset external/status interrupt
			m_rr[0] &= ~(RR0_DCD | RR0_RI | RR0_CTS | RR0_BREAK_ABORT);

			if (!m_dcd) m_rr[0] |= RR0_DCD;
			if (m_ri) m_rr[0] |= RR0_RI;
			if (m_cts) m_rr[0] |= RR0_CTS;

			m_rx_rr0_latch = 0;

			LOG(("Z80DART \"%s\" Channel %c : Reset External/Status Interrupt\n", m_owner->tag(), 'A' + m_index));
			break;

		case WR0_CHANNEL_RESET:
			// channel reset
			LOG(("Z80DART \"%s\" Channel %c : Channel Reset\n", m_owner->tag(), 'A' + m_index));
			device_reset();
			break;

		case WR0_ENABLE_INT_NEXT_RX:
			// enable interrupt on next receive character
			LOG(("Z80DART \"%s\" Channel %c : Enable Interrupt on Next Received Character\n", m_owner->tag(), 'A' + m_index));
			m_rx_first = 1;
			break;

		case WR0_RESET_TX_INT:
			// reset transmitter interrupt pending
			LOG(("Z80DART \"%s\" Channel %c : Reset Transmitter Interrupt Pending\n", m_owner->tag(), 'A' + m_index));
			logerror("Z80DART \"%s\" Channel %c : unsupported command: Reset Transmitter Interrupt Pending\n", m_owner->tag(), 'A' + m_index);
			break;

		case WR0_ERROR_RESET:
			// error reset
			LOG(("Z80DART \"%s\" Channel %c : Error Reset\n", m_owner->tag(), 'A' + m_index));
			m_rr[1] &= ~(RR1_CRC_FRAMING_ERROR | RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR);
			break;

		case WR0_RETURN_FROM_INT:
			// return from interrupt
			LOG(("Z80DART \"%s\" Channel %c : Return from Interrupt\n", m_owner->tag(), 'A' + m_index));
			m_uart->z80daisy_irq_reti();
			break;
		}
		break;

	case 1:
		LOG(("Z80DART \"%s\" Channel %c : External Interrupt Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_EXT_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmit Interrupt Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_TX_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Status Affects Vector %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_STATUS_VECTOR) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR1_WRDY_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready Function %s\n", m_owner->tag(), 'A' + m_index, (data & WR1_WRDY_FUNCTION) ? "Ready" : "Wait"));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready on %s\n", m_owner->tag(), 'A' + m_index, (data & WR1_WRDY_ON_RX_TX) ? "Receive" : "Transmit"));

		switch (data & WR1_RX_INT_MODE_MASK)
		{
		case WR1_RX_INT_DISABLE:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt Disabled\n", m_owner->tag(), 'A' + m_index));
			break;

		case WR1_RX_INT_FIRST:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on First Character\n", m_owner->tag(), 'A' + m_index));
			break;

		case WR1_RX_INT_ALL_PARITY:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters, Parity Affects Vector\n", m_owner->tag(), 'A' + m_index));
			break;

		case WR1_RX_INT_ALL:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters\n", m_owner->tag(), 'A' + m_index));
			break;
		}

		m_uart->check_interrupts();
		break;

	case 2:
		// interrupt vector
		if (m_index == z80dart_device::CHANNEL_B)
			m_rr[2] = ( m_rr[2] & 0x0e ) | ( m_wr[2] & 0xF1);;

		m_uart->check_interrupts();
		LOG(("Z80DART \"%s\" Channel %c : Interrupt Vector %02x\n", m_owner->tag(), 'A' + m_index, data));
		break;

	case 3:
		LOG(("Z80DART \"%s\" Channel %c : Receiver Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR3_RX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Auto Enables %u\n", m_owner->tag(), 'A' + m_index, (data & WR3_AUTO_ENABLES) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Receiver Bits/Character %u\n", m_owner->tag(), 'A' + m_index, get_rx_word_length()));
		break;

	case 4:
		LOG(("Z80DART \"%s\" Channel %c : Parity Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR4_PARITY_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Parity %s\n", m_owner->tag(), 'A' + m_index, (data & WR4_PARITY_EVEN) ? "Even" : "Odd"));
		LOG(("Z80DART \"%s\" Channel %c : Stop Bits %f\n", m_owner->tag(), 'A' + m_index, get_stop_bits()));
		LOG(("Z80DART \"%s\" Channel %c : Clock Mode %uX\n", m_owner->tag(), 'A' + m_index, get_clock_mode()));
		break;

	case 5:
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Enable %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_TX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Bits/Character %u\n", m_owner->tag(), 'A' + m_index, get_tx_word_length()));
		LOG(("Z80DART \"%s\" Channel %c : Send Break %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_SEND_BREAK) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Request to Send %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_RTS) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Data Terminal Ready %u\n", m_owner->tag(), 'A' + m_index, (data & WR5_DTR) ? 1 : 0));

		if (data & WR5_RTS)
		{
			// when the RTS bit is set, the _RTS output goes low
			RTS(0);

			m_rts = 1;
		}
		else
		{
			// when the RTS bit is reset, the _RTS output goes high after the transmitter empties
			m_rts = 0;
		}

		// data terminal ready output follows the state programmed into the DTR bit*/
		m_dtr = (data & WR5_DTR) ? 0 : 1;
		DTR(m_dtr);
		break;

	case 6:
		LOG(("Z80DART \"%s\" Channel %c : Transmit Sync %02x\n", m_owner->tag(), 'A' + m_index, data));
		m_sync = (m_sync & 0xff00) | data;
		break;

	case 7:
		LOG(("Z80DART \"%s\" Channel %c : Receive Sync %02x\n", m_owner->tag(), 'A' + m_index, data));
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

		// load error status from the FIFO, retain overrun and parity errors
		m_rr[1] = (m_rr[1] & (RR1_RX_OVERRUN_ERROR | RR1_PARITY_ERROR)) | m_rx_error_fifo[m_rx_fifo];

		// decrease FIFO pointer
		m_rx_fifo--;

		if (m_rx_fifo < 0)
		{
			// no more characters available in the FIFO
			m_rr[0] &= ~ RR0_RX_CHAR_AVAILABLE;
		}
	}

	LOG(("Z80DART \"%s\" Channel %c : Data Register Read '%02x'\n", m_owner->tag(), 'A' + m_index, data));

	return data;
}


//-------------------------------------------------
//  data_write - write data register
//-------------------------------------------------

void z80dart_channel::data_write(UINT8 data)
{
	m_tx_data = data;

	m_rr[0] &= ~RR0_TX_BUFFER_EMPTY;
	m_rr[1] &= ~RR1_ALL_SENT;

	LOG(("Z80DART \"%s\" Channel %c : Data Register Write '%02x'\n", m_owner->tag(), 'A' + m_index, data));
}


//-------------------------------------------------
//  receive_data - receive data word
//-------------------------------------------------

void z80dart_channel::receive_data(UINT8 data)
{
	LOG(("Z80DART \"%s\" Channel %c : Receive Data Byte '%02x'\n", m_owner->tag(), 'A' + m_index, data));

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

void z80dart_channel::cts_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : CTS %u\n", m_owner->tag(), 'A' + m_index, state));

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

void z80dart_channel::dcd_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : DCD %u\n", m_owner->tag(), 'A' + m_index, state));

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

void z80dart_channel::ri_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : RI %u\n", m_owner->tag(), 'A' + m_index, state));

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

void z80dart_channel::sync_w(int state)
{
	LOG(("Z80DART \"%s\" Channel %c : SYNC %u\n", m_owner->tag(), 'A' + m_index, state));
}


//-------------------------------------------------
//  rxc_w - receive clock
//-------------------------------------------------

void z80dart_channel::rxc_w(int state)
{
	int clocks = get_clock_mode();

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel %c : Receiver Clock Pulse\n", m_owner->tag(), m_index + 'A'));

	m_rx_clock++;
	if (m_rx_clock == clocks)
	{
		m_rx_clock = 0;
		receive();
	}
}


//-------------------------------------------------
//  txc_w - transmit clock
//-------------------------------------------------

void z80dart_channel::txc_w(int state)
{
	int clocks = get_clock_mode();

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel %c : Transmitter Clock Pulse\n", m_owner->tag(), m_index + 'A'));

	m_tx_clock++;
	if (m_tx_clock == clocks)
	{
		m_tx_clock = 0;
		transmit();
	}
}



//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_MEMBER( z80dart_device::cd_ba_r )
{
	return (offset & 2) ? control_read(offset & 1) : data_read(offset & 1);
}

WRITE8_MEMBER( z80dart_device::cd_ba_w )
{
	if (offset & 2)
		control_write(offset & 1, data);
	else
		data_write(offset & 1, data);
}

READ8_MEMBER( z80dart_device::ba_cd_r )
{
	int channel = BIT(offset, 1);

	return (offset & 1) ? control_read(channel) : data_read(channel);
}

WRITE8_MEMBER( z80dart_device::ba_cd_w )
{
	int channel = BIT(offset, 1);

	if (offset & 1)
		control_write(channel, data);
	else
		data_write(channel, data);
}
