/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "cdp1852.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cdp1852_t cdp1852_t;
struct _cdp1852_t
{
	devcb_resolved_write_line	out_sr_func;
	devcb_resolved_read8		in_data_func;
	devcb_resolved_write8		out_data_func;

	cdp1852_mode mode;				/* operation mode */
	int new_data;					/* new data written */
	UINT8 data;						/* data latch */
	UINT8 next_data;				/* next data*/

	int sr;							/* service request flag */
	int next_sr;					/* next value of service request flag */

	/* timers */
	emu_timer *scan_timer;			/* scan timer */
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE cdp1852_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	return (cdp1852_t *)downcast<legacy_device_base *>(device)->token();
}

INLINE const cdp1852_interface *get_interface(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == CDP1852));
	return (const cdp1852_interface *) device->baseconfig().static_config();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    set_sr_line - service request out
-------------------------------------------------*/

static void set_sr_line(cdp1852_t *cdp1852, int level)
{
	if (cdp1852->sr != level)
	{
		cdp1852->sr = level;

		devcb_call_write_line(&cdp1852->out_sr_func, cdp1852->sr);
	}
}

/*-------------------------------------------------
    TIMER_CALLBACK( cdp1852_scan_tick )
-------------------------------------------------*/

static TIMER_CALLBACK( cdp1852_scan_tick )
{
	device_t *device = (device_t *)ptr;
	cdp1852_t *cdp1852 = get_safe_token(device);

	switch (cdp1852->mode)
	{
	case CDP1852_MODE_INPUT:
		/* input data into register */
		cdp1852->data = devcb_call_read8(&cdp1852->in_data_func, 0);

		/* signal processor */
		set_sr_line(cdp1852, 0);
		break;

	case CDP1852_MODE_OUTPUT:
		if (cdp1852->new_data)
		{
			cdp1852->new_data = 0;

			/* latch data into register */
			cdp1852->data = cdp1852->next_data;

			/* output data */
			devcb_call_write8(&cdp1852->out_data_func, 0, cdp1852->data);

			/* signal peripheral device */
			set_sr_line(cdp1852, 1);

			cdp1852->next_sr = 0;
		}
		else
		{
			set_sr_line(cdp1852, cdp1852->next_sr);
		}
		break;
	}
}

/*-------------------------------------------------
    cdp1852_data_r - data register read
-------------------------------------------------*/

READ8_DEVICE_HANDLER( cdp1852_data_r )
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	if (cdp1852->mode == CDP1852_MODE_INPUT && device->clock() == 0)
	{
		// input data into register
		cdp1852->data = devcb_call_read8(&cdp1852->in_data_func, 0);
	}

	set_sr_line(cdp1852, 1);

	return cdp1852->data;
}

/*-------------------------------------------------
    cdp1852_data_r - data register write
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( cdp1852_data_w )
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	if (cdp1852->mode == CDP1852_MODE_OUTPUT)
	{
		cdp1852->next_data = data;
		cdp1852->new_data = 1;
	}
}

/*-------------------------------------------------
    DEVICE_START( cdp1852 )
-------------------------------------------------*/

static DEVICE_START( cdp1852 )
{
	cdp1852_t *cdp1852 = get_safe_token(device);
	const cdp1852_interface *intf = get_interface(device);

	/* resolve callbacks */
	devcb_resolve_read8(&cdp1852->in_data_func, &intf->in_data_func, device);
	devcb_resolve_write8(&cdp1852->out_data_func, &intf->out_data_func, device);
	devcb_resolve_write_line(&cdp1852->out_sr_func, &intf->out_sr_func, device);

	/* set initial values */
	cdp1852->mode = (cdp1852_mode)intf->mode;

	if (device->clock() > 0)
	{
		/* create the scan timer */
		cdp1852->scan_timer = timer_alloc(device->machine, cdp1852_scan_tick, (void *)device);
		timer_adjust_periodic(cdp1852->scan_timer, attotime_zero, 0, ATTOTIME_IN_HZ(device->clock()));
	}

	/* register for state saving */
	state_save_register_device_item(device, 0, cdp1852->new_data);
	state_save_register_device_item(device, 0, cdp1852->data);
	state_save_register_device_item(device, 0, cdp1852->next_data);
	state_save_register_device_item(device, 0, cdp1852->sr);
	state_save_register_device_item(device, 0, cdp1852->next_sr);
}

/*-------------------------------------------------
    DEVICE_RESET( cdp1852 )
-------------------------------------------------*/

static DEVICE_RESET( cdp1852 )
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	/* reset data register */
	cdp1852->data = 0;

	if (cdp1852->mode == CDP1852_MODE_INPUT)
	{
		/* reset service request flip-flop */
		set_sr_line(cdp1852, 1);
	}
	else
	{
		/* output data */
		devcb_call_write8(&cdp1852->out_data_func, 0, cdp1852->data);

		/* reset service request flip-flop */
		set_sr_line(cdp1852, 0);
	}
}

/*-------------------------------------------------
    DEVICE_GET_INFO( cdp1852 )
-------------------------------------------------*/

DEVICE_GET_INFO( cdp1852 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1852_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cdp1852);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(cdp1852);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RCA CDP1852");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "RCA CDP1800");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_DEVICE(CDP1852, cdp1852);
