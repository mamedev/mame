/***************************************************************************

    Z80 DART Dual Asynchronous Receiver/Transmitter emulation

    Copyright (c) 2008, The MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - break detection
    - wr0 reset tx interrupt pending
    - wait/ready
    - 1.5 stop bits

*/

#include "emu.h"
#include "z80dart.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

enum
{
	STATE_START = 0,
	STATE_DATA,
	STATE_PARITY,
	STATE_STOP,
	STATE_STOP2
};

enum
{
	INT_TRANSMIT = 0,
	INT_EXTERNAL,
	INT_RECEIVE,
	INT_SPECIAL
};

#define Z80DART_RR0_RX_CHAR_AVAILABLE	0x01
#define Z80DART_RR0_INTERRUPT_PENDING	0x02
#define Z80DART_RR0_TX_BUFFER_EMPTY		0x04
#define Z80DART_RR0_DCD					0x08
#define Z80DART_RR0_RI					0x10
#define Z80DART_RR0_CTS					0x20
#define Z80DART_RR0_BREAK				0x80 /* not supported */

#define Z80DART_RR1_ALL_SENT			0x01
#define Z80DART_RR1_PARITY_ERROR		0x10
#define Z80DART_RR1_RX_OVERRUN_ERROR	0x20
#define Z80DART_RR1_FRAMING_ERROR		0x40

#define Z80DART_WR0_REGISTER_MASK		0x07
#define Z80DART_WR0_COMMAND_MASK		0x38
#define Z80DART_WR0_NULL_CODE			0x00
#define Z80DART_WR0_RESET_EXT_STATUS	0x10
#define Z80DART_WR0_CHANNEL_RESET		0x18
#define Z80DART_WR0_ENABLE_INT_NEXT_RX	0x20
#define Z80DART_WR0_RESET_TX_INT		0x28 /* not supported */
#define Z80DART_WR0_ERROR_RESET			0x30
#define Z80DART_WR0_RETURN_FROM_INT		0x38 /* not supported */

#define Z80DART_WR1_EXT_INT_ENABLE		0x01
#define Z80DART_WR1_TX_INT_ENABLE		0x02
#define Z80DART_WR1_STATUS_VECTOR		0x04
#define Z80DART_WR1_RX_INT_ENABLE_MASK	0x18
#define Z80DART_WR1_RX_INT_DISABLE		0x00
#define Z80DART_WR1_RX_INT_FIRST		0x08
#define Z80DART_WR1_RX_INT_ALL_PARITY	0x10 /* not supported */
#define Z80DART_WR1_RX_INT_ALL			0x18
#define Z80DART_WR1_WRDY_ON_RX_TX		0x20 /* not supported */
#define Z80DART_WR1_WRDY_FUNCTION		0x40 /* not supported */
#define Z80DART_WR1_WRDY_ENABLE			0x80 /* not supported */

#define Z80DART_WR3_RX_ENABLE			0x01
#define Z80DART_WR3_AUTO_ENABLES		0x20
#define Z80DART_WR3_RX_WORD_LENGTH_MASK	0xc0
#define Z80DART_WR3_RX_WORD_LENGTH_5	0x00
#define Z80DART_WR3_RX_WORD_LENGTH_7	0x40
#define Z80DART_WR3_RX_WORD_LENGTH_6	0x80
#define Z80DART_WR3_RX_WORD_LENGTH_8	0xc0

#define Z80DART_WR4_PARITY_ENABLE		0x01 /* not supported */
#define Z80DART_WR4_PARITY_EVEN			0x02 /* not supported */
#define Z80DART_WR4_STOP_BITS_MASK		0x0c
#define Z80DART_WR4_STOP_BITS_1			0x04
#define Z80DART_WR4_STOP_BITS_1_5		0x08 /* not supported */
#define Z80DART_WR4_STOP_BITS_2			0x0c
#define Z80DART_WR4_CLOCK_MODE_MASK		0xc0
#define Z80DART_WR4_CLOCK_MODE_X1		0x00
#define Z80DART_WR4_CLOCK_MODE_X16		0x40
#define Z80DART_WR4_CLOCK_MODE_X32		0x80
#define Z80DART_WR4_CLOCK_MODE_X64		0xc0

#define Z80DART_WR5_RTS					0x02
#define Z80DART_WR5_TX_ENABLE			0x08
#define Z80DART_WR5_SEND_BREAK			0x10
#define Z80DART_WR5_TX_WORD_LENGTH_MASK	0xc0
#define Z80DART_WR5_TX_WORD_LENGTH_5	0x00
#define Z80DART_WR5_TX_WORD_LENGTH_7	0x40
#define Z80DART_WR5_TX_WORD_LENGTH_6	0x80
#define Z80DART_WR5_TX_WORD_LENGTH_8	0xc0
#define Z80DART_WR5_DTR					0x80

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _dart_channel dart_channel;
struct _dart_channel
{
	devcb_resolved_read_line	in_rxd_func;
	devcb_resolved_write_line	out_txd_func;
	devcb_resolved_write_line	out_dtr_func;
	devcb_resolved_write_line	out_rts_func;
	devcb_resolved_write_line	out_wrdy_func;

	/* register state */
	UINT8 rr[3];			/* read register */
	UINT8 wr[6];			/* write register */

	/* receiver state */
	UINT8 rx_data_fifo[3];	/* receive data FIFO */
	UINT8 rx_error_fifo[3];	/* receive error FIFO */
	UINT8 rx_shift;			/* 8-bit receive shift register */
	UINT8 rx_error;			/* current receive error */
	int rx_fifo;			/* receive FIFO pointer */

