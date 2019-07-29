// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra-24F EISA SCSI Host Adapter

***************************************************************************/

#include "emu.h"
#include "ultra24f.h"

#include "bus/nscsi/devices.h"
#include "cpu/m68000/m68000.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"

DEFINE_DEVICE_TYPE(ULTRA24F, ultra24f_device, "ultra24f", "Ultra-24F SCSI Host Adapter")

ultra24f_device::ultra24f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ULTRA24F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_uscpu(*this, "uscpu")
	, m_bmic(*this, "bmic")
	, m_fdc(*this, "fdc")
	, m_bios16(*this, "bios16")
{
}

void ultra24f_device::device_start()
{
	// Firmware EPROMs have a couple of lines scrambled
	u16 *firmware = &memregion("firmware")->as_u16();
	for (offs_t b = 0; b < 0x8000; b += 0x100)
	{
		std::swap_ranges(&firmware[b | 0x10], &firmware[b | 0x20], &firmware[b | 0x80]);
		std::swap_ranges(&firmware[b | 0x30], &firmware[b | 0x40], &firmware[b | 0xa0]);
		std::swap_ranges(&firmware[b | 0x50], &firmware[b | 0x60], &firmware[b | 0xc0]);
		std::swap_ranges(&firmware[b | 0x70], &firmware[b | 0x80], &firmware[b | 0xe0]);
	}
}

u8 ultra24f_device::bmic_r(offs_t offset)
{
	return m_bmic->local_r(offset >> 3);
}

void ultra24f_device::bmic_w(offs_t offset, u8 data)
{
	m_bmic->local_w(offset >> 3, data);
}

void ultra24f_device::uscpu_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("firmware", 0);
	map(0xa00000, 0xa00001).nopr();
	map(0xff8001, 0xff8001).select(0x18).rw(FUNC(ultra24f_device::bmic_r), FUNC(ultra24f_device::bmic_w));
	map(0xff9000, 0xff901f).m("scsi:7:scsic", FUNC(ncr53cf94_device::map)).umask16(0x00ff);
	map(0xffc000, 0xffffff).ram();
}

void ultra24f_device::scsic_config(device_t *device)
{
	device->set_clock(40_MHz_XTAL);
	//downcast<ncr53cf94_device &>(*device).irq_handler_cb().set_inputline(m_uscpu, M68K_IRQ_1);
}

void ultra24f_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_uscpu, 32_MHz_XTAL / 4); // custom-marked as USC080-5-12A; clock guessed
	m_uscpu->set_addrmap(AS_PROGRAM, &ultra24f_device::uscpu_map);

	I82355(config, m_bmic, 0);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("scsic", NCR53CF94) // Emulex FAS216
		.machine_config([this] (device_t *device) { scsic_config(device); });

	DP8473(config, m_fdc, 24_MHz_XTAL); // custom-marked as USC020-1-24
}

ROM_START(ultra24f)
	ROM_REGION16_BE(0x10000, "firmware", 0)
	ROM_LOAD16_BYTE("28111008.bin", 0x00000, 0x08000, CRC(fcced531) SHA1(2ebcf57c4aea56eb2646e826d9cb3370ebef042e))
	ROM_LOAD16_BYTE("28110008.bin", 0x00001, 0x08000, CRC(3f26d927) SHA1(4ab98840e3af72bf08adfcf478e708fad309f4b7))
	//ROM_LOAD16_BYTE("28110.008", 0x00001, 0x08000, CRC(1d24cf9e) SHA1(6120af5308ed1cb0e5c3d14a4b436edb6559f8a5)) // identical except for last byte being 0xff instead of 0x49

	ROM_REGION16_LE(0x8000, "bios16", 0) // "Date : 06/24/92 Version 2.00"
	ROM_LOAD16_BYTE("38110005.bin", 0x0000, 0x4000, CRC(498f2aa5) SHA1(8405a91bb445cb99059c65a2e711d425fc69f5d6))
	ROM_LOAD16_BYTE("38111005.bin", 0x0001, 0x4000, CRC(826d5ad1) SHA1(052602e4788b7364670f7c0b5fe0bb0eacc4d9ac))
	// UltraStor also offered double-size binaries for programming onto 27C256 ROMs, with identical data in both halves of each file
ROM_END

const tiny_rom_entry *ultra24f_device::device_rom_region() const
{
	return ROM_NAME(ultra24f);
}
