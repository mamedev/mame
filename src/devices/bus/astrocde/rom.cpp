// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Bally Astrocade cart emulation


 ***********************************************************************************************************/


#include "emu.h"
#include "rom.h"


//-------------------------------------------------
//  astrocade_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(ASTROCADE_ROM_STD,  astrocade_rom_device,      "astrocade_rom",      "Bally Astrocade Standard Carts")
DEFINE_DEVICE_TYPE(ASTROCADE_ROM_256K, astrocade_rom_256k_device, "astrocade_rom_256k", "Bally Astrocade 256K Carts")
DEFINE_DEVICE_TYPE(ASTROCADE_ROM_512K, astrocade_rom_512k_device, "astrocade_rom_512k", "Bally Astrocade 512K Carts")
DEFINE_DEVICE_TYPE(ASTROCADE_ROM_CASS, astrocade_rom_cass_device, "astrocade_rom_cass", "Bally Astrocade AstroBASIC Cart")


astrocade_rom_device::astrocade_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), device_astrocade_cart_interface(mconfig, *this)
{
}

astrocade_rom_device::astrocade_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: astrocade_rom_device(mconfig, ASTROCADE_ROM_STD, tag, owner, clock)
{
}

astrocade_rom_256k_device::astrocade_rom_256k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: astrocade_rom_device(mconfig, ASTROCADE_ROM_256K, tag, owner, clock), m_base_bank(0)
{
}

astrocade_rom_512k_device::astrocade_rom_512k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: astrocade_rom_device(mconfig, ASTROCADE_ROM_512K, tag, owner, clock), m_base_bank(0)
{
}

astrocade_rom_cass_device::astrocade_rom_cass_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: astrocade_rom_device(mconfig, ASTROCADE_ROM_CASS, tag, owner, clock)
	, m_cassette(*this, "cassette")
{
}


void astrocade_rom_256k_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void astrocade_rom_256k_device::device_reset()
{
	m_base_bank = 0;
}

void astrocade_rom_512k_device::device_start()
{
	save_item(NAME(m_base_bank));
}

void astrocade_rom_512k_device::device_reset()
{
	m_base_bank = 0;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(astrocade_rom_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else
		return 0xff;
}

READ8_MEMBER(astrocade_rom_256k_device::read_rom)
{
	if (offset < 0x1000)    // 0x2000-0x2fff
		return m_rom[offset + 0x1000 * 0x3f];
	else if (offset < 0x1fc0)   // 0x3000-0x3fbf
		return m_rom[(offset & 0xfff) + (0x1000 * m_base_bank)];
	else    // 0x3fc0-0x3fff
		return m_base_bank = offset & 0x3f;
}

READ8_MEMBER(astrocade_rom_512k_device::read_rom)
{
	if (offset < 0x1000)    // 0x2000-0x2fff
		return m_rom[offset + 0x1000 * 0x7f];
	else if (offset < 0x1f80)   // 0x3000-0x3fbf
		return m_rom[(offset & 0xfff) + (0x1000 * m_base_bank)];
	else    // 0x3fc0-0x3fff
		return m_base_bank = offset & 0x7f;
}

READ8_MEMBER(astrocade_rom_cass_device::read_rom)
{
	if (offset < m_rom_size)
		return m_rom[offset];
	else if ((offset & 0x1c00) == 0x1800)
	{
		m_cassette->output(+1);
		return 0xff;
	}
	else if ((offset & 0x1c00) == 0x1c00)
	{
		m_cassette->output(-1);
		return m_cassette->input() > 0.0 ? 0 : 1;
	}
	return 0xff;
}

/*-------------------------------------------------
 mapper specific device configuration
 -------------------------------------------------*/

void astrocade_rom_cass_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->set_interface("astrocade_cass");
}