	int rx_clock;			/* receive clock pulse count */
	int rx_state;			/* receive state */
	int rx_bits;			/* bits received */
	int rx_first;			/* first character received */
	int rx_parity;			/* received data parity */
	int rx_break;			/* receive break condition */
	UINT8 rx_rr0_latch;		/* read register 0 latched */

	int ri;					/* ring indicator latch */
	int cts;				/* clear to send latch */
	int dcd;				/* data carrier detect latch */

	/* transmitter state */
	UINT8 tx_data;			/* transmit data register */
	UINT8 tx_shift;			/* transmit shift register */

	int tx_clock;			/* transmit clock pulse count */
	int tx_state;			/* transmit state */
	int tx_bits;			/* bits transmitted */
	int tx_parity;			/* transmitted data parity */

	int dtr;				/* data terminal ready */
	int rts;				/* request to send */
};

typedef struct _z80dart_t z80dart_t;
struct _z80dart_t
{
	devcb_resolved_write_line	out_int_func;

	dart_channel channel[2];		/* channels */

	int int_state[8];				/* interrupt state */

	/* timers */
	emu_timer *rxca_timer;
	emu_timer *txca_timer;
	emu_timer *rxtxcb_timer;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int z80dart_irq_state(running_device *device);
static void z80dart_irq_reti(running_device *device);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80dart_t *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80DART);
	return (z80dart_t *)device->token;
}

INLINE const z80dart_interface *get_interface(running_device *device)
{
	assert(device != NULL);
	assert((device->type == Z80DART));
	return (const z80dart_interface *) device->baseconfig().static_config;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

#define RXD \
	devcb_call_read_line(&ch->in_rxd_func)

#define TXD(_state) \
	devcb_call_write_line(&ch->out_txd_func, _state)

#define RTS(_state) \
	devcb_call_write_line(&ch->out_rts_func, _state)

#define DTR(_state) \
	devcb_call_write_line(&ch->out_dtr_func, _state)

/*-------------------------------------------------
    check_interrupts - control interrupt line
-------------------------------------------------*/

static void check_interrupts(running_device *device)
{
	z80dart_t *z80dart = get_safe_token(device);
	int state = (z80dart_irq_state(device) & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE;

	devcb_call_write_line(&z80dart->out_int_func, state);
}

/*-------------------------------------------------
    take_interrupt - trigger interrupt
-------------------------------------------------*/

static void take_interrupt(running_device *device, int channel, int level)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];
	UINT8 vector = ch->wr[2];
	int priority = (channel << 2) | level;

	LOG(("Z80DART \"%s\" Channel %c : Interrupt Request %u\n", device->tag(), 'A' + channel, level));

	if ((channel == Z80DART_CH_B) && (ch->wr[1] & Z80DART_WR1_STATUS_VECTOR))
	{
		/* status affects vector */
		vector = (ch->wr[2] & 0xf1) | (!channel << 3) | (level << 1);
	}

	/* update vector register */
	ch->rr[2] = vector;

	/* trigger interrupt */
	z80dart->int_state[priority] |= Z80_DAISY_INT;
	z80dart->channel[Z80DART_CH_A].rr[0] |= Z80DART_RR0_INTERRUPT_PENDING;

	/* check for interrupt */
	check_interrupts(device);
}

/*-------------------------------------------------
    get_clock_mode - get clock divisor
-------------------------------------------------*/

static int get_clock_mode(dart_channel *ch)
{
	int clocks = 1;

	switch (ch->wr[4] & Z80DART_WR4_CLOCK_MODE_MASK)
	{
	case Z80DART_WR4_CLOCK_MODE_X1:		clocks = 1;		break;
	case Z80DART_WR4_CLOCK_MODE_X16:	clocks = 16;	break;
	case Z80DART_WR4_CLOCK_MODE_X32:	clocks = 32;	break;
	case Z80DART_WR4_CLOCK_MODE_X64:	clocks = 64;	break;
	}

	return clocks;
}

/*-------------------------------------------------
    get_stop_bits - get number of stop bits
-------------------------------------------------*/

static float get_stop_bits(dart_channel *ch)
{
	float bits = 1;

	switch (ch->wr[4] & Z80DART_WR4_STOP_BITS_MASK)
	{
	case Z80DART_WR4_STOP_BITS_1:		bits = 1;		break;
	case Z80DART_WR4_STOP_BITS_1_5:		bits = 1.5;		break;
	case Z80DART_WR4_STOP_BITS_2:		bits = 2;		break;
	}

	return bits;
}

/*-------------------------------------------------
    get_rx_word_length - get receive word length
-------------------------------------------------*/

static int get_rx_word_length(dart_channel *ch)
{
	int bits = 5;

	switch (ch->wr[3] & Z80DART_WR3_RX_WORD_LENGTH_MASK)
	{
	case Z80DART_WR3_RX_WORD_LENGTH_5:	bits = 5;		break;
	case Z80DART_WR3_RX_WORD_LENGTH_6:	bits = 6;		break;
	case Z80DART_WR3_RX_WORD_LENGTH_7:	bits = 7;		break;
	case Z80DART_WR3_RX_WORD_LENGTH_8:	bits = 8;		break;
	}

	return bits;
}

/*-------------------------------------------------
    get_rx_word_length - get transmit word length
-------------------------------------------------*/

