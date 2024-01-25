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

DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3J, pce_cdsys3j_device, "pce_cdsys3j", "PCE Super System Card")
DEFINE_DEVICE_TYPE(PCE_ROM_CDSYS3U, pce_cdsys3u_device, "pce_cdsys3u", "TG16 Super System Card")


pce_cdsys3_device::pce_cdsys3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_pce_cart_interface(mconfig, *this)
	, m_scdsys()
{
}

pce_cdsys3j_device::pce_cdsys3j_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_CDSYS3J, tag, owner, clock)
{
}

pce_cdsys3u_device::pce_cdsys3u_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pce_cdsys3_device(mconfig, PCE_ROM_CDSYS3U, tag, owner, clock)
{
}

pce_scdsys_shared::pce_scdsys_shared()
	: m_ram(nullptr)
	, m_region(false)
{
}


//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void pce_cdsys3_device::device_start()
{
	m_scdsys.init(*this);
	m_scdsys.set_region(false);
}

void pce_cdsys3u_device::device_start()
{
	pce_cdsys3_device::device_start();
	m_scdsys.set_region(true);
}

void pce_scdsys_shared::init(device_t &device)
{
	m_ram = make_unique_clear<uint8_t[]>(0x30000);

	device.save_pointer(NAME(m_ram), 0x30000);
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t pce_scdsys_shared::register_r(offs_t offset)
{
	switch (offset & 0x0f)
	{
		// TODO: 0x1 through 0x3 is Super CD-ROM²/PCE Duo stuff?
		case 0x1: return 0xaa;
		case 0x2: return 0x55;
		case 0x3: return 0x00;
		case 0x5: return (m_region) ? 0x55 : 0xaa;
		case 0x6: return (m_region) ? 0xaa : 0x55;
		case 0x7: return 0x03;
	}
	return 0x00;
}


void pce_cdsys3_device::install_memory_handlers(address_space &space)
{
	// TODO: ROM address mirrored?
	space.install_rom(0x000000, 0x03ffff, 0x040000, m_rom);
	space.install_ram(0x0d0000, 0x0fffff, m_scdsys.ram());
	space.install_read_handler(0x1ff8c0, 0x1ff8c7, 0, 0x330, 0, emu::rw_delegate(*this, FUNC(pce_cdsys3_device::register_r)));
}
