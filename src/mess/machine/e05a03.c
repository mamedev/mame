/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

***************************************************************************/

#include "emu.h"
#include "e05a03.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct e05a03_state
{
	/* 24-bit shift register, port 0x00, 0x01 and 0x02 */
	UINT32 shift;

	/* port 0x03 */
	int busy_leading;
	int busy_software;
	int nlqlp;
	int cndlp;

#if 0
	int pe;
	int pelp;
#endif

	/* port 0x04 and 0x05 (9-bit) */
	UINT16 printhead;

	/* port 0x06 (4-bit) */
	UINT8 pf_motor;

	/* port 0x07 (4-bit) */
	UINT8 cr_motor;

	/* callbacks */
	devcb_resolved_write_line out_nlq_lp_func; /* pin 2, nlq lamp output */
	devcb_resolved_write_line out_pe_lp_func;  /* pin 3, paper empty lamp output */
	devcb_resolved_write_line out_reso_func;   /* pin 25, reset output */
	devcb_resolved_write_line out_pe_func;     /* pin 35, centronics pe output */
	devcb_resolved_read8 in_data_func;         /* pin 47-54, centronics data input */
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE e05a03_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == E05A03);

	return (e05a03_state *)downcast<e05a03_device *>(device)->token();
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( e05a03 )
{
	e05a03_state *e05a03 = get_safe_token(device);
	const e05a03_interface *intf = (const e05a03_interface *)device->static_config();

	/* validate some basic stuff */
	assert(device->static_config() != NULL);

	/* resolve callbacks */
	e05a03->out_nlq_lp_func.resolve(intf->out_nlq_lp_func, *device);
	e05a03->out_pe_lp_func.resolve(intf->out_pe_lp_func, *device);
	e05a03->out_reso_func.resolve(intf->out_reso_func, *device);
	e05a03->out_pe_func.resolve(intf->out_pe_func, *device);
	e05a03->in_data_func.resolve(intf->in_data_func, *device);

	/* register for state saving */
	device->save_item(NAME(e05a03->shift));
	device->save_item(NAME(e05a03->busy_leading));
	device->save_item(NAME(e05a03->busy_software));
	device->save_item(NAME(e05a03->nlqlp));
	device->save_item(NAME(e05a03->cndlp));
#if 0
	device->save_item(NAME(e05a03->pe));
	device->save_item(NAME(e05a03->pelp));
#endif
	device->save_item(NAME(e05a03->printhead));
	device->save_item(NAME(e05a03->pf_motor));
	device->save_item(NAME(e05a03->cr_motor));
}

static DEVICE_RESET( e05a03 )
{
	e05a03_state *e05a03 = get_safe_token(device);

	e05a03->printhead = 0x00;
	e05a03->pf_motor = 0x00;
	e05a03->cr_motor = 0x0f;

	e05a03->out_pe_func(0);
	e05a03->out_pe_lp_func(1);

	e05a03->busy_software = 1;
	e05a03->nlqlp = 1;
	e05a03->cndlp = 1;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

WRITE8_DEVICE_HANDLER( e05a03_w )
{
	e05a03_state *e05a03 = get_safe_token(device);

	logerror("%s: e05a03_w(%02x): %02x\n", space.machine().describe_context(), offset, data);

	switch (offset)
	{
	/* shift register */
	case 0x00: e05a03->shift = (e05a03->shift & 0x00ffff) | (data << 16); break;
	case 0x01: e05a03->shift = (e05a03->shift & 0xff00ff) | (data << 8); break;
	case 0x02: e05a03->shift = (e05a03->shift & 0xffff00) | (data << 0); break;

	case 0x03:
		e05a03->busy_leading = BIT(data, 7);
		e05a03->busy_software = BIT(data, 6);
		e05a03->nlqlp = BIT(data, 4);
		e05a03->cndlp = BIT(data, 3);

		e05a03->out_pe_func(BIT(data, 2));
		e05a03->out_pe_lp_func(!BIT(data, 2));

#if 0
		e05a03->pe = BIT(data, 2);
		e05a03->pelp = !BIT(data, 2);
#endif

		break;

	/* printhead */
	case 0x04: e05a03->printhead = (e05a03->printhead & 0x100) | !data; break;
	case 0x05: e05a03->printhead = (e05a03->printhead & 0x0ff) | (!(BIT(data, 7) << 8)); break;

	/* paper feed and carriage motor phase data*/
	case 0x06: e05a03->pf_motor = (data & 0xf0) >> 4; break;
	case 0x07: e05a03->cr_motor = (data & 0x0f) >> 0; break;
	}
}

READ8_DEVICE_HANDLER( e05a03_r )
{
	e05a03_state *e05a03 = get_safe_token(device);
	UINT8 result = 0;

	logerror("%s: e05a03_r(%02x)\n", space.machine().describe_context(), offset);

	switch (offset)
	{
	case 0x00:
		break;

	case 0x01:
		break;

	case 0x02:
		result = e05a03->in_data_func(0);
		break;

	case 0x03:
		result |= BIT(e05a03->shift, 23) << 7;
		e05a03->shift <<= 1;
		break;
	}

	return result;
}

/* home position signal */
WRITE_LINE_DEVICE_HANDLER( e05a03_home_w )
{
}

/* printhead solenoids trigger */
WRITE_LINE_DEVICE_HANDLER( e05a03_fire_w )
{
}

WRITE_LINE_DEVICE_HANDLER( e05a03_strobe_w )
{
}

READ_LINE_DEVICE_HANDLER( e05a03_busy_r )
{
	return 1;
}

WRITE_LINE_DEVICE_HANDLER( e05a03_resi_w )
{
	e05a03_state *e05a03 = get_safe_token(device);

	if (!state)
	{
		DEVICE_RESET_CALL( e05a03 );
		e05a03->out_reso_func(1);
	}
}

WRITE_LINE_DEVICE_HANDLER( e05a03_init_w )
{
	e05a03_resi_w(device, state);
}

const device_type E05A03 = &device_creator<e05a03_device>;

e05a03_device::e05a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, E05A03, "E05A03", tag, owner, clock)
{
	m_token = global_alloc_clear(e05a03_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void e05a03_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e05a03_device::device_start()
{
	DEVICE_START_NAME( e05a03 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e05a03_device::device_reset()
{
	DEVICE_RESET_NAME( e05a03 )(this);
}


