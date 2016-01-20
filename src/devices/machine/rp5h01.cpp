// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    RP5H01 - Ricoh 64x1bit(+8bit) PROM with 6/7-bit counter

    In reality, PROM data is 72bits (64 + 8bit 'dummy'). In 7-bit counter mode,
    from 64 to 127 (%1000000 to %1111111), the dummy bits are read repeatedly,
    with a mask of %1010111. For example if the 8 dummy bits are $7c,
    bits 64 to 127 are read as $7c $7c $00 $00 $7c $7c $00 $00.
    To simplify this, our emulation expects 'overdumps', 128bits total.

    TODO:
    - not sure if the polarity of our PROM dumps (playch10) is correct,
      same goes for the bit order (note: does not require new dumps)

***************************************************************************/

#include "emu.h"
#include "machine/rp5h01.h"

// this is the contents of an unprogrammed PROM
static const UINT8 initial_data[0x10] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00
};

//-------------------------------------------------
//  rp5h01_device - constructor
//-------------------------------------------------

const device_type RP5H01 = &device_creator<rp5h01_device>;

rp5h01_device::rp5h01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, RP5H01, "RP5H01 6/7-bit Counter", tag, owner, clock, "rp5h01", __FILE__)
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
	m_data = region()->base();
	if (m_data == nullptr)
		m_data = initial_data;
	else
		assert(region()->bytes() == 0x10);

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
	m_old_reset = 0;
	m_old_clock = 0;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    enable_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( rp5h01_device::enable_w )
{
	/* process the /CE signal and enable/disable the IC */
	m_enabled = state ? 0 : 1;
}

/*-------------------------------------------------
    reset_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( rp5h01_device::reset_w )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	/* now look for a 0->1 transition */
	if (!m_old_reset && state)
	{
		/* reset the counter */
		m_counter = 0;
	}

	/* update the pin */
	m_old_reset = state;
}

/*-------------------------------------------------
    cs_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( rp5h01_device::cs_w )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	if (state)
	{
		/* reset the counter */
		m_counter = 0;
	}
}

/*-------------------------------------------------
    clock_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( rp5h01_device::clock_w )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	/* now look for a 1->0 transition */
	if (m_old_clock && !state)
	{
		/* increment the counter, and mask it with the mode */
		m_counter++;
	}

	/* update the pin */
	m_old_clock = state;
}

/*-------------------------------------------------
    test_w
-------------------------------------------------*/

WRITE_LINE_MEMBER( rp5h01_device::test_w )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return;

	/* process the test signal and change the counter mode */
	m_counter_mode = (state) ? COUNTER_MODE_7_BITS : COUNTER_MODE_6_BITS;
}

/*-------------------------------------------------
    counter_r
-------------------------------------------------*/

READ_LINE_MEMBER( rp5h01_device::counter_r )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return 1; /* high impedance */

	/* return A5 */
	return (m_counter >> 5) & 1;
}

/*-------------------------------------------------
    data_r
-------------------------------------------------*/

READ_LINE_MEMBER( rp5h01_device::data_r )
{
	/* if it's not enabled, ignore */
	if (!m_enabled)
		return 1; /* high impedance */

	/* get the byte offset and bit offset */
	int byte = (m_counter & m_counter_mode) >> 3;
	int bit = 7 - (m_counter & 7);

	/* return the data */
	return (m_data[byte] >> bit) & 1;
}
