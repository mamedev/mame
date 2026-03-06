// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Spea TIGA based video cards

**************************************************************************************************/

#include "emu.h"
#include "tiga_spea.h"

DEFINE_DEVICE_TYPE(ISA16_FGA4HE, isa16_fga4he_device, "fga4he", "SPEA Graphiti FGA 4/HE TIGA card")

isa16_fga4he_device::isa16_fga4he_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa16_fga4he_device(mconfig, ISA16_FGA4HE, tag, owner, clock)
{
}

isa16_fga4he_device::isa16_fga4he_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_vga(*this, "vga")
	, m_bios(*this, "bios")
{
}

ROM_START( fga4he )
	ROM_REGION(0x8000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v219", "Video Seven v2.19 01/18/90")
	ROMX_LOAD("spea_fga4he.vbi", 0x00000, 0x8000, CRC(ce472e63) SHA1(fe4bc168ad7ccb186a6f8158964e2116435c994b), ROM_BIOS(0) )
ROM_END

const tiny_rom_entry *isa16_fga4he_device::device_rom_region() const
{
	return ROM_NAME( fga4he );
}

void isa16_fga4he_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update(m_vga, FUNC(vga_device::screen_update));

	HT208_VIDEO7_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(256*1024);
//	m_vga->setup_rom_bank_cb().set([this] (u8 data) { printf("%02x\n", data); });
//	m_vga->vsync_cb().set([this](int state) { m_isa->irq2_w(state); });

	// TODO: TMS34020-32 + Bt459 + Spea "VEMU"
	// 2MB VRAM + 4MB of DRAM, dual monitor with separate DB9 connector
}

void isa16_fga4he_device::device_start()
{
	set_isa_device();
}

void isa16_fga4he_device::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_fga4he_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(vga_device::mem_w)));
		m_isa->install_rom(this, 0xc0000, 0xc7fff, "bios");
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_fga4he_device::io_isa_map);
}

void isa16_fga4he_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(ht208_video7_vga_device::io_map));
	map(0x46e8, 0x46e8).w(m_vga, FUNC(ht208_video7_vga_device::mode_setup_w));
}
