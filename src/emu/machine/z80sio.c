/***************************************************************************

    Z80 SIO (Z8440) implementation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "z80sio.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE		0

#define VPRINTF(x) do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* interrupt states */
#define INT_CHB_TRANSMIT	0x00		/* not confirmed */
#define INT_CHB_STATUS		0x01
#define INT_CHB_RECEIVE		0x02
#define INT_CHB_ERROR		0x03

#define INT_CHA_TRANSMIT	0x04		/* not confirmed */
#define INT_CHA_STATUS		0x05
#define INT_CHA_RECEIVE		0x06
#define INT_CHA_ERROR		0x07

/* SIO write register 0 */
#define SIO_WR0_RESET_MASK				0xc0		/* D7-D6: Reset control */
#define SIO_WR0_RESET_NULL					0x00	/*  00 = NULL code */
#define SIO_WR0_RESET_RX_CRC				0x40	/*  01 = Reset Rx CRC checker */
#define SIO_WR0_RESET_TX_CRC				0x80	/*  10 = Reset Tx CRC generator */
#define SIO_WR0_RESET_TX_LATCH				0xc0	/*  11 = Reset Tx Underrun/EOM latch */
#define SIO_WR0_COMMAND_MASK			0x38		/* D5-D3: Command */
#define SIO_WR0_COMMAND_NULL				0x00	/*  000 = NULL code */
#define SIO_WR0_COMMAND_SET_ABORT			0x08	/*  001 = Set abort (SDLC) */
#define SIO_WR0_COMMAND_RES_STATUS_INT		0x10	/*  010 = reset ext/status interrupts */
#define SIO_WR0_COMMAND_CH_RESET			0x18	/*  011 = Channel reset */
#define SIO_WR0_COMMAND_ENA_RX_INT			0x20	/*  100 = Enable int on next Rx character */
#define SIO_WR0_COMMAND_RES_TX_INT			0x28	/*  101 = Reset Tx int pending */
#define SIO_WR0_COMMAND_RES_ERROR			0x30	/*  110 = Error reset */
#define SIO_WR0_COMMAND_RETI				0x38	/*  111 = Return from int (CH-A only) */
#define SIO_WR0_REGISTER_MASK			0x07		/* D2-D0: Register select (0-7) */

/* SIO write register 1 */
#define SIO_WR1_READY_WAIT_ENA			0x80		/* D7 = READY/WAIT enable */
#define SIO_WR1_READY_WAIT_FUNCTION		0x40		/* D6 = READY/WAIT function */
#define SIO_WR1_READY_WAIT_ON_RT		0x20		/* D5 = READY/WAIT on R/T */
#define SIO_WR1_RXINT_MASK				0x18		/* D4-D3 = Rx int control */
#define SIO_WR1_RXINT_DISABLE				0x00	/*  00 = Rx int disable */
#define SIO_WR1_RXINT_FIRST					0x08	/*  01 = Rx int on first character */
#define SIO_WR1_RXINT_ALL_PARITY			0x10	/*  10 = int on all Rx characters (parity affects vector) */
#define SIO_WR1_RXINT_ALL_NOPARITY			0x18	/*  11 = int on all Rx characters (parity ignored) */
#define SIO_WR1_STATUS_AFFECTS_VECTOR	0x04		/* D2 = Status affects vector (CH-B only) */
#define SIO_WR1_TXINT_ENABLE			0x02		/* D1 = Tx int enable */
#define SIO_WR1_STATUSINT_ENABLE		0x01		/* D0 = Ext int enable */

/* SIO write register 2 (CH-B only) */
#define SIO_WR2_INT_VECTOR_MASK			0xff		/* D7-D0 = interrupt vector */

