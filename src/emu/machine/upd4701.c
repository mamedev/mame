/***************************************************************************

    NEC uPD4701

    Incremental Encoder Control

    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "upd4701.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct upd4701_state
{
	int cs;
	int xy;
	int ul;
	int resetx;
	int resety;
	int latchx;
	int latchy;
	int startx;
	int starty;
	int x;
	int y;
	int switches;
	int latchswitches;
	int cf;
};

/* x,y increments can be 12bit (see MASK_COUNTER), hence we need a couple of
16bit handlers in the following  */

#define MASK_SWITCHES ( 7 )
#define MASK_COUNTER ( 0xfff )


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE upd4701_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == UPD4701));
	return (upd4701_state *)downcast<upd4701_device *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    upd4701_ul_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_ul_w )
{
	upd4701_state *upd4701 = get_safe_token(device);
	upd4701->ul = data;
}

/*-------------------------------------------------
    upd4701_xy_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_xy_w )
{
	upd4701_state *upd4701 = get_safe_token(device);
	upd4701->xy = data;
}

/*-------------------------------------------------
    upd4701_cs_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_cs_w )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (data != upd4701->cs)
	{
		upd4701->cs = data;

		if (!upd4701->cs)
		{
			upd4701->latchx = (upd4701->x - upd4701->startx) & MASK_COUNTER;
			upd4701->latchy = (upd4701->y - upd4701->starty) & MASK_COUNTER;

			upd4701->latchswitches = (~upd4701->switches) & MASK_SWITCHES;
			if (upd4701->latchswitches != 0)
			{
				upd4701->latchswitches |= 8;
			}

			upd4701->cf = 1;
		}
	}
}

/*-------------------------------------------------
    upd4701_resetx_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_resetx_w )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (upd4701->resetx != data)
	{
		upd4701->resetx = data;

		if (upd4701->resetx)
		{
			upd4701->startx = upd4701->x;
		}
	}
}

/*-------------------------------------------------
    upd4701_resety_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_resety_w )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (upd4701->resety != data)
	{
		upd4701->resety = data;

		if (upd4701->resety)
		{
			upd4701->starty = upd4701->y;
		}
	}
}

/*-------------------------------------------------
    upd4701_x_add
-------------------------------------------------*/

WRITE16_DEVICE_HANDLER( upd4701_x_add )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (!upd4701->resetx && data != 0)
	{
		upd4701->x += data;

		if (upd4701->cs)
		{
			upd4701->cf = 0;
		}
	}
}

/*-------------------------------------------------
    upd4701_y_add
-------------------------------------------------*/

WRITE16_DEVICE_HANDLER( upd4701_y_add )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if (!upd4701->resety && data != 0)
	{
		upd4701->y += data;

		if (upd4701->cs)
		{
			upd4701->cf = 0;
		}
	}
}

/*-------------------------------------------------
    upd4701_switches_set
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( upd4701_switches_set )
{
	upd4701_state *upd4701 = get_safe_token(device);
	upd4701->switches = data;
}

/*-------------------------------------------------
    upd4701_d_r
-------------------------------------------------*/

READ16_DEVICE_HANDLER( upd4701_d_r )
{
	upd4701_state *upd4701 = get_safe_token(device);
	int data;

	if (upd4701->cs)
	{
		return 0xff;
	}

	if (upd4701->xy)
	{
		data = upd4701->latchy;
	}
	else
	{
		data = upd4701->latchx;
	}

	data |= upd4701->latchswitches << 12;

	if (upd4701->ul)
	{
		return data >> 8;
	}
	else
	{
		return data & 0xff;
	}
}

/*-------------------------------------------------
    upd4701_sf_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( upd4701_sf_r )
{
	upd4701_state *upd4701 = get_safe_token(device);

	if ((upd4701->switches & MASK_SWITCHES) != MASK_SWITCHES)
	{
		return 0;
	}

	return 1;
}

/*-------------------------------------------------
    upd4701_cf_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( upd4701_cf_r )
{
	upd4701_state *upd4701 = get_safe_token(device);
	return upd4701->cf;
}

/*-------------------------------------------------
    DEVICE_START( upd4701 )
-------------------------------------------------*/

static DEVICE_START( upd4701 )
{
	upd4701_state *upd4701 = get_safe_token(device);

	/* register for state saving */
	device->save_item(NAME(upd4701->cs));
	device->save_item(NAME(upd4701->xy));
	device->save_item(NAME(upd4701->ul));
	device->save_item(NAME(upd4701->resetx));
	device->save_item(NAME(upd4701->resety));
	device->save_item(NAME(upd4701->latchx));
	device->save_item(NAME(upd4701->latchy));
	device->save_item(NAME(upd4701->startx));
	device->save_item(NAME(upd4701->starty));
	device->save_item(NAME(upd4701->x));
	device->save_item(NAME(upd4701->y));
	device->save_item(NAME(upd4701->switches));
	device->save_item(NAME(upd4701->latchswitches));
	device->save_item(NAME(upd4701->cf));
}

/*-------------------------------------------------
    DEVICE_RESET( upd4701 )
-------------------------------------------------*/

static DEVICE_RESET( upd4701 )
{
	upd4701_state *upd4701 = get_safe_token(device);

	upd4701->cs = 1;
	upd4701->xy = 0;
	upd4701->ul = 0;
	upd4701->resetx = 0;
	upd4701->resety = 0;
	upd4701->latchx = 0;
	upd4701->latchy = 0;
	upd4701->startx = 0;
	upd4701->starty = 0;
	upd4701->x = 0;
	upd4701->y = 0;
	upd4701->switches = 0;
	upd4701->latchswitches = 0;
	upd4701->cf = 1;
}

const device_type UPD4701 = &device_creator<upd4701_device>;

upd4701_device::upd4701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD4701, "NEC uPD4701 Encoder", tag, owner, clock)
{
	m_token = global_alloc_clear(upd4701_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd4701_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4701_device::device_start()
{
	DEVICE_START_NAME( upd4701 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd4701_device::device_reset()
{
	DEVICE_RESET_NAME( upd4701 )(this);
}