static int get_tx_word_length(dart_channel *ch)
{
	int bits = 5;

	switch (ch->wr[5] & Z80DART_WR5_TX_WORD_LENGTH_MASK)
	{
	case Z80DART_WR5_TX_WORD_LENGTH_5:	bits = 5;	break;
	case Z80DART_WR5_TX_WORD_LENGTH_6:	bits = 6;	break;
	case Z80DART_WR5_TX_WORD_LENGTH_7:	bits = 7;	break;
	case Z80DART_WR5_TX_WORD_LENGTH_8:	bits = 8;	break;
	}

	return bits;
}

/*-------------------------------------------------
    reset_channel - reset channel status
-------------------------------------------------*/

static void reset_channel(running_device *device, int channel)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	/* disable receiver */
	ch->wr[3] &= ~Z80DART_WR3_RX_ENABLE;
	ch->rx_state = STATE_START;

	/* disable transmitter */
	ch->wr[5] &= ~Z80DART_WR5_TX_ENABLE;
	ch->tx_state = STATE_START;

	/* reset external lines */
	RTS(1);
	DTR(1);

	if (channel == Z80DART_CH_A)
	{
		/* reset interrupt logic */
		int i;

		for (i = 0; i < 8; i++)
		{
			z80dart->int_state[i] = 0;
		}

		check_interrupts(device);
	}
}

/*-------------------------------------------------
    detect_start_bit - detect start bit
-------------------------------------------------*/

static int detect_start_bit(dart_channel *ch)
{
	if (!(ch->wr[3] & Z80DART_WR3_RX_ENABLE)) return 0;

	return !RXD;
}

/*-------------------------------------------------
    shift_data_in - shift in serial data
-------------------------------------------------*/

static void shift_data_in(dart_channel *ch)
{
	if (ch->rx_bits < 8)
	{
		int rxd = RXD;

		ch->rx_shift >>= 1;
		ch->rx_shift = (rxd << 7) | (ch->rx_shift & 0x7f);
		ch->rx_parity ^= rxd;
		ch->rx_bits++;
	}
}

/*-------------------------------------------------
    character_completed - check if complete
    data word has been transferred
-------------------------------------------------*/

static int character_completed(dart_channel *ch)
{
	return ch->rx_bits == get_rx_word_length(ch);
}

/*-------------------------------------------------
    detect_parity_error - detect parity error
-------------------------------------------------*/

static void detect_parity_error(running_device *device, int channel)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	int parity = (ch->wr[1] & Z80DART_WR4_PARITY_EVEN) ? 1 : 0;

	if (RXD != (ch->rx_parity ^ parity))
	{
		/* parity error detected */
		ch->rx_error |= Z80DART_RR1_PARITY_ERROR;

		switch (ch->wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_FIRST:
			if (!ch->rx_first)
			{
				take_interrupt(device, channel, INT_SPECIAL);
			}
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
			take_interrupt(device, channel, INT_SPECIAL);
			break;

		case Z80DART_WR1_RX_INT_ALL:
			take_interrupt(device, channel, INT_RECEIVE);
			break;
		}
	}
}

/*-------------------------------------------------
    detect_framing_error - detect framing error
-------------------------------------------------*/

static void detect_framing_error(running_device *device, int channel)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	if (!RXD)
	{
		/* framing error detected */
		ch->rx_error |= Z80DART_RR1_FRAMING_ERROR;

		switch (ch->wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_FIRST:
			if (!ch->rx_first)
			{
				take_interrupt(device, channel, INT_SPECIAL);
			}
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
		case Z80DART_WR1_RX_INT_ALL:
			take_interrupt(device, channel, INT_SPECIAL);
			break;
		}
	}
}

/*-------------------------------------------------
    receive - receive serial data
-------------------------------------------------*/

static void receive(running_device *device, int channel)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	float stop_bits = get_stop_bits(ch);

	switch (ch->rx_state)
	{
	case STATE_START:
		/* check for start bit */
		if (detect_start_bit(ch))
		{
			/* start bit detected */
			ch->rx_shift = 0;
			ch->rx_error = 0;
			ch->rx_bits = 0;
			ch->rx_parity = 0;

			/* next bit is a data bit */
			ch->rx_state = STATE_DATA;
		}
		break;

	case STATE_DATA:
		/* shift bit into shift register */
		shift_data_in(ch);

		if (character_completed(ch))
		{
			/* all data bits received */
			if (ch->wr[4] & Z80DART_WR4_PARITY_ENABLE)
			{
				/* next bit is the parity bit */
				ch->rx_state = STATE_PARITY;
			}
			else
			{
				/* next bit is a STOP bit */
				if (stop_bits == 1)
					ch->rx_state = STATE_STOP2;
				else
					ch->rx_state = STATE_STOP;
			}
		}
		break;

	case STATE_PARITY:
		/* shift bit into shift register */
		shift_data_in(ch);

		/* check for parity error */
		detect_parity_error(device, channel);

		/* next bit is a STOP bit */
		if (stop_bits == 1)
			ch->rx_state = STATE_STOP2;
		else
			ch->rx_state = STATE_STOP;
		break;

	case STATE_STOP:
		/* shift bit into shift register */
		shift_data_in(ch);

		/* check for framing error */
		detect_framing_error(device, channel);

		/* next bit is the second STOP bit */
		ch->rx_state = STATE_STOP2;
		break;

	case STATE_STOP2:
		/* shift bit into shift register */
		shift_data_in(ch);

		/* check for framing error */
		detect_framing_error(device, channel);

		/* store data into FIFO */
		z80dart_receive_data(device, channel, ch->rx_shift);

		/* next bit is the START bit */
		ch->rx_state = STATE_START;
		break;
	}
}

/*-------------------------------------------------
    transmit - transmit serial data
-------------------------------------------------*/

