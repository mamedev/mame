// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A7800 HighScore passthrough cart emulation


***********************************************************************************************************/


#include "emu.h"
#include "hiscore.h"
#include "a78_carts.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type A78_HISCORE = &device_creator<a78_hiscore_device>;


a78_hiscore_device::a78_hiscore_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: a78_rom_device(mconfig, A78_HISCORE, "Atari 7800 High Score Cart", tag, owner, clock, "a78_highscore", __FILE__),
						m_hscslot(*this, "hsc_slot")
{
}


static MACHINE_CONFIG_FRAGMENT( a78_highscore )
	MCFG_A78_CARTRIDGE_ADD("hsc_slot", a7800_cart, nullptr)
MACHINE_CONFIG_END

machine_config_constructor a78_hiscore_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a78_highscore );
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(a78_hiscore_device::read_10xx)
{
	return m_nvram[offset];
}

WRITE8_MEMBER(a78_hiscore_device::write_10xx)
{
	m_nvram[offset] = data;
}

READ8_MEMBER(a78_hiscore_device::read_30xx)
{
	return m_rom[offset];
}

READ8_MEMBER(a78_hiscore_device::read_04xx)
{
	return m_hscslot->read_04xx(space, offset);
}

WRITE8_MEMBER(a78_hiscore_device::write_04xx)
{
	m_hscslot->write_04xx(space, offset, data);
}

READ8_MEMBER(a78_hiscore_device::read_40xx)
{
	return m_hscslot->read_40xx(space, offset);
}

WRITE8_MEMBER(a78_hiscore_device::write_40xx)
{
	m_hscslot->write_40xx(space, offset, data);
}
