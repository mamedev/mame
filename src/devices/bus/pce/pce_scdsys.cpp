// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Wilbert Pol
/***********************************************************************************************************


 PC-Engine & Turbografx-16 Super System Card emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "pce_scdsys.h"



//-------------------------------------------------
//  pce_cdsys3_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3_BASE, pce_cdsys3_base_device, "pce_cdsys3_base", "PCE/TG16 Super System Card Base interface")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3J,     pce_cdsys3j_device,     "pce_cdsys3j",     "PCE Super System Card")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3U,     pce_cdsys3u_device,     "pce_cdsys3u",     "TG16 Super System Card")


pce_cdsys3_base_device::pce_cdsys3_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCE_ROM_CDSYS3_BASE, tag, owner, clock)
	, m_region(false)
{
}

pce_cdsys3_device::pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool region)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface(mconfig, *this)
	, m_cdsys3(*this, "cdsys3")
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


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------


void pce_cdsys3_base_device::device_start()
{
	/* Set up Arcade Card RAM buffer */
	m_ram = make_unique_clear<uint8_t[]>(0x30000);

	save_pointer(NAME(m_ram), 0x30000);
}

void pce_cdsys3_device::device_add_mconfig(machine_config &config)
{
	PCE_ROM_CDSYS3_BASE(config, m_cdsys3, DERIVED_CLOCK(1,1)).set_region(false);
}

void pce_cdsys3u_device::device_add_mconfig(machine_config &config)
{
	pce_cdsys3_device::device_add_mconfig(config);
	m_cdsys3->set_region(true);
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t pce_cdsys3_base_device::ram_r(offs_t offset)
{
	if (offset < 0x30000)
		return m_ram[offset];

	return 0xff;
}

void pce_cdsys3_base_device::ram_w(offs_t offset, uint8_t data)
{
	if (offset < 0x30000)
		m_ram[offset] = data;
}

uint8_t pce_cdsys3_device::read_cart(offs_t offset)
{
	if (offset >= 0xd0000)
		return m_cdsys3->ram_r(offset - 0xd0000);

	const int bank = offset / 0x20000;
	return m_rom[rom_bank_map[bank] * 0x20000 + (offset & 0x1ffff)];
}

void pce_cdsys3_device::write_cart(offs_t offset, uint8_t data)
{
	if (offset >= 0xd0000)
		m_cdsys3->ram_w(offset - 0xd0000, data);
}

uint8_t pce_cdsys3_base_device::register_r(offs_t offset)
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

uint8_t pce_cdsys3_device::read_ex(offs_t offset)
{
	return m_cdsys3->register_r(offset);
}

