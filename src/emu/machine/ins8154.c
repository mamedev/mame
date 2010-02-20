/*****************************************************************************
 *
 * machine/ins8154.h
 *
 * INS8154 N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)
 *
 * Written by Dirk Best, January 2008
 *
 * TODO: Strobed modes
 *
 ****************************************************************************/

#include "emu.h"
#include "ins8154.h"


#define VERBOSE 1
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)


/* Mode Definition Register */
enum
{
	MDR_BASIC                 = 0x00,
	MDR_STROBED_INPUT         = 0x20,
	MDR_STROBED_OUTPUT        = 0x60,
	MDR_STROBED_OUTPUT_3STATE = 0xe0
};

typedef struct _ins8154_t ins8154_t;
struct _ins8154_t
{
	const ins8154_interface *intf; /* Pointer to our interface */

	UINT8 in_a;  /* Input Latch Port A */
	UINT8 in_b;  /* Input Latch Port B */
	UINT8 out_a; /* Output Latch Port A */
	UINT8 out_b; /* Output Latch Port B */
	UINT8 mdr;   /* Mode Definition Register */
	UINT8 odra;  /* Output Definition Register Port A */
	UINT8 odrb;  /* Output Definition Register Port B */
};


static DEVICE_START( ins8154 )
{
	ins8154_t *ins8154 = (ins8154_t *)device->token;

	/* validate arguments */
	assert(device->tag != NULL);

	/* assign interface */
	ins8154->intf = (const ins8154_interface*)device->baseconfig().static_config;

	/* register for state saving */
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->in_a);
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->in_b);
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->out_a);
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->out_b);
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->mdr);
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->odra);
	state_save_register_item(device->machine, "ins8154", device->tag, 0, ins8154->odrb);
}


static DEVICE_RESET( ins8154 )
{
	ins8154_t *ins8154 = (ins8154_t *)device->token;

	ins8154->in_a = 0;
	ins8154->in_b = 0;
	ins8154->out_a = 0;
	ins8154->out_b = 0;
	ins8154->mdr = 0;
	ins8154->odra = 0;
	ins8154->odrb = 0;
}


READ8_DEVICE_HANDLER( ins8154_r )
{
	ins8154_t *i = (ins8154_t *)device->token;
	UINT8 val = 0xff;

	if (offset > 0x24)
	{
		logerror("INS8154 (%08x): Read from unknown offset %02x!\n",
			cpu_get_pc( device->machine->firstcpu ), offset);
		return 0xff;
	}

	switch (offset)
	{
	case 0x20:
		if (i->intf->in_a_func)
			val = i->intf->in_a_func(device, 0);
		i->in_a = val;
		break;

	case 0x21:
		if (i->intf->in_b_func)
			val = i->intf->in_b_func(device, 0);
		i->in_b = val;
		break;

	default:
		if (offset < 0x08)
		{
			if (i->intf->in_a_func)
				val = (i->intf->in_a_func(device, 0) << (8 - offset)) & 0x80;
			i->in_a = val;
		}
		else
		{
			if (i->intf->in_a_func)
				val = (i->intf->in_a_func(device, 0) << (8 - (offset >> 4))) & 0x80;
			i->in_b = val;
		}
		break;
	}

	return val;
}

WRITE8_DEVICE_HANDLER( ins8154_porta_w )
{
	ins8154_t *i = (ins8154_t *)device->token;

	i->out_a = data;

	/* Test if any pins are set as outputs */
	if (i->odra)
	{
		if (i->intf->out_a_func)
			i->intf->out_a_func(device, 0, (data & i->odra) | (i->odra ^ 0xff));
		else
			logerror("INS8154 (%08x): Write to port A but no write handler defined!\n",
				cpu_get_pc( device->machine->firstcpu ) );
	}
}


WRITE8_DEVICE_HANDLER( ins8154_portb_w )
{
	ins8154_t *i = (ins8154_t *)device->token;

	i->out_b = data;

	/* Test if any pins are set as outputs */
	if (i->odrb)
	{
		if (i->intf->out_b_func)
			i->intf->out_b_func(device, 0, (data & i->odrb) | (i->odrb ^ 0xff));
		else
			logerror("INS8154 (%08x): Write to port B but no write handler defined!\n",
				cpu_get_pc( device->machine->firstcpu ) );
	}
}


WRITE8_DEVICE_HANDLER( ins8154_w )
{
	ins8154_t *i = (ins8154_t *)device->token;

	if (offset > 0x24)
	{
		logerror("INS8154 (%04x): Write %02x to invalid offset %02x!\n",
			cpu_get_pc( device->machine->firstcpu ), data, offset);
		return;
	}

	switch (offset)
	{
	case 0x20:
		ins8154_porta_w(device, 0, data);
		break;

	case 0x21:
		ins8154_portb_w(device, 0, data);
		break;

	case 0x22:
		LOG(("INS8154 (%04x): ODR for port A set to %02x\n",
			cpu_get_pc( device->machine->firstcpu ), data));
		i->odra = data;
		break;

	case 0x23:
		LOG(("INS8154 (%04x): ODR for port B set to %02x\n",
			cpu_get_pc( device->machine->firstcpu ), data));
		i->odrb = data;
		break;

	case 0x24:
		LOG(("INS8154 (%04x): MDR set to %02x\n",
			cpu_get_pc( device->machine->firstcpu ), data));
		i->mdr = data;
		break;

	default:
		if (offset & 0x10)
		{
			/* Set bit */
			if (offset < 0x08)
			{
				ins8154_porta_w(device, 0, i->out_a |= offset & 0x07);
			}
			else
			{
				ins8154_portb_w(device, 0, i->out_b |= (offset >> 4) & 0x07);
			}
		}
		else
		{
			/* Clear bit */
			if (offset < 0x08)
			{
				ins8154_porta_w(device, 0, i->out_a & ~(offset & 0x07));
			}
			else
			{
				ins8154_portb_w(device, 0, i->out_b & ~((offset >> 4) & 0x07));
			}
		}

		break;
	}
}


DEVICE_GET_INFO( ins8154 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = 0;									break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL;				break;
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ins8154_t);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ins8154);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ins8154);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "National Semiconductor INS8154");		break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "INS8154");						break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MESS Team");			break;
	}
}
