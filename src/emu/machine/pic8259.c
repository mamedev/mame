/**********************************************************************

    8259 PIC interface and emulation

    The 8259 is a programmable interrupt controller used to multiplex
    interrupts for x86 and other computers.  The chip is set up by
    writing a series of Initialization Command Words (ICWs) after which
    the chip is operational and capable of dispatching interrupts.  After
    this, Operation Command Words (OCWs) can be written to control further
    behavior.

**********************************************************************/

#include "driver.h"
#include "memconv.h"
#include "machine/pic8259.h"

#define IRQ_COUNT	8

#define LOG_ICW		0
#define LOG_OCW		0
#define LOG_GENERAL	0

typedef enum
{
	STATE_ICW1,
	STATE_ICW2,
	STATE_ICW3,
	STATE_ICW4,
	STATE_READY
} pic8259_state_t;

struct pic8259
{
	emu_timer *timer;
	void (*set_int_line)(int which, int interrupt);

	pic8259_state_t state;

	UINT8 irq_lines;
	UINT8 in_service;
	UINT8 pending;
	UINT8 prio;
	UINT8 interrupt_mask;

	UINT8 input;
	UINT32 special : 1;

	/* ICW1 state */
	UINT32 level_trig_mode : 1;
	UINT32 vector_size : 1;
	UINT32 cascade : 1;
	UINT32 icw4_needed : 1;

	/* ICW2 state */
	UINT8 base;

	/* ICW3 state */
	UINT8 slave;

	/* ICW4 state */
	UINT32 nested : 1;
	UINT32 mode : 2;
	UINT32 auto_eoi : 1;
	UINT32 is_x86 : 1;
};

static struct pic8259 *pic;

static TIMER_CALLBACK( pic8259_timerproc );


/* initializer */
int pic8259_init(int count, void (*set_int_line)(int which, int interrupt))
{
	int i;

	/* allocate pic structures */
	pic = auto_malloc(count * sizeof(struct pic8259));
	memset(pic, 0, count * sizeof(struct pic8259));

	for (i = 0; i < count; i++)
	{
		pic[i].timer = timer_alloc(pic8259_timerproc, NULL);
		pic[i].set_int_line = set_int_line;
	}

	return 0;
}



static TIMER_CALLBACK( pic8259_timerproc )
{
	int which = param;
	struct pic8259 *p = &pic[which];
	int irq;
	UINT8 mask;

	/* check the various IRQs */
	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ in service? */
		if (p->in_service & mask)
		{
			if (LOG_GENERAL)
				logerror("pic8259_timerproc(): PIC #%d IRQ #%d still in service\n", which, irq);
			return;
		}

		/* is this IRQ pending and enabled? */
		if ((p->state == STATE_READY) && (p->pending & mask) && !(p->interrupt_mask & mask))
		{
			if (LOG_GENERAL)
				logerror("pic8259_timerproc(): PIC #%d triggering IRQ #%d\n", which, irq);
			if (p->set_int_line)
				p->set_int_line(which, 1);
			return;
		}
	}
	if (p->set_int_line)
		p->set_int_line(which, 0);
}



static void pic8259_set_timer(int which)
{
	timer_adjust_oneshot(pic[which].timer, attotime_zero, which);
}



void pic8259_set_irq_line(int which, int irq, int state)
{
	if (state)
	{
		/* setting IRQ line */
		if (!(pic[which].irq_lines & (1 << irq)))
		{
			if (LOG_GENERAL)
				logerror("pic8259_set_irq_line(): PIC #%d set IRQ line #%d\n", which, irq);

			pic[which].irq_lines |= 1 << irq;
			pic[which].pending |= 1 << irq;
			pic8259_set_timer(which);
		}
	}
	else
	{
		/* clearing IRQ line */
		if (pic[which].irq_lines & (1 << irq))
		{
			if (LOG_GENERAL)
				logerror("pic8259_set_irq_line(): PIC #%d cleared IRQ line #%d\n", which, irq);

			pic[which].irq_lines &= ~(1 << irq);
			pic8259_set_timer(which);
		}
	}
}



int pic8259_acknowledge(int which)
{
	struct pic8259 *p = &pic[which];
	UINT8 mask;
	int irq;

	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ pending and enabled? */
		if ((p->pending & mask) && !(p->interrupt_mask & mask))
		{
			if (LOG_GENERAL)
				logerror("pic8259_acknowledge(): PIC #%d acknowledge IRQ #%d\n", which, irq);

			p->pending &= ~mask;
			if (!p->auto_eoi)
				p->in_service |= mask;

			return irq + p->base;
		}
	}
	return 0;
}



static UINT8 pic8259_read(int which, offs_t offset)
{
	struct pic8259 *p = &pic[which];

	/* NPW 18-May-2003 - Changing 0xFF to 0x00 as per Ruslan */
	UINT8 data = 0x00;

	switch(offset)
	{
		case 0: /* PIC acknowledge IRQ */
			if (p->special)
			{
				p->special = 0;
				data = p->input;
			}
			break;

		case 1: /* PIC mask register */
			data = p->interrupt_mask;
			break;
	}
	return data;
}