static void transmit(running_device *device, int channel)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	int word_length = get_tx_word_length(ch);
	float stop_bits = get_stop_bits(ch);

	switch (ch->tx_state)
	{
	case STATE_START:
		if ((ch->wr[5] & Z80DART_WR5_TX_ENABLE) && !(ch->rr[0] & Z80DART_RR0_TX_BUFFER_EMPTY))
		{
			/* transmit start bit */
			TXD(0);

			ch->tx_bits = 0;
			ch->tx_shift = ch->tx_data;

			/* empty transmit buffer */
			ch->rr[0] |= Z80DART_RR0_TX_BUFFER_EMPTY;

			if (ch->wr[1] & Z80DART_WR1_TX_INT_ENABLE)
				take_interrupt(device, channel, INT_TRANSMIT);

			ch->tx_state = STATE_DATA;
		}
		else if (ch->wr[5] & Z80DART_WR5_SEND_BREAK)
		{
			/* transmit break */
			TXD(0);
		}
		else
		{
			/* transmit marking line */
			TXD(1);
		}

		break;

	case STATE_DATA:
		/* transmit data bit */
		TXD(BIT(ch->tx_shift, 0));

		/* shift data */
		ch->tx_shift >>= 1;
		ch->tx_bits++;

		if (ch->rx_bits == word_length)
		{
			if (ch->wr[4] & Z80DART_WR4_PARITY_ENABLE)
				ch->tx_state = STATE_PARITY;
			else
			{
				if (stop_bits == 1)
					ch->tx_state = STATE_STOP2;
				else
					ch->tx_state = STATE_STOP;
			}
		}
		break;

	case STATE_PARITY:
		// TODO: calculate parity
		if (stop_bits == 1)
			ch->tx_state = STATE_STOP2;
		else
			ch->tx_state = STATE_STOP;
		break;

	case STATE_STOP:
		/* transmit stop bit */
		TXD(1);

		ch->tx_state = STATE_STOP2;
		break;

	case STATE_STOP2:
		/* transmit stop bit */
		TXD(1);

		/* if transmit buffer is empty */
		if (ch->rr[0] & Z80DART_RR0_TX_BUFFER_EMPTY)
		{
			/* then all characters have been sent */
			ch->rr[1] |= Z80DART_RR1_ALL_SENT;

			/* when the RTS bit is reset, the _RTS output goes high after the transmitter empties */
			if (!ch->rts)
				RTS(1);
		}

		ch->tx_state = STATE_START;
		break;
	}
}

/*-------------------------------------------------
    z80dart_c_r - read control register
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80dart_c_r )
{
	z80dart_t *z80dart = get_safe_token(device);
	int channel = offset & 0x01;
	dart_channel *ch = &z80dart->channel[channel];
	UINT8 data = 0;

	int reg = ch->wr[0] & Z80DART_WR0_REGISTER_MASK;

	switch (reg)
	{
	case 0:
	case 1:
		data = ch->rr[reg];
		break;

	case 2:
		/* channel B only */
		if (channel == Z80DART_CH_B)
			data = ch->rr[reg];
		break;
	}

	LOG(("Z80DART \"%s\" Channel %c : Control Register Read '%02x'\n", device->tag(), 'A' + channel, data));

	return data;
}

