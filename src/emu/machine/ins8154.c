/***************************************************************************

    National Semiconductor INS8154

    N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)


    TODO: Strobed modes

***************************************************************************/

#include "emu.h"
#include "ins8154.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE 1

/* Mode Definition Register */
enum
{
	MDR_BASIC                 = 0x00,
	MDR_STROBED_INPUT         = 0x20,
	MDR_STROBED_OUTPUT        = 0x60,
	MDR_STROBED_OUTPUT_3STATE = 0xe0
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ins8154_state ins8154_state;
struct _ins8154_state
{
	/* i/o lines */
	devcb_resolved_read8 in_a_func;
	devcb_resolved_write8 out_a_func;
	devcb_resolved_read8 in_b_func;
	devcb_resolved_write8 out_b_func;
	devcb_resolved_write_line out_irq_func;

	/* registers */
	UINT8 in_a;  /* Input Latch Port A */
	UINT8 in_b;  /* Input Latch Port B */
	UINT8 out_a; /* Output Latch Port A */
	UINT8 out_b; /* Output Latch Port B */
	UINT8 mdr;   /* Mode Definition Register */
	UINT8 odra;  /* Output Definition Register Port A */
	UINT8 odrb;  /* Output Definition Register Port B */
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE ins8154_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == INS8154);

	return (ins8154_state *)device->token;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

READ8_DEVICE_HANDLER( ins8154_r )
{
	ins8154_state *ins8154 = get_safe_token(device);
	UINT8 val = 0xff;

	if (offset > 0x24)
	{
		if (VERBOSE)
			logerror("%s: INS8154 '%s' Read from invalid offset %02x!\n", cpuexec_describe_context(device->machine), device->tag.cstr(), offset);
		return 0xff;
	}

	switch (offset)
	{
	case 0x20:
		if (ins8154->in_a_func.read != NULL)
			val = devcb_call_read8(&ins8154->in_a_func, 0);
		ins8154->in_a = val;
		break;

	case 0x21:
		if (ins8154->in_b_func.read != NULL)
			val = devcb_call_read8(&ins8154->in_b_func, 0);
		ins8154->in_b = val;
		break;

	default:
		if (offset < 0x08)
		{
			if (ins8154->in_a_func.read != NULL)
				val = (devcb_call_read8(&ins8154->in_a_func, 0) << (8 - offset)) & 0x80;
			ins8154->in_a = val;
		}
		else
		{
			if (ins8154->in_b_func.read != NULL)
				val = (devcb_call_read8(&ins8154->in_b_func, 0) << (8 - (offset >> 4))) & 0x80;
			ins8154->in_b = val;
		}
		break;
	}

	return val;
}

WRITE8_DEVICE_HANDLER( ins8154_porta_w )
{
	ins8154_state *ins8154 = get_safe_token(device);

	ins8154->out_a = data;

	/* Test if any pins are set as outputs */
	if (ins8154->odra)
		devcb_call_write8(&ins8154->out_a_func, 0, (data & ins8154->odra) | (ins8154->odra ^ 0xff));
}

WRITE8_DEVICE_HANDLER( ins8154_portb_w )
{
	ins8154_state *ins8154 = get_safe_token(device);

	ins8154->out_b = data;

	/* Test if any pins are set as outputs */
	if (ins8154->odrb)
		devcb_call_write8(&ins8154->out_b_func, 0, (data & ins8154->odrb) | (ins8154->odrb ^ 0xff));
}

WRITE8_DEVICE_HANDLER( ins8154_w )
{
	ins8154_state *ins8154 = get_safe_token(device);

	if (offset > 0x24)
	{
		if (VERBOSE)
			logerror("%s: INS8154 '%s' Write %02x to invalid offset %02x!\n", cpuexec_describe_context(device->machine), device->tag.cstr(), data, offset);
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
		if (VERBOSE)
			logerror("%s: INS8154 '%s' ODRA set to %02x\n", cpuexec_describe_context(device->machine), device->tag.cstr(), data);

		ins8154->odra = data;
		break;

	case 0x23:
		if (VERBOSE)
			logerror("%s: INS8154 '%s' ODRB set to %02x\n", cpuexec_describe_context(device->machine), device->tag.cstr(), data);

		ins8154->odrb = data;
		break;

	case 0x24:
		if (VERBOSE)
			logerror("%s: INS8154 '%s' MDR set to %02x\n", cpuexec_describe_context(device->machine), device->tag.cstr(), data);

		ins8154->mdr = data;
		break;

	default:
		if (offset & 0x10)
		{
			/* Set bit */
			if (offset < 0x08)
			{
				ins8154_porta_w(device, 0, ins8154->out_a |= offset & 0x07);
			}
			else
			{
				ins8154_portb_w(device, 0, ins8154->out_b |= (offset >> 4) & 0x07);
			}
		}
		else
		{
			/* Clear bit */
			if (offset < 0x08)
			{
				ins8154_porta_w(device, 0, ins8154->out_a & ~(offset & 0x07));
			}
			else
			{
				ins8154_portb_w(device, 0, ins8154->out_b & ~((offset >> 4) & 0x07));
			}
		}

		break;
	}
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( ins8154 )
{
	ins8154_state *ins8154 = get_safe_token(device);
	const ins8154_interface *intf = (const ins8154_interface *)device->baseconfig().static_config;

	/* validate some basic stuff */
	assert(intf != NULL);

	/* resolve callbacks */
	devcb_resolve_read8(&ins8154->in_a_func, &intf->in_a_func, device);
	devcb_resolve_write8(&ins8154->out_a_func, &intf->out_a_func, device);
	devcb_resolve_read8(&ins8154->in_b_func, &intf->in_b_func, device);
	devcb_resolve_write8(&ins8154->out_b_func, &intf->out_b_func, device);
	devcb_resolve_write_line(&ins8154->out_irq_func, &intf->out_irq_func, device);

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
	ins8154_state *ins8154 = get_safe_token(device);

	ins8154->in_a = 0;
	ins8154->in_b = 0;
	ins8154->out_a = 0;
	ins8154->out_b = 0;
	ins8154->mdr = 0;
	ins8154->odra = 0;
	ins8154->odrb = 0;
}


/***************************************************************************
    DEVICE GETINFO
***************************************************************************/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##ins8154##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"INS8154"
#define DEVTEMPLATE_FAMILY				"INS8154"
#define DEVTEMPLATE_VERSION				"1.1"
#define DEVTEMPLATE_CREDITS				"Copyright MESS Team"
#include "devtempl.h"
