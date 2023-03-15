// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sun cgthree color video controller

***************************************************************************/

#include "emu.h"
#include "cgthree.h"
#include "screen.h"

DEFINE_DEVICE_TYPE(SBUS_CGTHREE, sbus_cgthree_device, "cgthree", "Sun cgthree SBus Video")

void sbus_cgthree_device::mem_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(FUNC(sbus_cgthree_device::unknown_r), FUNC(sbus_cgthree_device::unknown_w));
	map(0x00000000, 0x000007ff).r(FUNC(sbus_cgthree_device::rom_r));
	map(0x00400000, 0x0040000f).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff000000);
	map(0x007ff800, 0x007ff81f).rw(FUNC(sbus_cgthree_device::regs_r), FUNC(sbus_cgthree_device::regs_w));
	map(0x00800000, 0x008fffff).rw(FUNC(sbus_cgthree_device::vram_r), FUNC(sbus_cgthree_device::vram_w));
	map(0x00bff800, 0x00cff7ff).rw(FUNC(sbus_cgthree_device::vram_r), FUNC(sbus_cgthree_device::vram_w));
}

ROM_START( sbus_cgthree )
	ROM_REGION32_BE(0x800, "prom", ROMREGION_ERASEFF)
	ROM_LOAD( "sunw,501-1415.bin", 0x0000, 0x0800, CRC(d1eb6f4d) SHA1(9bef98b2784b6e70167337bb27cd07952b348b5a))
ROM_END

const tiny_rom_entry *sbus_cgthree_device::device_rom_region() const
{
	return ROM_NAME( sbus_cgthree );
}

void sbus_cgthree_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(sbus_cgthree_device::screen_update));
	m_screen->set_raw(92.9405_MHz_XTAL, 1504, 0, 1152, 937, 0, 900);

	BT458(config, m_ramdac, 0);
}

sbus_cgthree_device::sbus_cgthree_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SBUS_CGTHREE, tag, owner, clock)
	, device_sbus_card_interface(mconfig, *this)
	, m_rom(*this, "prom")
	, m_screen(*this, "screen")
	, m_ramdac(*this, "ramdac")
{
}

void sbus_cgthree_device::device_start()
{
	m_vram = std::make_unique<uint32_t[]>(0x100000/4);
	save_pointer(NAME(m_vram), 0x100000/4);
}

void sbus_cgthree_device::device_reset()
{
}

void sbus_cgthree_device::install_device()
{
	m_sbus->install_device(m_base, m_base + 0x1ffffff, *this, &sbus_cgthree_device::mem_map);
}

uint32_t sbus_cgthree_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t const *const pens = m_ramdac->pens();
	auto const vram = util::big_endian_cast<uint8_t const>(&m_vram[0]);

	for (int y = 0; y < 900; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);
		for (int x = 0; x < 1152; x++)
		{
			const uint8_t pixel = vram[y * 1152 + x];
			*scanline++ = pens[pixel];
		}
	}

	return 0;
}

uint32_t sbus_cgthree_device::unknown_r(offs_t offset, uint32_t mem_mask)
{
	logerror("%s: unknown_r: %08x & %08x\n", machine().describe_context(), offset << 2, mem_mask);
	return 0;
}

void sbus_cgthree_device::unknown_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("%s: unknown_w: %08x = %08x & %08x\n", machine().describe_context(), offset << 2, data, mem_mask);
}

uint8_t sbus_cgthree_device::regs_r(offs_t offset)
{
	logerror("%s: regs_r: Unimplemented: %08x\n", machine().describe_context(), 0x7ff800 + offset);
	return 0;
}

void sbus_cgthree_device::regs_w(offs_t offset, uint8_t data)
{
	logerror("%s: regs_w: Unimplemented: %08x = %02x\n", machine().describe_context(), 0x7ff800 + offset, data);
}

uint32_t sbus_cgthree_device::rom_r(offs_t offset)
{
	return ((uint32_t*)m_rom->base())[offset];
}

uint32_t sbus_cgthree_device::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void sbus_cgthree_device::vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}
