/*
    National Semiconductor PC16552D
    Dual Universal Asynchronous Receiver/Transmitter with FIFOs

    Written by Ville Linde
*/

#include "emu.h"
#include "pc16552d.h"

#define REG_RECV_BUFFER			0x0		// Read
#define REG_XMIT_HOLD			0x0		// Write
#define REG_INT_ENABLE			0x1
#define REG_FIFO_CTRL			0x2		// Write
#define REG_LINE_CTRL			0x3
#define REG_MODEL_CTRL			0x4
#define REG_LINE_STATUS			0x5
#define REG_MODEM_STATUS		0x6
#define REG_SCRATCH				0x7
#define REG_DIV_LATCH_LSB		0x0		// When DLAB == 1
#define REG_DIV_LATCH_MSB		0x1		// When DLAB == 1
#define REG_ALT_FUNCTION		0x2		// When DLAB == 1

#define LINE_CTRL_DLAB			0x80

#define IRQ_RX_LINE_STATUS			0x1
#define IRQ_RX_DATA_AVAILABLE		0x2
#define IRQ_CHARACTER_TIMEOUT		0x4
#define IRQ_TX_HOLDING_REG_EMPTY	0x8
#define IRQ_MODEM_STATUS			0x10

#define INT_ENABLE_RX_DATA			0x01
#define INT_ENABLE_TX_EMPTY			0x02
#define INT_ENABLE_RX_LINE_STATUS	0x04
#define INT_ENABLE_MODEM_STATUS		0x08

struct PC16552D_CHANNEL
{
	UINT16 divisor;
	UINT8 reg[8];
	UINT8 rx_fifo[16];
	UINT8 tx_fifo[16];
	int pending_interrupt;
	int rx_fifo_read_ptr;
	int rx_fifo_write_ptr;
	int rx_fifo_num;
	int tx_fifo_read_ptr;
	int tx_fifo_write_ptr;
	int tx_fifo_num;
	emu_timer *tx_fifo_timer;
};

struct PC16552D_REGS
{
	PC16552D_CHANNEL ch[2];
	int frequency;
	void (* irq_handler)(running_machine &machine, int channel, int value);
	void (* tx_callback)(running_machine &machine, int channel, int count, UINT8* data);
};

#define MAX_PC16552D_CHIPS		4

static PC16552D_REGS duart[MAX_PC16552D_CHIPS];



static const int rx_trigger_level[4] = { 1, 4, 8, 14 };


static void check_interrupts(running_machine &machine, int chip, int channel)
{
	PC16552D_CHANNEL *ch = &duart[chip].ch[channel];
	int signal = 0;

	if (ch->pending_interrupt != 0)
	{
		if (((ch->reg[REG_INT_ENABLE] & INT_ENABLE_RX_DATA) && (ch->pending_interrupt & IRQ_RX_DATA_AVAILABLE)) ||
			((ch->reg[REG_INT_ENABLE] & INT_ENABLE_TX_EMPTY) && (ch->pending_interrupt & IRQ_TX_HOLDING_REG_EMPTY)) ||
			((ch->reg[REG_INT_ENABLE] & INT_ENABLE_RX_LINE_STATUS) && (ch->pending_interrupt & IRQ_RX_LINE_STATUS)) ||
			((ch->reg[REG_INT_ENABLE] & INT_ENABLE_MODEM_STATUS) && (ch->pending_interrupt & IRQ_MODEM_STATUS)))
		{
			signal = 1;
		}
	}

	if (duart[chip].irq_handler != NULL)
	{
		duart[chip].irq_handler(machine, channel, signal ? ASSERT_LINE : CLEAR_LINE);
	}
}

static void duart_push_rx_fifo(running_machine &machine, int chip, int channel, UINT8 data)
{
	PC16552D_CHANNEL *ch = &duart[chip].ch[channel];

	if (ch->rx_fifo_num >= 16)
	{
		printf("duart_push_rx_fifo: %d, %d, %02X, FIFO overflow\n", chip, channel, data);
		return;
	}

	ch->rx_fifo[ch->rx_fifo_write_ptr++] = data;
	if (ch->rx_fifo_write_ptr == 16)
	{
		ch->rx_fifo_write_ptr = 0;
	}
	ch->rx_fifo_num++;

	if (ch->rx_fifo_num == rx_trigger_level[(ch->reg[REG_FIFO_CTRL] >> 6) & 3])
	{
		ch->pending_interrupt |= IRQ_RX_DATA_AVAILABLE;		// INT ID: received data available

		check_interrupts(machine, chip, channel);
	}
}

