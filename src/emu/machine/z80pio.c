/***************************************************************************

    Z80 PIO implementation

    based on original version (c) 1997, Tatsuyuki Satoh

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "z80pio.h"
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

#define PIO_MODE0		0x00		/* output mode */
#define PIO_MODE1		0x01		/* input  mode */
#define PIO_MODE2		0x02		/* i/o    mode */
#define PIO_MODE3		0x03		/* bit    mode */

/* pio controll port operation (bit 0-3) */
#define PIO_OP_MODE		0x0f		/* mode select        */
#define PIO_OP_INTC		0x07		/* interrupt controll */
#define PIO_OP_INTE		0x03		/* interrupt enable   */

/* pio interrupt controll nit */
#define PIO_INT_ENABLE	0x80		/* ENABLE : 0=disable , 1=enable */
#define PIO_INT_AND		0x40		/* LOGIC  : 0=OR      , 1=AND    */
#define PIO_INT_HIGH	0x20		/* LEVEL  : 0=low     , 1=high   */
#define PIO_INT_MASK	0x10		/* MASK   : 0=off     , 1=on     */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _z80pio z80pio_t;
struct _z80pio
{
	UINT8 vector[2];                      /* interrupt vector               */
	devcb_resolved_write_line intr;       /* interrupt callbacks            */
	devcb_resolved_write_line rdyr[2];    /* RDY active callback            */
	devcb_resolved_read8  port_read[2];   /* port read callbacks            */
	devcb_resolved_write8 port_write[2];  /* port write callbacks           */
	UINT8 mode[2];                        /* mode 00=in,01=out,02=i/o,03=bit*/
	UINT8 enable[2];                      /* interrupt enable               */
	UINT8 mask[2];                        /* mask folowers                  */
	UINT8 dir[2];                         /* direction (bit mode)           */
	UINT8 rdy[2];                         /* ready pin level                */
	UINT8 in[2];                          /* input port data                */
	UINT8 out[2];                         /* output port                    */
	UINT8 strobe[2];						/* strobe inputs */
	UINT8 int_state[2];                   /* interrupt status (daisy chain) */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static int z80pio_irq_state(const device_config *device);
static int z80pio_irq_ack(const device_config *device);
static void z80pio_irq_reti(const device_config *device);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE z80pio_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == Z80PIO);
	return (z80pio_t *)device->token;
}


/***************************************************************************
    INTERNAL STATE MANAGEMENT
***************************************************************************/

INLINE void set_rdy(const device_config *device, int ch, int state)
{
	z80pio_t *z80pio = get_safe_token( device );
	/* set state */
	z80pio->rdy[ch] = state;

	/* call callback with state */
	devcb_call_write_line(&z80pio->rdyr[ch], z80pio->rdy[ch]);
}


INLINE void interrupt_check(const device_config *device)
{
	z80pio_t *z80pio = get_safe_token( device );

	/* if we have a callback, update it with the current state */
	devcb_call_write_line(&z80pio->intr, (z80pio_irq_state(device) & Z80_DAISY_INT) ? ASSERT_LINE : CLEAR_LINE);
}


static void update_irq_state(const device_config *device, int ch)
{
	z80pio_t *z80pio = get_safe_token( device );
	int old_state = z80pio->int_state[ch];
	int irq = 0;
	int data;
	if (z80pio->mode[ch] == 0x13 || (z80pio->enable[ch] & PIO_INT_MASK)) return;

	/* only check if interrupts are enabled */
	if (z80pio->enable[ch] & PIO_INT_ENABLE)
	{
		/* in mode 3, interrupts are tricky */
		if (z80pio->mode[ch] == PIO_MODE3)
		{
			/* fetch input data (ignore output lines) */
			data = z80pio->in[ch] & z80pio->dir[ch];

			/* keep only relevant bits */
			data &= ~z80pio->mask[ch];

			/* if active low, invert the bits */
			if (!(z80pio->enable[ch] & PIO_INT_HIGH))
				data ^= z80pio->mask[ch];

			/* if AND logic, interrupt if all bits are set */
			if (z80pio->enable[ch] & PIO_INT_AND)
				irq = (data == z80pio->mask[ch]);

			/* otherwise, interrupt if at least one bit is set */
			else
				irq = (data != 0);

			/* if portB, portA mode 2 check */
			if (ch && (z80pio->mode[0] == PIO_MODE2))
			{
				if (z80pio->rdy[ch] == 0)
					irq = 1;
			}
		}

		/* otherwise, just interrupt when ready is cleared */
		else
			irq = (z80pio->rdy[ch] == 0);
	}

	if (irq)
		z80pio->int_state[ch] |=  Z80_DAISY_INT;
	else
		z80pio->int_state[ch] &= ~Z80_DAISY_INT;

	if (old_state != z80pio->int_state[ch])
		interrupt_check(device);
}



