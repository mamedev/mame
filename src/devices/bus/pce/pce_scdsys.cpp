// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol, Angelo Salese
/***********************************************************************************************************


 PC-Engine & Turbografx-16 Super System Card emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "pce_scdsys.h"



//-------------------------------------------------
//  pce_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3J,   pce_cdsys3j_device,   "pce_cdsys3j",   "PCE Super System Card")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3U,   pce_cdsys3u_device,   "pce_cdsys3u",   "TG16 Super System Card")


pce_cdsys3_device::pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool region)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface( mconfig, *this )
	, m_region(region)
{
}

pce_cdsys3j_device::pce_cdsys3j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_CDSYS3J, tag, owner, clock, false)
{
}

pce_cdsys3u_device::pce_cdsys3u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_CDSYS3U, tag, owner, clock, true)
{
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t pce_cdsys3_device::read_cart(offs_t offset)
{
	if (!m_ram.empty() && offset >= 0xd0000)
		return m_ram[offset - 0xd0000];

	const int bank = offset / 0x20000;
	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

void pce_cdsys3_device::write_cart(offs_t offset, uint8_t data)
{
	if (!m_ram.empty() && offset >= 0xd0000)
		m_ram[offset - 0xd0000] = data;
}

uint8_t pce_cdsys3_device::read_ex(offs_t offset)
{
	switch (offset & 0x0f)
	{
		case 0x1: return 0xaa;
		case 0x2: return 0x55;
		case 0x3: return 0x00;
		case 0x5: return (m_region) ? 0x55 : 0xaa;
		case 0x6: return (m_region) ? 0xaa : 0x55;
		case 0x7: return 0x03;
	}
	return 0x00;
}

