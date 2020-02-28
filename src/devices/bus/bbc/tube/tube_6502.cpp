// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC01 6502 2nd Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC01_65022ndproc.html

    Acorn ADC06 65C102 Co-processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ADC06_65C102CoPro.html

**********************************************************************/


#include "emu.h"
#include "tube_6502.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_TUBE_6502,    bbc_tube_6502_device,    "bbc_tube_6502",    "Acorn 6502 2nd Processor")
DEFINE_DEVICE_TYPE(BBC_TUBE_65C102,  bbc_tube_65c102_device,  "bbc_tube_65c102",  "Acorn 65C102 Co-Processor")


//-------------------------------------------------
//  ADDRESS_MAP( tube_6502_mem )
//-------------------------------------------------

void bbc_tube_6502_device::tube_6502_mem(address_map &map)
{
	map(0x0000, 0xffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
	map(0xfef0, 0xfeff).rw(FUNC(bbc_tube_6502_device::tube_r), FUNC(bbc_tube_6502_device::tube_w));
}

void bbc_tube_6502_device::tube_6502_bank(address_map &map)
{
	// ROM enabled
	map(0x00000, 0x0ffff).ram().share("ram");
	map(0x0f000, 0x0ffff).rom().region("rom", 0);
	// ROM disabled
	map(0x10000, 0x1ffff).ram().share("ram");
}

//-------------------------------------------------
//  ROM( tube_6502 )
//-------------------------------------------------

ROM_START( tube_6502 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("110")
	ROM_SYSTEM_BIOS(0, "110", "Tube 1.10")
	ROMX_LOAD("6502tube.rom", 0x0000, 0x1000, CRC(98b5fe42) SHA1(338269d03cf6bfa28e09d1651c273ea53394323b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "121", "Tube 1.21 (ReCo6502)")
	ROMX_LOAD("reco6502tube.rom", 0x0000, 0x1000, CRC(75b2a466) SHA1(9ecef24de58a48c3fbe01b12888c3f6a5d24f57f), ROM_BIOS(1))
ROM_END

ROM_START( tube_65c102 )
	ROM_REGION(0x1000, "rom", 0)
	ROM_DEFAULT_BIOS("110")
	ROM_SYSTEM_BIOS(0, "110", "Tube 1.10")
	ROMX_LOAD("65c102_boot_110.rom", 0x0000, 0x1000, CRC(ad5b70cc) SHA1(0ac9a1c70e55a79e2c81e102afae1d016af229fa), ROM_BIOS(0)) // 2201,243-02
	ROM_SYSTEM_BIOS(1, "121", "Tube 1.21 (ReCo6502)")
	ROMX_LOAD("reco6502tube.rom", 0x0000, 0x1000, CRC(75b2a466) SHA1(9ecef24de58a48c3fbe01b12888c3f6a5d24f57f), ROM_BIOS(1))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_tube_6502_device::add_common_devices(machine_config &config)
{
	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&bbc_tube_6502_device::tube_6502_bank).set_options(ENDIANNESS_LITTLE, 8, 17, 0x10000);

	TUBE(config, m_ula);
	m_ula->pnmi_handler().set_inputline(m_maincpu, M65C02_NMI_LINE);
	m_ula->pirq_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);

	/* internal ram */
	RAM(config, m_ram).set_default_size("64K").set_default_value(0);

	/* software lists */
	SOFTWARE_LIST(config, "flop_ls_6502").set_original("bbc_flop_6502");
	SOFTWARE_LIST(config, "flop_ls_65c102").set_original("bbc_flop_65c102");
}

void bbc_tube_6502_device::device_add_mconfig(machine_config &config)
{
	M65C02(config, m_maincpu, 12_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_6502_device::tube_6502_mem);

	add_common_devices(config);
}

void bbc_tube_65c102_device::device_add_mconfig(machine_config &config)
{
	M65C02(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbc_tube_65c102_device::tube_6502_mem);

	add_common_devices(config);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_tube_6502_device::device_rom_region() const
{
	return ROM_NAME( tube_6502 );
}

const tiny_rom_entry *bbc_tube_65c102_device::device_rom_region() const
{
	return ROM_NAME( tube_65c102 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_tube_6502_device - constructor
//-------------------------------------------------

bbc_tube_6502_device::bbc_tube_6502_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_bbc_tube_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_bankdev(*this, "bankdev")
	, m_ula(*this, "ula")
	, m_ram(*this, "ram")
	, m_rom(*this, "rom")
{
}

bbc_tube_6502_device::bbc_tube_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_6502_device(mconfig, BBC_TUBE_6502, tag, owner, clock)
{
}

bbc_tube_65c102_device::bbc_tube_65c102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: bbc_tube_6502_device(mconfig, BBC_TUBE_65C102, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_tube_6502_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bbc_tube_6502_device::device_reset()
{
	m_bankdev->set_bank(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_tube_6502_device::host_r(offs_t offset)
{
	return m_ula->host_r(offset);
}

void bbc_tube_6502_device::host_w(offs_t offset, uint8_t data)
{
	m_ula->host_w(offset, data);
}


uint8_t bbc_tube_6502_device::tube_r(offs_t offset)
{
	// Disable ROM on first access
	if (!machine().side_effects_disabled())
		m_bankdev->set_bank(1);

	return m_ula->parasite_r(offset);
}

void bbc_tube_6502_device::tube_w(offs_t offset, uint8_t data)
{
	m_ula->parasite_w(offset, data);
}