/* SIO write register 3 */
#define SIO_WR3_RX_DATABITS_MASK		0xc0		/* D7-D6 = Rx Data bits */
#define SIO_WR3_RX_DATABITS_5				0x00	/*  00 = Rx 5 bits/character */
#define SIO_WR3_RX_DATABITS_7				0x40	/*  01 = Rx 7 bits/character */
#define SIO_WR3_RX_DATABITS_6				0x80	/*  10 = Rx 6 bits/character */
#define SIO_WR3_RX_DATABITS_8				0xc0	/*  11 = Rx 8 bits/character */
#define SIO_WR3_AUTO_ENABLES			0x20		/* D5 = Auto enables */
#define SIO_WR3_ENTER_HUNT_PHASE		0x10		/* D4 = Enter hunt phase */
#define SIO_WR3_RX_CRC_ENABLE			0x08		/* D3 = Rx CRC enable */
#define SIO_WR3_ADDR_SEARCH_MODE		0x04		/* D2 = Address search mode (SDLC) */
#define SIO_WR3_SYNC_LOAD_INHIBIT		0x02		/* D1 = Sync character load inhibit */
#define SIO_WR3_RX_ENABLE				0x01		/* D0 = Rx enable */

/* SIO write register 4 */
#define SIO_WR4_CLOCK_MODE_MASK			0xc0		/* D7-D6 = Clock mode */
#define SIO_WR4_CLOCK_MODE_x1				0x00	/*  00 = x1 clock mode */
#define SIO_WR4_CLOCK_MODE_x16				0x40	/*  01 = x16 clock mode */
#define SIO_WR4_CLOCK_MODE_x32				0x80	/*  10 = x32 clock mode */
#define SIO_WR4_CLOCK_MODE_x64				0xc0	/*  11 = x64 clock mode */
#define SIO_WR4_SYNC_MODE_MASK			0x30		/* D5-D4 = Sync mode */
#define SIO_WR4_SYNC_MODE_8BIT				0x00	/*  00 = 8 bit sync character */
#define SIO_WR4_SYNC_MODE_16BIT				0x10	/*  01 = 16 bit sync character */
#define SIO_WR4_SYNC_MODE_SDLC				0x20	/*  10 = SDLC mode (01111110 flag)  */
#define SIO_WR4_SYNC_MODE_EXTERNAL			0x30	/*  11 = External sync mode */
#define SIO_WR4_STOPBITS_MASK			0x0c		/* D3-D2 = Stop bits */
#define SIO_WR4_STOPBITS_SYNC				0x00	/*  00 = Sync modes enable */
#define SIO_WR4_STOPBITS_1					0x04	/*  01 = 1 stop bit/character */
#define SIO_WR4_STOPBITS_15					0x08	/*  10 = 1.5 stop bits/character */
#define SIO_WR4_STOPBITS_2					0x0c	/*  11 = 2 stop bits/character */
#define SIO_WR4_PARITY_EVEN				0x02		/* D1 = Parity even/odd */
#define SIO_WR4_PARITY_ENABLE			0x01		/* D0 = Parity enable */

/* SIO write register 5 */
#define SIO_WR5_DTR						0x80		/* D7 = DTR */
#define SIO_WR5_TX_DATABITS_MASK		0x60		/* D6-D5 = Tx Data bits */
#define SIO_WR5_TX_DATABITS_5				0x00	/*  00 = Tx 5 bits/character */
#define SIO_WR5_TX_DATABITS_7				0x20	/*  01 = Tx 7 bits/character */
#define SIO_WR5_TX_DATABITS_6				0x40	/*  10 = Tx 6 bits/character */
#define SIO_WR5_TX_DATABITS_8				0x60	/*  11 = Tx 8 bits/character */
#define SIO_WR5_SEND_BREAK				0x10		/* D4 = Send break */
#define SIO_WR5_TX_ENABLE				0x08		/* D3 = Tx Enable */
#define SIO_WR5_CRC16_SDLC				0x04		/* D2 = CRC-16/SDLC */
#define SIO_WR5_RTS						0x02		/* D1 = RTS */
#define SIO_WR5_TX_CRC_ENABLE			0x01		/* D0 = Tx CRC enable */

