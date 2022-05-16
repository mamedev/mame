// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Memex B20 20K RAM expansion

    This is a DIY project from the May 1984 issue of Electronics and
    Computing, with part two of the project in the August 1984 issue.

**********************************************************************/


#include "emu.h"
#include "memex.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MEMEXB20, bbc_memexb20_device, "bbc_memexb20", "Memex-B20 RAM expansion");


//-------------------------------------------------
//  ROM( memexb20 )
//-------------------------------------------------

ROM_START(memexb20)
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_LOAD("memex-b20_v2.2.rom", 0x0000, 0x2000, CRC(98ee8eb6) SHA1(aad311797e204b0b69450460f8b4605409c31ddd))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_memexb20_device::device_rom_region() const
{
	return ROM_NAME(memexb20);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_memexb20_device - constructor
//-------------------------------------------------

bbc_memexb20_device::bbc_memexb20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MEMEXB20, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_shadow(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_memexb20_device::device_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_write_handler(0xd000, 0xd000, write8smo_delegate(*this, FUNC(bbc_memexb20_device::enable_w)));
	program.install_write_handler(0xc000, 0xc000, write8smo_delegate(*this, FUNC(bbc_memexb20_device::disable_w)));

	m_ram = make_unique_clear<uint8_t[]>(0x5000);

	/* register for save states */
	save_item(NAME(m_shadow));
	save_pointer(NAME(m_ram), 0x5000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_memexb20_device::enable_w(uint8_t data)
{
	m_shadow = true;
}

void bbc_memexb20_device::disable_w(uint8_t data)
{
	m_shadow = false;
}

uint8_t bbc_memexb20_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_shadow && offset >= 0x3000)
		data = m_ram[offset - 0x3000];
	else
		data = m_mb_ram->pointer()[offset];

	return data;
}

void bbc_memexb20_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_shadow && offset >= 0x3000)
		m_ram[offset - 0x3000] = data;
	else
		m_mb_ram->pointer()[offset] = data;
}