static UINT8 duart_pop_rx_fifo(running_machine &machine, int chip, int channel)
{
	UINT8 r;
	PC16552D_CHANNEL *ch = &duart[chip].ch[channel];

	if (ch->rx_fifo_num == 0)
	{
		printf("duart_pop_rx_fifo: %d, %d, FIFO underflow\n", chip, channel);
		return 0;
	}

	r = ch->rx_fifo[ch->rx_fifo_read_ptr++];
	if (ch->rx_fifo_read_ptr == 16)
	{
		ch->rx_fifo_read_ptr = 0;
	}
	ch->rx_fifo_num--;

	if (ch->rx_fifo_num < rx_trigger_level[(ch->reg[REG_FIFO_CTRL] >> 6) & 3])
	{
		ch->pending_interrupt &= ~IRQ_RX_DATA_AVAILABLE;

		check_interrupts(machine, chip, channel);
	}

	return r;
}

static TIMER_CALLBACK( tx_fifo_timer_callback )
{
	PC16552D_CHANNEL *ch;
	int chip = param >> 1;
	int channel = param & 1;

	ch = &duart[chip].ch[channel];

	if (duart[chip].tx_callback)
		duart[chip].tx_callback(machine, channel, ch->tx_fifo_num, ch->tx_fifo);

	ch->tx_fifo_num = 0;

	// set transmitter empty interrupt
	ch->pending_interrupt |= IRQ_TX_HOLDING_REG_EMPTY;
	check_interrupts(machine, chip, channel);

	duart[chip].ch[channel].tx_fifo_timer->adjust(attotime::never, (chip * 2) + channel);
}

static void duart_push_tx_fifo(int chip, int channel, UINT8 data)
{
	attotime period;
	PC16552D_CHANNEL *ch = &duart[chip].ch[channel];

	ch->tx_fifo[ch->tx_fifo_num] = data;
	ch->tx_fifo_num++;

	period = attotime::from_hz(duart[chip].frequency) * (ch->divisor * 16 * 16 * 8);

	duart[chip].ch[channel].tx_fifo_timer->adjust(period, (chip * 2) + channel);
}

#ifdef UNUSED_FUNCTION
static UINT8 duart_pop_tx_fifo(int chip, int channel, UINT8 data)
{
	return 0;
}
#endif


static UINT8 duart_r(running_machine &machine, int chip, int reg)
{
	int channel = (reg >> 3) & 1;
	PC16552D_CHANNEL *ch = &duart[chip].ch[channel];
	reg &= 7;

//  printf("duart_r: chip %d, ch %d, reg %d\n", chip, channel, reg);

	switch (reg)
	{
		case 0:
		{
			if (ch->reg[REG_LINE_CTRL] & LINE_CTRL_DLAB)
			{
				// Divisor Latch (LSB)
				return ch->divisor & 0xff;
			}
			else
			{
				// Receiver Buffer
				ch->pending_interrupt &= ~IRQ_RX_DATA_AVAILABLE;

				check_interrupts(machine, chip, channel);

				return duart_pop_rx_fifo(machine, chip, channel);
			}
		}
		case 1:
		{
			if (ch->reg[REG_LINE_CTRL] & LINE_CTRL_DLAB)
			{
				// Divisor Latch (MSB)
				return (ch->divisor >> 8) & 0xff;
			}
			else
			{

			}
			break;
		}

		case 2:
		{
			if (ch->reg[REG_LINE_CTRL] & LINE_CTRL_DLAB)
			{
				// Alternate Function
			}
			else
			{
				// Interrupt Identification Register
				int i;
				UINT8 r = 0x01;

				for (i=0; i < 5; i++)
				{
					if (ch->pending_interrupt & (1 << i))
					{
						switch (i)
						{
							case 0: r = 0x06; break;	// Receiver Line Status
							case 1: r = 0x04; break;	// Received Data Available
							case 2: r = 0x0c; break;	// Character Timeout Indication
							case 3: r = 0x02; break;	// Transmitter Holding Register Empty
							case 4: r = 0x00; break;	// MODEM Status
						}
						break;
					}
				}

				if (ch->reg[REG_FIFO_CTRL] & 1)
				{
					r |= 0xc0;
				}

				return r;
			}
			break;
		}

		case 5:		// Line Status Register
		{
			UINT8 r = 0;

			// set Data Ready flag
			if (ch->rx_fifo_num > 0)
			{
				r |= 0x1;
			}

			// set Transmitter Holding Register Empty flag
			if (ch->tx_fifo_num == 0)
			{
				r |= 0x20;
			}

			// set Transmitter Empty flag
			if (ch->tx_fifo_num == 0)
			{
				r |= 0x40;
			}

			return r;
		}
	}

	return ch->reg[reg];
}