/* SIO write register 6  */
#define SIO_WR6_SYNC_7_0_MASK			0xff		/* D7-D0 = Sync bits 7-0 */

/* SIO write register 7 */
#define SIO_WR7_SYNC_15_8_MASK			0xff		/* D7-D0 = Sync bits 15-8 */

/* SIO read register 0 */
#define SIO_RR0_BREAK_ABORT				0x80		/* D7 = Break/abort */
#define SIO_RR0_TX_UNDERRUN				0x40		/* D6 = Tx underrun/EOM */
#define SIO_RR0_CTS						0x20		/* D5 = CTS */
#define SIO_RR0_SYNC_HUNT				0x10		/* D4 = Sync/hunt */
#define SIO_RR0_DCD						0x08		/* D3 = DCD */
#define SIO_RR0_TX_BUFFER_EMPTY			0x04		/* D2 = Tx buffer empty */
#define SIO_RR0_INT_PENDING				0x02		/* D1 = int pending (CH-A only) */
#define SIO_RR0_RX_CHAR_AVAILABLE		0x01		/* D0 = Rx character available */

/* SIO read register 1 */
#define SIO_RR1_END_OF_FRAME			0x80		/* D7 = End of frame (SDLC) */
#define SIO_RR1_CRC_FRAMING_ERROR		0x40		/* D6 = CRC/Framing error */
#define SIO_RR1_RX_OVERRUN_ERROR		0x20		/* D5 = Rx overrun error */
#define SIO_RR1_PARITY_ERROR			0x10		/* D4 = Parity error */
#define SIO_RR1_IFIELD_BITS_MASK		0x0e		/* D3-D1 = I field bits */
													/*  100 = 0 in prev, 3 in 2nd prev */
													/*  010 = 0 in prev, 4 in 2nd prev */
													/*  110 = 0 in prev, 5 in 2nd prev */
													/*  001 = 0 in prev, 6 in 2nd prev */
													/*  101 = 0 in prev, 7 in 2nd prev */
													/*  011 = 0 in prev, 8 in 2nd prev */
													/*  111 = 1 in prev, 8 in 2nd prev */
													/*  000 = 2 in prev, 8 in 2nd prev */
#define SIO_RR1_ALL_SENT				0x01		/* D0 = All sent */

/* SIO read register 2 (CH-B only) */
#define SIO_RR2_VECTOR_MASK				0xff		/* D7-D0 = Interrupt vector */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _sio_channel sio_channel;
struct _sio_channel
{
	UINT8		regs[8];			/* 8 writeable registers */
	UINT8		status[4];			/* 3 readable registers */
	int			inbuf;				/* input buffer */
	int			outbuf;				/* output buffer */
	UINT8		int_on_next_rx;		/* interrupt on next rx? */
	emu_timer *	receive_timer;		/* timer to clock data in */
	UINT8		receive_buffer[16];	/* buffer for incoming data */
	UINT8		receive_inptr;		/* index of data coming in */
	UINT8		receive_outptr;		/* index of data going out */
};


typedef struct _z80sio z80sio;
struct _z80sio
{
	sio_channel	chan[2];			/* 2 channels */
	UINT8		int_state[8];		/* interrupt states */

	void (*irq_cb)(const device_config *device, int state);
	write8_device_func dtr_changed_cb;
	write8_device_func rts_changed_cb;
	write8_device_func break_changed_cb;
	write8_device_func transmit_cb;
	int (*receive_poll_cb)(const device_config *device, int channel);
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( serial_callback );



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


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int z80sio_irq_state(const device_config *device);
static int z80sio_irq_ack(const device_config *device);
static void z80sio_irq_reti(const device_config *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80sio *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80SIO);
	return (z80sio *)device->token;
}


