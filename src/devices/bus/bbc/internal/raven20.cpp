// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Raven-20 RAM expansion board

    TODO:
    - Verify behaviour of RAVEN-20 1.00, seems buggy or incomplete emulation.
    - FAST mode (*FRON) is not understood.

**********************************************************************/


#include "emu.h"
#include "raven20.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_RAVEN20, bbc_raven20_device, "bbc_raven20", "Raven-20 RAM expansion")


//-------------------------------------------------
//  ROM( raven20 )
//-------------------------------------------------

ROM_START(raven20)
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "106", "Raven-20 1.06")
	ROMX_LOAD("raven20_1.06.rom", 0x0000, 0x2000, CRC(7efb0ab3) SHA1(d395199ef8f51579fe74e3d7d8dab1de6e0b30e2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "100", "Raven-20 1.00")
	ROMX_LOAD("raven20_1.00.rom", 0x0000, 0x2000, CRC(e8c8f9d5) SHA1(e4f2acc07969e10c082355c82d8d1cbc1c08ee2e), ROM_BIOS(1))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_raven20_device::device_rom_region() const
{
	return ROM_NAME( raven20 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_raven20_device - constructor
//-------------------------------------------------

bbc_raven20_device::bbc_raven20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_RAVEN20, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_raven20_device::device_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_write_handler(0xd000, 0xd000, write8sm_delegate(*this, FUNC(bbc_raven20_device::enable_w)));
	program.install_write_handler(0xc000, 0xc000, write8sm_delegate(*this, FUNC(bbc_raven20_device::disable_w)));

	m_shadow = false;
	m_ram = std::make_unique<uint8_t[]>(0x5000);

	/* register for save states */
	save_item(NAME(m_shadow));
	save_pointer(NAME(m_ram), 0x5000);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_raven20_device::enable_w(offs_t offset, uint8_t data)
{
	m_shadow = true;
}

void bbc_raven20_device::disable_w(offs_t offset, uint8_t data)
{
	m_shadow = false;
}

uint8_t bbc_raven20_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_shadow && offset >= 0x3000)
		data = m_ram[offset - 0x3000];
	else
		data = m_mb_ram->pointer()[offset];

	return data;
}

void bbc_raven20_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_shadow && offset >= 0x3000)
		m_ram[offset - 0x3000] = data;
	else
		m_mb_ram->pointer()[offset] = data;
}
