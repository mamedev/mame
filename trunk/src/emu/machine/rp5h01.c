/***************************************************************************

    RP5H01

    TODO:
    - follow the datasheet better (all dumps presumably needs to be redone
      from scratch?)

***************************************************************************/

#include "emu.h"
#include "machine/rp5h01.h"


//-------------------------------------------------
//  rp5h01_device - constructor
//-------------------------------------------------

const device_type RP5H01 = &device_creator<rp5h01_device>;

rp5h01_device::rp5h01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RP5H01, "RP5H01", tag, owner, clock, "rp5h01", __FILE__)
{
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
	m_data = *region();

	/* register for state saving */
	save_item(NAME(m_counter));
	save_item(NAME(m_counter_mode));
	save_item(NAME(m_enabled));
	save_item(NAME(m_old_reset));
	save_item(NAME(m_old_clock));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rp5h01_device::device_reset()
{
	m_counter = 0;
	m_counter_mode = COUNTER_MODE_6_BITS;
	m_enabled = 0;
	m_old_reset = -1;
	m_old_clock = -1;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    enable_w
-------------------------------------------------*/

WRITE8_MEMBER( rp5h01_device::enable_w )
{
	/* process the /CE signal and enable/disable the IC */
	m_enabled = (data == 0) ? 1 : 0;
}

/*-------------------------------------------------
    reset_w
-------------------------------------------------*/

WRITE8_MEMBER( rp5h01_device::reset_w )
{
	int newstate = (data == 0) ? 0 : 1;

	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	/* now look for a 0->1 transition */
	if (m_old_reset == 0 && newstate == 1)
	{
		/* reset the counter */
		m_counter = 0;
	}

	/* update the pin */
	m_old_reset = newstate;
}

/*-------------------------------------------------
    cs_w
-------------------------------------------------*/

WRITE8_MEMBER( rp5h01_device::cs_w )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	if (data == 1)
	{
		/* reset the counter */
		m_counter = 0;
	}
}

/*-------------------------------------------------
    clock_w
-------------------------------------------------*/

WRITE8_MEMBER( rp5h01_device::clock_w )
{
	int newstate = (data == 0) ? 0 : 1;

	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	/* now look for a 1->0 transition */
	if (m_old_clock == 1 && newstate == 0)
	{
		/* increment the counter, and mask it with the mode */
		m_counter++;
	}

	/* update the pin */
	m_old_clock = newstate;
}

/*-------------------------------------------------
    test_w
-------------------------------------------------*/

WRITE8_MEMBER( rp5h01_device::test_w )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	/* process the test signal and change the counter mode */
	m_counter_mode = (data == 0) ? COUNTER_MODE_6_BITS : COUNTER_MODE_7_BITS;
}

/*-------------------------------------------------
    counter_r
-------------------------------------------------*/

READ8_MEMBER( rp5h01_device::counter_r )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return 0; /* ? (should be high impedance) */

	/* return A5 */
	return (m_counter >> 5) & 1;
}

/*-------------------------------------------------
    data_r
-------------------------------------------------*/

READ8_MEMBER( rp5h01_device::data_r )
{
	int byte, bit;

	/* if it's not enabled, ignore */
	if (!m_enabled)
		return 0; /* ? (should be high impedance) */

	/* get the byte offset and bit offset */
	byte = (m_counter & m_counter_mode) >> 3;
	bit = 7 - (m_counter & 7);

	/* return the data */
	return (m_data[byte] >> bit) & 1;
}