INLINE void interrupt_check(const device_config *device)
{
	/* if we have a callback, update it with the current state */
	z80sio *sio = get_safe_token(device);
	if (sio->irq_cb != NULL)
		(*sio->irq_cb)(device, (z80sio_irq_state(device) & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE);
}


INLINE attotime compute_time_per_character(z80sio *sio, int which)
{
	/* fix me -- should compute properly and include data, stop, parity bits */
	return attotime_mul(ATTOTIME_IN_HZ(9600), 10);
}



/***************************************************************************
    INITIALIZATION/CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    reset_channel - reset a single SIO channel
-------------------------------------------------*/

static void reset_channel(const device_config *device, int ch)
{
	z80sio *sio = get_safe_token(device);
	attotime tpc = compute_time_per_character(sio, ch);
	sio_channel *chan = &sio->chan[ch];

	chan->status[0] = SIO_RR0_TX_BUFFER_EMPTY;
	chan->status[1] = 0x00;
	chan->status[2] = 0x00;
	chan->int_on_next_rx = 0;
	chan->outbuf = -1;

	sio->int_state[0 + 4*ch] = 0;
	sio->int_state[1 + 4*ch] = 0;
	sio->int_state[2 + 4*ch] = 0;
	sio->int_state[3 + 4*ch] = 0;

	interrupt_check(device);

	/* start the receive timer running */
	timer_adjust_periodic(chan->receive_timer, tpc, ch, tpc);
}



/***************************************************************************
    CONTROL REGISTER READ/WRITE
***************************************************************************/

/*-------------------------------------------------
    z80sio_c_w - write to a control register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80sio_c_w )
{
	z80sio *sio = get_safe_token(device);
	int ch = offset & 1;
	sio_channel *chan = &sio->chan[ch];
	int reg = chan->regs[0] & 7;
	UINT8 old = chan->regs[reg];

	if (reg != 0 || (reg & 0xf8))
		VPRINTF(("%s:sio_reg_w(%c,%d) = %02X\n", cpuexec_describe_context(device->machine), 'A' + ch, reg, data));

	/* write a new value to the selected register */
	chan->regs[reg] = data;

	/* clear the register number for the next write */
	if (reg != 0)
		chan->regs[0] &= ~7;

	/* switch off the register for live state changes */
	switch (reg)
	{
		/* SIO write register 0 */
		case 0:
			switch (data & SIO_WR0_COMMAND_MASK)
			{
				case SIO_WR0_COMMAND_CH_RESET:
					VPRINTF(("%s:SIO reset channel %c\n", cpuexec_describe_context(device->machine), 'A' + ch));
					reset_channel(device, ch);
					break;

				case SIO_WR0_COMMAND_RES_STATUS_INT:
					sio->int_state[INT_CHA_STATUS - 4*ch] &= ~Z80_DAISY_INT;
					interrupt_check(device);
					break;

				case SIO_WR0_COMMAND_ENA_RX_INT:
					chan->int_on_next_rx = TRUE;
					interrupt_check(device);
					break;

				case SIO_WR0_COMMAND_RES_TX_INT:
					sio->int_state[INT_CHA_TRANSMIT - 4*ch] &= ~Z80_DAISY_INT;
					interrupt_check(device);
					break;

				case SIO_WR0_COMMAND_RES_ERROR:
					sio->int_state[INT_CHA_ERROR - 4*ch] &= ~Z80_DAISY_INT;
					interrupt_check(device);
					break;
			}
			break;

		/* SIO write register 1 */
		case 1:
			interrupt_check(device);
			break;

		/* SIO write register 5 */
		case 5:
			if (((old ^ data) & SIO_WR5_DTR) && sio->dtr_changed_cb)
				(*sio->dtr_changed_cb)(device, ch, (data & SIO_WR5_DTR) != 0);
			if (((old ^ data) & SIO_WR5_SEND_BREAK) && sio->break_changed_cb)
				(*sio->break_changed_cb)(device, ch, (data & SIO_WR5_SEND_BREAK) != 0);
			if (((old ^ data) & SIO_WR5_RTS) && sio->rts_changed_cb)
				(*sio->rts_changed_cb)(device, ch, (data & SIO_WR5_RTS) != 0);
			break;
	}
}


