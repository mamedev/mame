/***************************************************************************

    RP5H01

    TODO:
    - convert to modern and follow the datasheet better (all dumps
      presumably needs to be redone from scratch?)

    2009-06 Converted to be a device

***************************************************************************/

#include "emu.h"
#include "machine/rp5h01.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

/* these also work as the address masks */
enum {
	COUNTER_MODE_6_BITS = 0x3f,
	COUNTER_MODE_7_BITS = 0x7f
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct rp5h01_state
{
	int counter;
	int counter_mode;   /* test pin */
	int enabled;        /* chip enable */
	int old_reset;      /* reset pin state (level-triggered) */
	int old_clock;      /* clock pin state (level-triggered) */
	UINT8 *data;
};

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE rp5h01_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert((device->type() == RP5H01));
	return (rp5h01_state *)downcast<rp5h01_device *>(device)->token();
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    rp5h01_enable_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_enable_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* process the /CE signal and enable/disable the IC */
	rp5h01->enabled = (data == 0) ? 1 : 0;
}

/*-------------------------------------------------
    rp5h01_reset_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_reset_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);
	int newstate = (data == 0) ? 0 : 1;

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	/* now look for a 0->1 transition */
	if (rp5h01->old_reset == 0 && newstate == 1)
	{
		/* reset the counter */
		rp5h01->counter = 0;
	}

	/* update the pin */
	rp5h01->old_reset = newstate;
}

/*-------------------------------------------------
    rp5h01_cs_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_cs_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	if (data == 1)
	{
		/* reset the counter */
		rp5h01->counter = 0;
	}
}

/*-------------------------------------------------
    rp5h01_clock_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_clock_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);
	int newstate = (data == 0) ? 0 : 1;

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	/* now look for a 1->0 transition */
	if (rp5h01->old_clock == 1 && newstate == 0)
	{
		/* increment the counter, and mask it with the mode */
		rp5h01->counter++;
	}

	/* update the pin */
	rp5h01->old_clock = newstate;
}

/*-------------------------------------------------
    rp5h01_test_w
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER( rp5h01_test_w )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return;

	/* process the test signal and change the counter mode */
	rp5h01->counter_mode = (data == 0) ? COUNTER_MODE_6_BITS : COUNTER_MODE_7_BITS;
}

/*-------------------------------------------------
    rp5h01_counter_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( rp5h01_counter_r )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return 0; /* ? (should be high impedance) */

	/* return A5 */
	return (rp5h01->counter >> 5) & 1;
}

/*-------------------------------------------------
    rp5h01_data_r
-------------------------------------------------*/

READ8_DEVICE_HANDLER( rp5h01_data_r )
{
	rp5h01_state *rp5h01 = get_safe_token(device);
	int byte, bit;

	/* if it's not enabled, ignore */
	if (!rp5h01->enabled)
		return 0; /* ? (should be high impedance) */

	/* get the byte offset and bit offset */
	byte = (rp5h01->counter & rp5h01->counter_mode) >> 3;
	bit = 7 - (rp5h01->counter & 7);

	/* return the data */
	return (rp5h01->data[byte] >> bit) & 1;
}

/*-------------------------------------------------
    DEVICE_START( rp5h01 )
-------------------------------------------------*/

static DEVICE_START( rp5h01 )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	assert(device->static_config() == NULL);

	rp5h01->data = *device->region();

	/* register for state saving */
	device->save_item(NAME(rp5h01->counter));
	device->save_item(NAME(rp5h01->counter_mode));
	device->save_item(NAME(rp5h01->enabled));
	device->save_item(NAME(rp5h01->old_reset));
	device->save_item(NAME(rp5h01->old_clock));
}

/*-------------------------------------------------
    DEVICE_RESET( rp5h01 )
-------------------------------------------------*/

static DEVICE_RESET( rp5h01 )
{
	rp5h01_state *rp5h01 = get_safe_token(device);

	rp5h01->counter = 0;
	rp5h01->counter_mode = COUNTER_MODE_6_BITS;
	rp5h01->enabled = 0;
	rp5h01->old_reset = -1;
	rp5h01->old_clock = -1;
}

const device_type RP5H01 = &device_creator<rp5h01_device>;

rp5h01_device::rp5h01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RP5H01, "RP5H01", tag, owner, clock)
{
	m_token = global_alloc_clear(rp5h01_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void rp5h01_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rp5h01_device::device_start()
{
	DEVICE_START_NAME( rp5h01 )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rp5h01_device::device_reset()
{
	DEVICE_RESET_NAME( rp5h01 )(this);
}