/***************************************************************************
    CONTROL REGISTER READ/WRITE
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80pio_c_w )
{
	z80pio_t *z80pio = get_safe_token( device );

	/* There are only 2 channels */
	offset &= 0x01;

	/* load direction phase ? */
	if (z80pio->mode[offset] == 0x13)
	{
		VPRINTF(("PIO-%c Bits %02x\n", 'A' + offset, data));
		z80pio->dir[offset] = data;
		z80pio->mode[offset] = 0x03;
		return;
	}

	/* load mask folows phase ? */
	if (z80pio->enable[offset] & PIO_INT_MASK)
	{
		/* load mask folows */
		z80pio->mask[offset] = data;
		z80pio->enable[offset] &= ~PIO_INT_MASK;
		VPRINTF(("PIO-%c interrupt mask %02x\n",'A'+offset,data ));
		return;
	}

	switch (data & 0x0f)
	{
		case PIO_OP_MODE:	/* mode select 0=out,1=in,2=i/o,3=bit */
			z80pio->mode[offset] = (data >> 6);
			VPRINTF(("PIO-%c Mode %x\n", 'A' + offset, z80pio->mode[offset]));
			if (z80pio->mode[offset] == 0x03)
				z80pio->mode[offset] = 0x13;
			return;

		case PIO_OP_INTC:		/* interrupt control */
			z80pio->enable[offset] = data & 0xf0;
			z80pio->mask[offset] = 0x00;
			/* when interrupt enable , set vector request flag */
			VPRINTF(("PIO-%c Controll %02x\n", 'A' + offset, data));
			break;

		case PIO_OP_INTE:		/* interrupt enable controll */
			z80pio->enable[offset] &= ~PIO_INT_ENABLE;
			z80pio->enable[offset] |= (data & PIO_INT_ENABLE);
			VPRINTF(("PIO-%c enable %02x\n", 'A' + offset, data & 0x80));
			break;

		default:
			if (!(data & 1))
			{
				z80pio->vector[offset] = data;
				VPRINTF(("PIO-%c vector %02x\n", 'A' + offset, data));
			}
			else
				VPRINTF(("PIO-%c illegal command %02x\n", 'A' + offset, data));
			break;
	}

	/* interrupt check */
	update_irq_state(device, offset);
}


READ8_DEVICE_HANDLER( z80pio_c_r )
{
	/* There are only 2 channels */
	offset &= 0x01;

	VPRINTF(("PIO-%c controll read\n", 'A' + offset ));
	return 0;
}



/***************************************************************************
    DATA REGISTER READ/WRITE
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80pio_d_w )
{
	z80pio_t *z80pio = get_safe_token( device );

	/* There are only 2 channels */
	offset &= 0x01;

	z80pio->out[offset] = data;	/* latch out data */
	if (z80pio->mode[offset] == PIO_MODE3)
		devcb_call_write8(&z80pio->port_write[offset], 0, data & ~z80pio->dir[offset]);
	else
		devcb_call_write8(&z80pio->port_write[offset], 0, data);

	switch (z80pio->mode[offset])
	{
		case PIO_MODE0:			/* mode 0 output */
		case PIO_MODE2:			/* mode 2 i/o */
			set_rdy(device, offset, 1); /* ready = H */
			update_irq_state(device, offset);
			return;

		case PIO_MODE1:			/* mode 1 input */
		case PIO_MODE3:			/* mode 0 bit */
			return;

		default:
			VPRINTF(("PIO-%c data write,bad mode\n",'A'+offset ));
	}
}


READ8_DEVICE_HANDLER( z80pio_d_r )
{
	z80pio_t *z80pio = get_safe_token( device );

	/* There are only 2 channels */
	offset &= 0x01;

	switch (z80pio->mode[offset])
	{
		case PIO_MODE0:			/* mode 0 output */
			return z80pio->out[offset];

		case PIO_MODE1:			/* mode 1 input */
			set_rdy(device, offset, 1);	/* ready = H */
			if(z80pio->port_read[offset].read != NULL)
				z80pio->in[offset] = devcb_call_read8(&z80pio->port_read[offset], 0);
			update_irq_state(device, offset);
			return z80pio->in[offset];

		case PIO_MODE2:			/* mode 2 i/o */
			if (offset) VPRINTF(("PIO-B mode 2 \n"));
			set_rdy(device, 1, 1); /* brdy = H */
			if(z80pio->port_read[offset].read != NULL)
				z80pio->in[offset] = devcb_call_read8(&z80pio->port_read[offset], 0);
			update_irq_state(device, offset);
			return z80pio->in[offset];

		case PIO_MODE3:			/* mode 3 bit */
			if(z80pio->port_read[offset].read != NULL)
				z80pio->in[offset] = devcb_call_read8(&z80pio->port_read[offset], 0);
			return (z80pio->in[offset] & z80pio->dir[offset]) | (z80pio->out[offset] & ~z80pio->dir[offset]);
	}
	VPRINTF(("PIO-%c data read,bad mode\n",'A'+offset ));
	return 0;
}