/*-------------------------------------------------
    z80sio_c_r - read from a control register
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80sio_c_r )
{
	z80sio *sio = get_safe_token(device);
	int ch = offset & 1;
	sio_channel *chan = &sio->chan[ch];
	int reg = chan->regs[0] & 7;
	UINT8 result = chan->status[reg];

	/* switch off the register for live state changes */
	switch (reg)
	{
		/* SIO read register 0 */
		case 0:
			result &= ~SIO_RR0_INT_PENDING;
			if (z80sio_irq_state(device) & Z80_DAISY_INT)
				result |= SIO_RR0_INT_PENDING;
			break;
	}

	VPRINTF(("%s:sio_reg_r(%c,%d) = %02x\n", cpuexec_describe_context(device->machine), 'A' + ch, reg, chan->status[reg]));

	return chan->status[reg];
}




/***************************************************************************
    DATA REGISTER READ/WRITE
***************************************************************************/

/*-------------------------------------------------
    z80sio_d_w - write to a data register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80sio_d_w )
{
	z80sio *sio = get_safe_token(device);
	int ch = offset & 1;
	sio_channel *chan = &sio->chan[ch];

	VPRINTF(("%s:sio_data_w(%c) = %02X\n", cpuexec_describe_context(device->machine), 'A' + ch, data));

	/* if tx not enabled, just ignore it */
	if (!(chan->regs[5] & SIO_WR5_TX_ENABLE))
		return;

	/* update the status register */
	chan->status[0] &= ~SIO_RR0_TX_BUFFER_EMPTY;

	/* reset the transmit interrupt */
	sio->int_state[INT_CHA_TRANSMIT - 4*ch] &= ~Z80_DAISY_INT;
	interrupt_check(device);

	/* stash the character */
	chan->outbuf = data;
}


/*-------------------------------------------------
    z80sio_d_r - read from a data register
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80sio_d_r )
{
	z80sio *sio = get_safe_token(device);
	int ch = offset & 1;
	sio_channel *chan = &sio->chan[ch];

	/* update the status register */
	chan->status[0] &= ~SIO_RR0_RX_CHAR_AVAILABLE;

	/* reset the receive interrupt */
	sio->int_state[INT_CHA_RECEIVE - 4*ch] &= ~Z80_DAISY_INT;
	interrupt_check(device);

	VPRINTF(("%s:sio_data_r(%c) = %02X\n", cpuexec_describe_context(device->machine), 'A' + ch, chan->inbuf));

	return chan->inbuf;
}



/***************************************************************************
    CONTROL LINE READ/WRITE
***************************************************************************/

/*-------------------------------------------------
    z80sio_get_dtr - return the state of the DTR
    line
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80sio_get_dtr )
{
	z80sio *sio = get_safe_token(device);
	sio_channel *chan = &sio->chan[offset & 1];
	return ((chan->regs[5] & SIO_WR5_DTR) != 0);
}


/*-------------------------------------------------
    z80sio_get_rts - return the state of the RTS
    line
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80sio_get_rts )
{
	z80sio *sio = get_safe_token(device);
	sio_channel *chan = &sio->chan[offset & 1];
	return ((chan->regs[5] & SIO_WR5_RTS) != 0);
}


/*-------------------------------------------------
    z80sio_set_cts - set the state of the CTS
    line
-------------------------------------------------*/

static TIMER_CALLBACK( change_input_line )
{
	const device_config *device = (const device_config *)ptr;
	z80sio *sio = get_safe_token(device);
	sio_channel *chan = &sio->chan[param & 1];
	UINT8 line = (param >> 8) & 0xff;
	int state = (param >> 7) & 1;
	int ch = param & 1;
	UINT8 old;

	VPRINTF(("sio_change_input_line(%c, %s) = %d\n", 'A' + ch, (line == SIO_RR0_CTS) ? "CTS" : "DCD", state));

	/* remember the old value */
	old = chan->status[0];

	/* set the bit in the status register */
	chan->status[0] &= ~line;
	if (state)
		chan->status[0] |= line;

	/* if state change interrupts are enabled, signal */
	if (((old ^ chan->status[0]) & line) && (chan->regs[1] & SIO_WR1_STATUSINT_ENABLE))
	{
		sio->int_state[INT_CHA_STATUS - 4*ch] |= Z80_DAISY_INT;
		interrupt_check(device);
	}
}


