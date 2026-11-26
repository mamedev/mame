// license:BSD-3-Clause
// copyright-holders:Barry Rodewald

#include "emu.h"

#include "trident_tgui.h"

DEFINE_DEVICE_TYPE(TGUI9680_PCI, tgui9680_pci_device, "tgui9680_pci", "Trident TGUI9680 GUI Accelerator")

tgui9680_pci_device::tgui9680_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, TGUI9680_PCI, tag, owner, clock)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
	// derived from VGA PCIR
	// gets detected as "TGUI9660XGi/968x/938x"
	set_ids(0x10239660, 0x00, 0x030000, 0x10239660);
}


ROM_START( tgui9680 )
	ROM_REGION( 0x8000, "bios", 0 )
	// (BIOS X5.5 (02) 02/13/96)
	ROM_LOAD16_BYTE( "trident_tgui9680_bios.bin", 0x0000, 0x4000, CRC(1eebde64) SHA1(67896a854d43a575037613b3506aea6dae5d6a19) )
	ROM_CONTINUE(                                 0x0001, 0x4000 )
ROM_END

const tiny_rom_entry *tgui9680_pci_device::device_rom_region() const
{
	return ROM_NAME( tgui9680 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tgui9680_pci_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(trident_vga_device::screen_update));

	TGUI9680_VGA(config, m_vga);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(4*1024*1024);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------


void tgui9680_pci_device::device_start()
{
	pci_card_device::device_start();

	// TODO: untested, win3.1/95 either uses this or a non-BAR version
	add_map(4 * 1024 * 1024, M_MEM, FUNC(tgui9680_pci_device::vram_aperture_map));

	add_rom((u8 *)m_bios->base(), 0x8000);
	expansion_rom_base = 0xc0000;

}

void tgui9680_pci_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: not really documented
	command = 0x0020;
	command_mask = 0x23;
	// Medium DEVSEL
	status = 0x0200;

	remap_cb();
}


uint8_t tgui9680_pci_device::vram_r(offs_t offset)
{
	return downcast<tgui9680_vga_device *>(m_vga.target())->mem_r(offset);
}

void tgui9680_pci_device::vram_w(offs_t offset, uint8_t data)
{
	downcast<tgui9680_vga_device *>(m_vga.target())->mem_w(offset, data);
}


void tgui9680_pci_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (BIT(command, 1))
	{
		memory_space->install_readwrite_handler(0xa0000, 0xbffff, read8sm_delegate(*this, FUNC(tgui9680_pci_device::vram_r)), write8sm_delegate(*this, FUNC(tgui9680_pci_device::vram_w)));
	}

	if (BIT(command, 0))
	{
		io_space->install_device(0x0000, 0xffff, *this, &tgui9680_pci_device::io_isa_map);
	}
}

void tgui9680_pci_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(tgui9680_vga_device::io_map));

	map(0x43c4, 0x43cb).rw(m_vga, FUNC(tgui9680_vga_device::port_43c6_r), FUNC(tgui9680_vga_device::port_43c6_w));
	map(0x83c4, 0x83cb).rw(m_vga, FUNC(tgui9680_vga_device::port_83c6_r), FUNC(tgui9680_vga_device::port_83c6_w));

	// acceleration ports
	map(0x2120, 0x21ff).rw(m_vga, FUNC(tgui9680_vga_device::accel_r), FUNC(tgui9680_vga_device::accel_w));
}

void tgui9680_pci_device::vram_aperture_map(address_map &map)
{
	map(0x00'0000, 0x3f'ffff).rw(m_vga, FUNC(tgui9680_vga_device::vram_r), FUNC(tgui9680_vga_device::vram_w));
}

