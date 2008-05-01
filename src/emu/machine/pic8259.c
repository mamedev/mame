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

typedef struct pic8259	pic8259_t;

struct pic8259
{
	const struct pic8259_interface	*intf;

	emu_timer *timer;

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
	UINT32 vector_addr_low;
	/* ICW2 state */
	UINT8 base;
	UINT8 vector_addr_high;

	/* ICW3 state */
	UINT8 slave;

	/* ICW4 state */
	UINT32 nested : 1;
	UINT32 mode : 2;
	UINT32 auto_eoi : 1;
	UINT32 is_x86 : 1;
};


INLINE pic8259_t *get_safe_token(const device_config *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == DEVICE_GET_INFO_NAME(pic8259) );
	return ( pic8259_t *) device->token;
}


static TIMER_CALLBACK( pic8259_timerproc )
{
	const device_config *device = ptr;
	pic8259_t	*pic8259 = get_safe_token(device);
	int irq;
	UINT8 mask;

	/* check the various IRQs */
	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ in service? */
		if (pic8259->in_service & mask)
		{
			if (LOG_GENERAL)
				logerror("pic8259_timerproc(): PIC IRQ #%d still in service\n", irq);
			return;
		}

		/* is this IRQ pending and enabled? */
		if ((pic8259->state == STATE_READY) && (pic8259->pending & mask) && !(pic8259->interrupt_mask & mask))
		{
			if (LOG_GENERAL)
				logerror("pic8259_timerproc(): PIC triggering IRQ #%d\n", irq);
			if (pic8259->intf->set_int_line)
				pic8259->intf->set_int_line(device, 1);
			return;
		}
	}
	if (pic8259->intf->set_int_line)
		pic8259->intf->set_int_line(device, 0);
}


INLINE void pic8259_set_timer(pic8259_t *pic8259)
{
	timer_adjust_oneshot(pic8259->timer, attotime_zero, 0);
}


void pic8259_set_irq_line(const device_config *device, int irq, int state)
{
	pic8259_t	*pic8259 = get_safe_token(device);

	if (state)
	{
		/* setting IRQ line */
		if (!(pic8259->irq_lines & (1 << irq)))
		{
			if (LOG_GENERAL)
				logerror("pic8259_set_irq_line(): PIC set IRQ line #%d\n", irq);

			pic8259->irq_lines |= 1 << irq;
			pic8259->pending |= 1 << irq;
			pic8259_set_timer(pic8259);
		}
	}
	else
	{
		/* clearing IRQ line */
		if (pic8259->irq_lines & (1 << irq))
		{
			if (LOG_GENERAL)
				logerror("pic8259_set_irq_line(): PIC cleared IRQ line #%d\n", irq);

			pic8259->irq_lines &= ~(1 << irq);
			pic8259_set_timer(pic8259);
		}
	}
}



int pic8259_acknowledge(const device_config *device)
{
	pic8259_t	*pic8259 = get_safe_token(device);
	UINT8 mask;
	int irq;

	for (irq = 0; irq < IRQ_COUNT; irq++)
	{
		mask = 1 << irq;

		/* is this IRQ pending and enabled? */
		if ((pic8259->pending & mask) && !(pic8259->interrupt_mask & mask))
		{
			if (LOG_GENERAL)
				logerror("pic8259_acknowledge(): PIC acknowledge IRQ #%d\n", irq);
			pic8259->pending &= ~mask;
			if (!pic8259->auto_eoi)
				pic8259->in_service |= mask;
			if (pic8259->is_x86) {
				/* For x86 mode*/
				return irq + pic8259->base;
			} else {
				/* in case of 8080/85) */
				return 0xcd0000 + (pic8259->vector_addr_high << 8) + pic8259->vector_addr_low + (irq << (3-pic8259->vector_size));
			}
		}
	}
	return 0;
}



READ8_DEVICE_HANDLER( pic8259_r )
{
	pic8259_t	*pic8259 = get_safe_token(device);

	/* NPW 18-May-2003 - Changing 0xFF to 0x00 as per Ruslan */
	UINT8 data = 0x00;

	switch(offset)
	{
		case 0: /* PIC acknowledge IRQ */
			if (pic8259->special)
			{
				pic8259->special = 0;
				data = pic8259->input;
			}
			break;

		case 1: /* PIC mask register */
			data = pic8259->interrupt_mask;
			break;
	}
	return data;
}



