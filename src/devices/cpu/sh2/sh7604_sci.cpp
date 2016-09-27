// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

Template for skeleton device

***************************************************************************/

#include "emu.h"
#include "sh7604_sci.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SH7604_SCI = &device_creator<sh7604_sci_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

static ADDRESS_MAP_START( sci_regs, AS_0, 8, sh7604_sci_device )

ADDRESS_MAP_END

//-------------------------------------------------
//  sh7604_sci_device - constructor
//-------------------------------------------------

sh7604_sci_device::sh7604_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SH7604_SCI, "sh7604_sci_longname", tag, owner, clock, "sh7604_sci", __FILE__),
	device_memory_interface(mconfig, *this),
	m_space_config("regs", ENDIANNESS_BIG, 8, 4, 0, nullptr, *ADDRESS_MAP_NAME(sci_regs))

{
}


const address_space_config *sh7604_sci_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sh7604_sci_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sh7604_sci_device::device_reset()
{
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( sh7604_sci_device::read )
{
	return space.read_byte(offset);
}

WRITE8_MEMBER( sh7604_sci_device::write )
{
	space.write_byte(offset,data);
}
