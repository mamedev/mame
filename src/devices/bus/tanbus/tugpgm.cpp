// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    TUG Programmable Graphic Module

    http://www.microtan.ukpc.net/pageProducts.html#VIDEO

**********************************************************************/


#include "emu.h"
#include "tugpgm.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TUGPGM, tanbus_tugpgm_device, "tanbus_tugpgm", "TUG Programmable Graphic Module")


//-------------------------------------------------
//  INPUT_PORTS( tugpgm )
//-------------------------------------------------

INPUT_PORTS_START(tugpgm)
	PORT_START("LINKS")
	PORT_DIPNAME(0x07, 0x04, "Address Select")
	PORT_DIPSETTING(0x00, "$0400 - $07FF (Not Recommended)")
	PORT_DIPSETTING(0x01, "$2400 - $27FF")
	PORT_DIPSETTING(0x02, "$4400 - $47FF")
	PORT_DIPSETTING(0x03, "$6400 - $67FF")
	PORT_DIPSETTING(0x04, "$8400 - $87FF")
	PORT_DIPSETTING(0x05, "$A400 - $A7FF")
	PORT_DIPSETTING(0x06, "$C400 - $C7FF")
	PORT_DIPSETTING(0x07, "$E400 - $E7FF")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_tugpgm_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tugpgm);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tugpgm_device - constructor
//-------------------------------------------------

tanbus_tugpgm_device::tanbus_tugpgm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TUGPGM, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_links(*this, "LINKS")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tugpgm_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x0400);

	/* randomize video memory contents */
	for (uint16_t addr = 0; addr < 0x0400; addr++)
	{
		m_ram[addr] = machine().rand() & 0xff;
		m_tanbus->pgm_w(addr, m_ram[addr]);
	}

	save_pointer(NAME(m_ram), 0x0400);
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tugpgm_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	if ((offset & 0xfc00) == ((m_links->read() << 13) | 0x0400))
	{
		data = m_ram[offset & 0x3ff];
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tugpgm_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	if ((offset & 0xfc00) == ((m_links->read() << 13) | 0x0400))
	{
		m_ram[offset & 0x3ff] = data;

		/* update the chargen on the CPU board */
		m_tanbus->pgm_w(offset & 0x3ff, data);
	}
}