/*-------------------------------------------------
    z80dart_c_w - write control register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80dart_c_w )
{
	z80dart_t *z80dart = get_safe_token(device);
	int channel = offset & 0x01;
	dart_channel *ch = &z80dart->channel[channel];

	int reg = ch->wr[0] & Z80DART_WR0_REGISTER_MASK;

	LOG(("Z80DART \"%s\" Channel %c : Control Register Write '%02x'\n", device->tag(), 'A' + channel, data));

	/* write data to selected register */
	ch->wr[reg] = data;

	if (reg != 0)
	{
		/* mask out register index */
		ch->wr[0] &= ~Z80DART_WR0_REGISTER_MASK;
	}

	switch (reg)
	{
	case 0:
		switch (data & Z80DART_WR0_COMMAND_MASK)
		{
		case Z80DART_WR0_RESET_EXT_STATUS:
			/* reset external/status interrupt */
			ch->rr[0] &= ~(Z80DART_RR0_DCD | Z80DART_RR0_RI | Z80DART_RR0_CTS | Z80DART_RR0_BREAK);

			if (!ch->dcd) ch->rr[0] |= Z80DART_RR0_DCD;
			if (ch->ri) ch->rr[0] |= Z80DART_RR0_RI;
			if (ch->cts) ch->rr[0] |= Z80DART_RR0_CTS;

			ch->rx_rr0_latch = 0;

			LOG(("Z80DART \"%s\" Channel %c : Reset External/Status Interrupt\n", device->tag(), 'A' + channel));
			break;

		case Z80DART_WR0_CHANNEL_RESET:
			/* channel reset */
			LOG(("Z80DART \"%s\" Channel %c : Channel Reset\n", device->tag(), 'A' + channel));
			reset_channel(device, channel);
			break;

		case Z80DART_WR0_ENABLE_INT_NEXT_RX:
			/* enable interrupt on next receive character */
			LOG(("Z80DART \"%s\" Channel %c : Enable Interrupt on Next Received Character\n", device->tag(), 'A' + channel));
			ch->rx_first = 1;
			break;

		case Z80DART_WR0_RESET_TX_INT:
			/* reset transmitter interrupt pending */
			LOG(("Z80DART \"%s\" Channel %c : Reset Transmitter Interrupt Pending\n", device->tag(), 'A' + channel));
			break;

		case Z80DART_WR0_ERROR_RESET:
			/* error reset */
			LOG(("Z80DART \"%s\" Channel %c : Error Reset\n", device->tag(), 'A' + channel));
			ch->rr[1] &= ~(Z80DART_RR1_FRAMING_ERROR | Z80DART_RR1_RX_OVERRUN_ERROR | Z80DART_RR1_PARITY_ERROR);
			break;

		case Z80DART_WR0_RETURN_FROM_INT:
			/* return from interrupt */
			LOG(("Z80DART \"%s\" Channel %c : Return from Interrupt\n", device->tag(), 'A' + channel));
			z80dart_irq_reti(device);
			break;
		}
		break;

	case 1:
		LOG(("Z80DART \"%s\" Channel %c : External Interrupt Enable %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR1_EXT_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmit Interrupt Enable %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR1_TX_INT_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Status Affects Vector %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR1_STATUS_VECTOR) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Wait/Ready Enable %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR1_WRDY_ENABLE) ? 1 : 0));

		switch (data & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_DISABLE:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt Disabled\n", device->tag(), 'A' + channel));
			break;

		case Z80DART_WR1_RX_INT_FIRST:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on First Character\n", device->tag(), 'A' + channel));
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters, Parity Affects Vector\n", device->tag(), 'A' + channel));
			break;

		case Z80DART_WR1_RX_INT_ALL:
			LOG(("Z80DART \"%s\" Channel %c : Receiver Interrupt on All Characters\n", device->tag(), 'A' + channel));
			break;
		}

		check_interrupts(device);
		break;

	case 2:
		/* interrupt vector */
		check_interrupts(device);
		LOG(("Z80DART \"%s\" Channel %c : Interrupt Vector %02x\n", device->tag(), 'A' + channel, data));
		break;

	case 3:
		LOG(("Z80DART \"%s\" Channel %c : Receiver Enable %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR3_RX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Auto Enables %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR3_AUTO_ENABLES) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Receiver Bits/Character %u\n", device->tag(), 'A' + channel, get_rx_word_length(ch)));
		break;

	case 4:
		LOG(("Z80DART \"%s\" Channel %c : Parity Enable %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR4_PARITY_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Parity %s\n", device->tag(), 'A' + channel, (data & Z80DART_WR4_PARITY_EVEN) ? "Even" : "Odd"));
		LOG(("Z80DART \"%s\" Channel %c : Stop Bits %f\n", device->tag(), 'A' + channel, get_stop_bits(ch)));
		LOG(("Z80DART \"%s\" Channel %c : Clock Mode %uX\n", device->tag(), 'A' + channel, get_clock_mode(ch)));
		break;

	case 5:
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Enable %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR5_TX_ENABLE) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Transmitter Bits/Character %u\n", device->tag(), 'A' + channel, get_tx_word_length(ch)));
		LOG(("Z80DART \"%s\" Channel %c : Send Break %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR5_SEND_BREAK) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Request to Send %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR5_RTS) ? 1 : 0));
		LOG(("Z80DART \"%s\" Channel %c : Data Terminal Ready %u\n", device->tag(), 'A' + channel, (data & Z80DART_WR5_DTR) ? 1 : 0));

		if (data & Z80DART_WR5_RTS)
		{
			/* when the RTS bit is set, the _RTS output goes low */
			RTS(0);

			ch->rts = 1;
		}
		else
		{
			/* when the RTS bit is reset, the _RTS output goes high after the transmitter empties */
			ch->rts = 0;
		}

		/* data terminal ready output follows the state programmed into the DTR bit*/
		ch->dtr = (data & Z80DART_WR5_DTR) ? 0 : 1;
		DTR(ch->dtr);
		break;
	}
}

/*-------------------------------------------------
    z80dart_d_r - read data register
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80dart_d_r )
{
	z80dart_t *z80dart = get_safe_token(device);
	int channel = offset & 0x01;
	dart_channel *ch = &z80dart->channel[channel];
	UINT8 data = 0;

	if (ch->rx_fifo >= 0)
	{
		/* load data from the FIFO */
		data = ch->rx_data_fifo[ch->rx_fifo];

		/* load error status from the FIFO, retain overrun and parity errors */
		ch->rr[1] = (ch->rr[1] & (Z80DART_RR1_RX_OVERRUN_ERROR | Z80DART_RR1_PARITY_ERROR)) | ch->rx_error_fifo[ch->rx_fifo];

		/* decrease FIFO pointer */
		ch->rx_fifo--;

		if (ch->rx_fifo < 0)
		{
			/* no more characters available in the FIFO */
			ch->rr[0] &= ~ Z80DART_RR0_RX_CHAR_AVAILABLE;
		}
	}

	LOG(("Z80DART \"%s\" Channel %c : Data Register Read '%02x'\n", device->tag(), 'A' + channel, data));

	return data;
}

/*-------------------------------------------------
    z80dart_d_w - write data register
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80dart_d_w )
{
	z80dart_t *z80dart = get_safe_token(device);
	int channel = offset & 0x01;
	dart_channel *ch = &z80dart->channel[channel];

	ch->tx_data = data;

	ch->rr[0] &= ~Z80DART_RR0_TX_BUFFER_EMPTY;
	ch->rr[1] &= ~Z80DART_RR1_ALL_SENT;

	LOG(("Z80DART \"%s\" Channel %c : Data Register Write '%02x'\n", device->tag(), 'A' + channel, data));
}

/*-------------------------------------------------
    z80dart_receive_data - receive data word
-------------------------------------------------*/

