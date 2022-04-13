// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics 32K RAM card

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/WE_32KRAMcard.html

**********************************************************************/


#include "emu.h"
#include "we32kram.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_WE32KRAM, bbc_we32kram_device, "bbc_we32kram", "Watford Electronics 32K Shadow RAM");


//-------------------------------------------------
//  ROM( we32kram )
//-------------------------------------------------

ROM_START(we32kram)
	ROM_REGION(0x4000, "exp_rom", 0)
	ROM_SYSTEM_BIOS(0, "240", "v2.40")
	ROMX_LOAD("we32kram_v2.40.rom", 0x0000, 0x2000, CRC(db9c3a1a) SHA1(0a5efb084eb1ae353b9c79c026e9e26cd7695293), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "220", "v2.20")
	ROMX_LOAD("we32kram_v2.20.rom", 0x0000, 0x2000, CRC(bc7124e0) SHA1(7f8536576de449916cd77488e94775ed438c5c6c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "200", "v2.00")
	ROMX_LOAD("we32kram_v2.00.rom", 0x0000, 0x2000, CRC(1eff9b23) SHA1(98a3084b20c92427c2e9def58ade96f42659e20a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "106", "v1.06")
	ROMX_LOAD("we32kram_v1.06.rom", 0x0000, 0x2000, CRC(b9749390) SHA1(210f6162d634fca8d75197d4f64d9b50abc01e1e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "100", "v1.00")
	ROMX_LOAD("we32kram_v1.00.rom", 0x0000, 0x2000, CRC(7962b5db) SHA1(1b59a3354cfc704ad8aa60b0ed16e4cd2d815031), ROM_BIOS(4))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_we32kram_device::device_rom_region() const
{
	return ROM_NAME(we32kram);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_we32kram_device - constructor
//-------------------------------------------------

bbc_we32kram_device::bbc_we32kram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_WE32KRAM, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_we32kram_device::device_start()
{
	m_shadow = 2;
	m_ram = std::make_unique<uint8_t[]>(0x8000);

	/* register for save states */
	save_item(NAME(m_shadow));
	save_pointer(NAME(m_ram), 0x8000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_we32kram_device::device_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_write_handler(0xfffc, 0xffff, write8sm_delegate(*this, FUNC(bbc_we32kram_device::control_w)));
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bbc_we32kram_device::control_w(offs_t offset, uint8_t data)
{
	m_shadow = offset;
}

uint8_t bbc_we32kram_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_shadow)
	{
	case 0:
		/* &fffc - swap whole memory */
		data = m_ram[offset];
		break;
	case 2:
		/* &fffe - select BBC ram */
		data = m_mb_ram->pointer()[offset];
		break;
	case 3:
		/* &ffff - select Watford ram */
		if (offset >= 0x3000)
			data = m_ram[offset];
		else
			data = m_mb_ram->pointer()[offset];
		break;
	}

	return data;
}

void bbc_we32kram_device::ram_w(offs_t offset, uint8_t data)
{
	switch (m_shadow)
	{
	case 0:
		/* &fffc - swap whole memory */
		m_ram[offset] = data;
		break;
	case 2:
		/* &fffe - select BBC ram */
		m_mb_ram->pointer()[offset] = data;
		break;
	case 3:
		/* &ffff - select Watford ram */
		if (offset >= 0x3000)
			m_ram[offset] = data;
		else
			m_mb_ram->pointer()[offset] = data;
		break;
	}
}