static void duart_w(running_machine &machine, int chip, int reg, UINT8 data)
{
	int channel = (reg >> 3) & 1;
	PC16552D_CHANNEL *ch = &duart[chip].ch[channel];
	reg &= 7;

//  printf("duart_w: chip %d, ch %d, reg %d, data %02X\n", chip, channel, reg, data);

	switch (reg)
	{
		case 0:
		{
			if (ch->reg[REG_LINE_CTRL] & LINE_CTRL_DLAB)
			{
				// Divisor Latch (LSB)
				ch->divisor &= 0xff00;
				ch->divisor |= data;

				return;
			}
			else
			{
				// Transmitter Holding Register
				duart_push_tx_fifo(chip, channel, data);

				ch->pending_interrupt &= ~IRQ_TX_HOLDING_REG_EMPTY;
				check_interrupts(machine, chip, channel);

				return;
			}
		}
		case 1:
		{
			if (ch->reg[REG_LINE_CTRL] & LINE_CTRL_DLAB)
			{
				// Divisor Latch (MSB)
				ch->divisor &= 0x00ff;
				ch->divisor |= data << 8;

		//      printf("DUART %d %d bps\n", chip, duart[chip].frequency / (ch->divisor * 16));
				return;
			}
			else
			{
				// Interrupt enable
				ch->reg[REG_INT_ENABLE] = data;

				check_interrupts(machine, chip, channel);
				return;
			}
		}

		case 2:
		{
			if (ch->reg[REG_LINE_CTRL] & LINE_CTRL_DLAB)
			{
				// Alternate Function

				return;
			}
			else
			{
				// FIFO control
				if (data & 0x02)
				{
					ch->rx_fifo_write_ptr = 0;
					ch->rx_fifo_read_ptr = 0;
					ch->rx_fifo_num = 0;
				}
				if (data & 0x04)
				{
					ch->tx_fifo_write_ptr = 0;
					ch->tx_fifo_read_ptr = 0;
					ch->tx_fifo_num = 0;


				}

				/*if (data & 0x1 && (ch->reg[reg] & 0x1) == 0)
                {
                    // cause transmitter empty IRQ
                    ch->pending_interrupt |= IRQ_TX_HOLDING_REG_EMPTY;

                    check_interrupts(machine, chip, channel);
                }
                */
			}
			break;
		}
	}

	ch->reg[reg] = data;
}

/*****************************************************************************/

void pc16552d_init(running_machine &machine, int chip, int frequency, void (* irq_handler)(running_machine &machine, int channel, int value), void (* tx_callback)(running_machine &machine, int channel, int count, UINT8* data))
{
	memset(&duart[chip], 0, sizeof(PC16552D_REGS));

	duart[chip].frequency = frequency;
	duart[chip].irq_handler = irq_handler;
	duart[chip].tx_callback = tx_callback;

	// clear interrupts
	duart[chip].ch[0].pending_interrupt = 0;
	duart[chip].ch[1].pending_interrupt = 0;

	// allocate transmit timers
	duart[chip].ch[0].tx_fifo_timer = machine.scheduler().timer_alloc(FUNC(tx_fifo_timer_callback));
	duart[chip].ch[0].tx_fifo_timer->adjust(attotime::never, (chip * 2) + 0);

	duart[chip].ch[1].tx_fifo_timer = machine.scheduler().timer_alloc(FUNC(tx_fifo_timer_callback));
	duart[chip].ch[1].tx_fifo_timer->adjust(attotime::never, (chip * 2) + 1);
}

void pc16552d_rx_data(running_machine &machine, int chip, int channel, UINT8 data)
{
	if (duart[chip].ch[channel].reg[REG_FIFO_CTRL] & 0x01)	// RCVR & XMIT FIFO enable
	{
		duart_push_rx_fifo(machine, chip, channel, data);
	}
}

/*****************************************************************************/
/* Read/Write handlers */

READ8_HANDLER(pc16552d_0_r)
{
	return duart_r(space.machine(), 0, offset);
}

WRITE8_HANDLER(pc16552d_0_w)
{
	duart_w(space.machine(), 0, offset, data);
}

READ8_HANDLER(pc16552d_1_r)
{
	return duart_r(space.machine(), 1, offset);
}

WRITE8_HANDLER(pc16552d_1_w)
{
	duart_w(space.machine(), 1, offset, data);
}