/***************************************************************************
    PORT I/O
***************************************************************************/

WRITE8_DEVICE_HANDLER( z80pio_p_w )
{
	z80pio_t *z80pio = get_safe_token( device );

	/* There are only 2 channels */
	offset &= 0x01;

	z80pio->in[offset]  = data;
	switch (z80pio->mode[offset])
	{
		case PIO_MODE0:
			VPRINTF(("PIO-%c OUTPUT mode and data write\n",'A'+offset ));
			break;

		case PIO_MODE2:	/* only port A */
			offset = 1;		/* handshake and IRQ is use portB */

		case PIO_MODE1:
			set_rdy(device, offset, 0);
			update_irq_state(device, offset);
			break;

		case PIO_MODE3:
			/* irq check */
			update_irq_state(device, offset);
			break;
	}
}


READ8_DEVICE_HANDLER( z80pio_p_r )
{
	z80pio_t *z80pio = get_safe_token( device );

	/* There are only 2 channels */
	offset &= 0x01;

	switch (z80pio->mode[offset])
	{
		case PIO_MODE2:		/* port A only */
		case PIO_MODE0:
			set_rdy(device, offset, 0);
			update_irq_state(device, offset);
			break;

		case PIO_MODE1:
			VPRINTF(("PIO-%c INPUT mode and data read\n",'A'+offset ));
			break;

		case PIO_MODE3:
			return (z80pio->in[offset] & z80pio->dir[offset]) | (z80pio->out[offset] & ~z80pio->dir[offset]);
	}
	return z80pio->out[offset];
}


/***************************************************************************
    STROBE STATE MANAGEMENT
***************************************************************************/

static void z80pio_update_strobe(const device_config *device, int ch, int state)
{
	z80pio_t *z80pio = get_safe_token( device );

	switch (z80pio->mode[ch])
	{
		/* output mode: a positive edge is used by peripheral to acknowledge
            the receipt of data */
		case PIO_MODE0:
		{
			/* ensure valid */
			state = state & 0x01;

			/* strobe changed state? */
			if ((z80pio->strobe[ch] ^ state) != 0)
			{
				/* yes */
				if (state != 0)
				{
					/* positive edge */
					VPRINTF(("PIO-%c positive strobe\n",'A' + ch));

					/* ready is now inactive */
					set_rdy(device, ch, 0);

					/* int enabled? */
					if (z80pio->enable[ch] & PIO_INT_ENABLE)
					{
						/* trigger an int request */
						z80pio->int_state[ch] |= Z80_DAISY_INT;
					}
				}
			}

			/* store strobe state */
			z80pio->strobe[ch] = state;

			/* check interrupt */
			interrupt_check(device);
		}
		break;

		/* input mode: strobe is used by peripheral to load data from the peripheral
            into port a input register, data loaded into pio when signal is active */

		default:
			break;
	}
}


void z80pio_astb_w(const device_config *device, int state) { z80pio_update_strobe(device, 0, state); }
void z80pio_bstb_w(const device_config *device, int state) { z80pio_update_strobe(device, 1, state); }



/***************************************************************************
    DAISY CHAIN INTERFACE
***************************************************************************/

static int z80pio_irq_state(const device_config *device)
{
	z80pio_t *z80pio = get_safe_token( device );
	int state = 0;
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 2; ch++)
	{
		/* if we're servicing a request, don't indicate more interrupts */
		if (z80pio->int_state[ch] & Z80_DAISY_IEO)
		{
			state |= Z80_DAISY_IEO;
			break;
		}
		state |= z80pio->int_state[ch];
	}
	return state;
}


static int z80pio_irq_ack(const device_config *device)
{
	z80pio_t *z80pio = get_safe_token( device );
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 2; ch++)

		/* find the first channel with an interrupt requested */
		if (z80pio->int_state[ch] & Z80_DAISY_INT)
		{
			/* clear interrupt, switch to the IEO state, and update the IRQs */
			z80pio->int_state[ch] = Z80_DAISY_IEO;
			interrupt_check(device);
			return z80pio->vector[ch];
		}

	VPRINTF(("z80pio_irq_ack: failed to find an interrupt to ack!"));
	return z80pio->vector[0];
}


