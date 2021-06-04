// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    BennVenn SD Loader for VZ300

    Notes:
    - Also works for VZ200

    TODO:
    - No SD card emulation, so only the memory expansion part works

***************************************************************************/

#include "emu.h"
#include "sdloader.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VTECH_SDLOADER, vtech_sdloader_device, "vtech_sdloader", "BennVenn SD Loader")

//-------------------------------------------------
//  mem_map - memory space address map
//-------------------------------------------------

void vtech_sdloader_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x4000, 0x67ff).view(m_dosview);
	m_dosview[0](0x4000, 0x67ff).rom().region("software", 0).bankw(m_dosbank);
	m_dosview[1](0x4000, 0x67ff).bankrw(m_dosbank);
	map(0x9000, 0xffff).rw(FUNC(vtech_sdloader_device::exp_ram_r), FUNC(vtech_sdloader_device::exp_ram_w));
}

//-------------------------------------------------
//  io_map - io space address map
//-------------------------------------------------

void vtech_sdloader_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x37, 0x37).w(FUNC(vtech_sdloader_device::mapper_w));
	map(0x38, 0x38).w(FUNC(vtech_sdloader_device::sdcfg_w));
	map(0x39, 0x39).rw(FUNC(vtech_sdloader_device::sdio_r), FUNC(vtech_sdloader_device::sdio_w));
	map(0x3a, 0x3a).w(FUNC(vtech_sdloader_device::mode_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sdloader )
	ROM_REGION(0x2800, "software", 0)
	ROM_DEFAULT_BIOS("18")
	ROM_SYSTEM_BIOS(0, "15", "Version 1.5")
	ROMX_LOAD("vzdos15.bin", 0x0000, 0x16c2, CRC(828f7703) SHA1(150c6e5a8f20416c0dab1fa96f68726f415a8b7e), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "17", "Version 1.7")
	ROMX_LOAD("vzdos17.bin", 0x0000, 0x1783, CRC(7ef7fb1e) SHA1(6278ac675d6c08dca39a5a7f4c72988a178eff8a), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "18", "Version 1.8")
	ROMX_LOAD("vzdos18.bin", 0x0000, 0x1795, CRC(2b1cec28) SHA1(d4f8fa0c7a70984334be3e5c831017cfc53683b2), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *vtech_sdloader_device::device_rom_region() const
{
	return ROM_NAME( sdloader );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vtech_sdloader_device - constructor
//-------------------------------------------------

vtech_sdloader_device::vtech_sdloader_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vtech_memexp_device(mconfig, VTECH_SDLOADER, tag, owner, clock),
	m_dosbank(*this, "dosbank"),
	m_dosview(*this, "dosview"),
	m_expbank(*this, "expbank"),
	m_vz300_mode(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vtech_sdloader_device::device_start()
{
	vtech_memexp_device::device_start();

	// init ram
	m_ram = std::make_unique<uint8_t[]>(0x20000);

	// configure banks
	m_dosbank->configure_entry(0, m_ram.get() + 0x00000);
	m_dosbank->configure_entry(1, m_ram.get() + 0x08000);
	m_expbank->configure_entry(0, m_ram.get() + 0x10000);
	m_expbank->configure_entry(1, m_ram.get() + 0x18000);

	// register for savestates
	save_item(NAME(m_vz300_mode));
	save_pointer(NAME(m_ram), 0x20000);
}

//-------------------------------------------------
//  device_reset - device-specific startup
//-------------------------------------------------

void vtech_sdloader_device::device_reset()
{
	// startup in vz200 mode
	m_vz300_mode = false;

	// rom enabled
	m_dosview.select(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void vtech_sdloader_device::mapper_w(uint8_t data)
{
	// 7654----  not used
	// ----3---  led
	// -----2--  expansion ram bank
	// ------1-  dos ram bank
	// -------0  dos rom/ram switch

	m_dosview.select(BIT(data, 0));
	m_dosbank->set_entry(BIT(data, 1));
	m_expbank->set_entry(BIT(data, 2));
}

void vtech_sdloader_device::sdcfg_w(uint8_t data)
{
	logerror("sdcfg_w: %02x\n", data);
}

uint8_t vtech_sdloader_device::sdio_r()
{
	logerror("sdio_r\n");
	return 0xff;
}

void vtech_sdloader_device::sdio_w(uint8_t data)
{
	logerror("sdio_w: %02x\n", data);
}

void vtech_sdloader_device::mode_w(uint8_t data)
{
	logerror("Switching to %s mode\n", BIT(data, 0) ? "VZ-300" : "VZ-200");
	m_vz300_mode = bool(BIT(data, 0));
}

uint8_t vtech_sdloader_device::exp_ram_r(offs_t offset)
{
	offset += 0x9000;

	if (!m_vz300_mode || (m_vz300_mode && offset >= 0xb800))
		return reinterpret_cast<uint8_t *>(m_expbank->base())[offset & 0x7fff];

	return 0xff;
}

void vtech_sdloader_device::exp_ram_w(offs_t offset, uint8_t data)
{
	offset += 0x9000;

	if (!m_vz300_mode || (m_vz300_mode && offset >= 0xb800))
		reinterpret_cast<uint8_t *>(m_expbank->base())[offset & 0x7fff] = data;
}