void z80dart_receive_data(running_device *device, int channel, UINT8 data)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	LOG(("Z80DART \"%s\" Channel %c : Receive Data Byte '%02x'\n", device->tag(), 'A' + channel, data));

	if (ch->rx_fifo == 2)
	{
		/* receive overrun error detected */
		ch->rx_error |= Z80DART_RR1_RX_OVERRUN_ERROR;

		switch (ch->wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
		{
		case Z80DART_WR1_RX_INT_FIRST:
			if (!ch->rx_first)
			{
				take_interrupt(device, channel, INT_SPECIAL);
			}
			break;

		case Z80DART_WR1_RX_INT_ALL_PARITY:
		case Z80DART_WR1_RX_INT_ALL:
			take_interrupt(device, channel, INT_SPECIAL);
			break;
		}
	}
	else
	{
		ch->rx_fifo++;
	}

	/* store received character and error status into FIFO */
	ch->rx_data_fifo[ch->rx_fifo] = data;
	ch->rx_error_fifo[ch->rx_fifo] = ch->rx_error;

	ch->rr[0] |= Z80DART_RR0_RX_CHAR_AVAILABLE;

	/* receive interrupt */
	switch (ch->wr[1] & Z80DART_WR1_RX_INT_ENABLE_MASK)
	{
	case Z80DART_WR1_RX_INT_FIRST:
		if (ch->rx_first)
		{
			take_interrupt(device, channel, INT_RECEIVE);

			ch->rx_first = 0;
		}
		break;

	case Z80DART_WR1_RX_INT_ALL_PARITY:
	case Z80DART_WR1_RX_INT_ALL:
		take_interrupt(device, channel, INT_RECEIVE);
		break;
	}
}

/*-------------------------------------------------
    cts_w - clear to send handler
-------------------------------------------------*/

static void cts_w(running_device *device, int channel, int state)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	LOG(("Z80DART \"%s\" Channel %c : CTS %u\n", device->tag(), 'A' + channel, state));

	if (ch->cts != state)
	{
		/* enable transmitter if in auto enables mode */
		if (!state)
			if (ch->wr[3] & Z80DART_WR3_AUTO_ENABLES)
				ch->wr[5] |= Z80DART_WR5_TX_ENABLE;

		/* set clear to send */
		ch->cts = state;

		if (!ch->rx_rr0_latch)
		{
			if (!ch->cts)
				ch->rr[0] |= Z80DART_RR0_CTS;
			else
				ch->rr[0] &= ~Z80DART_RR0_CTS;

			/* trigger interrupt */
			if (ch->wr[1] & Z80DART_WR1_EXT_INT_ENABLE)
			{
				/* trigger interrupt */
				take_interrupt(device, channel, INT_EXTERNAL);

				/* latch read register 0 */
				ch->rx_rr0_latch = 1;
			}
		}
	}
}

/*-------------------------------------------------
    z80dart_ctsa_w - clear to send (channel A)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_ctsa_w )
{
	cts_w(device, Z80DART_CH_A, state);
}

/*-------------------------------------------------
    z80dart_ctsb_w - clear to send (channel B)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_ctsb_w )
{
	cts_w(device, Z80DART_CH_B, state);
}

/*-------------------------------------------------
    dcd_w - data carrier detected handler
-------------------------------------------------*/

static void dcd_w(running_device *device, int channel, int state)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	LOG(("Z80DART \"%s\" Channel %c : DCD %u\n", device->tag(), 'A' + channel, state));

	if (ch->dcd != state)
	{
		/* enable receiver if in auto enables mode */
		if (!state)
			if (ch->wr[3] & Z80DART_WR3_AUTO_ENABLES)
				ch->wr[3] |= Z80DART_WR3_RX_ENABLE;

		/* set data carrier detect */
		ch->dcd = state;

		if (!ch->rx_rr0_latch)
		{
			if (ch->dcd)
				ch->rr[0] |= Z80DART_RR0_DCD;
			else
				ch->rr[0] &= ~Z80DART_RR0_DCD;

			if (ch->wr[1] & Z80DART_WR1_EXT_INT_ENABLE)
			{
				/* trigger interrupt */
				take_interrupt(device, channel, INT_EXTERNAL);

				/* latch read register 0 */
				ch->rx_rr0_latch = 1;
			}
		}
	}
}

/*-------------------------------------------------
    z80dart_dcda_w - data carrier detected
    (channel A)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_dcda_w )
{
	dcd_w(device, Z80DART_CH_A, state);
}

/*-------------------------------------------------
    z80dart_dcdb_w - data carrier detected
    (channel B)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_dcdb_w )
{
	dcd_w(device, Z80DART_CH_B, state);
}

/*-------------------------------------------------
    ri_w - ring indicator handler
-------------------------------------------------*/

static void ri_w(running_device *device, int channel, int state)
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[channel];

	LOG(("Z80DART \"%s\" Channel %c : RI %u\n", device->tag(), 'A' + channel, state));

	if (ch->ri != state)
	{
		/* set ring indicator state */
		ch->ri = state;

		if (!ch->rx_rr0_latch)
		{
			if (ch->ri)
				ch->rr[0] |= Z80DART_RR0_RI;
			else
				ch->rr[0] &= ~Z80DART_RR0_RI;

			if (ch->wr[1] & Z80DART_WR1_EXT_INT_ENABLE)
			{
				/* trigger interrupt */
				take_interrupt(device, channel, INT_EXTERNAL);

				/* latch read register 0 */
				ch->rx_rr0_latch = 1;
			}
		}
	}
}