static void z80pio_irq_reti(const device_config *device)
{
	z80pio_t *z80pio = get_safe_token( device );
	int ch;

	/* loop over all channels */
	for (ch = 0; ch < 2; ch++)

		/* find the first channel with an IEO pending */
		if (z80pio->int_state[ch] & Z80_DAISY_IEO)
		{
			/* clear the IEO state and update the IRQs */
			z80pio->int_state[ch] &= ~Z80_DAISY_IEO;
			interrupt_check(device);
			return;
		}

	VPRINTF(("z80pio_irq_reti: failed to find an interrupt to clear IEO on!"));
}


/***************************************************************************
    READ/WRITE HANDLERS
***************************************************************************/
READ8_DEVICE_HANDLER(z80pio_r)
{
	return (offset & 2) ? z80pio_c_r(device, offset & 1) : z80pio_d_r(device, offset & 1);
}

WRITE8_DEVICE_HANDLER(z80pio_w)
{
	if (offset & 2)
		z80pio_c_w(device, offset & 1, data);
	else
		z80pio_d_w(device, offset & 1, data);
}

READ8_DEVICE_HANDLER(z80pio_alt_r)
{
	int channel = BIT(offset, 1);
	return (offset & 1) ? z80pio_c_r(device, channel) : z80pio_d_r(device, channel);
}

WRITE8_DEVICE_HANDLER(z80pio_alt_w)
{
	int channel = BIT(offset, 1);
	if (offset & 1)
		z80pio_c_w(device, channel, data);
	else
		z80pio_d_w(device, channel, data);
}

static DEVICE_START( z80pio )
{
	const z80pio_interface *intf = (const z80pio_interface *)device->static_config;
	z80pio_t *z80pio = get_safe_token( device );

	devcb_resolve_write_line(&z80pio->intr, &intf->intr, device);
	devcb_resolve_read8(&z80pio->port_read[0], &intf->portAread, device);
	devcb_resolve_read8(&z80pio->port_read[1], &intf->portBread, device);
	devcb_resolve_write8(&z80pio->port_write[0], &intf->portAwrite, device);
	devcb_resolve_write8(&z80pio->port_write[1], &intf->portBwrite, device);
	devcb_resolve_write_line(&z80pio->rdyr[0], &intf->rdyA, device);
	devcb_resolve_write_line(&z80pio->rdyr[1], &intf->rdyB, device);

	/* register for save states */
	state_save_register_device_item_array(device, 0, z80pio->vector);
	state_save_register_device_item_array(device, 0, z80pio->mode);
	state_save_register_device_item_array(device, 0, z80pio->enable);
	state_save_register_device_item_array(device, 0, z80pio->mask);
	state_save_register_device_item_array(device, 0, z80pio->dir);
	state_save_register_device_item_array(device, 0, z80pio->rdy);
	state_save_register_device_item_array(device, 0, z80pio->in);
	state_save_register_device_item_array(device, 0, z80pio->out);
	state_save_register_device_item_array(device, 0, z80pio->strobe);
	state_save_register_device_item_array(device, 0, z80pio->int_state);
}


static DEVICE_RESET( z80pio )
{
	z80pio_t	*z80pio = get_safe_token( device );
	int i;

	for (i = 0; i < 2; i++)
	{
		z80pio->mask[i]   = 0xff;	/* mask all on */
		z80pio->enable[i] = 0x00;	/* disable     */
		z80pio->mode[i]   = 0x01;	/* mode input  */
		z80pio->dir[i]    = 0x01;	/* dir  input  */
		set_rdy(device, i, 0);	/* RDY = low   */
		z80pio->out[i]    = 0x00;	/* outdata = 0 */
		z80pio->int_state[i] = 0;
		z80pio->strobe[i] = 0;
	}
	interrupt_check(device);
}


DEVICE_GET_INFO( z80pio )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(z80pio_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;							break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;		break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(z80pio);break;
		case DEVINFO_FCT_STOP:							/* Nothing */							break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(z80pio);break;
		case DEVINFO_FCT_IRQ_STATE:						info->f = (genf *)z80pio_irq_state;		break;
		case DEVINFO_FCT_IRQ_ACK:						info->f = (genf *)z80pio_irq_ack;		break;
		case DEVINFO_FCT_IRQ_RETI:						info->f = (genf *)z80pio_irq_reti;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Zilog Z80 PIO");		break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Z80");					break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

