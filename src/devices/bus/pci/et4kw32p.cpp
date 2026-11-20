// license:BSD-3-Clause
// copyright-holders: Miodrag Milanovic
/**************************************************************************************************

Tseng Labs ET4000/W32p PCI

TODO:
- Currently just an ET4000/W32i core hooked up to PCI.

**************************************************************************************************/

#include "emu.h"
#include "et4kw32p.h"

#include "screen.h"


DEFINE_DEVICE_TYPE(ET4KW32P_PCI, et4kw32p_pci_device, "et4kw32p_pci", "Tseng Labs ET4000/W32p PCI")


et4kw32p_pci_device::et4kw32p_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, ET4KW32P_PCI, tag, owner, clock)
	, m_vga(*this, "vga")
{
	// 0x3202 rev A
	// 0x3205 rev B
	// 0x3206 rev C
	// 0x3207 rev D
	set_ids(0x100c3206, 0x00, 0x030000, 0x00000000);
}

// [0x0020] PCIR
// [0x0060] BIOS string
// [0x017c] Card identifier
ROM_START( et4kw32p_pci )
	ROM_REGION32_LE( 0x8000, "bios", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "ddcc", "Cardex 1703-DDC (04/28/95 V2.10N)" )
	ROMX_LOAD("cardex.vbi",             0x0000, 0x8000, CRC(6ec56b03) SHA1(2df6bf7b4529057aa171f0eb200331dec3d04b31), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "icsc", "Cardex ICS5341 (09/28/94 V1.0DN)" )
	ROMX_LOAD("tsenget4000w32ppci.bin", 0x0000, 0x8000, CRC(ebcf0a17) SHA1(7a9d1c15874f8145deddcd111eaa641ee754e6bd), ROM_BIOS(1) )
	// purged by BIOS, probably bad (earlier date but using Rev D?)
	ROM_SYSTEM_BIOS( 2, "icsd", "Cardex ICS5341 (06/28/94 V1.0CN)" )
	ROMX_LOAD("et4000w32pcardex.bin",   0x0000, 0x8000, CRC(9b6f8548) SHA1(c020e1b1491dd5ff0d4a122cbdcbd8b05b265122), ROM_BIOS(2) )
ROM_END

const tiny_rom_entry *et4kw32p_pci_device::device_rom_region() const
{
	return ROM_NAME(et4kw32p_pci);
}


void et4kw32p_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(et4kw32i_vga_device::screen_update));

	// TODO: should be W32p specific core
	ET4KW32I_VGA(config, m_vga);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(2*1024*1024);
}

void et4kw32p_pci_device::device_start()
{
	pci_card_device::device_start();

	// TODO: make it coexist with pcd5h decoding from system ROM on its own thru strapping
	//add_rom((u8 *)memregion("bios")->base(), 0x8000);

	// INTA#
	intr_pin = 1;
}

void et4kw32p_pci_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x23;
	status = 0x0000;

	remap_cb();
}

void et4kw32p_pci_device::legacy_io_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(et4kw32i_vga_device::io_map));
	// IOD controls mirroring
	map(0x210a, 0x210a).mirror(0x70).rw(m_vga, FUNC(et4kw32i_vga_device::acl_index_r), FUNC(et4kw32i_vga_device::acl_index_w));
	map(0x210b, 0x210b).mirror(0x70).rw(m_vga, FUNC(et4kw32i_vga_device::acl_data_r), FUNC(et4kw32i_vga_device::acl_data_w));
}

u8 et4kw32p_pci_device::vram_r(offs_t offset)
{
	return m_vga->mem_r(offset);
}

void et4kw32p_pci_device::vram_w(offs_t offset, uint8_t data)
{
	m_vga->mem_w(offset, data);
}

void et4kw32p_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(et4kw32p_pci_device::vram_r)), write8sm_delegate(*this, FUNC(et4kw32p_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x0000, 0xffff, *this, &et4kw32p_pci_device::legacy_io_map);
	}
}