/*-------------------------------------------------
    z80dart_ria_w - ring indicator (channel A)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_ria_w )
{
	ri_w(device, Z80DART_CH_A, state);
}

/*-------------------------------------------------
    z80dart_rib_w - ring indicator (channel B)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_rib_w )
{
	ri_w(device, Z80DART_CH_B, state);
}

/*-------------------------------------------------
    z80dart_rxca_w - receive clock (channel A)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_rxca_w )
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[Z80DART_CH_A];

	int clocks = get_clock_mode(ch);

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel A : Receiver Clock Pulse\n", device->tag()));

	ch->rx_clock++;

	if (ch->rx_clock == clocks)
	{
		ch->rx_clock = 0;

		/* receive data */
		receive(device, Z80DART_CH_A);
	}
}

/*-------------------------------------------------
    z80dart_txca_w - transmit clock (channel A)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_txca_w )
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[Z80DART_CH_A];

	int clocks = get_clock_mode(ch);

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel A : Transmitter Clock Pulse\n", device->tag()));

	ch->tx_clock++;

	if (ch->tx_clock == clocks)
	{
		ch->tx_clock = 0;

		/* transmit data */
		transmit(device, Z80DART_CH_A);
	}
}

/*-------------------------------------------------
    z80dart_rxtxcb_w - receive/transmit clock
    (channel B)
-------------------------------------------------*/

