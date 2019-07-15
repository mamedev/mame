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

DEFINE_DEVICE_TYPE(A78_HISCORE, a78_hiscore_device, "a78_hiscore", "Atari 7800 High Score Cart")


a78_hiscore_device::a78_hiscore_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, A78_HISCORE, tag, owner, clock)
	, m_hscslot(*this, "hsc_slot")
{
}


void a78_hiscore_device::device_add_mconfig(machine_config &config)
{
	A78_CART_SLOT(config, "hsc_slot", a7800_cart, nullptr);
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
