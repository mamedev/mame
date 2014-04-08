/***************************************************************************

    Z80 SIO (Z8440) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "z80sio.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"


// device type definition
const device_type Z80SIO = &device_creator<z80sio_device>;


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE     0

#define VPRINTF(x) do { if (VERBOSE) logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// interrupt states
const int INT_TRANSMIT      = 0x00;     // not confirmed
const int INT_STATUS        = 0x01;
const int INT_RECEIVE       = 0x02;
const int INT_ERROR         = 0x03;

const int INT_CHB_TRANSMIT  = 0 + INT_TRANSMIT;
const int INT_CHB_STATUS    = 0 + INT_STATUS;
const int INT_CHB_RECEIVE   = 0 + INT_RECEIVE;
const int INT_CHB_ERROR     = 0 + INT_ERROR;
const int INT_CHA_TRANSMIT  = 4 + INT_TRANSMIT;
const int INT_CHA_STATUS    = 4 + INT_STATUS;
const int INT_CHA_RECEIVE   = 4 + INT_RECEIVE;
const int INT_CHA_ERROR     = 4 + INT_ERROR;

// SIO write register 0
//const int SIO_WR0_RESET_MASK                = 0xc0;     // D7-D6: Reset control
//const int SIO_WR0_RESET_NULL                =   0x00;   //  00 = NULL code
//const int SIO_WR0_RESET_RX_CRC              =   0x40;   //  01 = Reset Rx CRC checker
//const int SIO_WR0_RESET_TX_CRC              =   0x80;   //  10 = Reset Tx CRC generator
//const int SIO_WR0_RESET_TX_LATCH            =   0xc0;   //  11 = Reset Tx Underrun/EOM latch
const int SIO_WR0_COMMAND_MASK              = 0x38;     // D5-D3: Command
//const int SIO_WR0_COMMAND_NULL              =   0x00;   //  000 = NULL code
//const int SIO_WR0_COMMAND_SET_ABORT         =   0x08;   //  001 = Set abort (SDLC)
const int SIO_WR0_COMMAND_RES_STATUS_INT    =   0x10;   //  010 = reset ext/status interrupts
const int SIO_WR0_COMMAND_CH_RESET          =   0x18;   //  011 = Channel reset
const int SIO_WR0_COMMAND_ENA_RX_INT        =   0x20;   //  100 = Enable int on next Rx character
const int SIO_WR0_COMMAND_RES_TX_INT        =   0x28;   //  101 = Reset Tx int pending
const int SIO_WR0_COMMAND_RES_ERROR         =   0x30;   //  110 = Error reset
//const int SIO_WR0_COMMAND_RETI              =   0x38;   //  111 = Return from int (CH-A only)
//const int SIO_WR0_REGISTER_MASK             = 0x07;     // D2-D0: Register select (0-7)

// SIO write register 1
//const int SIO_WR1_READY_WAIT_ENA            = 0x80;     // D7 = READY/WAIT enable
//const int SIO_WR1_READY_WAIT_FUNCTION       = 0x40;     // D6 = READY/WAIT function
//const int SIO_WR1_READY_WAIT_ON_RT          = 0x20;     // D5 = READY/WAIT on R/T
const int SIO_WR1_RXINT_MASK                = 0x18;     // D4-D3 = Rx int control
//const int SIO_WR1_RXINT_DISABLE             =   0x00;   //  00 = Rx int disable
const int SIO_WR1_RXINT_FIRST               =   0x08;   //  01 = Rx int on first character
const int SIO_WR1_RXINT_ALL_PARITY          =   0x10;   //  10 = int on all Rx characters (parity affects vector)
const int SIO_WR1_RXINT_ALL_NOPARITY        =   0x18;   //  11 = int on all Rx characters (parity ignored)
//const int SIO_WR1_STATUS_AFFECTS_VECTOR     = 0x04;     // D2 = Status affects vector (CH-B only)
const int SIO_WR1_TXINT_ENABLE              = 0x02;     // D1 = Tx int enable
const int SIO_WR1_STATUSINT_ENABLE          = 0x01;     // D0 = Ext int enable

// SIO write register 2 (CH-B only)
//const int SIO_WR2_INT_VECTOR_MASK           = 0xff;     // D7-D0 = interrupt vector

// SIO write register 3
//const int SIO_WR3_RX_DATABITS_MASK          = 0xc0;     // D7-D6 = Rx Data bits
//const int SIO_WR3_RX_DATABITS_5             =   0x00;   //  00 = Rx 5 bits/character
//const int SIO_WR3_RX_DATABITS_7             =   0x40;   //  01 = Rx 7 bits/character
//const int SIO_WR3_RX_DATABITS_6             =   0x80;   //  10 = Rx 6 bits/character
//const int SIO_WR3_RX_DATABITS_8             =   0xc0;   //  11 = Rx 8 bits/character
//const int SIO_WR3_AUTO_ENABLES              = 0x20;     // D5 = Auto enables
//const int SIO_WR3_ENTER_HUNT_PHASE          = 0x10;     // D4 = Enter hunt phase
//const int SIO_WR3_RX_CRC_ENABLE             = 0x08;     // D3 = Rx CRC enable
//const int SIO_WR3_ADDR_SEARCH_MODE          = 0x04;     // D2 = Address search mode (SDLC)
//const int SIO_WR3_SYNC_LOAD_INHIBIT         = 0x02;     // D1 = Sync character load inhibit
const int SIO_WR3_RX_ENABLE                 = 0x01;     // D0 = Rx enable

// SIO write register 4
//const int SIO_WR4_CLOCK_MODE_MASK           = 0xc0;     // D7-D6 = Clock mode
//const int SIO_WR4_CLOCK_MODE_x1             =   0x00;   //  00 = x1 clock mode
//const int SIO_WR4_CLOCK_MODE_x16            =   0x40;   //  01 = x16 clock mode
//const int SIO_WR4_CLOCK_MODE_x32            =   0x80;   //  10 = x32 clock mode
//const int SIO_WR4_CLOCK_MODE_x64            =   0xc0;   //  11 = x64 clock mode
//const int SIO_WR4_SYNC_MODE_MASK            = 0x30;     // D5-D4 = Sync mode
//const int SIO_WR4_SYNC_MODE_8BIT            =   0x00;   //  00 = 8 bit sync character
//const int SIO_WR4_SYNC_MODE_16BIT           =   0x10;   //  01 = 16 bit sync character
//const int SIO_WR4_SYNC_MODE_SDLC            =   0x20;   //  10 = SDLC mode (01111110 flag)
//const int SIO_WR4_SYNC_MODE_EXTERNAL        =   0x30;   //  11 = External sync mode
//const int SIO_WR4_STOPBITS_MASK             = 0x0c;     // D3-D2 = Stop bits
//const int SIO_WR4_STOPBITS_SYNC             =   0x00;   //  00 = Sync modes enable
//const int SIO_WR4_STOPBITS_1                =   0x04;   //  01 = 1 stop bit/character
//const int SIO_WR4_STOPBITS_15               =   0x08;   //  10 = 1.5 stop bits/character
//const int SIO_WR4_STOPBITS_2                =   0x0c;   //  11 = 2 stop bits/character
//const int SIO_WR4_PARITY_EVEN               = 0x02;     // D1 = Parity even/odd
//const int SIO_WR4_PARITY_ENABLE             = 0x01;     // D0 = Parity enable

// SIO write register 5
const int SIO_WR5_DTR                       = 0x80;     // D7 = DTR
//const int SIO_WR5_TX_DATABITS_MASK          = 0x60;     // D6-D5 = Tx Data bits
//const int SIO_WR5_TX_DATABITS_5             =   0x00;   //  00 = Tx 5 bits/character
//const int SIO_WR5_TX_DATABITS_7             =   0x20;   //  01 = Tx 7 bits/character
//const int SIO_WR5_TX_DATABITS_6             =   0x40;   //  10 = Tx 6 bits/character
//const int SIO_WR5_TX_DATABITS_8             =   0x60;   //  11 = Tx 8 bits/character
const int SIO_WR5_SEND_BREAK                = 0x10;     // D4 = Send break
const int SIO_WR5_TX_ENABLE                 = 0x08;     // D3 = Tx Enable
//const int SIO_WR5_CRC16_SDLC                = 0x04;     // D2 = CRC-16/SDLC
const int SIO_WR5_RTS                       = 0x02;     // D1 = RTS
//const int SIO_WR5_TX_CRC_ENABLE             = 0x01;     // D0 = Tx CRC enable

// SIO write register 6
//const int SIO_WR6_SYNC_7_0_MASK             = 0xff;     // D7-D0 = Sync bits 7-0

// SIO write register 7
//const int SIO_WR7_SYNC_15_8_MASK            = 0xff;     // D7-D0 = Sync bits 15-8

// SIO read register 0
//const int SIO_RR0_BREAK_ABORT               = 0x80;     // D7 = Break/abort
//const int SIO_RR0_TX_UNDERRUN               = 0x40;     // D6 = Tx underrun/EOM
const int SIO_RR0_CTS                       = 0x20;     // D5 = CTS
//const int SIO_RR0_SYNC_HUNT                 = 0x10;     // D4 = Sync/hunt
const int SIO_RR0_DCD                       = 0x08;     // D3 = DCD
const int SIO_RR0_TX_BUFFER_EMPTY           = 0x04;     // D2 = Tx buffer empty
const int SIO_RR0_INT_PENDING               = 0x02;     // D1 = int pending (CH-A only)
const int SIO_RR0_RX_CHAR_AVAILABLE         = 0x01;     // D0 = Rx character available

// SIO read register 1
//const int SIO_RR1_END_OF_FRAME              = 0x80;     // D7 = End of frame (SDLC)
//const int SIO_RR1_CRC_FRAMING_ERROR         = 0x40;     // D6 = CRC/Framing error
//const int SIO_RR1_RX_OVERRUN_ERROR          = 0x20;     // D5 = Rx overrun error
//const int SIO_RR1_PARITY_ERROR              = 0x10;     // D4 = Parity error
//const int SIO_RR1_IFIELD_BITS_MASK          = 0x0e;     // D3-D1 = I field bits
														//  100 = 0 in prev, 3 in 2nd prev
														//  010 = 0 in prev, 4 in 2nd prev
														//  110 = 0 in prev, 5 in 2nd prev
														//  001 = 0 in prev, 6 in 2nd prev
														//  101 = 0 in prev, 7 in 2nd prev
														//  011 = 0 in prev, 8 in 2nd prev
														//  111 = 1 in prev, 8 in 2nd prev
														//  000 = 2 in prev, 8 in 2nd prev
//const int SIO_RR1_ALL_SENT                  = 0x01;     // D0 = All sent

// SIO read register 2 (CH-B only)
//const int SIO_RR2_VECTOR_MASK               = 0xff;     // D7-D0 = Interrupt vector


const UINT8 z80sio_device::k_int_priority[] =
{
	INT_CHA_RECEIVE,
	INT_CHA_TRANSMIT,
	INT_CHA_STATUS,
	INT_CHA_ERROR,
	INT_CHB_RECEIVE,
	INT_CHB_TRANSMIT,
	INT_CHB_STATUS,
	INT_CHB_ERROR
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/*

    Interrupt priorities:
        Ch A receive
        Ch A transmit
        Ch A external/status
        Ch B receive
        Ch B transmit
        Ch B external/status


    Initial configuration (both channels):
        005D:sio_reg_w(0,4) = 44
                    01 = x16 clock mode
                    00 = 8 bit sync character
                    01 = 1 stop bit/character
                    Parity odd
                    Parity disabled

        005D:sio_reg_w(0,3) = C1
                    11 = Rx 8 bits/character
                    No auto enables
                    No enter hunt phase
                    No Rx CRC enable
                    No address search mode
                    No sync character load inhibit
                    Rx enable

        005D:sio_reg_w(0,5) = 68
                    DTR = 0
                    11 = Tx 8 bits/character
                    No send break
                    Tx enable
                    SDLC
                    No RTS
                    No CRC enable

        005D:sio_reg_w(0,2) = 40
                    Vector = 0x40

        005D:sio_reg_w(0,1) = 1D
                    No READY/WAIT
                    No READY/WAIT function
                    No READY/WAIT on R/T
                    11 = int on all Rx characters (parity ignored)
                    Status affects vector
                    No Tx int enable
                    Ext int enable

*/


