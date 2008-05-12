/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "driver.h"
#include "cdp1852.h"

typedef struct _cdp1852_t cdp1852_t;
struct _cdp1852_t
{
	const cdp1852_interface *intf;	/* interface */

	int new_data;					/* new data written */
	UINT8 data;						/* data latch */
	UINT8 next_data;				/* next data*/

	int sr;							/* service request flag */
	int next_sr;					/* next value of service request flag */

	/* timers */
	emu_timer *scan_timer;			/* scan timer */
};

INLINE cdp1852_t *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);

	return (cdp1852_t *)device->token;
}

static void set_sr_line(const device_config *device, int level)
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	if (cdp1852->intf->on_sr_changed)
	{
		if (cdp1852->sr != level)
		{
			cdp1852->intf->on_sr_changed(device, level);
			cdp1852->sr = level;
		}
	}
}

static TIMER_CALLBACK( cdp1852_scan_tick )
{
	const device_config *device = ptr;
	cdp1852_t *cdp1852 = get_safe_token(device);

	switch (cdp1852->intf->mode)
	{
	case CDP1852_MODE_INPUT:
		// input data into register
		cdp1852->data = cdp1852->intf->data_r(device);

		// signal processor
		set_sr_line(device, 0);
		break;

	case CDP1852_MODE_OUTPUT:
		if (cdp1852->new_data)
		{
			cdp1852->new_data = 0;

			// latch data into register
			cdp1852->data = cdp1852->next_data;

			// output data
			cdp1852->intf->data_w(device, cdp1852->data);

			// signal peripheral device
			set_sr_line(device, 1);

			cdp1852->next_sr = 0;
		}
		else
		{
			set_sr_line(device, cdp1852->next_sr);
		}
		break;
	}
}

/* Data Access */

READ8_DEVICE_HANDLER( cdp1852_data_r )
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	if (cdp1852->intf->mode == CDP1852_MODE_INPUT && cdp1852->intf->clock == 0)
	{
		// input data into register
		cdp1852->data = cdp1852->intf->data_r(device);
	}

	set_sr_line(device, 1);

	return cdp1852->data;
}

WRITE8_DEVICE_HANDLER( cdp1852_data_w )
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	if (cdp1852->intf->mode == CDP1852_MODE_OUTPUT)
	{
		cdp1852->next_data = data;
		cdp1852->new_data = 1;
	}
}

/* Device Interface */

static DEVICE_START( cdp1852 )
{
	cdp1852_t *cdp1852 = get_safe_token(device);
	char unique_tag[30];

	/* validate arguments */
	assert(device != NULL);
	assert(device->tag != NULL);
	assert(strlen(device->tag) < 20);

	cdp1852->intf = device->static_config;

	assert(cdp1852->intf != NULL);

	if (cdp1852->intf->mode == CDP1852_MODE_INPUT)
	{
		assert(cdp1852->intf->data_r != NULL);
	}
	else
	{
		assert(cdp1852->intf->clock > 0);
		assert(cdp1852->intf->data_w != NULL);
	}

	/* create the timers */
	if (cdp1852->intf->clock > 0)
	{
		cdp1852->scan_timer = timer_alloc(cdp1852_scan_tick, (void *)device);
		timer_adjust_periodic(cdp1852->scan_timer, attotime_zero, 0, ATTOTIME_IN_HZ(cdp1852->intf->clock));
	}

	/* register for state saving */
	state_save_combine_module_and_tag(unique_tag, "CDP1852", device->tag);

	state_save_register_item(unique_tag, 0, cdp1852->new_data);
	state_save_register_item(unique_tag, 0, cdp1852->data);
	state_save_register_item(unique_tag, 0, cdp1852->next_data);
	state_save_register_item(unique_tag, 0, cdp1852->sr);
	state_save_register_item(unique_tag, 0, cdp1852->next_sr);
}

static DEVICE_RESET( cdp1852 )
{
	cdp1852_t *cdp1852 = get_safe_token(device);

	// reset data register
	cdp1852->data = 0;
	
	if (cdp1852->intf->mode == CDP1852_MODE_INPUT)
	{
		// reset service request flip-flop
		set_sr_line(device, 1);
	}
	else
	{
		// output data
		cdp1852->intf->data_w(device, 0);
	
		// reset service request flip-flop
		set_sr_line(device, 0);
	}
}

static DEVICE_SET_INFO( cdp1852 )
{
	switch (state)
	{
		/* no parameters to set */
	}
}

DEVICE_GET_INFO( cdp1852 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(cdp1852_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0;								break;
		case DEVINFO_INT_CLASS:							info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:						info->set_info = DEVICE_SET_INFO_NAME(cdp1852); break;
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(cdp1852);	break;
		case DEVINFO_FCT_STOP:							/* Nothing */								break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME(cdp1852);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							info->s = "RCA CDP1852";					break;
		case DEVINFO_STR_FAMILY:						info->s = "RCA CDP1800";					break;
		case DEVINFO_STR_VERSION:						info->s = "1.0";							break;
		case DEVINFO_STR_SOURCE_FILE:					info->s = __FILE__;							break;
		case DEVINFO_STR_CREDITS:						info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
