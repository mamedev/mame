// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Alpha Denshi ALPHA-8201 family protection emulation

----------------------------------------------------------------------------

abc


TODO:
- x

***************************************************************************/

#include "cpu/hmcs40/hmcs40.h"
#include "alpha8201.h"

/**************************************************************************/

const device_type ALPHA_8201 = &device_creator<alpha_8201_device>;

//-------------------------------------------------
//  alpha_8201_device - constructor
//-------------------------------------------------

alpha_8201_device::alpha_8201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ALPHA_8201, "ALPHA-8201", tag, owner, clock, "alpha8201", __FILE__),
	m_mcu(*this, "mcu")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void alpha_8201_device::device_start()
{
	m_shared_ram = auto_alloc_array_clear(machine(), UINT8, 0x400);

	// register for savestates
	save_pointer(NAME(m_shared_ram), 0x400);
}

// machine config additions
static MACHINE_CONFIG_FRAGMENT(alpha8201)

	MCFG_CPU_ADD("mcu", HD44801, DERIVED_CLOCK(1,1))
MACHINE_CONFIG_END

machine_config_constructor alpha_8201_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(alpha8201);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void alpha_8201_device::device_reset()
{
	m_mcu->set_input_line(0, CLEAR_LINE);
}


/***************************************************************************

  Internal I/O

***************************************************************************/



/***************************************************************************

  I/O for External Interface

***************************************************************************/