/*-------------------------------------------------
    z80sio_set_cts - set the state of the CTS
    line
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80sio_set_cts )
{
	/* operate deferred */
	void *ptr = (void *)device;
	timer_call_after_resynch(device->machine, ptr, (SIO_RR0_CTS << 8) + (data != 0) * 0x80 + (offset & 1), change_input_line);
}


/*-------------------------------------------------
    z80sio_set_dcd - set the state of the DCD
    line
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80sio_set_dcd )
{
	/* operate deferred */
	void *ptr = (void *)device;
	timer_call_after_resynch(device->machine, ptr, (SIO_RR0_DCD << 8) + (data != 0) * 0x80 + (offset & 1), change_input_line);
}


/*-------------------------------------------------
    z80sio_receive_data - receive data on the
    input lines
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80sio_receive_data )
{
	z80sio *sio = get_safe_token(device);
	sio_channel *chan = &sio->chan[offset & 1];
	int newinptr;

	/* put it on the queue */
	newinptr = (chan->receive_inptr + 1) % ARRAY_LENGTH(chan->receive_buffer);
	if (newinptr != chan->receive_outptr)
	{
		chan->receive_buffer[chan->receive_inptr] = data;
		chan->receive_inptr = newinptr;
	}
	else
		logerror("z80sio_receive_data: buffer overrun\n");
}


/*-------------------------------------------------
    serial_callback - callback to pump
    data through
-------------------------------------------------*/

static TIMER_CALLBACK( serial_callback )
{
	const device_config *device = (const device_config *)ptr;
	z80sio *sio = get_safe_token(device);
	sio_channel *chan = &sio->chan[param];
	int ch = param;
	int data = -1;

	/* first perform any outstanding transmits */
	if (chan->outbuf != -1)
	{
		VPRINTF(("serial_callback(%c): Transmitting %02x\n", 'A' + ch, chan->outbuf));

		/* actually transmit the character */
		if (sio->transmit_cb != NULL)
			(*sio->transmit_cb)(device, ch, chan->outbuf);

		/* update the status register */
		chan->status[0] |= SIO_RR0_TX_BUFFER_EMPTY;

		/* set the transmit buffer empty interrupt if enabled */
		if (chan->regs[1] & SIO_WR1_TXINT_ENABLE)
		{
			sio->int_state[INT_CHA_TRANSMIT - 4*ch] |= Z80_DAISY_INT;
			interrupt_check(device);
		}

		/* reset the output buffer */
		chan->outbuf = -1;
	}

	/* ask the polling callback if there is data to receive */
	if (sio->receive_poll_cb != NULL)
		data = (*sio->receive_poll_cb)(device, ch);

	/* if we have buffered data, pull it */
	if (chan->receive_inptr != chan->receive_outptr)
	{
		data = chan->receive_buffer[chan->receive_outptr];
		chan->receive_outptr = (chan->receive_outptr + 1) % ARRAY_LENGTH(chan->receive_buffer);
	}

	/* if we have data, receive it */
	if (data != -1)
	{
		VPRINTF(("serial_callback(%c): Receiving %02x\n", 'A' + ch, data));

		/* if rx not enabled, just ignore it */
		if (!(chan->regs[3] & SIO_WR3_RX_ENABLE))
		{
			VPRINTF(("  (ignored because receive is disabled)\n"));
			return;
		}

		/* stash the data and update the status */
		chan->inbuf = data;
		chan->status[0] |= SIO_RR0_RX_CHAR_AVAILABLE;

		/* update our interrupt state */
		switch (chan->regs[1] & SIO_WR1_RXINT_MASK)
		{
			case SIO_WR1_RXINT_FIRST:
				if (!chan->int_on_next_rx)
					break;

			case SIO_WR1_RXINT_ALL_NOPARITY:
			case SIO_WR1_RXINT_ALL_PARITY:
				sio->int_state[INT_CHA_RECEIVE - 4*ch] |= Z80_DAISY_INT;
				interrupt_check(device);
				break;
		}
		chan->int_on_next_rx = FALSE;
	}
}