//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  update_interrupt_state - update the interrupt
//  state to the external world
//-------------------------------------------------

inline void z80sio_device::update_interrupt_state()
{
	// if we have a callback, update it with the current state
	if (!m_irq.isnull())
		m_irq((z80daisy_irq_state() & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  set_interrupt - set the given interrupt state
//  on this channel and update the overall device
//  state
//-------------------------------------------------

inline void z80sio_device::sio_channel::set_interrupt(int type)
{
	int inum = (this == &m_device->m_channel[0] ? 4 : 0) + type;
	m_device->m_int_state[inum] = Z80_DAISY_INT;
	m_device->update_interrupt_state();
}


//-------------------------------------------------
//  clear_interrupt - clear the given interrupt
//  state on this channel and update the overall
//  device state
//-------------------------------------------------

inline void z80sio_device::sio_channel::clear_interrupt(int type)
{
	int inum = (this == &m_device->m_channel[0] ? 4 : 0) + type;
	m_device->m_int_state[inum] &= ~Z80_DAISY_INT;
	m_device->update_interrupt_state();
}


//-------------------------------------------------
//  compute_time_per_character - compute the
//  serial clocking period
//-------------------------------------------------

inline attotime z80sio_device::sio_channel::compute_time_per_character()
{
	// fix me -- should compute properly and include data, stop, parity bit
	return attotime::from_hz(9600) * 10;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  z80sio_device - constructor
//-------------------------------------------------

z80sio_device::z80sio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, Z80SIO, "Z80 SIO", tag, owner, clock, "z80sio", __FILE__),
		device_z80daisy_interface(mconfig, *this)
{
	for (int i = 0; i < 8; i++)
		m_int_state[i] = 0;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void z80sio_device::device_config_complete()
{
	// inherit a copy of the static data
	const z80sio_interface *intf = reinterpret_cast<const z80sio_interface *>(static_config());
	if (intf != NULL)
		*static_cast<z80sio_interface *>(this) = *intf;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void z80sio_device::device_start()
{
	m_irq.resolve(m_irq_cb, *this);
	m_dtr_changed.resolve(m_dtr_changed_cb, *this);
	m_rts_changed.resolve(m_rts_changed_cb, *this);
	m_break_changed.resolve(m_break_changed_cb, *this);
	m_transmit.resolve(m_transmit_cb, *this);
	m_received_poll.resolve(m_received_poll_cb, *this);

	m_channel[0].start(this, 0);
	m_channel[1].start(this, 1);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void z80sio_device::device_reset()
{
	// loop over channels
	for (int ch = 0; ch < 2; ch++)
		m_channel[ch].reset();
}



//**************************************************************************
//  DAISY CHAIN INTERFACE
//**************************************************************************

//-------------------------------------------------
//  z80daisy_irq_state - return the overall IRQ
//  state for this device
//-------------------------------------------------

int z80sio_device::z80daisy_irq_state()
{
	int state = 0;

	VPRINTF(("sio IRQ state = B:%d%d%d%d A:%d%d%d%d\n",
				m_int_state[0], m_int_state[1], m_int_state[2], m_int_state[3],
				m_int_state[4], m_int_state[5], m_int_state[6], m_int_state[7]));

	// loop over all interrupt sources
	for (int irqsource = 0; irqsource < 8; irqsource++)
	{
		int inum = k_int_priority[irqsource];

		// if we're servicing a request, don't indicate more interrupts
		if (m_int_state[inum] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= m_int_state[inum];
	}

	return state;
}


//-------------------------------------------------
//  z80daisy_irq_ack - acknowledge an IRQ and
//  return the appropriate vector
//-------------------------------------------------

int z80sio_device::z80daisy_irq_ack()
{
	// loop over all interrupt sources
	for (int irqsource = 0; irqsource < 8; irqsource++)
	{
		int inum = k_int_priority[irqsource];

		// find the first channel with an interrupt requested
		if (m_int_state[inum] & Z80_DAISY_INT)
		{
			VPRINTF(("sio IRQAck %d\n", inum));

			// clear interrupt, switch to the IEO state, and update the IRQs
			m_int_state[inum] = Z80_DAISY_IEO;
			update_interrupt_state();
			return m_channel[1].m_regs[2] + inum * 2;
		}
	}

	logerror("z80sio_irq_ack: failed to find an interrupt to ack!\n");
	return m_channel[1].m_regs[2];
}


//-------------------------------------------------
//  z80daisy_irq_reti - clear the interrupt
//  pending state to allow other interrupts through
//-------------------------------------------------

void z80sio_device::z80daisy_irq_reti()
{
	// loop over all interrupt sources
	for (int irqsource = 0; irqsource < 8; irqsource++)
	{
		int inum = k_int_priority[irqsource];

		// find the first channel with an IEO pending
		if (m_int_state[inum] & Z80_DAISY_IEO)
		{
			VPRINTF(("sio IRQReti %d\n", inum));

			// clear the IEO state and update the IRQs
			m_int_state[inum] &= ~Z80_DAISY_IEO;
			update_interrupt_state();
			return;
		}
	}

	logerror("z80sio_irq_reti: failed to find an interrupt to clear IEO on!\n");
}



//**************************************************************************
//  SIO CHANNEL
//**************************************************************************

//-------------------------------------------------
//  sio_channel - constructor
//-------------------------------------------------

z80sio_device::sio_channel::sio_channel()
	: m_device(NULL),
		m_index(0),
		m_inbuf(-1),
		m_outbuf(-1),
		m_int_on_next_rx(false),
		m_receive_timer(NULL),
		m_receive_inptr(0),
		m_receive_outptr(0)
{
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_status, 0, sizeof(m_status));
	memset(m_receive_buffer, 0, sizeof(m_receive_buffer));
}


//-------------------------------------------------
//  start - channel-specific startup
//-------------------------------------------------

void z80sio_device::sio_channel::start(z80sio_device *device, int index)
{
	m_device = device;
	m_index = index;
	m_receive_timer = device->machine().scheduler().timer_alloc(FUNC(static_serial_callback), this);
}


//-------------------------------------------------
//  reset - reset a single SIO channel
//-------------------------------------------------

void z80sio_device::sio_channel::reset()
{
	m_status[0] = SIO_RR0_TX_BUFFER_EMPTY;
	m_status[1] = 0x00;
	m_status[2] = 0x00;
	m_int_on_next_rx = false;
	m_outbuf = -1;

	// reset interrupts
	clear_interrupt(INT_TRANSMIT);
	clear_interrupt(INT_STATUS);
	clear_interrupt(INT_RECEIVE);
	clear_interrupt(INT_ERROR);

	// start the receive timer running
	attotime tpc = compute_time_per_character();
	m_receive_timer->adjust(tpc, 0, tpc);
}


//-------------------------------------------------
//  control_write - write to a control register
//-------------------------------------------------

void z80sio_device::sio_channel::control_write(UINT8 data)
{
	int regnum = m_regs[0] & 7;

	if (regnum != 0 || (regnum & 0xf8) != 0)
		VPRINTF(("%s:sio_reg_w(%c,%d) = %02X\n", m_device->machine().describe_context(), 'A' + m_index, regnum, data));

	// write a new value to the selected register
	UINT8 old = m_regs[regnum];
	m_regs[regnum] = data;

	// clear the register number for the next write
	if (regnum != 0)
		m_regs[0] &= ~7;

	// switch off the register for live state changes
	switch (regnum)
	{
		// SIO write register 0
		case 0:
			switch (data & SIO_WR0_COMMAND_MASK)
			{
				case SIO_WR0_COMMAND_CH_RESET:
					VPRINTF(("%s:SIO reset channel %c\n", m_device->machine().describe_context(), 'A' + m_index));
					reset();
					break;

				case SIO_WR0_COMMAND_RES_STATUS_INT:
					clear_interrupt(INT_STATUS);
					break;

				case SIO_WR0_COMMAND_ENA_RX_INT:
					m_int_on_next_rx = true;
					m_device->update_interrupt_state();
					break;

				case SIO_WR0_COMMAND_RES_TX_INT:
					clear_interrupt(INT_TRANSMIT);
					break;

				case SIO_WR0_COMMAND_RES_ERROR:
					clear_interrupt(INT_ERROR);
					break;
			}
			break;

		// SIO write register 1
		case 1:
			m_device->update_interrupt_state();
			break;

		// SIO write register 5
		case 5:
			if (((old ^ data) & SIO_WR5_DTR) && !m_device->m_dtr_changed.isnull())
				m_device->m_dtr_changed(m_index, (data & SIO_WR5_DTR) != 0);
			if (((old ^ data) & SIO_WR5_SEND_BREAK) && !m_device->m_break_changed.isnull())
				m_device->m_break_changed(m_index, (data & SIO_WR5_SEND_BREAK) != 0);
			if (((old ^ data) & SIO_WR5_RTS) && !m_device->m_rts_changed.isnull())
				m_device->m_rts_changed(m_index, (data & SIO_WR5_RTS) != 0);
			break;
	}
}


//-------------------------------------------------
//  control_read - read from a control register
//-------------------------------------------------

UINT8 z80sio_device::sio_channel::control_read()
{
	int regnum = m_regs[0] & 7;
	UINT8 result = m_status[regnum];

	// switch off the register for live state changes
	switch (regnum)
	{
		// SIO read register 0
		case 0:
			result &= ~SIO_RR0_INT_PENDING;
			if (m_device->z80daisy_irq_state() & Z80_DAISY_INT)
				result |= SIO_RR0_INT_PENDING;
			break;
	}

	VPRINTF(("%s:sio_reg_r(%c,%d) = %02x\n", m_device->machine().describe_context(), 'A' + m_index, regnum, m_status[regnum]));

	return result;
}


//-------------------------------------------------
//  data_write - write to a data register
//-------------------------------------------------

void z80sio_device::sio_channel::data_write(UINT8 data)
{
	VPRINTF(("%s:sio_data_w(%c) = %02X\n", m_device->machine().describe_context(), 'A' + m_index, data));

	// if tx not enabled, just ignore it
	if (!(m_regs[5] & SIO_WR5_TX_ENABLE))
		return;

	// update the status register
	m_status[0] &= ~SIO_RR0_TX_BUFFER_EMPTY;

	// reset the transmit interrupt
	clear_interrupt(INT_TRANSMIT);

	// stash the character
	m_outbuf = data;
}


//-------------------------------------------------
//  data_read - read from a data register
//-------------------------------------------------

UINT8 z80sio_device::sio_channel::data_read()
{
	// update the status register
	m_status[0] &= ~SIO_RR0_RX_CHAR_AVAILABLE;

	// reset the receive interrupt
	clear_interrupt(INT_RECEIVE);

	VPRINTF(("%s:sio_data_r(%c) = %02X\n", m_device->machine().describe_context(), 'A' + m_index, m_inbuf));

	return m_inbuf;
}


//-------------------------------------------------
//  dtr - return the state of the DTR line
//-------------------------------------------------

int z80sio_device::sio_channel::dtr()
{
	return ((m_regs[5] & SIO_WR5_DTR) != 0);
}


//-------------------------------------------------
//  rts - return the state of the RTS line
//-------------------------------------------------

int z80sio_device::sio_channel::rts()
{
	return ((m_regs[5] & SIO_WR5_RTS) != 0);
}


//-------------------------------------------------
//  set_cts - set the state of the CTS line
//-------------------------------------------------

void z80sio_device::sio_channel::set_cts(int state)
{
	m_device->machine().scheduler().synchronize(FUNC(static_change_input_line), (SIO_RR0_CTS << 1) + (state != 0), this);
}


//-------------------------------------------------
//  set_dcd - set the state of the DCD line
//-------------------------------------------------

void z80sio_device::sio_channel::set_dcd(int state)
{
	m_device->machine().scheduler().synchronize(FUNC(static_change_input_line), (SIO_RR0_DCD << 1) + (state != 0), this);
}


//-------------------------------------------------
//  receive_data - receive data on the input lines
//-------------------------------------------------

void z80sio_device::sio_channel::receive_data(int data)
{
	// put it on the queue
	int newinptr = (m_receive_inptr + 1) % ARRAY_LENGTH(m_receive_buffer);
	if (newinptr != m_receive_outptr)
	{
		m_receive_buffer[m_receive_inptr] = data;
		m_receive_inptr = newinptr;
	}
	else
		logerror("z80sio_receive_data: buffer overrun\n");
}


//-------------------------------------------------
//  change_input_line - generically change the
//  state of an input line; designed to be called
//  from a timer callback
//-------------------------------------------------

void z80sio_device::sio_channel::change_input_line(int line, int state)
{
	VPRINTF(("sio_change_input_line(%c, %s) = %d\n", 'A' + m_index, (line == SIO_RR0_CTS) ? "CTS" : "DCD", state));

	// remember the old value
	UINT8 old = m_status[0];

	// set the bit in the status register
	m_status[0] &= ~line;
	if (state)
		m_status[0] |= line;

	// if state change interrupts are enabled, signal
	if (((old ^ m_status[0]) & line) && (m_regs[1] & SIO_WR1_STATUSINT_ENABLE))
		set_interrupt(INT_STATUS);
}


//-------------------------------------------------
//  serial_callback - callback to pump
//  data through
//-------------------------------------------------

void z80sio_device::sio_channel::serial_callback()
{
	int data = -1;

	// first perform any outstanding transmits
	if (m_outbuf != -1)
	{
		VPRINTF(("serial_callback(%c): Transmitting %02x\n", 'A' + m_index, m_outbuf));

		// actually transmit the character
		if (!m_device->m_transmit.isnull())
			m_device->m_transmit(m_index, m_outbuf, 0xffff);

		// update the status register
		m_status[0] |= SIO_RR0_TX_BUFFER_EMPTY;

		// set the transmit buffer empty interrupt if enabled
		if (m_regs[1] & SIO_WR1_TXINT_ENABLE)
			set_interrupt(INT_TRANSMIT);

		// reset the output buffer
		m_outbuf = -1;
	}

	// ask the polling callback if there is data to receive
	if (!m_device->m_received_poll.isnull())
		data = INT16(m_device->m_received_poll(m_index, 0xffff));

	// if we have buffered data, pull it
	if (m_receive_inptr != m_receive_outptr)
	{
		data = m_receive_buffer[m_receive_outptr];
		m_receive_outptr = (m_receive_outptr + 1) % ARRAY_LENGTH(m_receive_buffer);
	}

	// if we have data, receive it
	if (data != -1)
	{
		VPRINTF(("serial_callback(%c): Receiving %02x\n", 'A' + m_index, data));

		// if rx not enabled, just ignore it
		if (!(m_regs[3] & SIO_WR3_RX_ENABLE))
		{
			VPRINTF(("  (ignored because receive is disabled)\n"));
			return;
		}

		// stash the data and update the status
		m_inbuf = data;
		m_status[0] |= SIO_RR0_RX_CHAR_AVAILABLE;

		// update our interrupt state
		switch (m_regs[1] & SIO_WR1_RXINT_MASK)
		{
			case SIO_WR1_RXINT_FIRST:
				if (!m_int_on_next_rx)
					break;

			case SIO_WR1_RXINT_ALL_NOPARITY:
			case SIO_WR1_RXINT_ALL_PARITY:
				set_interrupt(INT_RECEIVE);
				break;
		}
		m_int_on_next_rx = false;
	}
}



//**************************************************************************
//  GLOBAL STUBS
//**************************************************************************

READ8_MEMBER( z80sio_device::read )
{
	switch (offset & 3)
	{
		case 0: return data_read(0);
		case 1: return data_read(1);
		case 2: return control_read(0);
		case 3: return control_read(1);
	}

	return 0xff;
}

WRITE8_MEMBER( z80sio_device::write )
{
	switch (offset & 3)
	{
		case 0: data_write(0, data); break;
		case 1: data_write(1, data); break;
		case 2: control_write(0, data); break;
		case 3: control_write(1, data); break;
	}
}

READ8_MEMBER( z80sio_device::read_alt )
{
	switch (offset & 3)
	{
		case 0: return data_read(0);
		case 1: return control_read(0);
		case 2: return data_read(1);
		case 3: return control_read(1);
	}

	return 0xff;
}

WRITE8_MEMBER( z80sio_device::write_alt )
{
	switch (offset & 3)
	{
		case 0: data_write(0, data); break;
		case 1: control_write(0, data); break;
		case 2: data_write(1, data); break;
		case 3: control_write(1, data); break;
	}
}