static void pic8259_write(int which, offs_t offset, UINT8 data )
{
	struct pic8259 *p = &pic[which];

	switch(offset)
	{
		case 0:    /* PIC acknowledge IRQ */
			if (data & 0x10)
			{
				/* write ICW1 - this pretty much resets the chip */
				if (LOG_ICW)
					logerror("pic8259_write(): ICW1; which=%d data=0x%02X\n", which, data);

				p->interrupt_mask	= 0x00;
				p->level_trig_mode	= (data & 0x08) ? 1 : 0;
				p->vector_size		= (data & 0x04) ? 1 : 0;
				p->cascade			= (data & 0x02) ? 0 : 1;
				p->icw4_needed		= (data & 0x01) ? 1 : 0;
				p->state			= STATE_ICW2;
			}
			else if (p->state == STATE_READY)
			{
				if ((data & 0x98) == 0x08)
				{
					/* write OCW3 */
					if (LOG_OCW)
						logerror("pic8259_write(): OCW3; which=%d data=0x%02X\n", which, data);

					switch (data & 0x03)
					{
						case 0x02:
							p->special = 1;
							p->input = p->pending;
							break;
						case 0x03:
							p->special = 1;
							p->input = p->in_service & ~p->interrupt_mask;
							break;
					}
				}
				else if ((data & 0x18) == 0x00)
				{
					int n = data & 7;
					UINT8 mask = 1 << n;

					/* write OCW2 */
					if (LOG_OCW)
						logerror("pic8259_write(): OCW2; which=%d data=0x%02X\n", which, data);

					switch (data & 0xe0)
					{
						case 0x00:
							p->prio = 0;
							break;
						case 0x20:
							for (n = 0, mask = 1<<p->prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if (p->in_service & mask)
								{
									p->in_service &= ~mask;
									break;
								}
							}
							break;
						case 0x40:
							break;
						case 0x60:
							if( p->in_service & mask )
							{
								p->in_service &= ~mask;
							}
							break;
						case 0x80:
							p->prio = ++p->prio & 7;
							break;
						case 0xa0:
							for (n = 0, mask = 1<<p->prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if( p->in_service & mask )
								{
									p->in_service &= ~mask;
									p->prio = ++p->prio & 7;
									break;
								}
							}
							break;
						case 0xc0:
							p->prio = n & 7;
							break;
						case 0xe0:
							if( p->in_service & mask )
							{
								p->in_service &= ~mask;
								p->pending &= ~mask;
								p->prio = ++p->prio & 7;
							}
							break;
					}
				}
			}
			break;

		case 1:
			switch(p->state)
			{
				case STATE_ICW1:
					break;

				case STATE_ICW2:
					/* write ICW2 */
					if (LOG_ICW)
						logerror("pic8259_write(): ICW2; which=%d data=0x%02X\n", which, data);

					p->base = data & 0xf8;
					if (p->cascade)
						p->state = STATE_ICW3;
					else
						p->state = p->icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW3:
					/* write ICW3 */
					if (LOG_ICW)
						logerror("pic8259_write(): ICW3; which=%d data=0x%02X\n", which, data);

					p->slave = data;
					p->state = p->icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW4:
					/* write ICW4 */
					if (LOG_ICW)
						logerror("pic8259_write(): ICW4; which=%d data=0x%02X\n", which, data);

					p->nested	= (data & 0x10) ? 1 : 0;
					p->mode = (data >> 2) & 3;
					p->auto_eoi = (data & 0x02) ? 1 : 0;
					p->is_x86 = (data & 0x01) ? 1 : 0;
					p->state = STATE_READY;
					break;

				case STATE_READY:
					/* write OCW1 - set interrupt mask register */
					if (LOG_OCW)
						logerror("pic8259_write(): OCW1; which=%d data=0x%02X\n", which, data);

					p->interrupt_mask = data;
					break;
			}
			break;
    }
	pic8259_set_timer(which);
}



/* ----------------------------------------------------------------------- */

READ8_HANDLER ( pic8259_0_r )	{ return pic8259_read(0, offset); }
READ8_HANDLER ( pic8259_1_r )	{ return pic8259_read(1, offset); }
WRITE8_HANDLER ( pic8259_0_w )	{ pic8259_write(0, offset, data); }
WRITE8_HANDLER ( pic8259_1_w )	{ pic8259_write(1, offset, data); }

READ16_HANDLER ( pic8259_16le_0_r ) { return read16le_with_read8_handler(pic8259_0_r, machine, offset, mem_mask); }
READ16_HANDLER ( pic8259_16le_1_r ) { return read16le_with_read8_handler(pic8259_1_r, machine, offset, mem_mask); }
WRITE16_HANDLER ( pic8259_16le_0_w ) { write16le_with_write8_handler(pic8259_0_w, machine, offset, data, mem_mask); }
WRITE16_HANDLER ( pic8259_16le_1_w ) { write16le_with_write8_handler(pic8259_1_w, machine, offset, data, mem_mask); }

READ32_HANDLER ( pic8259_32le_0_r ) { return read32le_with_read8_handler(pic8259_0_r, machine, offset, mem_mask); }
READ32_HANDLER ( pic8259_32le_1_r ) { return read32le_with_read8_handler(pic8259_1_r, machine, offset, mem_mask); }
WRITE32_HANDLER ( pic8259_32le_0_w ) { write32le_with_write8_handler(pic8259_0_w, machine, offset, data, mem_mask); }
WRITE32_HANDLER ( pic8259_32le_1_w ) { write32le_with_write8_handler(pic8259_1_w, machine, offset, data, mem_mask); }

READ64_HANDLER ( pic8259_64be_0_r ) { return read64be_with_read8_handler(pic8259_0_r, machine, offset, mem_mask); }
READ64_HANDLER ( pic8259_64be_1_r ) { return read64be_with_read8_handler(pic8259_1_r, machine, offset, mem_mask); }
WRITE64_HANDLER ( pic8259_64be_0_w ) { write64be_with_write8_handler(pic8259_0_w, machine, offset, data, mem_mask); }
WRITE64_HANDLER ( pic8259_64be_1_w ) { write64be_with_write8_handler(pic8259_1_w, machine, offset, data, mem_mask); }