WRITE8_DEVICE_HANDLER( pic8259_w )
{
	pic8259_t	*pic8259 = get_safe_token(device);

	switch(offset)
	{
		case 0:    /* PIC acknowledge IRQ */
			if (data & 0x10)
			{
				/* write ICW1 - this pretty much resets the chip */
				if (LOG_ICW)
					logerror("pic8259_w(): ICW1; data=0x%02X\n", data);

				pic8259->interrupt_mask	= 0x00;
				pic8259->level_trig_mode	= (data & 0x08) ? 1 : 0;
				pic8259->vector_size		= (data & 0x04) ? 1 : 0;
				pic8259->cascade			= (data & 0x02) ? 0 : 1;
				pic8259->icw4_needed		= (data & 0x01) ? 1 : 0;
				pic8259->vector_addr_low	= (data & 0xe0);
				pic8259->state			= STATE_ICW2;
			}
			else if (pic8259->state == STATE_READY)
			{
				if ((data & 0x98) == 0x08)
				{
					/* write OCW3 */
					if (LOG_OCW)
						logerror("pic8259_w(): OCW3; data=0x%02X\n", data);

					switch (data & 0x03)
					{
						case 0x02:
							pic8259->special = 1;
							pic8259->input = pic8259->pending;
							break;
						case 0x03:
							pic8259->special = 1;
							pic8259->input = pic8259->in_service & ~pic8259->interrupt_mask;
							break;
					}
				}
				else if ((data & 0x18) == 0x00)
				{
					int n = data & 7;
					UINT8 mask = 1 << n;

					/* write OCW2 */
					if (LOG_OCW)
						logerror("pic8259_w(): OCW2; data=0x%02X\n", data);

					switch (data & 0xe0)
					{
						case 0x00:
							pic8259->prio = 0;
							break;
						case 0x20:
							for (n = 0, mask = 1<<pic8259->prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if (pic8259->in_service & mask)
								{
									pic8259->in_service &= ~mask;
									break;
								}
							}
							break;
						case 0x40:
							break;
						case 0x60:
							if( pic8259->in_service & mask )
							{
								pic8259->in_service &= ~mask;
							}
							break;
						case 0x80:
							pic8259->prio = ++pic8259->prio & 7;
							break;
						case 0xa0:
							for (n = 0, mask = 1<<pic8259->prio; n < 8; n++, mask = (mask<<1) | (mask>>7))
							{
								if( pic8259->in_service & mask )
								{
									pic8259->in_service &= ~mask;
									pic8259->prio = ++pic8259->prio & 7;
									break;
								}
							}
							break;
						case 0xc0:
							pic8259->prio = n & 7;
							break;
						case 0xe0:
							if( pic8259->in_service & mask )
							{
								pic8259->in_service &= ~mask;
								pic8259->pending &= ~mask;
								pic8259->prio = ++pic8259->prio & 7;
							}
							break;
					}
				}
			}
			break;

		case 1:
			switch(pic8259->state)
			{
				case STATE_ICW1:
					break;

				case STATE_ICW2:
					/* write ICW2 */
					if (LOG_ICW)
						logerror("pic8259_w(): ICW2; data=0x%02X\n", data);

					pic8259->base = data & 0xf8;
					pic8259->vector_addr_high = data ;
					if (pic8259->cascade)
						pic8259->state = STATE_ICW3;
					else
						pic8259->state = pic8259->icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW3:
					/* write ICW3 */
					if (LOG_ICW)
						logerror("pic8259_w(): ICW3; data=0x%02X\n", data);

					pic8259->slave = data;
					pic8259->state = pic8259->icw4_needed ? STATE_ICW4 : STATE_READY;
					break;

				case STATE_ICW4:
					/* write ICW4 */
					if (LOG_ICW)
						logerror("pic8259_w(): ICW4; data=0x%02X\n", data);

					pic8259->nested	= (data & 0x10) ? 1 : 0;
					pic8259->mode = (data >> 2) & 3;
					pic8259->auto_eoi = (data & 0x02) ? 1 : 0;
					pic8259->is_x86 = (data & 0x01) ? 1 : 0;
					pic8259->state = STATE_READY;
					break;

				case STATE_READY:
					/* write OCW1 - set interrupt mask register */
					if (LOG_OCW)
						logerror("pic8259_w(): OCW1; data=0x%02X\n", data);

					pic8259->interrupt_mask = data;
					break;
			}
			break;
    }
	pic8259_set_timer(pic8259);
}



static DEVICE_START( pic8259 ) {
	pic8259_t	*pic8259 = get_safe_token(device);

	pic8259->intf = device->static_config;
}


static DEVICE_RESET( pic8259 ) {
	pic8259_t	*pic8259 = get_safe_token(device);

	pic8259->timer = timer_alloc( pic8259_timerproc, (void *)device );

	pic8259->state = STATE_ICW1;	/* It is unclear from the original code whether this is correct */
	pic8259->irq_lines = 0;
	pic8259->in_service = 0;
	pic8259->pending = 0;
	pic8259->prio = 0;
	pic8259->interrupt_mask = 0;
	pic8259->input = 0;
	pic8259->special = 0;
	pic8259->level_trig_mode = 0;
	pic8259->vector_size = 0;
	pic8259->cascade = 0;
	pic8259->icw4_needed = 0;
	pic8259->base = 0;
	pic8259->slave = 0;
	pic8259->nested = 0;
	pic8259->mode = 0;
	pic8259->auto_eoi = 0;
	pic8259->is_x86 = 0;
	pic8259->vector_addr_low = 0;
	pic8259->vector_addr_high = 0;
}


static DEVICE_SET_INFO( pic8259 ) {
	switch ( state ) {
		/* no parameters to set */
	}
}


DEVICE_GET_INFO( pic8259 ) {
	switch ( state ) {
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(pic8259_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;								break;
		case DEVINFO_INT_CLASS:						info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:					info->set_info = DEVICE_SET_INFO_NAME(pic8259);	break;
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(pic8259);	break;
		case DEVINFO_FCT_STOP:						/* nothing */								break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(pic8259);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						info->s = "Intel PIC8259";					break;
		case DEVINFO_STR_FAMILY:					info->s = "PIC8259";						break;
		case DEVINFO_STR_VERSION:					info->s = "1.00";							break;
		case DEVINFO_STR_SOURCE_FILE:				info->s = __FILE__;							break;
		case DEVINFO_STR_CREDITS:					info->s = "Copyright the MAME and MESS Teams";	break;
	}
}