/***************************************************************************
    DAISY CHAIN INTERFACE
***************************************************************************/

static const UINT8 int_priority[] =
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


static int z80sio_irq_state(const device_config *device)
{
	z80sio *sio = get_safe_token(device);
	int state = 0;
	int i;

	VPRINTF(("sio IRQ state = B:%d%d%d%d A:%d%d%d%d\n",
				sio->int_state[0], sio->int_state[1], sio->int_state[2], sio->int_state[3],
				sio->int_state[4], sio->int_state[5], sio->int_state[6], sio->int_state[7]));

	/* loop over all interrupt sources */
	for (i = 0; i < 8; i++)
	{
		int inum = int_priority[i];

		/* if we're servicing a request, don't indicate more interrupts */
		if (sio->int_state[inum] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= sio->int_state[inum];
	}

	return state;
}


static int z80sio_irq_ack(const device_config *device)
{
	z80sio *sio = get_safe_token(device);
	int i;

	/* loop over all interrupt sources */
	for (i = 0; i < 8; i++)
	{
		int inum = int_priority[i];

		/* find the first channel with an interrupt requested */
		if (sio->int_state[inum] & Z80_DAISY_INT)
		{
			VPRINTF(("sio IRQAck %d\n", inum));

			/* clear interrupt, switch to the IEO state, and update the IRQs */
			sio->int_state[inum] = Z80_DAISY_IEO;
			interrupt_check(device);
			return sio->chan[1].regs[2] + inum * 2;
		}
	}

	logerror("z80sio_irq_ack: failed to find an interrupt to ack!\n");
	return sio->chan[1].regs[2];
}


static void z80sio_irq_reti(const device_config *device)
{
	z80sio *sio = get_safe_token(device);
	int i;

	/* loop over all interrupt sources */
	for (i = 0; i < 8; i++)
	{
		int inum = int_priority[i];

		/* find the first channel with an IEO pending */
		if (sio->int_state[inum] & Z80_DAISY_IEO)
		{
			VPRINTF(("sio IRQReti %d\n", inum));

			/* clear the IEO state and update the IRQs */
			sio->int_state[inum] &= ~Z80_DAISY_IEO;
			interrupt_check(device);
			return;
		}
	}

	logerror("z80sio_irq_reti: failed to find an interrupt to clear IEO on!\n");
}


static DEVICE_START( z80sio )
{
	const z80sio_interface *intf = (const z80sio_interface *)device->static_config;
	z80sio *sio = get_safe_token(device);
	void *ptr = (void *)device;
	astring tempstring;

	sio->chan[0].receive_timer = timer_alloc(device->machine, serial_callback, ptr);
	sio->chan[1].receive_timer = timer_alloc(device->machine, serial_callback, ptr);

	sio->irq_cb = intf->irq_cb;
	sio->dtr_changed_cb = intf->dtr_changed_cb;
	sio->rts_changed_cb = intf->rts_changed_cb;
	sio->break_changed_cb = intf->break_changed_cb;
	sio->transmit_cb = intf->transmit_cb;
	sio->receive_poll_cb = intf->receive_poll_cb;
}


static DEVICE_RESET( z80sio )
{
	int ch;

	/* loop over channels */
	for (ch = 0; ch < 2; ch++)
		reset_channel(device, ch);
}


DEVICE_GET_INFO( z80sio )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80sio);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80sio);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80sio);break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80sio_irq_state;		break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80sio_irq_ack;		break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80sio_irq_reti;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Zilog Z80 SIO");		break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