WRITE_LINE_DEVICE_HANDLER( z80dart_rxtxcb_w )
{
	z80dart_t *z80dart = get_safe_token(device);
	dart_channel *ch = &z80dart->channel[Z80DART_CH_B];

	int clocks = get_clock_mode(ch);

	if (!state) return;

	LOG(("Z80DART \"%s\" Channel A : Receiver/Transmitter Clock Pulse\n", device->tag()));

	ch->rx_clock++;
	ch->tx_clock++;

	if (ch->rx_clock == clocks)
	{
		ch->rx_clock = 0;
		ch->tx_clock = 0;

		/* receive and transmit data */
		receive(device, Z80DART_CH_B);
		transmit(device, Z80DART_CH_B);
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( rxca_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( rxca_tick )
{
	z80dart_rxca_w((running_device *)ptr, 1);
}

/*-------------------------------------------------
    TIMER_CALLBACK( txca_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( txca_tick )
{
	z80dart_txca_w((running_device *)ptr, 1);
}

/*-------------------------------------------------
    TIMER_CALLBACK( rxtxcb_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( rxtxcb_tick )
{
	z80dart_rxtxcb_w((running_device *)ptr, 1);
}

/*-------------------------------------------------
    z80dart_irq_state - get interrupt status
-------------------------------------------------*/

static int z80dart_irq_state(running_device *device)
{
	z80dart_t *z80dart = get_safe_token( device );
	int state = 0;
	int i;

	LOG(("Z80DART \"%s\" : Interrupt State B:%d%d%d%d A:%d%d%d%d\n", device->tag(),
				z80dart->int_state[0], z80dart->int_state[1], z80dart->int_state[2], z80dart->int_state[3],
				z80dart->int_state[4], z80dart->int_state[5], z80dart->int_state[6], z80dart->int_state[7]));

	/* loop over all interrupt sources */
	for (i = 0; i < 8; i++)
	{
		/* if we're servicing a request, don't indicate more interrupts */
		if (z80dart->int_state[i] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= z80dart->int_state[i];
	}

	LOG(("Z80DART \"%s\" : Interrupt State %u\n", device->tag(), state));

	return state;
}

/*-------------------------------------------------
    z80dart_irq_ack - interrupt acknowledge
-------------------------------------------------*/

static int z80dart_irq_ack(running_device *device)
{
	z80dart_t *z80dart = get_safe_token( device );
	int i;

	LOG(("Z80DART \"%s\" Interrupt Acknowledge\n", device->tag()));

	/* loop over all interrupt sources */
	for (i = 0; i < 8; i++)
	{
		/* find the first channel with an interrupt requested */
		if (z80dart->int_state[i] & Z80_DAISY_INT)
		{
			/* clear interrupt, switch to the IEO state, and update the IRQs */
			z80dart->int_state[i] = Z80_DAISY_IEO;
			z80dart->channel[Z80DART_CH_A].rr[0] &= ~Z80DART_RR0_INTERRUPT_PENDING;
			check_interrupts(device);

			LOG(("Z80DART \"%s\" : Interrupt Acknowledge Vector %02x\n", device->tag(), z80dart->channel[Z80DART_CH_B].rr[2]));

			return z80dart->channel[Z80DART_CH_B].rr[2];
		}
	}

	logerror("z80dart_irq_ack: failed to find an interrupt to ack!\n");

	return z80dart->channel[Z80DART_CH_B].rr[2];
}

/*-------------------------------------------------
    z80dart_irq_reti - return from interrupt
-------------------------------------------------*/

static void z80dart_irq_reti(running_device *device)
{
	z80dart_t *z80dart = get_safe_token( device );
	int i;

	LOG(("Z80DART \"%s\" Return from Interrupt\n", device->tag()));

	/* loop over all interrupt sources */
	for (i = 0; i < 8; i++)
	{
		/* find the first channel with an IEO pending */
		if (z80dart->int_state[i] & Z80_DAISY_IEO)
		{
			/* clear the IEO state and update the IRQs */
			z80dart->int_state[i] &= ~Z80_DAISY_IEO;
			check_interrupts(device);
			return;
		}
	}

	logerror("z80dart_irq_reti: failed to find an interrupt to clear IEO on!\n");
}

/*-------------------------------------------------
    z80dart_cd_ba_r - register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80dart_cd_ba_r )
{
	return (offset & 2) ? z80dart_c_r(device, offset & 1) : z80dart_d_r(device, offset & 1);
}

/*-------------------------------------------------
    z80dart_cd_ba_w - register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80dart_cd_ba_w )
{
	if (offset & 2)
		z80dart_c_w(device, offset & 1, data);
	else
		z80dart_d_w(device, offset & 1, data);
}

/*-------------------------------------------------
    z80dart_ba_cd_r - register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( z80dart_ba_cd_r )
{
	int channel = BIT(offset, 1);

	return (offset & 1) ? z80dart_c_r(device, channel) : z80dart_d_r(device, channel);
}

/*-------------------------------------------------
    z80dart_ba_cd_w - register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( z80dart_ba_cd_w )
{
	int channel = BIT(offset, 1);

	if (offset & 1)
		z80dart_c_w(device, channel, data);
	else
		z80dart_d_w(device, channel, data);
}

/*-------------------------------------------------
    DEVICE_START( z80dart )
-------------------------------------------------*/

static DEVICE_START( z80dart )
{
	z80dart_t *z80dart = get_safe_token(device);
	const z80dart_interface *intf = (const z80dart_interface *)device->baseconfig().static_config;
	int channel;

	/* resolve callbacks */
	devcb_resolve_read_line(&z80dart->channel[Z80DART_CH_A].in_rxd_func, &intf->in_rxda_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_A].out_txd_func, &intf->out_txda_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_A].out_dtr_func, &intf->out_dtra_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_A].out_rts_func, &intf->out_rtsa_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_A].out_wrdy_func, &intf->out_wrdya_func, device);
	devcb_resolve_read_line(&z80dart->channel[Z80DART_CH_B].in_rxd_func, &intf->in_rxdb_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_B].out_txd_func, &intf->out_txdb_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_B].out_dtr_func, &intf->out_dtrb_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_B].out_rts_func, &intf->out_rtsb_func, device);
	devcb_resolve_write_line(&z80dart->channel[Z80DART_CH_B].out_wrdy_func, &intf->out_wrdyb_func, device);
	devcb_resolve_write_line(&z80dart->out_int_func, &intf->out_int_func, device);

	if (intf->rx_clock_a)
	{
		/* allocate channel A receive timer */
		z80dart->rxca_timer = timer_alloc(device->machine, rxca_tick, (void *)device);
		timer_adjust_periodic(z80dart->rxca_timer, attotime_zero, 0, ATTOTIME_IN_HZ(intf->rx_clock_a));
	}

	if (intf->tx_clock_a)
	{
		/* allocate channel A transmit timer */
		z80dart->txca_timer = timer_alloc(device->machine, txca_tick, (void *)device);
		timer_adjust_periodic(z80dart->txca_timer, attotime_zero, 0, ATTOTIME_IN_HZ(intf->tx_clock_a));
	}

	if (intf->rx_tx_clock_b)
	{
		/* allocate channel B receive/transmit timer */
		z80dart->rxtxcb_timer = timer_alloc(device->machine, rxtxcb_tick, (void *)device);
		timer_adjust_periodic(z80dart->rxtxcb_timer, attotime_zero, 0, ATTOTIME_IN_HZ(intf->rx_tx_clock_b));
	}

	/* register for state saving */
	for (channel = Z80DART_CH_A; channel <= Z80DART_CH_B; channel++)
	{
		state_save_register_device_item_array(device, channel, z80dart->channel[channel].rr);
		state_save_register_device_item_array(device, channel, z80dart->channel[channel].wr);
		state_save_register_device_item_array(device, channel, z80dart->channel[channel].rx_data_fifo);
		state_save_register_device_item_array(device, channel, z80dart->channel[channel].rx_error_fifo);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_shift);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_error);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_fifo);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_clock);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_state);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_bits);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_first);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_parity);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_break);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rx_rr0_latch);
		state_save_register_device_item(device, channel, z80dart->channel[channel].ri);
		state_save_register_device_item(device, channel, z80dart->channel[channel].cts);
		state_save_register_device_item(device, channel, z80dart->channel[channel].dcd);
		state_save_register_device_item(device, channel, z80dart->channel[channel].tx_data);
		state_save_register_device_item(device, channel, z80dart->channel[channel].tx_shift);
		state_save_register_device_item(device, channel, z80dart->channel[channel].tx_clock);
		state_save_register_device_item(device, channel, z80dart->channel[channel].tx_state);
		state_save_register_device_item(device, channel, z80dart->channel[channel].tx_bits);
		state_save_register_device_item(device, channel, z80dart->channel[channel].tx_parity);
		state_save_register_device_item(device, channel, z80dart->channel[channel].dtr);
		state_save_register_device_item(device, channel, z80dart->channel[channel].rts);
	}

	state_save_register_device_item_array(device, 0, z80dart->int_state);
}

/*-------------------------------------------------
    DEVICE_RESET( z80dart )
-------------------------------------------------*/

static DEVICE_RESET( z80dart )
{
	int channel;

	LOG(("Z80DART \"%s\" Reset\n", device->tag()));

	for (channel = Z80DART_CH_A; channel <= Z80DART_CH_B; channel++)
	{
		reset_channel(device, channel);
	}

	check_interrupts(device);
}

/*-------------------------------------------------
    DEVICE_GET_INFO( z80dart )
-------------------------------------------------*/

DEVICE_GET_INFO( z80dart )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80dart_t);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;									break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80dart);		break;
		case DEVINFO_FCT_STOP:							/* Nothing */									break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80dart);		break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80dart_irq_state;			break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80dart_irq_ack;				break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80dart_irq_reti;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Zilog Z80 DART");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80");							break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright the MESS Team");		break;
	}
}
